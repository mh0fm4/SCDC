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


#ifndef __SCDC_TRANSPORT_HH__
#define __SCDC_TRANSPORT_HH__


#include "data.hh"


class scdc_transport;

struct scdc_transport_connection_t;


struct scdc_transport_connection_t
{
  scdc_transport_connection_t(scdc_transport *transport_)
    :transport(transport_), open(false), idle(true),
    on_idle_handler(0), on_idle_data(0),
    total_send(0), total_receive(0),
    current_incoming_input_size(0), current_incoming_output_size(0), current_incoming_input_next(false), current_incoming_output_next(false),
    current_incoming_input_intern(false), current_incoming_output_intern(false)
/*    current_outgoing_input_autorelease(false), current_outgoing_output_autorelease(false)*/
    { }

  ~scdc_transport_connection_t()
  {
  }

  scdc_transport *transport;
  bool open, idle;

  scdc_handler_f *on_idle_handler;
  void *on_idle_data;

  scdcint_t total_send, total_receive;

  scdc_data incoming, outgoing;
  scdc_dataset_input_t incoming_input, outgoing_input;
  scdc_dataset_output_t incoming_output, outgoing_output;

  scdc_dataset_input_t current_incoming_input;
  scdc_dataset_output_t current_incoming_output;
  scdcint_t current_incoming_input_size, current_incoming_output_size;
  bool current_incoming_input_next, current_incoming_output_next;

  bool current_incoming_input_intern, current_incoming_output_intern;
/*  bool current_incoming_input_autorelease, current_incoming_output_autorelease;*/

  scdc_data *get_incoming() { return &incoming; }
  scdc_data *get_outgoing() { return &outgoing; }

  bool acquire();
  void release();
  bool is_idle();
};


struct scdc_transport_connection_pool_entry_t;

struct scdc_transport_connection_pool_t;

class scdc_transport_connection_pool
{
  public:
    scdc_transport_connection_pool(scdcint_t max_connections, scdc_transport *transport_);
    ~scdc_transport_connection_pool();

    void insert(scdc_transport_connection_t *conn);
    void finish();

    scdc_transport_connection_pool_entry_t *create_entry(bool async);
    void exit_entry(scdc_transport_connection_pool_entry_t *entry);
    void destroy_entry(scdc_transport_connection_pool_entry_t *entry);

    scdc_transport_connection_pool_entry_t *acquire_entry(bool async);
    void release_entry(scdc_transport_connection_pool_entry_t *entry);

    void run_entry(scdc_transport_connection_pool_entry_t *entry);

    void execute(scdc_transport_connection_t *conn);
    void shutdown(bool interrupt);

  private:
    const scdcint_t max_connections;
    scdc_transport *transport;
    scdc_transport_connection_pool_t *pool;
};


struct scdc_transport_server_t;

class scdc_compcoup_transport;

class scdc_transport
{
  public:
    scdc_transport();
    virtual ~scdc_transport();

    virtual bool open();
    virtual void close();

    virtual bool start(scdcint_t mode);
    virtual bool stop();
    virtual bool cancel(bool interrupt);
/*    virtual bool resume();*/

    virtual void run();
    virtual scdc_transport_connection_t *accept() = 0;
    virtual scdc_transport_connection_t *accept(scdc_transport_connection_t *conn);
    virtual void shutdown(scdc_transport_connection_t *conn, bool interrupt);
    virtual void serve(scdc_transport_connection_t *conn);
    virtual void close(scdc_transport_connection_t *conn);

    virtual scdc_transport_connection_t *connect(scdc_transport_connection_t *conn);
    virtual bool disconnect(scdc_transport_connection_t *conn);

    virtual scdcint_t send(scdc_transport_connection_t *conn, const void *buf, scdcint_t buf_size) = 0;
    virtual scdcint_t receive(scdc_transport_connection_t *conn, void *buf, scdcint_t max_buf_size) = 0;

    void set_compcoup_transport(scdc_compcoup_transport *compcoup_transport_) { compcoup_transport = compcoup_transport_; }

    void set_loop_handler(scdc_nodeport_loop_handler_f *loop_handler_, void *loop_handler_data_) { loop_handler = loop_handler_; loop_handler_data = loop_handler_data_; }

    void set_max_connections(scdcint_t max_connections_) { max_connections = max_connections_; }

    virtual scdc_dataset_input_t *acquire_incoming_dataset_input(scdc_transport_connection_t *conn);
    virtual void release_incoming_dataset_input(scdc_transport_connection_t *conn);

    virtual scdc_dataset_output_t *acquire_outgoing_dataset_output(scdc_transport_connection_t *conn);
    virtual void release_outgoing_dataset_output(scdc_transport_connection_t *conn);

    virtual bool recv_incoming_dataset_input(scdc_transport_connection_t *conn, scdc_dataset_input_t *input, scdcint_t max_recv, bool &cont_recv);
    virtual bool recv_incoming_dataset_output(scdc_transport_connection_t *conn, scdc_dataset_output_t *output, scdcint_t max_recv, bool &cont_recv);

    virtual bool send_outgoing_dataset_input(scdc_transport_connection_t *conn, scdc_dataset_input_t *input);
    virtual bool send_outgoing_dataset_output(scdc_transport_connection_t *conn, scdc_dataset_output_t *output);

    virtual bool recv_incoming(scdc_transport_connection_t *conn);
    virtual bool send_outgoing(scdc_transport_connection_t *conn);

  protected:
    scdc_compcoup_transport *compcoup_transport;
    scdc_nodeport_loop_handler_f *loop_handler;
    void *loop_handler_data;
    scdcint_t max_connections;
    bool async, running, shutdowned, canceled;
    scdc_transport_server_t *server;
    scdc_transport_connection_pool *server_connections;

    virtual void create_incoming(scdc_transport_connection_t *conn, scdcint_t size = -1);
    virtual void destroy_incoming(scdc_transport_connection_t *conn);
    virtual void create_outgoing(scdc_transport_connection_t *conn, scdcint_t size = -1);
    virtual void destroy_outgoing(scdc_transport_connection_t *conn);
};


#endif /* __SCDC_TRANSPORT_HH__ */
