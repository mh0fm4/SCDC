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


#ifndef __SCDC_TRANSPORT_UDS_HH__
#define __SCDC_TRANSPORT_UDS_HH__


#include "transport.hh"


#define SCDC_TRANSPORT_UDS_USE_SHUTDOWN_EXCEPTION  0


struct scdc_transport_uds_server_t;


class scdc_transport_uds: public scdc_transport
{
  public:
    scdc_transport_uds();
    ~scdc_transport_uds();

    virtual bool open(const char *socketfile);
    virtual void close();

/*    virtual bool start(scdcint_t mode);*/
/*    virtual bool stop();*/
/*    virtual bool cancel(bool interrupt);*/
/*    virtual bool resume();*/

    virtual scdc_transport_connection_t *accept();
    virtual void shutdown(scdc_transport_connection_t *conn, bool interrupt);
    virtual void serve(scdc_transport_connection_t *conn);
    virtual void close(scdc_transport_connection_t *conn);

    virtual scdc_transport_connection_t *connect(const char *socketfile);
    virtual bool disconnect(scdc_transport_connection_t *conn);

    virtual scdcint_t send(scdc_transport_connection_t *conn, const void *buf, scdcint_t buf_size);
    virtual scdcint_t receive(scdc_transport_connection_t *conn, void *buf, scdcint_t max_buf_size);

  private:
    scdc_transport_uds_server_t *uds_server;
};


#endif /* __SCDC_TRANSPORT_UDS_HH__ */