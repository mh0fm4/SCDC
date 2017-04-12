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
#include <cstring>
#include <string>
#include <algorithm>

#define SCDC_TRACE_NOT  !SCDC_TRACE_COMPCOUP_TRANSPORT

#include "config.hh"
#include "log.hh"
#include "common.hh"
#include "dataset_inout.h"
/*#include "compression.hh"*/
#include "compcoup_transport.hh"


using namespace std;


#define SCDC_LOG_PREFIX  "compcoup-transport: "


static const char default_delim = ' ';
static const char state_begin = '<';
static const char state_end = '>';
static const char string_begin = '\'';
static const char string_end = '\'';


class scdc_dataset_transport: public scdc_dataset
{
  public:
    scdc_dataset_transport()
      :scdc_dataset(0), compcoup_transport(0), transport_connection(0) { }

    void set_compcoup_transport(scdc_compcoup_transport *compcoup_transport_) { compcoup_transport = compcoup_transport_; }

    void set_transport_connection(scdc_transport_connection_t *transport_connection_) { transport_connection = transport_connection_; }


    static bool write_state_begin(scdc_transport_connection_t *transport_connection_)
    {
      return write_bytes(transport_connection_, &state_begin, 1);
    }


    static bool read_state_begin(scdc_transport_connection_t *transport_connection_)
    {
      char c;
      bool ret = read_bytes(transport_connection_, &c, 1);

      SCDC_ASSERT(c == state_begin);

      return ret;
    }
    

    static bool write_state_end(scdc_transport_connection_t *transport_connection_)
    {
      bool ret = true;

      ret = ret && write_bytes(transport_connection_, &state_end, 1);
      ret = ret && write_bytes(transport_connection_, &default_delim, 1);

      return ret;
    }


    static bool read_state_end(scdc_transport_connection_t *transport_connection_)
    {
      bool ret = true;

      char c;
      ret = ret && read_bytes(transport_connection_, &c, 1);
      SCDC_ASSERT(c == state_end);

      ret = ret && read_bytes(transport_connection_, &c, 1);
      SCDC_ASSERT(c == default_delim);

      return ret;
    }


    bool write_state()
    {
      bool ret = true;

      ret = ret && write_state_begin(transport_connection);
      ret = ret && write_delim(transport_connection, state.data(), state.size(), state_end);

      ret = ret && write_bytes(transport_connection, &default_delim, 1);

      return ret;
    }


    bool read_state()
    {
      bool ret = true;

      ret = ret && read_state_begin(transport_connection);
      ret = ret && read_delim(transport_connection, &state, state_end);

      char c;
      ret = ret && read_bytes(transport_connection, &c, 1);
      SCDC_ASSERT(c == default_delim);

      return ret;
    }


    static bool write_string(scdc_transport_connection_t *transport_connection_, const char *s, scdcint_t n)
    {
      if (n < 0) n = strlen(s);

      char t[16];
      scdcint_t nt = sprintf(t, "%" scdcint_fmt, n);

      bool ret = true;

      ret = ret && write_delim(transport_connection_, t, nt, string_begin);
      ret = ret && write_bytes(transport_connection_, s, n);
      ret = ret && write_bytes(transport_connection_, &string_end, 1);
      ret = ret && write_bytes(transport_connection_, &default_delim, 1);

      return ret;
    }


    static bool read_string(scdc_transport_connection_t *transport_connection_, std::string *s)
    {
      bool ret = true;

      string t;
      ret = ret && read_delim(transport_connection_, &t, string_begin);

      scdcint_t n;
      sscanf(t.c_str(), "%" scdcint_fmt, &n);

      ret = ret && read_bytes(transport_connection_, s, n);

      char c;
      ret = ret && read_bytes(transport_connection_, &c, 1);
      SCDC_ASSERT(c == string_end);

      ret = ret && read_bytes(transport_connection_, &c, 1);
      SCDC_ASSERT(c == default_delim);

      return ret;
    }


    static bool write_inout_info(scdc_transport_connection_t *transport_connection_, scdc_dataset_inout_t *inout)
    {
      scdc_dataset_inout_t inout_null;
      if (!inout)
      {
        scdc_dataset_inout_unset(&inout_null);
        inout = &inout_null;
      }

      char info[256];
      scdcint_t n = sprintf(info, "%s|%" scdcint_fmt "|%c|%" scdcint_fmt "|%d", inout->format, inout->total_size, inout->total_size_given, SCDC_DATASET_INOUT_BUF_CURRENT(inout), (inout->next)?1:0);

      if (!write_delim(transport_connection_, info, n, default_delim))
      {
        SCDC_FAIL("write_inout_info: write failed");
        return false;
      }

      return true;
    }


    static bool read_inout_info(scdc_transport_connection_t *transport_connection_, scdc_dataset_inout_t *inout, bool &next)
    {
      scdc_dataset_inout_t inout_null;
      if (!inout) inout = &inout_null;

      string s;
      if (!read_delim(transport_connection_, &s, default_delim))
      {
        SCDC_FAIL("read_inout_info: read failed");
        return false;
      }

      /* parse format separately to allow empty format strings */
      size_t end = s.find('|');
      strncpy(inout->format, s.c_str(), end);
      inout->format[end] = '\0';

      int inext;
      int r = sscanf(s.c_str() + end, "|%" scdcint_fmt "|%c|%" scdcint_fmt "|%d", &inout->total_size, &inout->total_size_given, &SCDC_DATASET_INOUT_BUF_CURRENT(inout), &inext);

      if (r != 4)
      {
        SCDC_FAIL("read_inout_info: read failed");
        return false;
      }

      next = (inext != 0);

      return true;
    }


