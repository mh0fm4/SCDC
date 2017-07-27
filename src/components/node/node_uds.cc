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


#define SCDC_TRACE_NOT  !SCDC_TRACE_NODE_UDS

#include "config.hh"
#include "common.hh"
#include "log.hh"
#include "transport_uds.hh"
#include "compcoup_transport.hh"
#include "node_uds.hh"


using namespace std;


#define SCDC_LOG_PREFIX  "nodeport-uds: "


string make_uds_socketfile(const char *socketname)
{
  char s[256];
  snprintf(s, sizeof(s), NODE_UDS_SOCKETFILE_DEFAULT, socketname);

  return string(s);
}


/*bool scdc_nodeport_uds::init()
{
  SCDC_TRACE("init:");

  return true;
}


void scdc_nodeport_uds::release()
{
  SCDC_TRACE("release:");
}*/


bool scdc_nodeport_uds::authority(const char *conf, scdc_args *args, std::string &auth)
{
  SCDC_TRACE("authority: conf: '" << conf << "'");

  auth.clear();

  const char *socketname;

  if (!args->get<const char *>(SCDC_ARGS_TYPE_CSTR, &socketname))
  {
    SCDC_FAIL("authority: failed to get socket name parameter!");
    return false;
  }

  auth = socketname;

  SCDC_TRACE("authority: return: '" << auth << "'");

  return true;
}


bool scdc_nodeport_uds::supported(const char *uri, scdc_args *args)
{
  SCDC_TRACE("supported: uri: '" << uri << "'");

  bool ret = (string(uri).compare(0, strlen(SCDC_NODE_UDS_SCHEME) + 1, SCDC_NODE_UDS_SCHEME ":") == 0);

  SCDC_TRACE("supported: return: '" << ret << "'");

  return ret;
}


scdc_nodeport_uds::scdc_nodeport_uds()
  :scdc_nodeport("uds", SCDC_NODEPORT_START_LOOP_UNTIL_CANCEL|SCDC_NODEPORT_START_ASYNC_UNTIL_CANCEL)
{
  transport = new scdc_transport_uds();

  scdc_compcoup_transport *compcoup_transport = new scdc_compcoup_transport(transport);

  transport->set_compcoup_transport(compcoup_transport);
  compcoup = compcoup_transport;

  socketfile = make_uds_socketfile(NODE_UDS_SOCKETNAME_DEFAULT);
}


scdc_nodeport_uds::~scdc_nodeport_uds()
{
  delete compcoup; compcoup = 0;
  delete transport; transport = 0;
}


bool scdc_nodeport_uds::open_config_conf(const std::string &conf, scdc_args *args, bool &done)
{
  done = true;
  bool ret = true;

  if (conf == "socketname")
  {
    const char *s;

    if (args->get<const char *>(SCDC_ARGS_TYPE_CSTR, &s) == SCDC_ARG_REF_NULL)
    {
      SCDC_ERROR("open_config_conf: getting socket name");
      ret = false;

    } else
    {
      SCDC_TRACE("open_config_conf: socketname: '" << s << "'");
      socketfile = make_uds_socketfile(s);
    }

  } else if (conf == "socketfile")
  {
    const char *s;

    if (args->get<const char *>(SCDC_ARGS_TYPE_CSTR, &s) == SCDC_ARG_REF_NULL)
    {
      SCDC_ERROR("open_config_conf: getting socket file");
      ret = false;

    } else
    {
      SCDC_TRACE("open_config_conf: socketfile: '" << s << "'");
      socketfile = s;
    }

  } else
  {
    done = false;
    ret = scdc_nodeport::open_config_conf(conf, args, done);
  }

  return ret;
}


bool scdc_nodeport_uds::open(const char *conf, scdc_args *args)
{
  SCDC_TRACE("open: conf: '" << conf << "'");

  bool ret = true;

  if (!scdc_nodeport::open(conf, args))
  {
    SCDC_FAIL("open: opening base");
    ret = false;

  } else
  {
    scdc_transport_uds *transport_uds = static_cast<scdc_transport_uds *>(transport);

    SCDC_TRACE("open: socketfile: '" << socketfile << "'");

    if (!transport_uds->open(socketfile.c_str()))
    {
      SCDC_FAIL("open: opening UDS transport");
      ret = false;
      goto do_close;
    }

do_close:
    if (!ret) scdc_nodeport::close();
  }

  return ret;
}


void scdc_nodeport_uds::close()
{
  SCDC_TRACE("open: close");

  scdc_transport_uds *transport_uds = static_cast<scdc_transport_uds *>(transport);

  transport_uds->close();

  scdc_nodeport::close();
}


bool scdc_nodeport_uds::start(scdcint_t mode)
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

  if (max_connections >= 0) transport->set_max_connections(max_connections);

  scdc_transport_uds *transport_uds = static_cast<scdc_transport_uds *>(transport);

  return transport_uds->start(mode);
}


bool scdc_nodeport_uds::stop()
{
  scdc_transport_uds *transport_uds = static_cast<scdc_transport_uds *>(transport);

  transport_uds->stop();

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


bool scdc_nodeport_uds::cancel(bool interrupt)
{
  scdc_transport_uds *transport_uds = static_cast<scdc_transport_uds *>(transport);

  transport_uds->cancel(interrupt);

  return scdc_nodeport::cancel(interrupt);
}


#undef SCDC_LOG_PREFIX


#define SCDC_LOG_PREFIX  "nodeport-uds: "


scdc_nodeconn_uds::scdc_nodeconn_uds()
  :scdc_nodeconn("uds")
{
  transport = new scdc_transport_uds();

  scdc_compcoup_transport *compcoup_transport = new scdc_compcoup_transport(transport);

  transport->set_compcoup_transport(compcoup_transport);
  compcoup = compcoup_transport;

  transport_connection = 0;
}


scdc_nodeconn_uds::~scdc_nodeconn_uds()
{
  delete compcoup; compcoup = 0;
  delete transport; transport = 0;
}


bool scdc_nodeconn_uds::open(const char *authority)
{
  SCDC_TRACE("open: connect to '" << authority << "'");

  if (strlen(authority) <= 0) authority = NODE_UDS_SOCKETNAME_DEFAULT;

  stringlist sl(':', authority);

  string type = sl.front_pop();
  string val = sl.conflate();

  string socketfile;

  if (sl.size() <= 0)
  {
    socketfile = make_uds_socketfile(type.c_str());

  } else if (type == "socketname" || type == "")
  {
    socketfile = make_uds_socketfile(val.c_str());

  } else if (type == "socketfile")
  {
    socketfile = val;

  } else
  {
    SCDC_ERROR("unknown UDS nodeport type '" << type << "'");
    return false;
  }

  scdc_transport_uds *transport_uds = static_cast<scdc_transport_uds *>(transport);

  transport_connection = transport_uds->connect(socketfile.c_str());

  if (!transport_connection)
  {
    SCDC_FAIL("open: connect failed!");
    return false;
  }

  transport_connection->on_idle_handler = scdc_nodeconn_on_idle_close_handler;
  transport_connection->on_idle_data = this;

  return scdc_nodeconn::open(authority);
}


void scdc_nodeconn_uds::close()
{
  SCDC_TRACE("close:");

  scdc_nodeconn::close();

  transport->disconnect(transport_connection);

  transport_connection = 0;
}


bool scdc_nodeconn_uds::is_idle()
{
  return transport_connection->is_idle();
}


#undef SCDC_LOG_PREFIX
