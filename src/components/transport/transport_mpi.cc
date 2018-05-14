/*
 *  Copyright (C) 2014, 2015, 2016, 2017, 2018 Michael Hofmann
 *  
 *  This file is part of the Simulation Component and Data Coupling (SCDC) library.
 *  
 *  The SCDC library is free software: you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser Public License as published
 *  by the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *  
 *  The SCDC library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser Public License for more details.
 *  
 *  You should have received a copy of the GNU Lesser Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#include <cstring>
#include <exception>
#include <unistd.h>

#define SCDC_TRACE_NOT  !SCDC_TRACE_TRANSPORT_MPI

#include "config.hh"
#include "log.hh"
#include "transport_mpi.hh"


using namespace std;


#define SCDC_LOG_PREFIX  "transport-mpi: "


#if SCDCINT_IS_SHORT
# define SCDCINT_MPI  MPI_SHORT
#elif SCDCINT_IS_INT
# define SCDCINT_MPI  MPI_INT
#elif SCDCINT_IS_LONG
# define SCDCINT_MPI  MPI_LONG
#elif SCDCINT_IS_LONG_LONG
# define SCDCINT_MPI  MPI_LONG_LONG
#else
# error unknown data type of scdcint_t 
#endif

#define ACCEPT_USLEEP  10

#define TAG_ACCEPT  0
#define TAG_CLOSE   1
#define TAG_INFO    2
#define TAG_DATA    3


struct scdc_transport_mpi_connection_t: public scdc_transport_connection_t
{
  MPI_Comm comm;
  int rank;
  bool disconnect;
  
  scdc_transport_mpi_connection_t(scdc_transport *transport_)
    :scdc_transport_connection_t(transport_), comm(MPI_COMM_NULL), rank(-1), disconnect(false) { };
};


struct scdc_transport_mpi_server_t
{
  MPI_Comm comm, self_dup;
  char port_name[MPI_MAX_PORT_NAME + 1];

  scdc_transport_mpi_server_t()
    :comm(MPI_COMM_NULL), self_dup(MPI_COMM_NULL) { port_name[0] = '\0'; };
};


#if SCDC_TRANSPORT_MPI_USE_SHUTDOWN_EXCEPTION
class scdc_transport_mpi_shutdown_ex: public exception
{
};
#endif


scdc_transport_mpi::scdc_transport_mpi()
  :scdc_transport(), mpi_server(0)
{
  max_connections = 0;
}


scdc_transport_mpi::~scdc_transport_mpi()
{
}


bool scdc_transport_mpi::open(MPI_Comm comm)
{
  SCDC_TRACE("open: comm: '" << comm << "'");

  mpi_server = new scdc_transport_mpi_server_t();

  mpi_server->comm = comm;

  return scdc_transport::open();
}


bool scdc_transport_mpi::open()
{
  SCDC_TRACE("open:");

  mpi_server = new scdc_transport_mpi_server_t();

  if (MPI_Open_port(MPI_INFO_NULL, mpi_server->port_name) != MPI_SUCCESS)
  {
    SCDC_FAIL("open: MPI_Open_port failed!");
    return false;
  }

  MPI_Comm_dup(MPI_COMM_SELF, &mpi_server->self_dup);

  return scdc_transport::open();
}


void scdc_transport_mpi::close()
{
  SCDC_TRACE("close:");

  MPI_Comm_free(&mpi_server->self_dup);

  if (mpi_server->port_name[0] != '\0')
  {
    if (MPI_Close_port(mpi_server->port_name) != MPI_SUCCESS)
    {
      SCDC_FAIL("close: MPI_Close_port failed!");
    }
  }

  delete mpi_server;
  mpi_server = 0;
}


bool scdc_transport_mpi::get_comm(MPI_Comm *comm)
{
  if (!mpi_server) return false;

  if (comm) *comm = mpi_server->comm;

  return true;
}


bool scdc_transport_mpi::get_port_name(char *port_name)
{
  if (!mpi_server) return false;

  if (port_name) strncpy(port_name, mpi_server->port_name, MPI_MAX_PORT_NAME);

  return (mpi_server->port_name[0] != '\0');
}


/*bool scdc_transport_mpi::start(scdcint_t mode)
{
  SCDC_TRACE("start: mode '" << mode << "'");

  return scdc_transport::start(mode);
}*/