    static bool write_input_info(scdc_transport_connection_t *transport_connection_, scdc_dataset_input_t *input, bool cont = false)
    {
      return write_inout_info(transport_connection_, input);
    }


    static bool read_input_info(scdc_transport_connection_t *transport_connection_, scdc_dataset_input_t *input, bool &next)
    {
      return read_inout_info(transport_connection_, input, next);
    }


    static bool write_output_info(scdc_transport_connection_t *transport_connection_, scdc_dataset_output_t *output, bool cont = false)
    {
      return write_inout_info(transport_connection_, output);
    }


    static bool read_output_info(scdc_transport_connection_t *transport_connection_, scdc_dataset_output_t *output, bool &next)
    {
      return read_inout_info(transport_connection_, output, next);
    }


    static bool write_delim(scdc_transport_connection_t *transport_connection_, const char *s, scdcint_t n, char delim)
    {
      if (n < 0) n = strlen(s);

      bool ret = true;

      ret = ret && write_bytes(transport_connection_, s, n);
      ret = ret && write_bytes(transport_connection_, &delim, 1);

      if (!ret) SCDC_FAIL("write_delim: failed");

      return ret;
    }


    /* read until delim, if stop_at_delim != 0 then stop also if delim is not found, set *stop_at_delim to whether delim was found or not */
    static bool read_delim(scdc_transport_connection_t *transport_connection_, std::string *s, char delim, bool *stop_at_delim = 0)
    {
      scdc_data *incoming = transport_connection_->get_incoming();
      bool ret = true;

      (*s).clear();

      while (ret)
      {
        const char *src = incoming->get_read_pos_buf();
        scdcint_t src_size = incoming->get_read_pos_buf_size();

/*        SCDC_TRACE("read_delim: '" << string(src, src_size) << "'");*/

        const char *end = memchr(src, delim, src_size);

        scdcint_t n = (end)?(end - src):src_size;

        *s += string(src, n);

        if (end) n += 1;
        incoming->inc_read_pos(n);

        if (stop_at_delim && n > 0)
        {
          *stop_at_delim = (end != 0);
          break;
        }

        if (end) break;

        ret = incoming->do_next();
      }

      return ret;
    }


    static bool write_bytes(scdc_transport_connection_t *transport_connection_, const void *buf, scdcint_t nbytes)
    {
      scdc_data *outgoing = transport_connection_->get_outgoing();
      bool ret = true;

      const char *src = static_cast<const char*>(buf);

      while (ret)
      {
        scdcint_t n = min(nbytes, outgoing->get_write_pos_buf_size());

        memcpy(outgoing->get_write_pos_buf(), src, n);

        src += n;
        outgoing->inc_write_pos(n);
        nbytes -= n;

        if (nbytes <= 0) break;

        ret = outgoing->do_next();
      }

      return ret;
    }


    static bool read_bytes(scdc_transport_connection_t *transport_connection_, string *s, scdcint_t nbytes)
    {
      scdc_data *incoming = transport_connection_->get_incoming();
      bool ret = true;

      (*s).clear();

      while (ret)
      {
        const char *src = incoming->get_read_pos_buf();
        scdcint_t src_size = incoming->get_read_pos_buf_size();

        scdcint_t n = min(nbytes, src_size);

        *s += string(src, n);

        incoming->inc_read_pos(n);
        nbytes -= n;

        if (nbytes <= 0) break;

        ret = incoming->do_next();
      }

      return ret;
    }


    static bool read_bytes(scdc_transport_connection_t *transport_connection_, void *buf, scdcint_t nbytes)
    {
      scdc_data *incoming = transport_connection_->get_incoming();
      bool ret = true;

      char *dst = static_cast<char *>(buf);

      while (ret)
      {
        const char *src = incoming->get_read_pos_buf();
        scdcint_t src_size = incoming->get_read_pos_buf_size();

        scdcint_t n = min(nbytes, src_size);

        memcpy(dst, src, n);
        dst += n;

        incoming->inc_read_pos(n);
        nbytes -= n;

        if (nbytes <= 0) break;

        ret = incoming->do_next();
      }

      return ret;
    }


    bool do_cmd(const char *cmd, scdcint_t cmd_size, scdc_dataset_input_t *input, scdc_dataset_output_t *output)
    {
      return compcoup_transport->dataset_cmd(this, cmd, cmd_size, input, output);
    }


  private:
    string state;
    scdc_compcoup_transport *compcoup_transport;
    scdc_transport_connection_t *transport_connection;
};


#undef SCDC_LOG_PREFIX

#define SCDC_LOG_PREFIX  "compcoup-transport: "


scdc_compcoup_transport::scdc_compcoup_transport(scdc_transport *transport_)
  :scdc_compcoup(), transport(transport_), transport_connection(0),
  cmd_handler(0), cmd_handler_data(0)
{
}


scdc_compcoup_transport::~scdc_compcoup_transport()
{
}


