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


#ifndef __COMPCOUP_TRANSPORT_HH__
#define __COMPCOUP_TRANSPORT_HH__


#include "compcoup.hh"
#include "transport.hh"


class scdc_compcoup_transport: public scdc_compcoup
{
  public:
    scdc_compcoup_transport(scdc_transport *transport_);
    virtual ~scdc_compcoup_transport();

    virtual scdc_dataset *dataset_open(const char *path, scdcint_t path_size, scdc_dataset_output_t *output);
    virtual void dataset_close(scdc_dataset *dataset, scdc_dataset_output_t *output);
    virtual bool dataset_cmd(const char *cmd, scdcint_t cmd_size, scdc_dataset_input_t *input, scdc_dataset_output_t *output);

    bool dataset_cmd(scdc_dataset *dataset, const char *cmd, scdcint_t cmd_size, scdc_dataset_input_t *input, scdc_dataset_output_t *output);

    virtual bool start(scdcint_t mode);
    virtual bool stop();
    virtual bool cancel(bool interrupt);
/*    virtual bool resume();*/

    virtual bool connect(scdc_transport_connection_t *conn);
    virtual void disconnect(scdc_transport_connection_t *conn);

    void serve(scdc_transport_connection_t *conn);

    virtual void handshake();

    scdc_transport *get_transport() { return transport; }

    void set_transport_connection(scdc_transport_connection_t *transport_connection_) { transport_connection = transport_connection_; }

    void set_cmd_handler(scdc_nodeport_cmd_handler_f *cmd_handler_, void *cmd_handler_data_) { cmd_handler = cmd_handler_; cmd_handler_data = cmd_handler_data_; }

    static bool dataset_input_incoming_recv(scdc_transport_connection_t *conn, scdc_dataset_input_t *input, bool *next);
    static bool dataset_input_outgoing_send(scdc_transport_connection_t *conn, scdc_dataset_input_t *input);

    static bool dataset_output_incoming_recv(scdc_transport_connection_t *conn, scdc_dataset_output_t *output, bool *next);
    static bool dataset_output_outgoing_send(scdc_transport_connection_t *conn, scdc_dataset_output_t *output);

    static bool dataset_input_incoming_consume(scdc_transport_connection_t *conn, scdc_dataset_input_t *input, bool *next);
    static bool dataset_input_outgoing_produce(scdc_transport_connection_t *conn, scdc_dataset_input_t *input);

    static bool dataset_output_incoming_consume(scdc_transport_connection_t *conn, scdc_dataset_output_t *output, bool *next);
    static bool dataset_output_outgoing_produce(scdc_transport_connection_t *conn, scdc_dataset_output_t *output);

  private:
    scdc_transport *transport;
    scdc_transport_connection_t *transport_connection;

    scdc_nodeport_cmd_handler_f *cmd_handler;
    void *cmd_handler_data;

    void on_dataset_open(scdc_transport_connection_t *conn);
    void on_dataset_close(scdc_transport_connection_t *conn);
    void on_dataset_cmd(scdc_transport_connection_t *conn);

    bool on_cmd_handler(const char *cmd, scdc_data *incoming);
};


#endif /* __COMPCOUP_TRANSPORT_HH__ */
