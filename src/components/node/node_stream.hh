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


#ifndef __NODE_STREAM_HH__
#define __NODE_STREAM_HH__


#include "nodeport.hh"
#include "nodeconn.hh"


class scdc_nodeport_stream: public scdc_nodeport
{
  public:
    scdc_nodeport_stream();
    virtual ~scdc_nodeport_stream();

    virtual bool open_config_conf(const std::string &conf, scdc_args *args, bool &done);

/*    virtual bool open(const char *conf, scdc_args *args);
    virtual void close();*/

    virtual bool start(scdcint_t mode);
    virtual bool stop();
    virtual bool cancel(bool interrupt);
/*    virtual bool resume();*/

  private:
    scdc_transport *transport;
};


#if 0
class scdc_nodeconn_stream: public scdc_nodeconn
{
  public:
    scdc_nodeconn_stream();
    virtual ~scdc_nodeconn_stream();

    virtual bool open(const char *authority);
    virtual void close();

    virtual bool is_idle();
};
#endif


#endif /* __NODE_STREAM_HH__ */