static scdcint_t read_input_next(scdc_dataset_input_t *input)
{
  SCDC_TRACE("read_input_next:");

  scdc_transport_connection_t *transport_connection = static_cast<scdc_transport_connection_t *>(input->data);

  bool next = false, ret = true;

  if (transport_connection->current_incoming_input_next || transport_connection->current_incoming_input_size > 0) ret = scdc_compcoup_transport::dataset_input_incoming_recv(transport_connection, input, &next);
  else SCDC_DATASET_INOUT_BUF_CURRENT(input) = 0;

  if (next)
  {
    /* continue with current input->next and input->data */

  } else
  {
    if (transport_connection->current_incoming_input_intern)
    {
      /* continue with current input->next and input->data */

      /* unset internal buffer usage for next 'next' */
      transport_connection->current_incoming_input_intern = false;

    } else
    {
      /* stop and release connection */
      input->next = 0;
      input->data = 0;

      /* read_input_next is only used by the server, and thus connection acquire/release is not required */
    }
  }

  return (ret)?SCDC_SUCCESS:SCDC_FAILURE;
}


static scdcint_t read_output_next(scdc_dataset_output_t *output)
{
  SCDC_TRACE("read_output_next:");

  scdc_transport_connection_t *transport_connection = static_cast<scdc_transport_connection_t *>(output->data);

  bool next = false, ret = true;

  if (transport_connection->current_incoming_output_next || transport_connection->current_incoming_output_size > 0) ret = scdc_compcoup_transport::dataset_output_incoming_recv(transport_connection, output, &next);
  else SCDC_DATASET_INOUT_BUF_CURRENT(output) = 0;

  if (next)
  {
    /* continue with current output->next and output->data */

  } else
  {
    if (transport_connection->current_incoming_output_intern)
    {
      /* continue with current output->next and output->data */

      /* unset internal buffer usage for next 'next' */
      transport_connection->current_incoming_output_intern = false;

    } else
    {
      /* stop and release connection */
      output->next = 0;
      output->data = 0;

      transport_connection->release();
    }
  }

  return (ret)?SCDC_SUCCESS:SCDC_FAILURE;
}


scdc_dataset *scdc_compcoup_transport::dataset_open(const char *path, scdcint_t path_size, scdc_dataset_output_t *output)
{
  SCDC_TRACE("dataset_open: '" << string(path, path_size) << "'");

  if (!transport_connection->acquire())
  {
    SCDC_FAIL("dataset_open: acquiring connection failed!");
    SCDC_DATASET_OUTPUT_PRINTF(output, "acquiring connection failed");
    return 0;
  }

  bool ret = true;

  /* send request: DATASET_OPEN <output-info> <path> */

  ret = ret && scdc_dataset_transport::write_delim(transport_connection, "DATASET_OPEN", -1, default_delim);
  ret = ret && scdc_dataset_transport::write_output_info(transport_connection, output);
  ret = ret && scdc_dataset_transport::write_string(transport_connection, path, path_size);

  ret = ret && transport->send_outgoing(transport_connection);

  if (!ret)
  {
    SCDC_FAIL("dataset_open: sending open request failed!");
    SCDC_DATASET_OUTPUT_PRINTF(output, "sending open request failed");
    transport_connection->release();
    return 0;
  }

  /* read answer: DATASET_OPEN (OK|FAILED) (STATE <state>|NOSTATE) (<output-info> <output-data> )+ */

  string s;

  ret = ret && scdc_dataset_transport::read_delim(transport_connection, &s, default_delim);
  SCDC_ASSERT(s == "DATASET_OPEN");

  scdc_dataset_transport *dataset_transport = 0;

  ret = ret && scdc_dataset_transport::read_delim(transport_connection, &s, default_delim);
  if (s == "OK")
  {
    dataset_transport = new scdc_dataset_transport();

    dataset_transport->set_compcoup_transport(this);
    dataset_transport->set_transport_connection(transport_connection);

  } else
  {
    SCDC_ASSERT(s == "FAILED");
  }

  ret = ret && scdc_dataset_transport::read_delim(transport_connection, &s, default_delim);
  if (s == "STATE")
  {
    ret = ret && dataset_transport->read_state();

  } else
  {
    SCDC_ASSERT(s == "NOSTATE");
  }

  if (!ret)
  {
    SCDC_FAIL("dataset_open: reading open answer failed!");
    SCDC_DATASET_OUTPUT_PRINTF(output, "reading open answer failed");
    delete dataset_transport;
    transport_connection->release();
    return 0;
  }

  /* final output is either consumed by a given next function or has to be further consumed using the next function */
  bool output_next = false;
  ret = ret && dataset_output_incoming_consume(transport_connection, output, &output_next);

  if (output)
  {
    if (output_next)
    {
      output->next = read_output_next;
      output->data = transport_connection;

    } else
    {
      if (transport_connection->current_incoming_output_intern)
      {
        output->next = read_output_next;
        output->data = transport_connection;

        /* unset internal buffer usage for next 'next' */
        transport_connection->current_incoming_output_intern = false;

      } else
      {
        output->next = 0;
        output->data = 0;
      }
    }
  }

  if (!ret)
  {
    SCDC_FAIL("dataset_open: reading output failed");
    SCDC_DATASET_OUTPUT_PRINTF(output, "reading output failed");
    delete dataset_transport;
    transport_connection->release();
    return 0;
  }

  if (!dataset_transport)
  {
    SCDC_FAIL("dataset_open: failed: '" << SCDC_DATASET_OUTPUT_STR(output) << "'");
  }

  if (!output || !output->next) transport_connection->release();

  return dataset_transport;
}


