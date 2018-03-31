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


#ifndef __NODE_TCP_HH__
#define __NODE_TCP_HH__


#include "transport.hh"
#include "nodeport.hh"
#include "nodeconn.hh"


#define SCDC_NODE_TCP_SCHEME  "scdc+tcp"


class scdc_nodeport_tcp: public scdc_nodeport
{
  public:
    static bool authority(const char *conf, scdc_args *args, std::string &auth);
    static bool supported(const char *uri, scdc_args *args);

    scdc_nodeport_tcp();
    virtual ~scdc_nodeport_tcp();

    virtual bool open_config_conf(const std::string &conf, scdc_args *args, bool &done);

    virtual bool open(const char *conf, scdc_args *args);
    virtual void close();

    virtual bool start(scdcint_t mode);
    virtual bool stop();
    virtual bool cancel(bool interrupt);
/*    virtual bool resume();*/

  private:
    scdc_transport *transport;
    std::string address;
    int port;
};


class scdc_nodeconn_tcp: public scdc_nodeconn
{
  public:
    scdc_nodeconn_tcp();
    virtual ~scdc_nodeconn_tcp();

    virtual bool open(const char *authority);
    virtual void close();

    virtual bool is_idle();

  private:
    scdc_transport *transport;
    scdc_transport_connection_t *transport_connection;
};


#endif /* __NODE_TCP_HH__ */
