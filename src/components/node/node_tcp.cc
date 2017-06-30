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
#include <cstring>

#define SCDC_TRACE_NOT  !SCDC_TRACE_NODE_TCP

#include "config.hh"
#include "common.hh"
#include "log.hh"
#include "transport_tcp.hh"
#include "compcoup_transport.hh"
#include "node_tcp.hh"


using namespace std;


#define SCDC_LOG_PREFIX  "nodeport-tcp: "


/*bool scdc_nodeport_tcp::init()
{
  SCDC_TRACE("init:");

  return true;
}


void scdc_nodeport_tcp::release()
{
  SCDC_TRACE("release:");
}*/


bool scdc_nodeport_tcp::authority(const char *conf, scdc_args *args, std::string &auth)
{
  SCDC_TRACE("authority: conf: '" << conf << "'");

  auth.clear();

  const char *hostname;

  if (!args->get<const char *>(SCDC_ARGS_TYPE_CSTR, &hostname))
  {
    SCDC_FAIL("authority: failed to get hostname parameter!");
    return false;
  }

  auth = hostname;

  string c(conf);

  if (c == "port")
  {
    int port;

    if (!args->get<int>(SCDC_ARGS_TYPE_INT, &port))
    {
      SCDC_FAIL("authority: failed to get port parameter!");
      return false;
    }

    char port_str[16];

    sprintf(port_str, ":%d", port);
    auth += port_str;
  }

  SCDC_TRACE("authority: return: '" << auth << "'");

  return true;
}


bool scdc_nodeport_tcp::supported(const char *uri, scdc_args *args)
{
  SCDC_TRACE("supported: uri: '" << uri << "'");

  bool ret = (string(uri).compare(0, strlen(SCDC_NODE_TCP_SCHEME) + 1, SCDC_NODE_TCP_SCHEME ":") == 0);

  SCDC_TRACE("authority: return: '" << ret << "'");

  return ret;
}


scdc_nodeport_tcp::scdc_nodeport_tcp()
  :scdc_nodeport("tcp", SCDC_NODEPORT_START_LOOP_UNTIL_CANCEL|SCDC_NODEPORT_START_ASYNC_UNTIL_CANCEL), port(NODE_TCP_PORT_DEFAULT)
{
  transport = new scdc_transport_tcp();

  scdc_compcoup_transport *compcoup_transport = new scdc_compcoup_transport(transport);

  transport->set_compcoup_transport(compcoup_transport);
  compcoup = compcoup_transport;
}


scdc_nodeport_tcp::~scdc_nodeport_tcp()
{
  delete compcoup; compcoup = 0;
  delete transport; transport = 0;
}


bool scdc_nodeport_tcp::open_config_conf(const std::string &conf, scdc_args *args, bool &done)
{
  done = true;
  bool ret = true;

  if (conf == "address")
  {
    const char *a;

    if (args->get<const char *>(SCDC_ARGS_TYPE_CSTR, &a) == SCDC_ARG_REF_NULL)
    {
      SCDC_ERROR("open_config_conf: getting address");
      ret = false;

    } else
    {
      SCDC_TRACE("open_config_conf: address: '" << address << "'");
      address = a;
    }

  } else if (conf == "port")
  {
    if (args->get<int>(SCDC_ARGS_TYPE_INT, &port) == SCDC_ARG_REF_NULL)
    {
      SCDC_ERROR("open_config_conf: getting port");
      ret = false;

    } else
    {
      SCDC_TRACE("open_config_conf: port: " << port);
    }

  } else
  {
    done = false;
    ret = scdc_nodeport::open_config_conf(conf, args, done);
  }

  return ret;
}


bool scdc_nodeport_tcp::open(const char *conf, scdc_args *args)
{
  SCDC_TRACE("open: conf: '" << conf << "'");

  bool ret = true;

  if (!scdc_nodeport::open(conf, args))
  {
    SCDC_FAIL("open: opening base");
    ret = false;

  } else
  {
    scdc_transport_tcp *transport_tcp = static_cast<scdc_transport_tcp *>(transport);

    SCDC_TRACE("open: address: '" << address << "', port: '" << port << "'");

    if (!transport_tcp->open(address.c_str(), port))
    {
      SCDC_FAIL("open: opening TCP transport");
      ret = false;
      goto do_close;
    }

do_close:
    if (!ret) scdc_nodeport::close();
  }

  return ret;
}


void scdc_nodeport_tcp::close()
{
  SCDC_TRACE("open: close");

  scdc_transport_tcp *transport_tcp = static_cast<scdc_transport_tcp *>(transport);

  transport_tcp->close();

  scdc_nodeport::close();
}


bool scdc_nodeport_tcp::start(scdcint_t mode)
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

  scdc_transport_tcp *transport_tcp = static_cast<scdc_transport_tcp *>(transport);

  return transport_tcp->start(mode);
}


bool scdc_nodeport_tcp::stop()
{
  scdc_transport_tcp *transport_tcp = static_cast<scdc_transport_tcp *>(transport);

  transport_tcp->stop();

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


bool scdc_nodeport_tcp::cancel(bool interrupt)
{
  scdc_transport_tcp *transport_tcp = static_cast<scdc_transport_tcp *>(transport);

  transport_tcp->cancel(interrupt);

  return scdc_nodeport::cancel(interrupt);
}


#undef SCDC_LOG_PREFIX


#define SCDC_LOG_PREFIX  "nodeport-tcp: "


scdc_nodeconn_tcp::scdc_nodeconn_tcp()
  :scdc_nodeconn("tcp")
{
  transport = new scdc_transport_tcp();

  scdc_compcoup_transport *compcoup_transport = new scdc_compcoup_transport(transport);

  transport->set_compcoup_transport(compcoup_transport);
  compcoup = compcoup_transport;

  transport_connection = 0;
}


scdc_nodeconn_tcp::~scdc_nodeconn_tcp()
{
  delete compcoup; compcoup = 0;
  delete transport; transport = 0;
}


bool scdc_nodeconn_tcp::open(const char *authority)
{
  SCDC_TRACE("open: connect to '" << authority << "'");

  stringlist sl(':', authority);

  string host = sl.front_pop();
  if (host.size() <= 0) host = NODE_TCP_HOST_DEFAULT;

  int port = NODE_TCP_PORT_DEFAULT;

  if (sl.size() >= 1)
  {
    sscanf(sl.front_pop().c_str(), "%d", &port);
  }

  scdc_transport_tcp *transport_tcp = static_cast<scdc_transport_tcp *>(transport);

  transport_connection = transport_tcp->connect(host.c_str(), port);

  if (!transport_connection)
  {
    SCDC_FAIL("open: connect failed!");
    return false;
  }

  transport_connection->on_idle_handler = scdc_nodeconn_on_idle_close_handler;
  transport_connection->on_idle_data = this;

  return scdc_nodeconn::open(authority);
}


void scdc_nodeconn_tcp::close()
{
  SCDC_TRACE("close:");

  scdc_nodeconn::close();

  transport->disconnect(transport_connection);

  transport_connection = 0;
}


bool scdc_nodeconn_tcp::is_idle()
{
  return transport_connection->is_idle();
}


#undef SCDC_LOG_PREFIX
