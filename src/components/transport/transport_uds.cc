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


#include <cstdio>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/stat.h>

#include <cerrno>
#include <exception>

#define SCDC_TRACE_NOT  !SCDC_TRACE_TRANSPORT_UDS

#include "config.hh"
#include "log.hh"
#include "transport_uds.hh"


using namespace std;


#define SCDC_LOG_PREFIX  "transport-uds: "


struct scdc_transport_uds_connection_t: public scdc_transport_connection_t
{
  int socket;

  scdc_transport_uds_connection_t(scdc_transport *transport_)
    :scdc_transport_connection_t(transport_), socket(0) { };
};


struct scdc_transport_uds_server_t
{
  struct sockaddr_un saddr;
  int server_socket;
  string uds_filename;

  scdc_transport_uds_server_t()
    :server_socket(0) { };
};


#if SCDC_TRANSPORT_UDS_USE_SHUTDOWN_EXCEPTION
class scdc_transport_uds_shutdown_ex: public exception
{
};
#endif


scdc_transport_uds::scdc_transport_uds()
  :scdc_transport(), uds_server(0)
{
}


scdc_transport_uds::~scdc_transport_uds()
{
}


bool scdc_transport_uds::open(const char *socketfile)
{
  SCDC_TRACE("open: socketfile: '" << socketfile << "'");

  uds_server = new scdc_transport_uds_server_t();

  uds_server->server_socket = ::socket(AF_LOCAL, SOCK_STREAM, 0);
  if (uds_server->server_socket == -1)
  {
    SCDC_ERROR("open: creating server socket failed: '" << strerror(errno) << "'");
    return false;
  }

  uds_server->uds_filename = socketfile;

  ::unlink(uds_server->uds_filename.c_str());

  uds_server->saddr.sun_family = AF_LOCAL;
  strcpy(uds_server->saddr.sun_path, uds_server->uds_filename.c_str());

  if (::bind(uds_server->server_socket, (struct sockaddr *) &uds_server->saddr, sizeof(uds_server->saddr)) == -1)
  {
    SCDC_ERROR("open: binding server socket failed: '" << strerror(errno) << "'");
    ::close(uds_server->server_socket);
    return false;
  }

  ::chmod(uds_server->uds_filename.c_str(), S_IRWXU|S_IRWXG|S_IRWXO);

  if (::listen(uds_server->server_socket, TRANSPORT_UDS_LISTEN_BACKLOG) == -1)
  {
    SCDC_ERROR("open: listen server socket failed: '" << strerror(errno) << "'");
    ::close(uds_server->server_socket);
    return false;
  }

  return scdc_transport::open();
}


void scdc_transport_uds::close()
{
  SCDC_TRACE("close:");

  ::close(uds_server->server_socket);

  ::unlink(uds_server->uds_filename.c_str());

  delete uds_server;
  uds_server = 0;
}


/*bool scdc_transport_uds::start(scdcint_t mode)
{
  SCDC_TRACE("start: mode '" << mode << "'");

  return scdc_transport::start(mode);
}*/


/*bool scdc_transport_uds::stop()
{
  SCDC_TRACE("stop:");

  return scdc_transport::stop();
}*/


/*bool scdc_transport_uds::cancel(bool interrupt)
{
  SCDC_TRACE("cancel: interrupt: '" << interrupt << "'");

  return scdc_transport::cancel(interrupt);
}*/


/*bool scdc_transport_uds::resume()
{
  SCDC_TRACE("resume:");

  return scdc_transport::resume();
}*/


