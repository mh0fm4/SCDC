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


#include <pthread.h>
#include <cstring>
#include <iostream>
#include <list>
#include <algorithm>

#define SCDC_TRACE_NOT  !SCDC_TRACE_TRANSPORT

#include "config.hh"
#include "log.hh"
#include "dataset_inout.h"
#include "transport.hh"
#include "compcoup_transport.hh"


using namespace std;


#define THREAD_POOLING  1


#define SCDC_LOG_PREFIX  "transport-connection: "


bool scdc_transport_connection_t::acquire()
{
  SCDC_TRACE("acquire: idle: '" << idle << "'");

  if (!idle) return false;

  idle = false;

  return true;
}


void scdc_transport_connection_t::release()
{
  SCDC_TRACE("release: idle: '" << idle << "'");

  SCDC_ASSERT(!idle);

  idle = true;
  
  if (on_idle_handler) on_idle_handler(on_idle_data);
}


bool scdc_transport_connection_t::is_idle()
{
#if 0
/*  SCDC_TRACE("is_idle: input next/size: " << current_incoming_input_next << " / " << current_incoming_input_size << ", output next/size: " << current_incoming_output_next << " / " << current_incoming_output_size);*/

  return !(current_incoming_input_next || current_incoming_input_size > 0 || current_incoming_output_next || current_incoming_output_size > 0);
#else
  SCDC_TRACE("is_idle: idle: '" << idle << "'");

  return idle;
#endif
}


#undef SCDC_LOG_PREFIX

#define SCDC_LOG_PREFIX  "transport-connection-pool: "


struct scdc_transport_connection_pool_entry_t
{
  scdc_transport_connection_pool *pool;

  pthread_mutex_t lock;
  pthread_cond_t cond;
  pthread_t thread;

  bool async, run, wait;
  scdc_transport_connection_t *conn;
};


struct scdc_transport_connection_pool_t
{
  pthread_mutex_t lock;
  pthread_cond_t cond;
  bool run;

  std::list<scdc_transport_connection_t *> conns;
  scdcint_t naquired_entries;
  std::list<scdc_transport_connection_pool_entry_t *> total_entries,
#if THREAD_POOLING
    free_entries
#else
    dead_entries
#endif
    ;
};


static void *entry_thread_run(void *arg)
{
  scdc_transport_connection_pool_entry_t *entry = static_cast<scdc_transport_connection_pool_entry_t *>(arg);

  entry->pool->run_entry(entry);

  return NULL;
}



scdc_transport_connection_pool::scdc_transport_connection_pool(scdcint_t max_connections_, scdc_transport *transport_)
  :max_connections(max_connections_), transport(transport_)
{
#if THREAD_POOLING
  SCDC_TRACE("scdc_transport_connection_pool: max. " << max_connections << " connections, with thread pooling");
#else
  SCDC_TRACE("scdc_transport_connection_pool: max. " << max_connections << " connections, without thread pooling");
#endif

  pool = new scdc_transport_connection_pool_t();

  pool->run = true;
  pool->naquired_entries = 0;

  pthread_mutex_init(&pool->lock, NULL);
  pthread_cond_init(&pool->cond, NULL);
}


scdc_transport_connection_pool::~scdc_transport_connection_pool()
{
  pthread_cond_destroy(&pool->cond);
  pthread_mutex_destroy(&pool->lock);

  delete pool; pool = 0;

  transport = 0;
}


void scdc_transport_connection_pool::insert(scdc_transport_connection_t *conn)
{
  SCDC_TRACE("insert: conn: '" << conn << "'");

  bool async = (max_connections > 1);

  scdc_transport_connection_pool_entry_t *entry = acquire_entry(async);

  if (!entry)
  {
    SCDC_TRACE("insert: aquire entry failed: closing connection '" << conn << "'");
    transport->close(conn);
    return;
  }

  if (async)
  {
    SCDC_TRACE("insert: run async in thread");

    pthread_mutex_lock(&entry->lock);
    entry->conn = conn;
    pthread_mutex_unlock(&entry->lock);

    pthread_cond_signal(&entry->cond);

  } else
  {
    SCDC_TRACE("insert: run direct in function");

    entry->conn = conn;

    run_entry(entry);
  }
}


