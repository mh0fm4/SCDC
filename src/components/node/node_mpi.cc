/*
 *  Copyright (C) 2014, 2015, 2016, 2017 Michael Hofmann
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

#include "config.hh"
#include "common.hh"
#include "log.hh"
#include "transport_mpi.hh"
#include "compcoup_transport.hh"
#include "node_mpi.hh"


using namespace std;


#define SCDC_LOG_PREFIX  "nodeport-mpi: "


int scdc_nodeport_mpi_do_finalize = 0;


bool scdc_nodeport_mpi::init()
{
  SCDC_TRACE("init:");

  int x;
  MPI_Initialized(&x);

  if (!x)
  {
    SCDC_TRACE("MPI was not initialized!");

    int argc = 0;
    char **argv = NULL;

#if 0
    SCDC_TRACE("MPI_Init_thread");
    int required = MPI_THREAD_MULTIPLE;
    int provided;
    x = MPI_Init_thread(&argc, &argv, required, &provided);
#else
    SCDC_TRACE("MPI_Init");
    x = MPI_Init(&argc, &argv);
#endif

    if (x != MPI_SUCCESS)
    {
      SCDC_ERROR("MPI initialization failed!");
      return false;
    }

    scdc_nodeport_mpi_do_finalize = 1;
  }

  MPI_Query_thread(&x);

  SCDC_TRACE("MPI thread provided: " << ((x == MPI_THREAD_SINGLE)?"single":(x == MPI_THREAD_FUNNELED)?"funneled":(x == MPI_THREAD_SERIALIZED)?"serialized":(x == MPI_THREAD_MULTIPLE)?"multiple":"unknown"));

  int size, rank;
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  SCDC_TRACE("MPI size: " << size << ", rank: " << rank);

  return true;
}


void scdc_nodeport_mpi::release()
{
  SCDC_TRACE("release:");

  if (scdc_nodeport_mpi_do_finalize)
  {
    SCDC_TRACE("MPI_Finalize");
    MPI_Finalize();
  }
}


bool scdc_nodeport_mpi::authority(const char *conf, scdc_args *args, std::string &auth)
{
  SCDC_TRACE("authority: conf: '" << conf << "'");

  auth.clear();

  string c(conf);

  if (c == "world" || c == "")
  {
    auth = c;

  } else if (c == "comm")
  {
    MPI_Comm *comm_ptr;
    if (!args->get<MPI_Comm *>(SCDC_ARGS_TYPE_PTR, &comm_ptr))
    {
      SCDC_FAIL("authority: failed to get communicator parameter!");
      return false;
    }

    char comm_str[16];

    sprintf(comm_str, "%p", comm_ptr);

    auth = "comm:";
    auth += comm_str;

  } else if (c == "port")
  {
    const char *port_str;
    if (!args->get<const char *>(SCDC_ARGS_TYPE_CSTR, &port_str))
    {
      SCDC_FAIL("authority: failed to get port name parameter!");
      return false;
    }

    auth = "port:";
    auth += port_str;

  } else if (c == "publ")
  {
    const char *publ_str;
    if (!args->get<const char *>(SCDC_ARGS_TYPE_CSTR, &publ_str))
    {
      SCDC_FAIL("authority: failed to get published name parameter!");
      return false;
    }

    auth = "publ:";
    auth += publ_str;

  } else
  {
    SCDC_ERROR("unknown MPI nodeport type '" << c << "'");
    return false;
  }

  int rank;

  if (!args->get<int>(SCDC_ARGS_TYPE_INT, &rank))
  {
    SCDC_FAIL("authority: failed to get rank parameter!");
    return false;
  }

  if (auth.size() > 0) auth += ":";

  char rank_str[16];

  sprintf(rank_str, "%d", rank);
  auth += rank_str;

  SCDC_TRACE("authority: return: '" << auth << "'");

  return true;
}


bool scdc_nodeport_mpi::supported(const char *uri, scdc_args *args)
{
  SCDC_TRACE("supported: uri: '" << uri << "'");

  bool ret = (string(uri).compare(0, strlen(SCDC_NODE_MPI_SCHEME) + 1, SCDC_NODE_MPI_SCHEME ":") == 0);

  SCDC_TRACE("supported: return: '" << ret << "'");

  return ret;
}


scdc_nodeport_mpi::scdc_nodeport_mpi()
  :scdc_nodeport("mpi", SCDC_NODEPORT_START_LOOP_UNTIL_CANCEL|SCDC_NODEPORT_START_ASYNC_UNTIL_CANCEL), port_type("world")
{
  transport = new scdc_transport_mpi();

  scdc_compcoup_transport *compcoup_transport = new scdc_compcoup_transport(transport);

  transport->set_compcoup_transport(compcoup_transport);
  compcoup = compcoup_transport;

  port_val_comm = ::operator new(sizeof(MPI_Comm));
  port_val_port = 0;
}


scdc_nodeport_mpi::~scdc_nodeport_mpi()
{
  ::operator delete(port_val_comm);

  delete compcoup; compcoup = 0;
  delete transport; transport = 0;
}


bool scdc_nodeport_mpi::open_config_conf(const std::string &conf, scdc_args *args, bool &done)
{
  done = true;
  bool ret = true;

  if (conf == "world")
  {
    port_type = "world";

  } else if (conf == "comm")
  {
    port_type = "comm";

    if (args->get<MPI_Comm>(SCDC_ARGS_TYPE_PTR, static_cast<MPI_Comm *>(port_val_comm)) == SCDC_ARG_REF_NULL)
    {
      SCDC_ERROR("open_config_conf: getting MPI communicator");
      ret = false;

    } else
    {
      SCDC_TRACE("open_config_conf: port_val_comm: " << port_val_comm);
    }

  } else if (conf == "port")
  {
    port_type = "port";

    if (args->get<char *>(SCDC_ARGS_TYPE_PTR, &port_val_port) == SCDC_ARG_REF_NULL)
    {
      SCDC_ERROR("open_config_conf: getting port string ");
      ret = false;

    } else
    {
      SCDC_TRACE("open_config_conf: port_val_port: " << static_cast<void *>(port_val_port));
    }

  } else if (conf == "publ")
  {
    port_type = "publ";

    const char *s;

    if (args->get<const char *>(SCDC_ARGS_TYPE_CSTR, &s) == SCDC_ARG_REF_NULL)
    {
      SCDC_ERROR("open_config_conf: getting port string ");
      ret = false;

    } else
    {
      SCDC_TRACE("open_config_conf: publ_str: '" << s << "'");
      port_val_publ = s;
    }

  } else
  {
    done = false;
    ret = scdc_nodeport::open_config_conf(conf, args, done);
  }

  return ret;
}


bool scdc_nodeport_mpi::open(const char *conf, scdc_args *args)
{
  SCDC_TRACE("open: conf: '" << conf << "'");

  bool ret = true;

  if (!scdc_nodeport::open(conf, args))
  {
    SCDC_FAIL("open: opening base");
    ret = false;

  } else
  {
    scdc_transport_mpi *transport_mpi = static_cast<scdc_transport_mpi *>(transport);

    MPI_Comm comm = MPI_COMM_NULL;
    bool open_port = false;

    if (port_type == "world")
    {
      comm = MPI_COMM_WORLD;

    } else if (port_type == "comm")
    {
      comm = *static_cast<MPI_Comm *>(port_val_comm);

    } else if (port_type == "port")
    {
      open_port = true;

    } else if (port_type == "publ")
    {
      open_port = true;

    } else
    {
      SCDC_ERROR("open: unknown MPI nodeport type '" << port_type << "'");
      ret = false;
      goto do_close;
    }

    ret = (open_port)?transport_mpi->open():transport_mpi->open(comm);
    if (!ret)
    {
      SCDC_FAIL("open: opening MPI transport " << ((open_port)?"port":"communicator") << " failed!");
      ret = false;
      goto do_close;
    }

    if (port_type == "port")
    {
      char port_name[MPI_MAX_PORT_NAME + 1];
      transport_mpi->get_port_name(port_name);

      SCDC_TRACE("open: MPI port name: '" << port_name << "'");

      if (port_val_port) strncpy(port_val_port, port_name, MPI_MAX_PORT_NAME);

    } else if (port_type == "publ")
    {
      char port_name[MPI_MAX_PORT_NAME + 1];
      transport_mpi->get_port_name(port_name);

      if (MPI_Publish_name(const_cast<char *>(port_val_publ.c_str()), MPI_INFO_NULL, port_name) != MPI_SUCCESS)
      {
        SCDC_FAIL("open: publishing MPI port");
        ret = false;

      } else
      {
        SCDC_TRACE("open: MPI port name: '" << port_name << "', publish name: '" << port_val_publ << "'");
      }
    }

    if (!ret) transport_mpi->close();

do_close:
    if (!ret) scdc_nodeport::close();
  }

  return ret;
}


void scdc_nodeport_mpi::close()
{
  SCDC_TRACE("open: close");

  scdc_transport_mpi *transport_mpi = static_cast<scdc_transport_mpi *>(transport);

  char port_name[MPI_MAX_PORT_NAME + 1];

  if (port_type == "publ" && transport_mpi->get_port_name(port_name))
  {
    if (MPI_Unpublish_name(const_cast<char *>(port_val_publ.c_str()), MPI_INFO_NULL, port_name) != MPI_SUCCESS)
    {
      SCDC_FAIL("open: unpublishing MPI port");
      return;
    }
  }

  transport_mpi->close();

  scdc_nodeport::close();
}


bool scdc_nodeport_mpi::start(scdcint_t mode)
{
  if (!scdc_nodeport::start(mode)) return false;

  if (cmd_handler_args.handler)
  {
    scdc_compcoup_transport *compcoup_transport = static_cast<scdc_compcoup_transport *>(compcoup);
    compcoup_transport->set_cmd_handler(cmd_handler_args.handler, cmd_handler_args.data);
  }

  if (mode == SCDC_NODEPORT_START_LOOP_UNTIL_CANCEL && loop_handler_dummy_args.handler)
  {
    transport->set_loop_handler(loop_handler_dummy_args.handler, loop_handler_dummy_args.data);
  }

  scdc_transport_mpi *transport_mpi = static_cast<scdc_transport_mpi *>(transport);

  return transport_mpi->start(mode);
}


bool scdc_nodeport_mpi::stop()
{
  scdc_transport_mpi *transport_mpi = static_cast<scdc_transport_mpi *>(transport);

  transport_mpi->stop();

  if (loop_handler_dummy_args.handler)
  {
    transport->set_loop_handler(0, 0);
  }

  if (cmd_handler_args.handler)
  {
    scdc_compcoup_transport *compcoup_transport = static_cast<scdc_compcoup_transport *>(compcoup);
    compcoup_transport->set_cmd_handler(0, 0);
  }

  return scdc_nodeport::stop();
}


bool scdc_nodeport_mpi::cancel(bool interrupt)
{
  scdc_transport_mpi *transport_mpi = static_cast<scdc_transport_mpi *>(transport);

  transport_mpi->cancel(interrupt);

  return scdc_nodeport::cancel(interrupt);
}


#undef SCDC_LOG_PREFIX


#define SCDC_LOG_PREFIX  "nodeport-mpi: "


scdc_nodeconn_mpi::scdc_nodeconn_mpi()
  :scdc_nodeconn("mpi")
{
  transport = new scdc_transport_mpi();

  scdc_compcoup_transport *compcoup_transport = new scdc_compcoup_transport(transport);

  transport->set_compcoup_transport(compcoup_transport);
  compcoup = compcoup_transport;

  transport_connection = 0;
}


scdc_nodeconn_mpi::~scdc_nodeconn_mpi()
{
  delete compcoup; compcoup = 0;
  delete transport; transport = 0;
}


bool scdc_nodeconn_mpi::open(const char *authority)
{
  SCDC_TRACE("open: connect to '" << authority << "'");

  stringlist sl(':', authority);

  if (sl.size() <= 0) return false;

  int rank;
  MPI_Comm comm = MPI_COMM_NULL;
  char port_name[MPI_MAX_PORT_NAME + 1] = { '\0' };

  string s = sl.back_pop();
  sscanf(s.c_str(), "%d", &rank);

  string port_type = sl.front_pop();
  string port_val = sl.conflate();

  if (port_type == "world" || port_type == "")
  {
    SCDC_TRACE("open: connecting to rank '" << rank << "' of communicator MPI_COMM_WORLD");

    comm = MPI_COMM_WORLD;

  } else if (port_type == "comm")
  {
    SCDC_TRACE("open: connecting to rank '" << rank << "' of communicator '" << port_val << "'");

/*    str2mem(port_val, &comm, 0);*/

  } else if (port_type == "port")
  {
    SCDC_TRACE("open: connecting to rank '" << rank << "' of port name '" << port_val << "'");

    strncpy(port_name, port_val.c_str(), MPI_MAX_PORT_NAME);

  } else if (port_type == "publ")
  {
    SCDC_TRACE("open: connecting to rank '" << rank << "' of published name '" << port_val << "'");

    MPI_Lookup_name(const_cast<char *>(port_val.c_str()), MPI_INFO_NULL, port_name);

  } else
  {
    SCDC_ERROR("unknown MPI nodeport type '" << port_type << "'");
    return false;
  }

  scdc_transport_mpi *transport_mpi = static_cast<scdc_transport_mpi *>(transport);

  transport_connection = (port_name[0] != '\0')?transport_mpi->connect(port_name, rank):transport_mpi->connect(comm, rank);

  if (!transport_connection)
  {
    SCDC_FAIL("open: connect failed!");
    return false;
  }

  transport_connection->on_idle_handler = scdc_nodeconn_on_idle_close_handler;
  transport_connection->on_idle_data = this;

  return scdc_nodeconn::open(authority);
}


void scdc_nodeconn_mpi::close()
{
  SCDC_TRACE("close:");

  scdc_nodeconn::close();

  transport->disconnect(transport_connection);

  transport_connection = 0;
}


bool scdc_nodeconn_mpi::is_idle()
{
  return transport_connection->is_idle();
}


#undef SCDC_LOG_PREFIX