void scdc_compcoup_transport::dataset_close(scdc_dataset *dataset, scdc_dataset_output_t *output)
{
  SCDC_TRACE("dataset_close: dataset: '" << dataset << "'");

  if (!transport_connection->acquire())
  {
    SCDC_FAIL("dataset_close: acquiring connection failed!");
    SCDC_DATASET_OUTPUT_PRINTF(output, "acquiring connection failed");
    return;
  }

  bool ret = true;

  scdc_dataset_transport *dataset_transport = static_cast<scdc_dataset_transport *>(dataset);

  /* send request: DATASET_CLOSE <output-info> <state> */

  ret = ret && scdc_dataset_transport::write_delim(transport_connection, "DATASET_CLOSE", -1, default_delim);
  ret = ret && scdc_dataset_transport::write_output_info(transport_connection, output);
  ret = ret && dataset_transport->write_state();

  ret = ret && transport->send_outgoing(transport_connection);

  if (!ret)
  {
    SCDC_FAIL("dataset_close: sending close request failed!");
    SCDC_DATASET_OUTPUT_PRINTF(output, "sending close request failed");
    transport_connection->release();
    return;
  }

  /* read answer: DATASET_CLOSE (OK|FAILED) (<output-info> <output-data> )+ */

  string s;

  ret = ret && scdc_dataset_transport::read_delim(transport_connection, &s, default_delim);
  SCDC_ASSERT(s == "DATASET_CLOSE");

  ret = ret && scdc_dataset_transport::read_delim(transport_connection, &s, default_delim);
  if (s == "OK")
  {
    delete dataset_transport;
    dataset_transport = 0;

  } else
  {
    SCDC_ASSERT(s == "FAILED");
  }

  if (!ret)
  {
    SCDC_FAIL("dataset_close: reading close answer failed!");
    SCDC_DATASET_OUTPUT_PRINTF(output, "reading close answer failed");
    delete dataset_transport;
    transport_connection->release();
    return;
  }

  /* final output is either consumed by a given next function or has to be further consumed using the next function */
  bool output_next = false;
  ret = ret && dataset_output_incoming_consume(transport_connection, output, &output_next);

  if (output)
  {
    if (output_next)
    {
      output->next = read_output_next;
      output->data = transport_connection;

    } else
    {
      if (transport_connection->current_incoming_output_intern)
      {
        output->next = read_output_next;
        output->data = transport_connection;

        /* unset internal buffer usage for next 'next' */
        transport_connection->current_incoming_output_intern = false;

      } else
      {
        output->next = 0;
        output->data = 0;
      }
    }
  }

  if (!ret)
  {
    SCDC_FAIL("dataset_close: reading output failed");
    SCDC_DATASET_OUTPUT_PRINTF(output, "reading output failed");
    delete dataset_transport;
    transport_connection->release();
    return;
  }

  if (dataset_transport)
  {
    SCDC_FAIL("dataset_close: failed: '" << SCDC_DATASET_OUTPUT_STR(output) << "'");
    delete dataset_transport;
    dataset_transport = 0;
  }

  if (!output || !output->next) transport_connection->release();
}


bool scdc_compcoup_transport::dataset_cmd(const char *cmd, scdcint_t cmd_size, scdc_dataset_input_t *input, scdc_dataset_output_t *output)
{
  return dataset_cmd(0, cmd, cmd_size, input, output);
}