void scdc_transport_connection_pool::finish()
{
  SCDC_TRACE("finish:");

  pthread_mutex_lock(&pool->lock);

  /* wait until all entries have been released */
  while (pool->naquired_entries > 0) pthread_cond_wait(&pool->cond, &pool->lock);

#if THREAD_POOLING
  while (pool->free_entries.size() > 0)
  {
    scdc_transport_connection_pool_entry_t *entry = pool->free_entries.front();
    pool->free_entries.pop_front();
    exit_entry(entry);
    destroy_entry(entry);
  }
#else
  while (pool->dead_entries.size() > 0)
  {
    scdc_transport_connection_pool_entry_t *entry = pool->dead_entries.front();
    pool->dead_entries.pop_front();
    destroy_entry(entry);
  }
#endif

  pthread_mutex_unlock(&pool->lock);
}


scdc_transport_connection_pool_entry_t *scdc_transport_connection_pool::create_entry(bool async)
{
  SCDC_TRACE("create_entry: async: '" << async << "'");

  scdc_transport_connection_pool_entry_t *entry = new scdc_transport_connection_pool_entry_t();

  entry->pool = this;

  pthread_mutex_init(&entry->lock, NULL);
  pthread_cond_init(&entry->cond, NULL);

  entry->async = async;
  entry->run = async;
  entry->wait = false;
  entry->conn = 0;

  if (entry->async)
  {
    SCDC_TRACE("acquire_entry: creating thread");
    pthread_create(&entry->thread, NULL, entry_thread_run, entry);
    SCDC_TRACE("acquire_entry: id: '" << entry->thread << "'");
  }

  return entry;
}


void scdc_transport_connection_pool::exit_entry(scdc_transport_connection_pool_entry_t *entry)
{
  SCDC_TRACE("exit_entry: entry: '" << entry << "'");

  pthread_mutex_lock(&entry->lock);
  entry->run = false;
  pthread_mutex_unlock(&entry->lock);

  pthread_cond_signal(&entry->cond);
}


void scdc_transport_connection_pool::destroy_entry(scdc_transport_connection_pool_entry_t *entry)
{
  SCDC_TRACE("destroy_entry: entry: '" << entry << "'");

  if (entry->async)
  {
    SCDC_TRACE("destroy_entry: joining thread with id: '" << entry->thread << "'");
    pthread_join(entry->thread, NULL);
  }

  pthread_cond_destroy(&entry->cond);
  pthread_mutex_destroy(&entry->lock);

  delete entry;
}


scdc_transport_connection_pool_entry_t *scdc_transport_connection_pool::acquire_entry(bool async)
{
  SCDC_TRACE("acquire_entry:");

  pthread_mutex_lock(&pool->lock);

  while (pool->run && pool->total_entries.size() >= static_cast<unsigned>(max_connections)
#if THREAD_POOLING
    && pool->free_entries.size() <= 0
#endif
    ) pthread_cond_wait(&pool->cond, &pool->lock);

  if (!pool->run)
  {
    SCDC_TRACE("acquire_entry: stopped running");
    pthread_mutex_unlock(&pool->lock);
    return 0;
  }

#if !THREAD_POOLING
  /* cleanup dead entries */
  while (pool->dead_entries.size() > 0)
  {
    scdc_transport_connection_pool_entry_t *entry = pool->dead_entries.front();
    pool->dead_entries.pop_front();
    destroy_entry(entry);
  }
#endif

  scdc_transport_connection_pool_entry_t *entry;

#if THREAD_POOLING
  if (pool->free_entries.size() > 0)
  {
    entry = pool->free_entries.front();
    pool->free_entries.pop_front();

  } else
#endif
  {
    entry = create_entry(async);
    pool->total_entries.push_back(entry);
  }

  ++pool->naquired_entries;

  pthread_mutex_unlock(&pool->lock);

  SCDC_TRACE("acquire_entry: return: '" << entry << "'");

  return entry;
}


