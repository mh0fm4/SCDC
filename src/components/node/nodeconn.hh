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


#ifndef __NODECONN_HH__
#define __NODECONN_HH__


#include <string>

#include "compcoup.hh"


class scdc_nodeconn_pool;

class scdc_nodeconn
{
  public:
    scdc_nodeconn(const char *type_);
    virtual ~scdc_nodeconn();

    virtual bool open(const char *authority);
    virtual void close();

    virtual bool is_idle() = 0;

    std::string get_type() { return type; }
    scdc_compcoup *get_compcoup() { return compcoup; }

  public:
    std::string key;
    scdcint_t ref, close_on_idle;
    scdc_nodeconn_pool *nodeconn_pool;

  protected:
    const std::string type;
    scdc_compcoup *compcoup;
};


void scdc_nodeconn_on_idle_close(scdc_nodeconn *nodeconn);
scdcint_t scdc_nodeconn_on_idle_close_handler(void *data);


#endif /* __NODECONN_HH__ */