bool scdc_compcoup_transport::dataset_cmd(scdc_dataset *dataset, const char *cmd, scdcint_t cmd_size, scdc_dataset_input_t *input, scdc_dataset_output_t *output)
{
  SCDC_TRACE("dataset_cmd: dataset: '" << dataset << "'");

  if (!transport_connection->acquire())
  {
    SCDC_FAIL("dataset_cmd: acquiring connection failed!");
    SCDC_DATASET_OUTPUT_PRINTF(output, "acquiring connection failed");
    return false;
  }

  bool ret = true;

  scdc_dataset_transport *dataset_transport = static_cast<scdc_dataset_transport *>(dataset);

  /* send request: DATASET_CMD <output-info> (STATE <state>|NOSTATE) <cmd> (<input-info> <input-data> )+ */

  ret = ret && scdc_dataset_transport::write_delim(transport_connection, "DATASET_CMD", -1, default_delim);
  ret = ret && scdc_dataset_transport::write_output_info(transport_connection, output);
  if (dataset_transport)
  {
    ret = ret && scdc_dataset_transport::write_delim(transport_connection, "STATE", -1, default_delim);
    ret = ret && dataset_transport->write_state();

  } else
  {
    ret = ret && scdc_dataset_transport::write_delim(transport_connection, "NOSTATE", -1, default_delim);
  }
  ret = ret && scdc_dataset_transport::write_string(transport_connection, cmd, cmd_size);

  if (!ret)
  {
    SCDC_FAIL("dataset_cmd: sending cmd request failed!");
    SCDC_DATASET_OUTPUT_PRINTF(output, "sending cmd request failed");
    transport_connection->release();
    return false;
  }

  /* send input */
  ret = ret && dataset_input_outgoing_produce(transport_connection, input);

  ret = ret && transport->send_outgoing(transport_connection);

  if (!ret)
  {
    SCDC_FAIL("dataset_cmd: sending input failed!");
    SCDC_DATASET_OUTPUT_PRINTF(output, "sending input failed");
    transport_connection->release();
    return false;
  }

  /* read answer: DATASET_CMD (<output-info> <output-data> )+(OK|FAILED) (STATE <state>|NOSTATE) (<output-info> <output-data> )+ */

  string s;

  ret = ret && scdc_dataset_transport::read_delim(transport_connection, &s, default_delim);
  SCDC_ASSERT(s == "DATASET_CMD");

  if (!ret)
  {
    SCDC_FAIL("dataset_cmd: reading cmd answer (1) failed!");
    SCDC_DATASET_OUTPUT_PRINTF(output, "reading cmd answer (1) failed");
    transport_connection->release();
    return false;
  }

  bool output_next = false;

  /* consume immediate output with a given next function */
  ret = ret && dataset_output_incoming_consume(transport_connection, output, &output_next);
  SCDC_ASSERT(!output_next);

  if (!ret)
  {
    SCDC_FAIL("dataset_cmd: reading immediate output failed!");
    SCDC_DATASET_OUTPUT_PRINTF(output, "reading immediate output failed");
    transport_connection->release();
    return false;
  }

  bool cmd_ret = false;

  ret = ret && scdc_dataset_transport::read_delim(transport_connection, &s, default_delim);

  if (s == "OK")
  {
    cmd_ret = true;

  } else
  {
    SCDC_ASSERT(s == "FAILED");
  }

  ret = ret && scdc_dataset_transport::read_delim(transport_connection, &s, default_delim);
  if (s == "STATE")
  {
    ret = ret && dataset_transport->read_state();

  } else
  {
    SCDC_ASSERT(s == "NOSTATE");
    SCDC_ASSERT(!dataset_transport);
  }

  if (!ret)
  {
    SCDC_FAIL("dataset_open: reading cmd answer (2) failed!");
    SCDC_DATASET_OUTPUT_PRINTF(output, "reading cmd answer (2) failed");
    delete dataset_transport;
    transport_connection->release();
    return false;
  }

  /* final output is either consumed by a given next function or has to be further consumed using the next function */
  output_next = false;
  ret = ret && dataset_output_incoming_consume(transport_connection, output, &output_next);

  if (output)
  {
    if (output_next)
    {
      output->next = read_output_next;
      output->data = transport_connection;

    } else
    {
      if (transport_connection->current_incoming_output_intern)
      {
        output->next = read_output_next;
        output->data = transport_connection;

        /* unset internal buffer usage for next 'next' */
        transport_connection->current_incoming_output_intern = false;

      } else
      {
        output->next = 0;
        output->data = 0;
      }
    }
  }

  if (!ret)
  {
    SCDC_FAIL("dataset_open: reading output failed");
    SCDC_DATASET_OUTPUT_PRINTF(output, "reading output failed");
    delete dataset_transport;
    transport_connection->release();
    return false;
  }

  if (!cmd_ret)
  {
    SCDC_FAIL("dataset_open: failed: '" << SCDC_DATASET_OUTPUT_STR(output) << "'");
  }

  if (!output || !output->next) transport_connection->release();

  return cmd_ret;
}


bool scdc_compcoup_transport::start(scdcint_t mode)
{
  SCDC_TRACE("start:");

  return true;
}


bool scdc_compcoup_transport::stop()
{
  SCDC_TRACE("stop:");

  return true;
}


bool scdc_compcoup_transport::cancel(bool interrupt)
{
  SCDC_TRACE("cancel:");

  return true;
}


/*bool scdc_compcoup_transport::resume()
{
}*/


bool scdc_compcoup_transport::connect(scdc_transport_connection_t *conn)
{
  transport_connection = conn;

  handshake();

  return true;
}


void scdc_compcoup_transport::disconnect(scdc_transport_connection_t *conn)
{
  transport_connection = 0;
}


void scdc_compcoup_transport::serve(scdc_transport_connection_t *conn)
{
  handshake();

  const scdcint_t loop_max = -1;

  scdcint_t l = 0;
  string prev;

  while (conn->open && (loop_max < 0 || l < loop_max))
  {
    string head;
    bool stop_at_delim = false;

    bool ret = scdc_dataset_transport::read_delim(conn, &head, default_delim, &stop_at_delim);

    if (!ret)
    {
      SCDC_TRACE("serve_connection: read failed!");
      break;
    }

    head = prev + head;

    SCDC_TRACE("serve_connection: head: '" << head << "', stop_at_delim: '" << stop_at_delim << "'");

    prev.erase();

    if (head == "DATASET_OPEN" && stop_at_delim) on_dataset_open(conn);
    else if (head == "DATASET_CLOSE" && stop_at_delim) on_dataset_close(conn);
    else if (head == "DATASET_CMD" && stop_at_delim) on_dataset_cmd(conn);
    else if (on_cmd_handler(head.c_str(), conn->get_incoming()))
    {
      SCDC_TRACE("serve_connection: cmd_handler sccessfull");

    } else if (stop_at_delim)
    {
      SCDC_ERROR("serve_connection: unknown command '" << head << "'");

    } else prev = head;

    ++l;
  }
}


void scdc_compcoup_transport::handshake()
{
}