void scdc_transport_connection_pool::release_entry(scdc_transport_connection_pool_entry_t *entry)
{
  SCDC_TRACE("release_entry: entry: '" << entry << "'");

  pthread_mutex_lock(&pool->lock);

#if THREAD_POOLING
  pool->free_entries.push_back(entry);
#else
  pool->total_entries.erase(std::find(pool->total_entries.begin(), pool->total_entries.end(), entry));
  exit_entry(entry);
  pool->dead_entries.push_back(entry);
#endif

  --pool->naquired_entries;

  pthread_mutex_unlock(&pool->lock);

  pthread_cond_signal(&pool->cond);
}


void scdc_transport_connection_pool::run_entry(scdc_transport_connection_pool_entry_t *entry)
{
  SCDC_TRACE("run_entry: entry: '" << entry << "'");

  pthread_mutex_lock(&entry->lock);

  do
  {
    if (entry->conn)
    {
      pthread_mutex_unlock(&entry->lock);

      execute(entry->conn);

/*      pthread_mutex_lock(&entry->lock);*/
      entry->conn = 0;
/*      pthread_mutex_unlock(&entry->lock);*/

      release_entry(entry);

#if THREAD_POOLING
      pthread_mutex_lock(&entry->lock);
#else
      SCDC_TRACE("run_entry: return A");
      return;
#endif

    } else
    {
      entry->wait = true;
      pthread_cond_wait(&entry->cond, &entry->lock);
      entry->wait = false;
    }

  } while (entry->run);

  pthread_mutex_unlock(&entry->lock);

  SCDC_TRACE("run_entry: return B");
}


void scdc_transport_connection_pool::execute(scdc_transport_connection_t *conn)
{
  SCDC_TRACE("execute: conn: '" << conn << "'");

  SCDC_TRACE("connection: " << conn << ": execute: serve");

  transport->serve(conn);

  SCDC_TRACE("connection: " << conn << ": execute: close");

  transport->close(conn);
}


void scdc_transport_connection_pool::shutdown(bool interrupt)
{
  SCDC_TRACE("shutdown:");

  pthread_mutex_lock(&pool->lock);

  for (std::list<scdc_transport_connection_pool_entry_t *>::iterator i = pool->total_entries.begin(); i != pool->total_entries.end(); ++i)
  {
    scdc_transport_connection_pool_entry_t *entry = *i;

    pthread_mutex_lock(&entry->lock);
    
    if (entry->conn) transport->shutdown(entry->conn, interrupt);

    pthread_mutex_unlock(&entry->lock);
  }

  pool->run = false;

  pthread_mutex_unlock(&pool->lock);

  pthread_cond_signal(&pool->cond);
}


#undef SCDC_LOG_PREFIX


#define SCDC_LOG_PREFIX  "transport: "


struct scdc_transport_server_t
{
  pthread_t thread;
};


scdc_transport::scdc_transport()
  :compcoup_transport(0), loop_handler(0), loop_handler_data(0), max_connections(1), async(false), running(false), shutdowned(false), canceled(true), server(0), server_connections(0)
{
}


scdc_transport::~scdc_transport()
{
  compcoup_transport = 0;
}


bool scdc_transport::open()
{
  SCDC_TRACE("open:");

  return true;
}


void scdc_transport::close()
{
  SCDC_TRACE("close:");
}


static void *server_run(void *arg)
{
  scdc_transport *transport = static_cast<scdc_transport *>(arg);

  transport->run();

  pthread_exit(NULL);
}


