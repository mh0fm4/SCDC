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


#ifndef __NODECONN_POOL_HH__
#define __NODECONN_POOL_HH__


#include <string>
#include <map>
#include <set>
#include <list>

#include "nodeport_pool.hh"
#include "nodeconn.hh"


#define NODECONN_CACHE_CONNECTIONS  0
#define NODECONN_SHARE_CONNECTIONS  0


class scdc_nodeconn_pool: public std::map< std::string, std::set<scdc_nodeconn *> >
{
  public:
    static bool init() { return true; }
    static void release() { }

    scdc_nodeconn_pool()
      :nodeport_pool(0)
#if NODECONN_CACHE_CONNECTIONS
      , cache_connections(NODECONN_CACHE_CONNECTIONS)
#endif
      { };

    void add(const char *scheme, const char *authority, scdc_nodeconn *nodeconn);
    void del(scdc_nodeconn *nodeconn);
#if NODECONN_CACHE_CONNECTIONS
    scdc_nodeconn *lookup_cache(const char *scheme, const char *authority);
    bool cache_add(scdc_nodeconn *nodeconn);
    void cache_del(scdc_nodeconn *nodeconn);
#endif
#if NODECONN_SHARE_CONNECTIONS
    scdc_nodeconn *lookup_idle(const char *scheme, const char *authority);
#endif

    scdc_nodeconn *open(const char *scheme, const char *authority, scdc_args *args);
    void close(scdc_nodeconn *nodeconn);
    void close_idle(scdc_nodeconn *nodeconn);
    void close_real(scdc_nodeconn *nodeconn);

    void close_all();

    void set_nodeport_pool(scdc_nodeport_pool *nodeport_pool_) { nodeport_pool = nodeport_pool_; }

  private:
    scdc_nodeport_pool *nodeport_pool;
#if NODECONN_CACHE_CONNECTIONS
    typedef std::list<scdc_nodeconn *> cache_t;
    cache_t cache;
#endif

  public:
#if NODECONN_CACHE_CONNECTIONS
    scdcint_t cache_connections;
#endif
};


#endif /* __NODECONN_POOL_HH__ */