bool scdc_compcoup_transport::dataset_input_incoming_recv(scdc_transport_connection_t *conn, scdc_dataset_input_t *input, bool *next)
{
  SCDC_TRACE("dataset_input_incoming_recv:");

  SCDC_ASSERT(conn->current_incoming_input_size >= 0);

  /* empty input */
  SCDC_DATASET_INOUT_BUF_CURRENT(input) = 0;

  bool cont_recv = true;

  do
  {
    if (conn->current_incoming_input_size <= 0)
    {
      SCDC_TRACE("dataset_input_incoming_recv: read_input_info");
      if (!scdc_dataset_transport::read_input_info(conn, &conn->current_incoming_input, conn->current_incoming_input_next))
      {
        SCDC_FAIL("dataset_input_incoming_recv: read input info failed");
        return false;
      }

      conn->current_incoming_input_size = SCDC_DATASET_INOUT_BUF_CURRENT(&conn->current_incoming_input);
    }

    strncpy(input->format, conn->current_incoming_input.format, SCDC_FORMAT_MAX_SIZE);
    input->total_size = conn->current_incoming_input.total_size;
    input->total_size_given = conn->current_incoming_input.total_size_given;

    scdcint_t prev_current = SCDC_DATASET_INOUT_BUF_CURRENT(input);

    if (!conn->transport->recv_incoming_dataset_input(conn, input, conn->current_incoming_input_size, cont_recv))
    {
      SCDC_FAIL("dataset_input_incoming_recv: receive dataset input failed");
      return false;
    }

    conn->current_incoming_input_size -= SCDC_DATASET_INOUT_BUF_CURRENT(input) - prev_current;

    *next = (conn->current_incoming_input_next || conn->current_incoming_input_size > 0);

    SCDC_TRACE("dataset_input_incoming_recv: cont: '" << cont_recv << "', next: '" << *next << "', buf: " << SCDC_DATASET_INOUT_BUF_CURRENT(input) << " of " << SCDC_DATASET_INOUT_BUF_SIZE(input));

  } while (cont_recv && *next && SCDC_DATASET_INOUT_BUF_CURRENT(input) < SCDC_DATASET_INOUT_BUF_SIZE(input));

  return true;
}


bool scdc_compcoup_transport::dataset_input_outgoing_send(scdc_transport_connection_t *conn, scdc_dataset_input_t *input)
{
  SCDC_TRACE("dataset_input_outgoing_send:");

  if (!scdc_dataset_transport::write_input_info(conn, input))
  {
    SCDC_FAIL("dataset_input_outgoing_send: write input info failed");
    return false;
  }

  if (input)
  {
    if (!conn->transport->send_outgoing_dataset_input(conn, input))
    {
      SCDC_FAIL("dataset_input_outgoing_send: send dataset input failed");
      return false;
    }
  }

  return true;
}


bool scdc_compcoup_transport::dataset_output_incoming_recv(scdc_transport_connection_t *conn, scdc_dataset_output_t *output, bool *next)
{
  SCDC_TRACE("dataset_output_incoming_recv:");

  SCDC_ASSERT(conn->current_incoming_output_size >= 0);

  /* empty output */
  SCDC_DATASET_INOUT_BUF_CURRENT(output) = 0;

  bool cont_recv = true;

  do
  {
    if (conn->current_incoming_output_size <= 0)
    {
      if (!scdc_dataset_transport::read_output_info(conn, &conn->current_incoming_output, conn->current_incoming_output_next))
      {
        SCDC_FAIL("dataset_output_incoming_recv: read output info failed");
        return false;
      }

      conn->current_incoming_output_size = SCDC_DATASET_INOUT_BUF_CURRENT(&conn->current_incoming_output);

      SCDC_TRACE("dataset_output_incoming_recv: current_incoming_output_next/size: " << conn->current_incoming_output_next << " / " << conn->current_incoming_output_size);
    }

    strncpy(output->format, conn->current_incoming_output.format, SCDC_FORMAT_MAX_SIZE);
    output->total_size = conn->current_incoming_output.total_size;
    output->total_size_given = conn->current_incoming_output.total_size_given;

    scdcint_t prev_current = SCDC_DATASET_INOUT_BUF_CURRENT(output);

    if (!conn->transport->recv_incoming_dataset_output(conn, output, conn->current_incoming_output_size, cont_recv))
    {
      SCDC_FAIL("dataset_output_incoming_recv: receive dataset output failed");
      return false;
    }

    conn->current_incoming_output_size -= SCDC_DATASET_INOUT_BUF_CURRENT(output) - prev_current;

    *next = (conn->current_incoming_output_next || conn->current_incoming_output_size > 0);

    SCDC_TRACE("dataset_output_incoming_recv: cont: '" << cont_recv << "', next: '" << *next << "', buf: " << SCDC_DATASET_INOUT_BUF_CURRENT(output) << " of " << SCDC_DATASET_INOUT_BUF_SIZE(output));
 
  } while (cont_recv && *next && SCDC_DATASET_INOUT_BUF_CURRENT(output) < SCDC_DATASET_INOUT_BUF_SIZE(output));

  return true;
}


bool scdc_compcoup_transport::dataset_output_outgoing_send(scdc_transport_connection_t *conn, scdc_dataset_output_t *output)
{
  SCDC_TRACE("dataset_output_outgoing_send:");

  if (!scdc_dataset_transport::write_output_info(conn, output))
  {
    SCDC_FAIL("dataset_output_outgoing_send: write output info failed");
    return false;
  }

  if (output)
  {
    if (!conn->transport->send_outgoing_dataset_output(conn, output))
    {
      SCDC_FAIL("dataset_output_outgoing_send: send dataset output failed");
      return false;
    }
  }

  return true;
}