bool scdc_transport::start(scdcint_t mode)
{
  SCDC_TRACE("start: running: '" << running << "'");

  if (running)
  {
    SCDC_FAIL("start: already running");
    return false;
  }

  server = new scdc_transport_server_t();

  /* use connection pool */
  if (max_connections > 0) server_connections = new scdc_transport_connection_pool(max_connections, this);

  compcoup_transport->start(mode);

  if (mode == SCDC_NODEPORT_START_LOOP_UNTIL_CANCEL)
  {
    async = false;
    running = true;
    shutdowned = false;
    canceled = false;
    run();

  } else if (mode == SCDC_NODEPORT_START_ASYNC_UNTIL_CANCEL)
  {
    async = true;
    running = true;
    shutdowned = false;
    canceled = false;
    pthread_create(&server->thread, NULL, server_run, this);

  } else
  {
    SCDC_ERROR("mode '" << mode << "' not supported");
    return false;
  }

  return true;
}


bool scdc_transport::stop()
{
  SCDC_TRACE("stop: running: '" << running << "', canceled: '" << canceled << "'");

  if (!running && !canceled)
  {
    SCDC_FAIL("stop: not running");
    return false;
  }

  if (!canceled) cancel(true);

  if (async)
  {
    pthread_join(server->thread, NULL);
    async = false;
  }

  /* wait for all connections of the connection pool to be finished */
  if (server_connections) server_connections->finish();

  compcoup_transport->stop();

  delete server_connections; server_connections = 0;
  delete server; server = 0;

  return true;
}


bool scdc_transport::cancel(bool interrupt)
{
  SCDC_TRACE("cancel: interrupt: '" << interrupt << "', running: '" << running << "'");

  if (!running)
  {
    SCDC_FAIL("cancel: not running");
    return false;
  }

  compcoup_transport->cancel(interrupt);

  running = false;
  canceled = true;

  /* shutdown (potential) server */
  shutdown(0, interrupt);

  /* shutdown all connections of the connection pool */
  if (server_connections) server_connections->shutdown(interrupt);

  return true;
}


/*bool scdc_transport::resume()
{
  return compcoup_transport->resume();
}*/


void scdc_transport::run()
{
  SCDC_TRACE("run:");

  scdcint_t i = 0;

  while (running)
  {
    SCDC_TRACE("run: loop: " << i << ": accept");

    scdc_transport_connection_t *conn = accept();

    SCDC_TRACE("run: loop: " << i << ": accept: '" << conn << "'");

    if (conn)
    {
      if (server_connections) server_connections->insert(conn);
      else
      {
        serve(conn);
        close(conn);
      }
    }

    if (loop_handler) loop_handler(loop_handler_data, i);

    ++i;
  }
}


scdc_transport_connection_t *scdc_transport::accept(scdc_transport_connection_t *conn)
{
  SCDC_TRACE("accept: conn: '" << conn << "'");

  create_incoming(conn);
  create_outgoing(conn);

  return conn;
}


void scdc_transport::shutdown(scdc_transport_connection_t *conn, bool interrupt)
{
  SCDC_TRACE("shutdown: conn: '" << conn << "', interrupt: " << interrupt);

  if (!conn) shutdowned = true;
}


void scdc_transport::serve(scdc_transport_connection_t *conn)
{
  SCDC_TRACE("serve: conn: '" << conn << "'");

  compcoup_transport->serve(conn);
}


void scdc_transport::close(scdc_transport_connection_t *conn)
{
  SCDC_TRACE("close: conn: '" << conn << "'");

  destroy_incoming(conn);
  destroy_outgoing(conn);

  SCDC_TRACE("close: conn: '" << conn << "', total_send: " << conn->total_send << ", total_receive: " << conn->total_receive);
}


scdc_transport_connection_t *scdc_transport::connect(scdc_transport_connection_t *conn)
{
  SCDC_TRACE("connect:");

  create_incoming(conn);
  create_outgoing(conn);

  compcoup_transport->connect(conn);

  conn->open = true;

  return conn;
}


bool scdc_transport::disconnect(scdc_transport_connection_t *conn)
{
  SCDC_TRACE("disconnect:");

  conn->open = false;

  compcoup_transport->disconnect(conn);

  destroy_incoming(conn);
  destroy_outgoing(conn);

  SCDC_TRACE("close: conn: '" << conn << "', total_send: " << conn->total_send << ", total_receive: " << conn->total_receive);

  return true;
}