/*bool scdc_transport_mpi::stop()
{
  SCDC_TRACE("stop:");

  return scdc_transport::stop();
}*/


/*bool scdc_transport_mpi::cancel(bool interrupt)
{
  SCDC_TRACE("cancel: interrupt: '" << interrupt << "'");

  return scdc_transport::cancel(interrupt);
}*/


/*bool scdc_transport_mpi::resume()
{
  SCDC_TRACE("resume:");

  return scdc_transport::resume();
}*/


scdc_transport_connection_t *scdc_transport_mpi::accept()
{
  SCDC_TRACE("accept:");

  scdc_transport_mpi_connection_t *mpi_conn = new scdc_transport_mpi_connection_t(this);

#if SCDC_TRANSPORT_MPI_USE_SHUTDOWN_EXCEPTION
  try
  {
#endif
    MPI_Comm comm = mpi_server->comm;

    if (mpi_server->port_name[0] != '\0')
    {
      

      if (MPI_Comm_accept(mpi_server->port_name, MPI_INFO_NULL, 0, MPI_COMM_SELF, &comm) != MPI_SUCCESS);
      {
        SCDC_FAIL("accept: MPI_Comm_accept failed!");
        comm = MPI_COMM_NULL;
      }
    }

    if (comm != MPI_COMM_NULL)
    {
      MPI_Request request = MPI_REQUEST_NULL;
      MPI_Status status;

      MPI_Irecv(0, 0, MPI_INT, MPI_ANY_SOURCE, TAG_ACCEPT, comm, &request);

      while (!canceled)
      {
        int x;
        MPI_Test(&request, &x, &status);
        if (x) break;
#if ACCEPT_USLEEP
        usleep(ACCEPT_USLEEP);
#endif
      }

      if (request == MPI_REQUEST_NULL)
      {
        mpi_conn->comm = comm;
        mpi_conn->rank = status.MPI_SOURCE;

      } else
      {
        /* cancel the request */
        MPI_Cancel(&request);
        MPI_Request_free(&request);
      }
    }

#if SCDC_TRANSPORT_MPI_USE_SHUTDOWN_EXCEPTION
  } catch (const scdc_transport_mpi_shutdown_ex &ex)
  {
    SCDC_INFO("connection " << mpi_conn << ": exception during accept");
    mpi_conn->rank = -1;
  }
#endif

  if (mpi_conn->comm != MPI_COMM_NULL && mpi_conn->rank >= 0)
  {
    mpi_conn->open = true;

  } else
  {
    if (!shutdowned) SCDC_FAIL("accept: accepting connection failed");
    delete mpi_conn;
    mpi_conn = 0;
    return 0;
  }

  SCDC_TRACE("accept: '" << mpi_conn << "'");

  return scdc_transport::accept(mpi_conn);
}


void scdc_transport_mpi::shutdown(scdc_transport_connection_t *conn, bool interrupt)
{
  SCDC_TRACE("shutdown: interrupt: '" << interrupt << "'");

  scdc_transport::shutdown(conn, interrupt);

#if SCDC_TRANSPORT_MPI_USE_SHUTDOWN_EXCEPTION
  if (interrupt) throw scdc_transport_mpi_shutdown_ex();
#endif

  if (mpi_server->port_name[0] != '\0')
  {
    MPI_Comm comm;

    if (MPI_Comm_connect(mpi_server->port_name, MPI_INFO_NULL, 0, mpi_server->self_dup, &comm) != MPI_SUCCESS)
    {
      SCDC_FAIL("connect: MPI_Comm_connect failed!");
      return;
    }

    MPI_Comm_disconnect(&comm);
  }
}


