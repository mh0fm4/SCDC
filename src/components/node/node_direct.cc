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


#define SCDC_TRACE_NOT  !SCDC_TRACE_NODE_DIRECT

#include "config.hh"
#include "common.hh"
#include "log.hh"
#include "compcoup_direct.hh"
#include "node_direct.hh"


using namespace std;


#define SCDC_LOG_PREFIX  "nodeport-direct: "


/*bool scdc_nodeport_direct::init()
{
  SCDC_TRACE("init:");

  return true;
}


void scdc_nodeport_direct::release()
{
  SCDC_TRACE("release:");
}*/


bool scdc_nodeport_direct::authority(const char *conf, scdc_args *args, std::string &auth)
{
  SCDC_TRACE("authority: conf: '" << conf << "'");

  auth.clear();

  const char *id;

  if (args->get<const char *>(SCDC_ARGS_TYPE_CSTR, &id) == SCDC_ARG_REF_NULL)
  {
    SCDC_ERROR("authority: getting id");
    return false;
  }

  auth = id;

  SCDC_TRACE("authority: return: '" << auth << "'");

  return true;
}


bool scdc_nodeport_direct::supported(const char *uri, scdc_args *args)
{
  SCDC_TRACE("supported: uri: '" << uri << "'");

  bool ret = (string(uri).compare(0, strlen(SCDC_NODE_DIRECT_SCHEME) + 1, SCDC_NODE_DIRECT_SCHEME ":") == 0);

  SCDC_TRACE("supported: return: '" << ret << "'");

  return ret;
}


scdc_nodeport_direct::scdc_nodeport_direct()
  :scdc_nodeport("direct", SCDC_NODEPORT_START_LOOP_UNTIL_CANCEL|SCDC_NODEPORT_START_ASYNC_UNTIL_CANCEL)
{
  compcoup = new scdc_compcoup_direct();
}


scdc_nodeport_direct::~scdc_nodeport_direct()
{
  delete compcoup;
}


bool scdc_nodeport_direct::open(const char *conf, scdc_args *args)
{
  const char *s;

  if (args->get<const char *>(SCDC_ARGS_TYPE_CSTR, &s) == SCDC_ARG_REF_NULL)
  {
    SCDC_ERROR("open: getting id");
    return false;
  }

  id = s;

  return scdc_nodeport::open(conf, args);
}


bool scdc_nodeport_direct::start(scdcint_t mode)
{
  if (!scdc_nodeport::start(mode)) return false;

  scdc_compcoup_direct *compcoup_direct = static_cast<scdc_compcoup_direct *>(compcoup);

  return compcoup_direct->start(mode);
}


bool scdc_nodeport_direct::stop()
{
  scdc_compcoup_direct *compcoup_direct = static_cast<scdc_compcoup_direct *>(compcoup);

  if (!compcoup_direct->stop()) return false;

  return scdc_nodeport::stop();
}


bool scdc_nodeport_direct::cancel(bool interrupt)
{
  scdc_compcoup_direct *compcoup_direct = static_cast<scdc_compcoup_direct *>(compcoup);

  if (!compcoup_direct->cancel(interrupt)) return false;

  return scdc_nodeport::stop();
}


#undef SCDC_LOG_PREFIX


#define SCDC_LOG_PREFIX  "nodeconn-direct: "


scdc_nodeconn_direct::scdc_nodeconn_direct(scdc_nodeport_pool *nodeport_pool_)
  :scdc_nodeconn("direct"), nodeport_pool(nodeport_pool_)
{
}


scdc_nodeconn_direct::~scdc_nodeconn_direct()
{
  nodeport_pool = 0;
}


bool scdc_nodeconn_direct::open(const char *authority)
{
  compcoup = 0;

  for (scdc_nodeport_pool::iterator np = nodeport_pool->begin(); np != nodeport_pool->end(); ++np)
  {
    scdc_nodeport *nodeport = *np;
    if (nodeport->get_type() == "direct" && static_cast<scdc_nodeport_direct *>(nodeport)->get_id() == authority)
    {
      compcoup = nodeport->get_compcoup();
      break;
    }
  }

  if (!compcoup) return false;

  return scdc_nodeconn::open(authority);
}


void scdc_nodeconn_direct::close()
{
  compcoup = 0;

  scdc_nodeconn::close();
}


#undef SCDC_LOG_PREFIX
