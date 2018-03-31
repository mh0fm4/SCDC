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


#define SCDC_TRACE_NOT  !SCDC_TRACE_NODEPORT_POOL

#include "config.hh"
#include "common.hh"
#include "log.hh"
#include "nodeport.hh"
#include "nodeport_pool.hh"
#if USE_NODE_DIRECT
# include "node_direct.hh"
#endif
#if USE_NODE_TCP
# include "node_tcp.hh"
#endif
#if USE_NODE_UDS
# include "node_uds.hh"
#endif
#if USE_NODE_MPI
#include "node_mpi.hh"
#endif
#if USE_NODE_STREAM
# include "node_stream.hh"
#endif
#if USE_NODE_TIMER
# include "nodeport_timer.hh"
#endif


using namespace std;


#define SCDC_LOG_PREFIX  "nodeport-pool: "


bool scdc_nodeport_pool::init()
{
  bool ret = true;

#if USE_NODE_DIRECT
  ret = ret && scdc_nodeport_direct::init();
#endif
#if USE_NODE_TCP
  ret = ret && scdc_nodeport_tcp::init();
#endif
#if USE_NODE_UDS
  ret = ret && scdc_nodeport_uds::init();
#endif
#if USE_NODE_MPI
  ret = ret && scdc_nodeport_mpi::init();
#endif
#if USE_NODE_STREAM
  ret = ret && scdc_nodeport_stream::init();
#endif
#if USE_NODE_TIMER
  ret = ret && scdc_nodeport_timer::init();
#endif

  return ret;
}


void scdc_nodeport_pool::release()
{
#if USE_NODE_DIRECT
  scdc_nodeport_direct::release();
#endif
#if USE_NODE_TCP
  scdc_nodeport_tcp::release();
#endif
#if USE_NODE_UDS
  scdc_nodeport_uds::release();
#endif
#if USE_NODE_MPI
  scdc_nodeport_mpi::release();
#endif
#if USE_NODE_STREAM
  scdc_nodeport_stream::release();
#endif
#if USE_NODE_TIMER
  scdc_nodeport_timer::release();
#endif
}


bool scdc_nodeport_pool::authority(const char *conf, scdc_args *args, std::string &auth)
{
  stringlist confs(':', conf);
  string np_type = confs.front_pop();
  conf += confs.offset();

#if USE_NODE_DIRECT
  if (np_type == "direct") return scdc_nodeport_direct::authority(conf, args, auth);
#endif
#if USE_NODE_TCP
  if (np_type == "tcp") return scdc_nodeport_tcp::authority(conf, args, auth);
#endif
#if USE_NODE_UDS
  if (np_type == "uds") return scdc_nodeport_uds::authority(conf, args, auth);
#endif
#if USE_NODE_MPI
  if (np_type == "mpi") return scdc_nodeport_mpi::authority(conf, args, auth);
#endif
#if USE_NODE_STREAM
  if (np_type == "stream") return scdc_nodeport_stream::authority(conf, args, auth);
#endif
#if USE_NODE_TIMER
  if (np_type == "timer") return scdc_nodeport_timer::authority(conf, args, auth);
#endif

  if (np_type.size() > 0)
  {
    SCDC_ERROR("unknown nodeport type '" << np_type << "'");
    auth.clear();
    return false;
  }

  return scdc_nodeport::authority(conf, args, auth);
}


bool scdc_nodeport_pool::supported(const char *uri, scdc_args *args)
{
#if USE_NODE_DIRECT
  if (scdc_nodeport_direct::supported(uri, args)) return true;
#endif
#if USE_NODE_TCP
  if (scdc_nodeport_tcp::supported(uri, args)) return true;
#endif
#if USE_NODE_UDS
  if (scdc_nodeport_uds::supported(uri, args)) return true;
#endif
#if USE_NODE_MPI
  if (scdc_nodeport_mpi::supported(uri, args)) return true;
#endif
#if USE_NODE_STREAM
  if (scdc_nodeport_stream::supported(uri, args)) return true;
#endif
#if USE_NODE_TIMER
  if (scdc_nodeport_timer::supported(uri, args)) return true;
#endif

  return scdc_nodeport::supported(uri, args);
}


scdc_nodeport *scdc_nodeport_pool::open(const char *conf, scdc_args *args)
{
  SCDC_TRACE("open: conf: '" << conf << "'");

  stringlist confs(':', conf);
  string np_type = confs.front_pop();

  scdc_nodeport *nodeport = 0;

#if USE_NODE_DIRECT
  if (np_type == "direct") nodeport = new scdc_nodeport_direct();
  else
#endif
#if USE_NODE_TCP
  if (np_type == "tcp") nodeport = new scdc_nodeport_tcp();
  else
#endif
#if USE_NODE_UDS
  if (np_type == "uds") nodeport = new scdc_nodeport_uds();
  else
#endif
#if USE_NODE_MPI
  if (np_type == "mpi") nodeport = new scdc_nodeport_mpi();
  else
#endif
#if USE_NODE_STREAM
  if (np_type == "stream") nodeport = new scdc_nodeport_stream();
  else
#endif
#if USE_NODE_TIMER
  if (np_type == "timer") nodeport = new scdc_nodeport_timer();
  else
#endif
  SCDC_ERROR("unkown nodeport type '" << np_type << "'");

  if (!nodeport) return 0;

  if (!nodeport->open(confs.conflate().c_str(), args))
  {
    delete nodeport;
    return 0;
  }

  pair<iterator, bool> ret = insert(nodeport);

  if (!ret.second)
  {
    delete nodeport;
    return 0;
  }

  return nodeport;
}


void scdc_nodeport_pool::close(scdc_nodeport *nodeport)
{
  SCDC_TRACE("close: nodeport: '" << nodeport << "'");

  erase(nodeport);

  nodeport->close();

  delete nodeport;

  SCDC_TRACE("close: return");
}


void scdc_nodeport_pool::close_all()
{
  SCDC_TRACE("close_all:");

  while (!empty()) close(*begin());

  SCDC_TRACE("close_all: return");
}


#undef SCDC_LOG_PREFIX