void scdc_transport_mpi::serve(scdc_transport_connection_t *conn)
{
#if SCDC_TRANSPORT_MPI_USE_SHUTDOWN_EXCEPTION
  /* catch our shutdown exceptions while serving (i.e., outside send or receive) */
  try
  {
#endif
    scdc_transport::serve(conn);

#if SCDC_TRANSPORT_MPI_USE_SHUTDOWN_EXCEPTION
  } catch (const scdc_transport_mpi_shutdown_ex &ex)
  {
    SCDC_INFO("serve: catch shutdown exception");
  }
#endif
}


void scdc_transport_mpi::close(scdc_transport_connection_t *conn)
{
  SCDC_TRACE("close: '" << conn << "'");

  scdc_transport_mpi_connection_t *mpi_conn = static_cast<scdc_transport_mpi_connection_t *>(conn);

  if (mpi_conn->disconnect)
  {
    if (MPI_Comm_disconnect(&mpi_conn->comm) != MPI_SUCCESS)
    {
      SCDC_FAIL("disconnect: MPI_Comm_disconnect failed!");
    }
  }

  scdc_transport::close(conn);

  delete mpi_conn;
}


scdc_transport_connection_t *scdc_transport_mpi::connect(MPI_Comm comm, int rank)
{
  SCDC_TRACE("connect: comm: '" << comm << "', rank: " << rank);

  scdc_transport_mpi_connection_t *mpi_conn = new scdc_transport_mpi_connection_t(this);

  mpi_conn->comm = comm;
  mpi_conn->rank = rank;

#if SCDC_TRANSPORT_MPI_USE_SHUTDOWN_EXCEPTION
  try
  {
#endif
    MPI_Ssend(0, 0, MPI_INT, mpi_conn->rank, TAG_ACCEPT, mpi_conn->comm);

#if SCDC_TRANSPORT_MPI_USE_SHUTDOWN_EXCEPTION
  } catch (const scdc_transport_mpi_shutdown_ex &ex)
  {
    SCDC_INFO("connect: catch shutdown exception");
  }
#endif

  return scdc_transport::connect(mpi_conn);
}


scdc_transport_connection_t *scdc_transport_mpi::connect(const char *port_name, int rank)
{
  SCDC_TRACE("connect: port_name: '" << port_name << "', rank: " << rank);

  MPI_Comm comm;

#if SCDC_TRANSPORT_MPI_USE_SHUTDOWN_EXCEPTION
  try
  {
#endif
    if (MPI_Comm_connect(const_cast<char *>(port_name), MPI_INFO_NULL, 0, MPI_COMM_SELF, &comm) != MPI_SUCCESS)
    {
      SCDC_FAIL("connect: MPI_Comm_connect failed!");
      return 0;
    }
#if SCDC_TRANSPORT_MPI_USE_SHUTDOWN_EXCEPTION
  } catch (const scdc_transport_mpi_shutdown_ex &ex)
  {
    SCDC_INFO("connect: catch shutdown exception");
  }
#endif

  scdc_transport_mpi_connection_t *mpi_conn = static_cast<scdc_transport_mpi_connection_t *>(connect(comm, rank));

  mpi_conn->disconnect = true;

  return mpi_conn;
}


bool scdc_transport_mpi::disconnect(scdc_transport_connection_t *conn)
{
  SCDC_TRACE("disconnect: conn: '" << conn << "'");

  if (!scdc_transport::disconnect(conn)) return false;

  scdc_transport_mpi_connection_t *mpi_conn = static_cast<scdc_transport_mpi_connection_t *>(conn);

#if SCDC_TRANSPORT_MPI_USE_SHUTDOWN_EXCEPTION
  try
  {
#endif
    scdcint_t size;
    MPI_Status status;
    MPI_Recv(&size, 1, SCDCINT_MPI, mpi_conn->rank, TAG_INFO, mpi_conn->comm, &status);

    MPI_Ssend(0, 0, MPI_INT, mpi_conn->rank, TAG_CLOSE, mpi_conn->comm);

#if SCDC_TRANSPORT_MPI_USE_SHUTDOWN_EXCEPTION
  } catch (const scdc_transport_mpi_shutdown_ex &ex)
  {
    SCDC_INFO("connect: catch shutdown exception");
  }
#endif

  bool ret = true;

  if (mpi_conn->disconnect)
  {
    if (MPI_Comm_disconnect(&mpi_conn->comm) != MPI_SUCCESS)
    {
      SCDC_FAIL("disconnect: MPI_Comm_disconnect failed!");
      ret = false;
    }
  }

  delete mpi_conn;

  return ret;
}