scdc_dataset_input_t *scdc_transport::acquire_incoming_dataset_input(scdc_transport_connection_t *conn)
{
  scdc_dataset_input_create(&conn->incoming_input, "alloc", DEFAULT_TRANSPORT_ALLOC_INPUT_SIZE);

  return &conn->incoming_input;
}


void scdc_transport::release_incoming_dataset_input(scdc_transport_connection_t *conn)
{
  scdc_dataset_input_destroy(&conn->incoming_input);
}


scdc_dataset_output_t *scdc_transport::acquire_outgoing_dataset_output(scdc_transport_connection_t *conn)
{
  scdc_dataset_output_create(&conn->outgoing_output, "alloc", DEFAULT_TRANSPORT_ALLOC_OUTPUT_SIZE);

  return &conn->outgoing_output;
}


void scdc_transport::release_outgoing_dataset_output(scdc_transport_connection_t *conn)
{
  scdc_dataset_output_destroy(&conn->outgoing_output);
}


static bool incoming_dataset_inout_has_buf(scdc_transport_connection_t *conn, scdc_dataset_inout_t *inout)
{
  return (SCDC_DATASET_INOUT_BUF_PTR(inout) && SCDC_DATASET_INOUT_BUF_SIZE(inout) > 0 && !conn->incoming.ptr_inside(SCDC_DATASET_INOUT_BUF_PTR(inout)));
}


static bool recv_incoming_dataset_inout_extern(scdc_transport_connection_t *conn, scdc_dataset_inout_t *inout, scdcint_t max_recv, bool &cont_recv)
{
  bool ret = true;

  char *dst = static_cast<char *>(SCDC_DATASET_INOUT_BUF_PTR(inout));
  scdcint_t dst_size = SCDC_DATASET_INOUT_BUF_SIZE(inout);

  dst += SCDC_DATASET_INOUT_BUF_CURRENT(inout);
  dst_size -= SCDC_DATASET_INOUT_BUF_CURRENT(inout);

  if (max_recv > 0)
  {
    /* 1st: copy from incoming to inout */
    const char *src = conn->incoming.get_read_pos_buf();
    scdcint_t src_size = conn->incoming.get_read_pos_buf_size();

    scdcint_t n = min(src_size, min(dst_size, max_recv));

    memcpy(dst, src, n);

    conn->incoming.inc_read_pos(n);

    dst += n;
    dst_size -= n;

    max_recv -= n;
    SCDC_DATASET_INOUT_BUF_CURRENT(inout) += n;
  }

  if (max_recv > 0)
  {
    /* 2nd: receive in inout */

    scdcint_t n;

    while (1)
    {
      n = min(dst_size, max_recv);

      if (n <= 0) break;

      n = conn->transport->receive(conn, dst, n);

/*      SCDC_INFO("conn->transport->receive: " << n);*/

      if (n < 0) break;

      conn->total_receive += n;

      dst += n;
      dst_size -= n;

      max_recv -= n;
      SCDC_DATASET_INOUT_BUF_CURRENT(inout) += n;
    }

    ret = ret && (n >= 0);
  }

  cont_recv = true;

  return ret;
}


static bool recv_incoming_dataset_inout_intern(scdc_transport_connection_t *conn, scdc_dataset_inout_t *inout, scdcint_t max_recv, bool &cont_recv, scdc_transport *transport)
{
  bool ret = true;

  if (SCDC_DATASET_INOUT_BUF_CURRENT(inout) != 0)
  {
    SCDC_FATAL("recv_incoming_dataset_inout_intern: non-empty inout during internal receive through incoming buffer");
    return false;
  }

  if (max_recv > 0)
  {
    if (conn->incoming.get_read_pos_buf_size() <= 0) transport->recv_incoming(conn);

    scdcint_t n = min(conn->incoming.get_read_pos_buf_size(), max_recv);

    SCDC_DATASET_INOUT_BUF_PTR(inout) = conn->incoming.get_read_pos_buf();
    SCDC_DATASET_INOUT_BUF_SIZE(inout) = n;
    SCDC_DATASET_INOUT_BUF_CURRENT(inout) = n;

    conn->incoming.inc_read_pos(n);

  } else
  {
    SCDC_DATASET_INOUT_BUF_PTR(inout) = 0;
    SCDC_DATASET_INOUT_BUF_SIZE(inout) = 0;
    SCDC_DATASET_INOUT_BUF_CURRENT(inout) = 0;
  }

  cont_recv = false;

  return ret;
}


