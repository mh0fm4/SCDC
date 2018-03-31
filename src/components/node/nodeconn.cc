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


#define SCDC_TRACE_NOT  !SCDC_TRACE_NODECONN

#include "config.hh"
#include "log.hh"
#include "nodeconn.hh"
#include "nodeconn_pool.hh"


using namespace std;


#define SCDC_LOG_PREFIX  "nodeconn: "


scdc_nodeconn::scdc_nodeconn(const char *type_)
  :ref(0), close_on_idle(0), nodeconn_pool(0), type(type_), compcoup(0)
{
}


scdc_nodeconn::~scdc_nodeconn()
{
}


bool scdc_nodeconn::open(const char *authority)
{
  if (compcoup) compcoup->handshake();

  return true;
}


void scdc_nodeconn::close()
{
}


void scdc_nodeconn_on_idle_close(scdc_nodeconn *nodeconn)
{
  ++nodeconn->close_on_idle;
}


scdcint_t scdc_nodeconn_on_idle_close_handler(void *data)
{
  scdc_nodeconn *nodeconn = static_cast<scdc_nodeconn *>(data);

  SCDC_TRACE("scdc_nodeconn_on_idle_close_handler: close_on_idle: '" << nodeconn->close_on_idle << "'");

  if (nodeconn->close_on_idle > 0)
  {
    --nodeconn->close_on_idle;

    nodeconn->nodeconn_pool->close(nodeconn);
  }

  return SCDC_SUCCESS;
}


#undef SCDC_LOG_PREFIX