scdc_transport_connection_t *scdc_transport_uds::accept()
{
  SCDC_TRACE("accept:");

  scdc_transport_uds_connection_t *uds_conn = new scdc_transport_uds_connection_t(this);

  socklen_t saddrlen = sizeof(uds_server->saddr);

#if SCDC_TRANSPORT_UDS_USE_SHUTDOWN_EXCEPTION
  try
  {
#endif
    uds_conn->socket = ::accept(uds_server->server_socket, (struct sockaddr *) &uds_server->saddr, &saddrlen);

#if SCDC_TRANSPORT_UDS_USE_SHUTDOWN_EXCEPTION
  } catch (const scdc_transport_uds_shutdown_ex &ex)
  {
    SCDC_INFO("connection " << uds_conn << ": exception during accept");
    uds_conn->socket = -1;
  }
#endif

  if (uds_conn->socket != -1)
  {
    uds_conn->open = true;

  } else
  {
    if (!shutdowned) SCDC_FAIL("accept: accepting connection failed: '" << strerror(errno) << "'");
    delete uds_conn;
    uds_conn = 0;
    return 0;
  }

  SCDC_TRACE("accept: '" << uds_conn << "'");

  return scdc_transport::accept(uds_conn);
}


void scdc_transport_uds::shutdown(scdc_transport_connection_t *conn, bool interrupt)
{
  SCDC_TRACE("shutdown: interrupt: '" << interrupt << "'");

  scdc_transport::shutdown(conn, interrupt);

#if SCDC_TRANSPORT_UDS_USE_SHUTDOWN_EXCEPTION
  if (interrupt) throw scdc_transport_uds_shutdown_ex();
#endif

  if (conn)
  {
    SCDC_TRACE("shutdown: connection socket shutdown");

    scdc_transport_uds_connection_t *uds_conn = static_cast<scdc_transport_uds_connection_t *>(conn);

    ::shutdown(uds_conn->socket, SHUT_RDWR);

  } else if (uds_server)
  {
    SCDC_TRACE("shutdown: server socket shutdown");

    ::shutdown(uds_server->server_socket, SHUT_RDWR);
  }
}


void scdc_transport_uds::serve(scdc_transport_connection_t *conn)
{
#if SCDC_TRANSPORT_UDS_USE_SHUTDOWN_EXCEPTION
  /* catch our shutdown exceptions while serving (i.e., outside send or receive) */
  try
  {
#endif
    scdc_transport::serve(conn);

#if SCDC_TRANSPORT_UDS_USE_SHUTDOWN_EXCEPTION
  } catch (const scdc_transport_uds_shutdown_ex &ex)
  {
    SCDC_INFO("serve: catch shutdown exception");
  }
#endif
}


void scdc_transport_uds::close(scdc_transport_connection_t *conn)
{
  SCDC_TRACE("close: '" << conn << "'");

  scdc_transport_uds_connection_t *uds_conn = static_cast<scdc_transport_uds_connection_t *>(conn);

  ::close(uds_conn->socket);

  scdc_transport::close(conn);

  delete uds_conn;
}


scdc_transport_connection_t *scdc_transport_uds::connect(const char *socketfile)
{
  SCDC_TRACE("connect: socketfile: '" << socketfile << "'");

  scdc_transport_uds_connection_t *uds_conn = new scdc_transport_uds_connection_t(this);

  uds_conn->socket = ::socket(PF_LOCAL, SOCK_STREAM, 0);
  if (uds_conn->socket == -1)
  {
    SCDC_ERROR("start: creating client socket failed: '" << strerror(errno) << "'");
    return 0;
  }

  struct sockaddr_un saddr;
  memset(&saddr, 0, sizeof(saddr));

  saddr.sun_family = AF_LOCAL;
  strcpy(saddr.sun_path, socketfile);

  int r;
  bool except = false;

#if SCDC_TRANSPORT_UDS_USE_SHUTDOWN_EXCEPTION
  try
  {
#endif
    r = ::connect(uds_conn->socket, (struct sockaddr *) &saddr, sizeof(saddr));

#if SCDC_TRANSPORT_UDS_USE_SHUTDOWN_EXCEPTION
  } catch (const scdc_transport_uds_shutdown_ex &ex)
  {
    SCDC_INFO("connect: catch shutdown exception");
    r = -1;
    except = true;
  }
#endif

  if (r == -1)
  {
    if (!except) SCDC_FAIL("connect: connecting to server failed: '" << strerror(errno) << "'");
    ::close(uds_conn->socket);
    delete uds_conn;
    return 0;
  }

  return scdc_transport::connect(uds_conn);
}


