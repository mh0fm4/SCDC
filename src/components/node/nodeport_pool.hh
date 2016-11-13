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


#ifndef __NODEPORT_POOL_HH__
#define __NODEPORT_POOL_HH__


#include <set>
#include <string>

#include "config.hh"
#include "args.hh"
#include "nodeport.hh"


class scdc_nodeport_pool: public std::set<scdc_nodeport *>
{
  public:
    static bool init();
    static void release();

    static bool authority(const char *conf, scdc_args *args, std::string &auth);
    static bool supported(const char *uri, scdc_args *args);

    scdc_nodeport_pool() { };

    scdc_nodeport *open(const char *conf, scdc_args *args);
    void close(scdc_nodeport *nodeport);

    void close_all();
};


#endif /* __NODEPORT_POOL_HH__ */