scdcint_t scdc_transport_mpi::send(scdc_transport_connection_t *conn, const void *buf, scdcint_t buf_size)
{
  SCDC_TRACE("send: conn: '" << conn << "', buf: '" << buf << "', buf_size: " << buf_size);

/*  SCDC_TRACE("send: conn: '" << conn << "', buf: '" << string(static_cast<const char *>(buf), buf_size) << "'");*/

  scdc_transport_mpi_connection_t *mpi_conn = static_cast<scdc_transport_mpi_connection_t *>(conn);

  scdcint_t n = 0;

#if SCDC_TRANSPORT_MPI_USE_SHUTDOWN_EXCEPTION
  try
  {
#endif
    char *b = static_cast<char *>(const_cast<void *>(buf));

    while (n < buf_size)
    {
      scdcint_t size;
      MPI_Status status;
      MPI_Recv(&size, 1, SCDCINT_MPI, mpi_conn->rank, TAG_INFO, mpi_conn->comm, &status);

      if (size > buf_size) size = buf_size;

      MPI_Send(b, size, MPI_BYTE, mpi_conn->rank, TAG_DATA, mpi_conn->comm);

      b += size;
      n += size;
    }

#if SCDC_TRANSPORT_MPI_USE_SHUTDOWN_EXCEPTION
  } catch (const scdc_transport_mpi_shutdown_ex &ex)
  {
    SCDC_INFO("connection: " << mpi_conn << ": send: catch shutdown exception");
    n = -1;
  }
#endif

  if (n < 0)
  {
    return -1;
  }

  SCDC_TRACE("connection: " << mpi_conn << ": send: sent " << n << " byte(s)");
  
  return n;
}


scdcint_t scdc_transport_mpi::receive(scdc_transport_connection_t *conn, void *buf, scdcint_t max_buf_size)
{
  SCDC_TRACE("receive: conn: '" << conn << "', buf: '" << buf << "', max_buf_size: " << max_buf_size);

  scdc_transport_mpi_connection_t *mpi_conn = static_cast<scdc_transport_mpi_connection_t *>(conn);

  if (max_buf_size <= 0)
  {
    SCDC_TRACE("connection: " << mpi_conn << ": receive: nothing to receive");
    return 0;
  }

  scdcint_t n = 0;

#if SCDC_TRANSPORT_MPI_USE_SHUTDOWN_EXCEPTION
  try
  {
#endif
    /* send max. recv size */
    MPI_Send(&max_buf_size, 1, SCDCINT_MPI, mpi_conn->rank, TAG_INFO, mpi_conn->comm);

    MPI_Status status;
    MPI_Recv(buf, max_buf_size, MPI_BYTE, mpi_conn->rank, MPI_ANY_TAG, mpi_conn->comm, &status);

    if (status.MPI_TAG == TAG_CLOSE) n = 0;
    else
    {
      int s;
      MPI_Get_count(&status, MPI_BYTE, &s);
      n = s;
    }

#if SCDC_TRANSPORT_MPI_USE_SHUTDOWN_EXCEPTION
  } catch (const scdc_transport_mpi_shutdown_ex &ex)
  {
    SCDC_INFO("connection " << mpi_conn << ": receive: catch shutdown receive");
    n = -1;
  }
#endif

  if (n < 0)
  {
    return -1;
  }

  SCDC_TRACE("connection " << mpi_conn << ": receive: received " << n << " byte(s)");

/*  SCDC_TRACE("connection " << mpi_conn << ": receive: buf: '" << string(static_cast<char *>(buf), n) << "'");*/

  /* receiving 0 bytes signals that the connection was closed */
  if (n == 0) n = -1;

  return n;
}


#undef SCDC_LOG_PREFIX