bool scdc_transport_uds::disconnect(scdc_transport_connection_t *conn)
{
  SCDC_TRACE("disconnect: conn: '" << conn << "'");

  if (!scdc_transport::disconnect(conn)) return false;

  scdc_transport_uds_connection_t *uds_conn = static_cast<scdc_transport_uds_connection_t *>(conn);

  ::close(uds_conn->socket);

  delete uds_conn;

  return true;
}


scdcint_t scdc_transport_uds::send(scdc_transport_connection_t *conn, const void *buf, scdcint_t buf_size)
{
  SCDC_TRACE("send: conn: '" << conn << "', buf: '" << buf << "', buf_size: " << buf_size);

  scdc_transport_uds_connection_t *uds_conn = static_cast<scdc_transport_uds_connection_t *>(conn);

  scdcint_t n = 0;

  bool except = false;

#if SCDC_TRANSPORT_UDS_USE_SHUTDOWN_EXCEPTION
  try
  {
#endif
do_send:
    n = ::send(uds_conn->socket, buf, buf_size, 0);

    if (n < 0 && errno == EINTR)
    {
      SCDC_TRACE("send: continuing after EINTR");
      goto do_send;
    }

#if SCDC_TRANSPORT_UDS_USE_SHUTDOWN_EXCEPTION
  } catch (const scdc_transport_uds_shutdown_ex &ex)
  {
    SCDC_INFO("connection: " << uds_conn << ": send: catch shutdown exception");
    n = -1;
    except = false;
  }
#endif

  if (n < 0)
  {
    if (!except) SCDC_ERROR("connection: " << uds_conn << ": send: send failed: '" << strerror(errno) << "'");
    return -1;
  }

  SCDC_TRACE("connection: " << uds_conn << ": send: sent " << n << " byte(s)");
  
  return n;
}


scdcint_t scdc_transport_uds::receive(scdc_transport_connection_t *conn, void *buf, scdcint_t max_buf_size)
{
  SCDC_TRACE("receive: conn: '" << conn << "', buf: '" << buf << "', max_buf_size: " << max_buf_size);

  scdc_transport_uds_connection_t *uds_conn = static_cast<scdc_transport_uds_connection_t *>(conn);

  if (max_buf_size <= 0)
  {
    SCDC_TRACE("connection: " << uds_conn << ": receive: nothing to receive");
    return 0;
  }

  scdcint_t n = 0;

  bool except = false;

#if SCDC_TRANSPORT_UDS_USE_SHUTDOWN_EXCEPTION
  try
  {
#endif

do_recv:
    n = ::recv(uds_conn->socket, buf, max_buf_size, 0);

    if (n < 0 && errno == EINTR)
    {
      SCDC_TRACE("receive: continuing after EINTR");
      goto do_recv;
    }

#if SCDC_TRANSPORT_UDS_USE_SHUTDOWN_EXCEPTION
  } catch (const scdc_transport_uds_shutdown_ex &ex)
  {
    SCDC_INFO("connection " << uds_conn << ": receive: catch shutdown receive");
    n = -1;
    except = false;
  }
#endif

  if (n < 0)
  {
    if (!except) SCDC_ERROR("connection: " << uds_conn << ": receive: receive failed: '" << strerror(errno) << "'");
    return -1;
  }

  SCDC_TRACE("connection " << uds_conn << ": receive: received " << n << " byte(s)");

  /* receiving 0 bytes signals that the socket was closed */
  if (n == 0) n = -1;

  return n;
}


#undef SCDC_LOG_PREFIX
