/*
 *  Copyright (C) 2014, 2015, 2016 Michael Hofmann
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

#define SCDC_TRACE_NOT  !SCDC_TRACE_NODECONN_POOL

#include "config.hh"
#include "common.hh"
#include "log.hh"
#include "nodeconn_pool.hh"
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


#define SCDC_LOG_PREFIX  "nodeconn-pool: "


void scdc_nodeconn_pool::add(const char *scheme, const char *authority, scdc_nodeconn *nodeconn)
{
  SCDC_TRACE("add: scheme: '" << scheme << "', authority: '" << authority << "', nodeconn: '" << nodeconn << "'");

  join_uri(scheme, authority, 0, nodeconn->key);

  (*this)[nodeconn->key].insert(nodeconn);

  SCDC_TRACE("add: key: '" << nodeconn->key << "'");
}


void scdc_nodeconn_pool::del(scdc_nodeconn *nodeconn)
{
  SCDC_TRACE("del: nodeconn: '" << nodeconn << "'");

  SCDC_ASSERT(nodeconn->ref <= 0);

  iterator i = find(nodeconn->key);

  if (i == end() || i->second.erase(nodeconn) < 1)
  {
    SCDC_FAIL("del: nodeconn '" << nodeconn << "' not found");
    return;
  }

  if (i != end() && i->second.size() <= 0) erase(i);
}


#if NODECONN_CACHE_CONNECTIONS

scdc_nodeconn *scdc_nodeconn_pool::lookup_cache(const char *scheme, const char *authority)
{
  SCDC_TRACE("lookup_cache: scheme: '" << scheme << "', authority: '" << authority << "'");

  string key;
  join_uri(scheme, authority, 0, key);

  iterator i = find(key);

  scdc_nodeconn *nodeconn = 0;

  if (i != end())
  {
    for (mapped_type::iterator j = i->second.begin(); !nodeconn && j != i->second.end(); ++j)
    {
      if ((*j)->ref == 0) nodeconn = *j;
    }
  }

  if (nodeconn) cache_del(nodeconn);

  SCDC_TRACE("lookup_cache: nodeconn: '" << nodeconn << "'");

  return nodeconn;
}


bool scdc_nodeconn_pool::cache_add(scdc_nodeconn *nodeconn)
{
  while (cache.size() > static_cast<cache_t::size_type>(max(cache_connections - 1, static_cast<scdcint_t>(0))))
  {
    close_real(cache.back());

    cache.pop_back();
  }

  if (static_cast<scdcint_t>(cache.size()) < cache_connections)
  {
    cache.push_front(nodeconn);
    return true;
  }

  return false;
}


void scdc_nodeconn_pool::cache_del(scdc_nodeconn *nodeconn)
{
  for (cache_t::iterator j = cache.begin(); j != cache.end(); ++j)
  {
    if (*j == nodeconn)
    {
      cache.erase(j);
      return;
    }
  }

  SCDC_FAIL("cache_del: nodeconn '" << nodeconn << "' not found");
}

#endif


#if SHARE_CONNECTIONS

scdc_nodeconn *scdc_nodeconn_pool::lookup_idle(const char *scheme, const char *authority)
{
  SCDC_TRACE("lookup_idle: scheme: '" << scheme << "', authority: '" << authority << "'");

  string key;
  join_uri(scheme, authority, 0, key);

  iterator i = find(key);

  scdc_nodeconn *nodeconn = 0;

  if (i != end())
  {
    for (mapped_type::iterator j = i->second.begin(); !nodeconn && j != i->second.end(); ++j)
    {
      if ((*j)->ref > 0 && (*j)->is_idle()) nodeconn = *j;
    }
  }

  SCDC_TRACE("lookup_idle: nodeconn: '" << nodeconn << "'");

  return nodeconn;
}

#endif


scdc_nodeconn *scdc_nodeconn_pool::open(const char *scheme, const char *authority, scdc_args *args)
{
  SCDC_TRACE("open: scheme: '" << scheme << "', authority: '" << authority << "'");

  if (string(authority) == "%s")
  {
    if (!args->get<const char *>(SCDC_ARGS_TYPE_CSTR, &authority))
    {
      SCDC_FAIL("open: failed to get authority parameter!");
      return 0;
    }
    SCDC_TRACE("open: authority: '" << authority << "'");
  }

  scdc_nodeconn *nodeconn = 0;

#if NODECONN_CACHE_CONNECTIONS
  if (!nodeconn) nodeconn = lookup_cache(scheme, authority);
#endif
#if NODECONN_SHARE_CONNECTIONS
  if (!nodeconn) nodeconn = lookup_idle(scheme, authority);
#endif

  if (nodeconn)
  {
    SCDC_TRACE("open: reusing existing connection '" << nodeconn << "'");

  } else
  {
    SCDC_TRACE("open: creating new connection");

    string s(scheme);

#if USE_NODE_DIRECT
    if (s == SCDC_NODE_DIRECT_SCHEME) nodeconn = new scdc_nodeconn_direct(nodeport_pool);
    else
#endif
#if USE_NODE_TCP
    if (s == SCDC_NODE_TCP_SCHEME) nodeconn = new scdc_nodeconn_tcp();
    else
#endif
#if USE_NODE_UDS
    if (s == SCDC_NODE_UDS_SCHEME) nodeconn = new scdc_nodeconn_uds();
    else
#endif
#if USE_NODE_MPI
    if (s == SCDC_NODE_MPI_SCHEME) nodeconn = new scdc_nodeconn_mpi();
    else
#endif
    {
      SCDC_ERROR("unknown scheme '" << s << "'");
      return 0;
    }

    if (!nodeconn->open(authority))
    {
      SCDC_FAIL("open: nodeconn open failed");
      delete nodeconn;
      return 0;
    }

    nodeconn->nodeconn_pool = this;

    add(scheme, authority, nodeconn);
  }

  ++nodeconn->ref;

  SCDC_TRACE("open: nodeconn: '" << nodeconn << "', ref: " << nodeconn->ref);

  return nodeconn;
}


void scdc_nodeconn_pool::close(scdc_nodeconn *nodeconn)
{
  SCDC_TRACE("close: nodeconn: '" << nodeconn << "'");

  bool is_idle = nodeconn->is_idle();

  if (is_idle)
  {
    SCDC_TRACE("close: closing idle connection");

    close_idle(nodeconn);

  } else
  {
    SCDC_TRACE("close: enable closing on idle");

    scdc_nodeconn_on_idle_close(nodeconn);
  }
}


void scdc_nodeconn_pool::close_idle(scdc_nodeconn *nodeconn)
{
  SCDC_TRACE("close_idle: nodeconn: '" << nodeconn << "', ref: " << nodeconn->ref);

  SCDC_ASSERT(nodeconn->ref > 0);

  --nodeconn->ref;

  if (nodeconn->ref <= 0)
  {
#if NODECONN_CACHE_CONNECTIONS
    if (cache_add(nodeconn))
    {
      SCDC_TRACE("close_idle: caching unreferenced connection '" << nodeconn << "'");

    } else
#endif
    {
      SCDC_TRACE("close_idle: closing unreferenced connection '" << nodeconn << "'");

      close_real(nodeconn);
    }

  } else
  {
    SCDC_TRACE("close_idle: keeping connection '" << nodeconn << "' with " << nodeconn->ref << " reference(s)");
  }
}


void scdc_nodeconn_pool::close_real(scdc_nodeconn *nodeconn)
{
  SCDC_TRACE("close_real: nodeconn: '" << nodeconn << "'");

  del(nodeconn);

  nodeconn->close();

  delete nodeconn;
}


void scdc_nodeconn_pool::close_all()
{
  SCDC_TRACE("close_all:");

  while (size() > 0)
  {
    close_real(*begin()->second.begin());
  }

#if NODECONN_CACHE_CONNECTIONS
  cache.clear();
#endif
}


#undef SCDC_LOG_PREFIX
