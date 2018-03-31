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

#include "config.hh"
#include "common.hh"
#include "log.hh"
#include "transport_stream.hh"
#include "compcoup_transport.hh"
#include "node_stream.hh"


using namespace std;


#define SCDC_LOG_PREFIX  "nodeport-stream: "


scdc_nodeport_stream::scdc_nodeport_stream()
  :scdc_nodeport("stream", SCDC_NODEPORT_START_LOOP_UNTIL_CANCEL)
{
  transport = new scdc_transport_stream();

  scdc_compcoup_transport *compcoup_transport = new scdc_compcoup_transport(transport);

  transport->set_compcoup_transport(compcoup_transport);
  compcoup = compcoup_transport;
}


scdc_nodeport_stream::~scdc_nodeport_stream()
{
  delete compcoup; compcoup = 0;
  delete transport; transport = 0;
}


bool scdc_nodeport_stream::open_config_conf(const std::string &conf, scdc_args *args, bool &done)
{
  return scdc_nodeport::open_config_conf(conf, args, done);
}


bool scdc_nodeport_stream::start(scdcint_t mode)
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

  scdc_transport_stream *transport_stream = static_cast<scdc_transport_stream *>(transport);

#if SCDC_TRANSPORT_STREAM_USE_IOSTREAM
  return transport_stream->start(mode, &cin, &cout);
#else
  return transport_stream->start(mode, stdin, stdout);
#endif
}


bool scdc_nodeport_stream::stop()
{
  scdc_transport_stream *transport_stream = static_cast<scdc_transport_stream *>(transport);

  transport_stream->stop();

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


bool scdc_nodeport_stream::cancel(bool interrupt)
{
  scdc_transport_stream *transport_stream = static_cast<scdc_transport_stream *>(transport);

  transport_stream->cancel(interrupt);

  return scdc_nodeport::cancel(interrupt);
}


#undef SCDC_LOG_PREFIX