static bool recv_incoming_dataset_inout(scdc_transport_connection_t *conn, scdc_dataset_inout_t *inout, scdcint_t max_recv, bool &cont_recv, scdc_transport *transport, bool &inout_intern)
{
  inout_intern = !incoming_dataset_inout_has_buf(conn, inout);

  if (inout_intern)
  {
    bool ret = recv_incoming_dataset_inout_intern(conn, inout, max_recv, cont_recv, transport);

    if (!SCDC_DATASET_INOUT_BUF_PTR(inout)) inout_intern = false;

    return ret;
  }

  return recv_incoming_dataset_inout_extern(conn, inout, max_recv, cont_recv);
}


bool scdc_transport::recv_incoming_dataset_input(scdc_transport_connection_t *conn, scdc_dataset_input_t *input, scdcint_t max_recv, bool &cont_recv)
{
  return recv_incoming_dataset_inout(conn, input, max_recv, cont_recv, this, conn->current_incoming_input_intern);
}


bool scdc_transport::recv_incoming_dataset_output(scdc_transport_connection_t *conn, scdc_dataset_output_t *output, scdcint_t max_recv, bool &cont_recv)
{
  return recv_incoming_dataset_inout(conn, output, max_recv, cont_recv, this, conn->current_incoming_output_intern);
}


static bool send_outgoing_buf(scdc_transport_connection_t *conn, scdc_buf_t *buf)
{
  bool ret = true;

  scdcint_t mx = buf->current;
  buf->current = 0;

  const char *src = static_cast<char *>(buf->ptr);
  scdcint_t src_size = buf->size;

  if (mx > 0)
  {
    /* 1st: copy from inout to outgoing */
    char *dst = conn->outgoing.get_write_pos_buf();
    scdcint_t dst_size = conn->outgoing.get_write_pos_buf_size();

    scdcint_t n = min(src_size, mx);

    /* only short messages that fit into outgoing */
    if (n <= DEFAULT_TRANSPORT_SEND_OUTGOING_DATASET_INOUT_THRESHOLD && n <= dst_size)
    {
      memcpy(dst, src, n);

      conn->outgoing.inc_write_pos(n);

      src += n;
      src_size -= n;

      mx -= n;
      buf->current += n;
    }
  }

  if (mx > 0)
  {
    /* send outgoing before sending inout directly */
    ret = ret && conn->transport->send_outgoing(conn);

    /* 2nd: send from inout */

    scdcint_t n;

    while (1)
    {
      n = min(src_size, mx);

      if (n <= 0) break;

      n = conn->transport->send(conn, src, n);

/*      SCDC_INFO("conn->transport->send: " << n);*/

      if (n < 0) break;

      conn->total_send += n;

      src += n;
      src_size -= n;

      mx -= n;
      buf->current += n;
    }

    ret = ret && (n >= 0);
  }

  return ret;
}