bool scdc_compcoup_transport::dataset_input_incoming_consume(scdc_transport_connection_t *conn, scdc_dataset_input_t *input, bool *next)
{
  scdc_dataset_input_t input_null;

  /* use null input for consuming all input */
  if (!input)
  {
    scdc_dataset_input_unset(&input_null);
    input = &input_null;
  }

  *next = true;

  while (*next)
  {
    SCDC_TRACE("dataset_input_incoming_consume: incoming receive");
    if (!dataset_input_incoming_recv(conn, input, next))
    {
      SCDC_FAIL("dataset_input_incoming_consume: receive failed");
      return false;
    }

    if (input == &input_null) continue; 

    if (!input->next) break;

    SCDC_TRACE("dataset_input_incoming_consume: next");
    if (!input->next(input))
    {
      SCDC_FAIL("dataset_input_incoming_consume: next failed");
      return false;
    }
  }

  return true;
}


bool scdc_compcoup_transport::dataset_input_outgoing_produce(scdc_transport_connection_t *conn, scdc_dataset_input_t *input)
{
  while (1)
  {
    SCDC_TRACE("dataset_input_outgoing_produce: outgoing produce");
    if (!dataset_input_outgoing_send(conn, input))
    {
      SCDC_FAIL("dataset_input_outgoing_produce: send failed");
      return false;
    }

    if (!input || !input->next) break;

    SCDC_TRACE("dataset_input_outgoing_produce: next");
    if (!input->next(input))
    {
      SCDC_FAIL("dataset_input_outgoing_produce: next failed");
      return false;
    }
  }

  return true;
}


bool scdc_compcoup_transport::dataset_output_incoming_consume(scdc_transport_connection_t *conn, scdc_dataset_output_t *output, bool *next)
{
  scdc_dataset_output_t output_null;

  /* use null output for consuming all output */
  if (!output)
  {
    scdc_dataset_output_unset(&output_null);
    output = &output_null;
  }

  *next = true;

  while (*next)
  {
    SCDC_TRACE("dataset_output_incoming_consume: incoming receive");
    if (!dataset_output_incoming_recv(conn, output, next))
    {
      SCDC_FAIL("dataset_output_incoming_consume: receive failed");
      return false;
    }

    if (output == &output_null) continue; 

    if (!output->next) break;

    SCDC_TRACE("dataset_output_incoming_consume: next");
    if (!output->next(output))
    {
      SCDC_FAIL("dataset_output_incoming_consume: next failed");
      return false;
    }
  }

  return true;
}


bool scdc_compcoup_transport::dataset_output_outgoing_produce(scdc_transport_connection_t *conn, scdc_dataset_output_t *output)
{
  while (1)
  {
    SCDC_TRACE("dataset_output_outgoing_produce: outgoing produce");
    dataset_output_outgoing_send(conn, output);

    if (!output || !output->next) break;

    SCDC_TRACE("dataset_output_outgoing_produce: next");
    if (!output->next(output))
    {
      SCDC_FAIL("dataset_output_outgoing_produce: next failed");
      return false;
    }
  }

  return true;
}


static scdcint_t write_output_next(scdc_dataset_output_t *output)
{
  scdc_transport_connection_t *transport_connection = static_cast<scdc_transport_connection_t *>(output->data);

  bool ret = true;

  ret = ret && scdc_dataset_transport::write_output_info(transport_connection, output);
  ret = ret && transport_connection->transport->send_outgoing_dataset_output(transport_connection, output);

  return (ret)?SCDC_SUCCESS:SCDC_FAILURE;
}


void scdc_compcoup_transport::on_dataset_open(scdc_transport_connection_t *conn)
{
  SCDC_TRACE("on_dataset_open:");

  SCDC_ASSERT(dataprovs);

  /* read request: DATASET_OPEN <output-info> <path> */

  bool output_next = false;
  scdc_dataset_transport::read_output_info(conn, 0, output_next);
  SCDC_ASSERT(!output_next);

  scdc_dataset_output_t output = *transport->acquire_outgoing_dataset_output(conn);

  output.next = 0;
  output.data = 0;

  string path;
  scdc_dataset_transport::read_string(conn, &path);
  SCDC_TRACE("on_dataset_open: path: '" << path << "'");

  /* send answer: DATASET_OPEN (OK|FAILED) (STATE <state>|NOSTATE) (<output-info> <output-data> )+ */

  scdc_dataset_transport::write_delim(conn, "DATASET_OPEN", -1, default_delim);

  scdc_dataset *dataset = dataprovs->dataset_open(path.data(), path.size(), &output);

  if (dataset)
  {
    scdc_dataset_transport::write_delim(conn, "OK", -1, default_delim);

    scdc_dataset_transport::write_delim(conn, "STATE", -1, default_delim);
    scdc_dataset_transport::write_state_begin(conn);
    dataprovs->dataset_close_write_state(dataset, conn->get_outgoing(), 0);
    scdc_dataset_transport::write_state_end(conn);

  } else
  {
    SCDC_FAIL("on_dataset_open: failed");
    scdc_dataset_transport::write_delim(conn, "FAILED", -1, default_delim);
    scdc_dataset_transport::write_delim(conn, "NOSTATE", -1, default_delim);
  }

  dataset_output_outgoing_produce(conn, &output);

  transport->release_outgoing_dataset_output(conn);

  transport->send_outgoing(conn);
}


void scdc_compcoup_transport::on_dataset_close(scdc_transport_connection_t *conn)
{
  SCDC_TRACE("on_dataset_close:");

  /* read request: DATASET_CLOSE <output-info> <state> */

  bool output_next = false;
  scdc_dataset_transport::read_output_info(conn, 0, output_next);
  SCDC_ASSERT(!output_next);

  scdc_dataset_output_t output = *transport->acquire_outgoing_dataset_output(conn);

  output.next = 0;
  output.data = 0;

  /* send answer: DATASET_CLOSE (OK|FAILED) (<output-info> <output-data> )+ */

  scdc_dataset_transport::write_delim(conn, "DATASET_CLOSE", -1, default_delim);

  scdc_dataset_transport::read_state_begin(conn);
  scdc_dataset *dataset = dataprovs->dataset_open_read_state(conn->get_incoming(), &output);
  scdc_dataset_transport::read_state_end(conn);

  if (dataset)
  {
    scdc_dataset_transport::write_delim(conn, "OK", -1, default_delim);
    dataprovs->dataset_close(dataset, 0);

  } else
  {
    SCDC_FAIL("on_dataset_close: failed");
    scdc_dataset_transport::write_delim(conn, "FAILED", -1, default_delim);
  }

  dataset_output_outgoing_produce(conn, &output);

  transport->release_outgoing_dataset_output(conn);

  transport->send_outgoing(conn);
}


static scdcint_t read_input_next_dummy(scdc_dataset_input_t *input)
{
  return SCDC_SUCCESS;
}


void scdc_compcoup_transport::on_dataset_cmd(scdc_transport_connection_t *conn)
{
  SCDC_TRACE("on_dataset_cmd:");

  /* read request: DATASET_CMD <output-info> (STATE <state>|NOSTATE) <cmd> (<input-info> <input-data> )+ */

  bool output_next = false;
  scdc_dataset_transport::read_output_info(conn, 0, output_next);

  scdc_dataset_output_t output = *transport->acquire_outgoing_dataset_output(conn);

  string s;
  scdc_dataset_transport::read_delim(conn, &s, default_delim);

  /* send answer: DATASET_CMD (<output-info> <output-data> )+(OK|FAILED) (STATE <state>|<NOSTATE) (<output-info> <output-data> )+ */

  scdc_dataset_transport::write_delim(conn, "DATASET_CMD", -1, default_delim);

  bool stateless = true;
  scdc_dataset *dataset = 0;

  if (s == "STATE")
  {
    stateless = false;

    scdc_dataset_transport::read_state_begin(conn);
    dataset = dataprovs->dataset_open_read_state(conn->get_incoming(), &output);
    scdc_dataset_transport::read_state_end(conn);

  } else
  {
    SCDC_ASSERT(s == "NOSTATE");
  }

  /* read cmd */
  scdc_dataset_transport::read_string(conn, &s);

  /* prepare input */
  scdc_dataset_input_t input = *transport->acquire_incoming_dataset_input(conn);

  input.next = 0;
  input.data = 0;

  bool input_next;
  dataset_input_incoming_consume(conn, &input, &input_next);

  if (input_next)
  {
    input.next = read_input_next;
    input.data = conn;

  } else
  {
    if (conn->current_incoming_input_intern)
    {
      input.next = read_input_next;
      input.data = conn;

      conn->current_incoming_input_intern = false;

    } else
    {
      input.next = 0;
      input.data = 0;
    }
  }

  /* prepare output */
  if (output_next)
  {
    output.next = write_output_next;
    output.data = conn;

  } else
  {
    output.next = 0;
    output.data = 0;
  }

  bool ret = false;

  if (stateless)
  {
    ret = scdc_compcoup::dataset_cmd(s.c_str(), s.size(), &input, &output);

  } else if (dataset)
  {
    ret = dataset->do_cmd(s.c_str(), s.size(), &input, &output);

  } else
  {
    /* error: not stateless but also no dataset handle available */
    SCDC_ERROR("no dataset available");
  }

  if (input.next == read_input_next)
  {
    /* consume remaining input */
    input.next = read_input_next_dummy;
    input.data = 0;

    dataset_input_incoming_consume(conn, &input, &input_next);
    SCDC_ASSERT(!input_next);
  }

  transport->release_incoming_dataset_input(conn);

  /* finalize immediate output */
  dataset_output_outgoing_produce(conn, 0);

  if (output.next == write_output_next)
  {
    output.next = 0;
    output.data = 0;
  }

  if (ret)
  {
    scdc_dataset_transport::write_delim(conn, "OK", -1, default_delim);

  } else
  {
    SCDC_FAIL("on_dataset_cmd: dataset_cmd failed");
    scdc_dataset_transport::write_delim(conn, "FAILED", -1, default_delim);
  }

  if (stateless)
  {
    scdc_dataset_transport::write_delim(conn, "NOSTATE", -1, default_delim);

  } else if (dataset)
  {
    scdc_dataset_transport::write_delim(conn, "STATE", -1, default_delim);
    scdc_dataset_transport::write_state_begin(conn);
    dataprovs->dataset_close_write_state(dataset, conn->get_outgoing(), 0);
    scdc_dataset_transport::write_state_end(conn);
  }

  dataset_output_outgoing_produce(conn, &output);

  transport->release_outgoing_dataset_output(conn);

  transport->send_outgoing(conn);
}


bool scdc_compcoup_transport::on_cmd_handler(const char *cmd, scdc_data *incoming)
{
  if (!cmd_handler) return false;
  
  if (cmd[0] == '\0') return true;

  scdcint_t n = cmd_handler(cmd_handler_data, cmd, incoming->get_read_pos_buf(), incoming->get_read_pos_buf_size());

  return (n >= 0);
}


#undef SCDC_LOG_PREFIX