static bool send_outgoing_dataset_inout(scdc_transport_connection_t *conn, scdc_dataset_input_t *inout)
{
#if 0
  return send_outgoing_dataset_inout(conn, input);
#else

  bool ret = true;

#if SCDC_DATASET_INOUT_BUF_MULTIPLE
  if (SCDC_DATASET_INOUT_MBUF_ISSET(inout))
  {
    scdcint_t i;
    for (i = 0; i < SCDC_DATASET_INOUT_MBUF_GET_C(inout); ++i)
    {
      ret = ret && send_outgoing_buf(conn, SCDC_DATASET_INOUT_MBUF_M(inout, i));
    }

  } else
#endif
  {
    scdc_buf_t *inout_buf;

#if SCDC_DEPRECATED
    scdc_buf_t buf;

    buf.ptr = SCDC_DATASET_INOUT_BUF_PTR(inout);
    buf.size = SCDC_DATASET_INOUT_BUF_SIZE(inout);
    buf.current = SCDC_DATASET_INOUT_BUF_CURRENT(inout);

    inout_buf = &buf;
#else /* SCDC_DEPRECATED */
    inout_buf = &inout->buf;
#endif /* SCDC_DEPRECATED */

    ret = send_outgoing_buf(conn, inout_buf);

#if SCDC_DEPRECATED
    SCDC_DATASET_INOUT_BUF_CURRENT(inout) = buf.current;
#endif /* SCDC_DEPRECATED */
  }

  return ret;

#endif
}

bool scdc_transport::send_outgoing_dataset_input(scdc_transport_connection_t *conn, scdc_dataset_input_t *input)
{
  return send_outgoing_dataset_inout(conn, input);
}


bool scdc_transport::send_outgoing_dataset_output(scdc_transport_connection_t *conn, scdc_dataset_output_t *output)
{
  return send_outgoing_dataset_inout(conn, output);
}


bool scdc_transport::send_outgoing(scdc_transport_connection_t *conn)
{
  SCDC_TRACE("send_outgoing:");

  scdcint_t n = conn->outgoing.get_read_pos_buf_size();
  
  if (n > 0) n = send(conn, conn->outgoing.get_read_pos_buf(), n);

/*  SCDC_INFO("send: " << n);*/

  if (n < 0) return false;

  conn->total_send += n;

  conn->outgoing.inc_read_pos(n);

  conn->outgoing.compact();

  return true;
}


bool scdc_transport::recv_incoming(scdc_transport_connection_t *conn)
{
  SCDC_TRACE("recv_incoming:");

  conn->incoming.compact();

  scdcint_t n = conn->incoming.get_write_pos_buf_size();

  if (n > 0) n = receive(conn, conn->incoming.get_write_pos_buf(), n);

/*  SCDC_INFO("receive: " << n);*/

  if (n < 0) return false;

  conn->total_receive += n;

  conn->incoming.inc_write_pos(n);

  return true;
}


static bool scdc_transport_incoming_next(scdc_data *data)
{
  SCDC_TRACE("scdc_transport_incoming_next:");

  scdc_transport_connection_t *conn = static_cast<scdc_transport_connection_t *>(data->get_data());

  return conn->transport->recv_incoming(conn);
}


void scdc_transport::create_incoming(scdc_transport_connection_t *conn, scdcint_t size)
{
  if (size < 0) size = DEFAULT_TRANSPORT_ALLOC_INCOMING_SIZE;
  conn->incoming.alloc_buf(size);

  conn->incoming.set_data(conn);
  conn->incoming.set_next(scdc_transport_incoming_next);
}


void scdc_transport::destroy_incoming(scdc_transport_connection_t *conn)
{
  conn->incoming.set_next(0);
  conn->incoming.set_data(0);

  conn->incoming.free_buf();
}


static bool scdc_transport_outgoing_next(scdc_data *data)
{
  scdc_transport_connection_t *conn = static_cast<scdc_transport_connection_t *>(data->get_data());

  return conn->transport->send_outgoing(conn);
}


void scdc_transport::create_outgoing(scdc_transport_connection_t *conn, scdcint_t size)
{
  if (size < 0) size = DEFAULT_TRANSPORT_ALLOC_OUTGOING_SIZE;
  conn->outgoing.alloc_buf(size);

  conn->outgoing.set_data(conn);
  conn->outgoing.set_next(scdc_transport_outgoing_next);
}


void scdc_transport::destroy_outgoing(scdc_transport_connection_t *conn)
{
  conn->outgoing.set_next(0);
  conn->outgoing.set_data(0);

  conn->outgoing.free_buf();
}


#undef SCDC_LOG_PREFIX
