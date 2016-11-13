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


#include <cstdarg>
#include <string>
#include <sstream>
#include <set>

#include "config.hh"
#include "common.hh"
#include "log.hh"
/*#include "data.hh"*/
#include "dataset_inout.h"
#include "dataprov_pool.hh"
#include "nodeport_pool.hh"
#include "nodeconn_pool.hh"

#include "scdc.h"
#include "scdc_intern.h"


using namespace std;


#define SCDC_LOG_PREFIX  "libscdc: "


typedef struct _scdc_args_data_va_t
{
  scdcint_t arg;
  va_list *ap;
  
} scdc_args_data_va_t;


static void *scdc_dataprov_hook_open_intern(void *intern_data, const char *conf, scdc_args_t *args, scdcint_t *ret)
{
  SCDC_TRACE("scdc_dataprov_hook_open_intern: intern_data: " << intern_data << ", conf: '" << conf << "', args: " << args);

  scdc_dataprov_hook_open_f *open = reinterpret_cast<scdc_dataprov_hook_open_f *>(intern_data);

  void *dataprov = 0;

  *ret = SCDC_SUCCESS;

  if (open)
  {
    scdc_args_data_va_t *args_data_va = static_cast<scdc_args_data_va_t *>(args->data);

    va_list aq;

    va_copy(aq, *args_data_va->ap);

    dataprov = open(conf, aq);

    if (!dataprov) *ret = SCDC_FAILURE;

    va_end(aq);
  }

  return dataprov;
}


static scdc_arg_ref_t scdc_args_get_va(void *data, scdcint_t type, void *v)
{
  scdc_args_data_va_t *args_data_va = static_cast<scdc_args_data_va_t *>(data);

  SCDC_TRACE("scdc_args_get_va: data: '" << args_data_va << "' (arg: '" << args_data_va->arg << "', ap: '" << args_data_va->ap << "'), type: '" << type << "', v: " << v);

  switch (type)
  {
    case SCDC_ARGS_TYPE_INT:
      *((int *) v) = va_arg(*args_data_va->ap, int);
      args_data_va->arg += 1;
      return SCDC_ARG_REF_NONE;
    case SCDC_ARGS_TYPE_SCDCINT:
      *((scdcint_t *) v) = va_arg(*args_data_va->ap, scdcint_t);
      args_data_va->arg += 1;
      return SCDC_ARG_REF_NONE;
    case SCDC_ARGS_TYPE_DOUBLE:
      *((double *) v) = va_arg(*args_data_va->ap, double);
      args_data_va->arg += 1;
      return SCDC_ARG_REF_NONE;
    case SCDC_ARGS_TYPE_CSTR:
    case SCDC_ARGS_TYPE_PTR:
      *((void **) v) = va_arg(*args_data_va->ap, void *);
      args_data_va->arg += 1;
      return SCDC_ARG_REF_NONE;
    case SCDC_ARGS_TYPE_IN_STREAM:
    case SCDC_ARGS_TYPE_OUT_STREAM:
      *((FILE **) v) = va_arg(*args_data_va->ap, FILE *);
      args_data_va->arg += 1;
      return SCDC_ARG_REF_NONE;
    case SCDC_ARGS_TYPE_DATAPROV_HOOK:
      ((scdc_args_dataprov_hook_t *) v)->hook = *va_arg(*args_data_va->ap, scdc_dataprov_hook_t *);
      ((scdc_args_dataprov_hook_t *) v)->hook_open_intern = scdc_dataprov_hook_open_intern;
      ((scdc_args_dataprov_hook_t *) v)->intern_data = reinterpret_cast<void *>(((scdc_args_dataprov_hook_t *) v)->hook.open);
      args_data_va->arg += 1;
      return SCDC_ARG_REF_NONE;
    case SCDC_ARGS_TYPE_DATAPROV_JOBRUN_HANDLER:
      ((scdc_dataprov_jobrun_handler_args_t *) v)->handler = va_arg(*args_data_va->ap, scdc_dataprov_jobrun_handler_f *);
      ((scdc_dataprov_jobrun_handler_args_t *) v)->data = va_arg(*args_data_va->ap, void *);
      args_data_va->arg += 2;
      return SCDC_ARG_REF_NONE;
    case SCDC_ARGS_TYPE_LOG_HANDLER:
      ((scdc_log_handler_args_t *) v)->handler = va_arg(*args_data_va->ap, scdc_log_handler_f *);
      ((scdc_log_handler_args_t *) v)->data = va_arg(*args_data_va->ap, void *);
      args_data_va->arg += 2;
      return SCDC_ARG_REF_NONE;
    case SCDC_ARGS_TYPE_NODEPORT_CMD_HANDLER:
      ((scdc_nodeport_cmd_handler_args_t *) v)->handler = va_arg(*args_data_va->ap, scdc_nodeport_cmd_handler_f *);
      ((scdc_nodeport_cmd_handler_args_t *) v)->data = va_arg(*args_data_va->ap, void *);
      args_data_va->arg += 2;
      return SCDC_ARG_REF_NONE;
    case SCDC_ARGS_TYPE_NODEPORT_TIMER_HANDLER:
       ((scdc_nodeport_timer_handler_args_t *) v)->handler = va_arg(*args_data_va->ap, scdc_nodeport_timer_handler_f *);
       ((scdc_nodeport_timer_handler_args_t *) v)->data = va_arg(*args_data_va->ap, void *);
      args_data_va->arg += 2;
      return SCDC_ARG_REF_NONE;
    case SCDC_ARGS_TYPE_NODEPORT_LOOP_HANDLER:
      ((scdc_nodeport_loop_handler_args_t *) v)->handler = va_arg(*args_data_va->ap, scdc_nodeport_loop_handler_f *);
      ((scdc_nodeport_loop_handler_args_t *) v)->data = va_arg(*args_data_va->ap, void *);
      args_data_va->arg += 2;
      return SCDC_ARG_REF_NONE;
    case SCDC_ARGS_TYPE_NODEPORT_LOOP_HANDLER_DUMMY:
      ((scdc_nodeport_loop_handler_args_t *) v)->handler = 0;
      ((scdc_nodeport_loop_handler_args_t *) v)->data = 0;
      return SCDC_ARG_REF_NONE;
  }

  SCDC_TRACE("scdc_args_get_va: error: unknown type '" << type << "'");

  return SCDC_ARG_REF_NULL;
}


static scdc_arg_ref_t scdc_args_set_va(void *data, scdcint_t type, void *v, scdc_arg_ref_t arg_ref)
{
  SCDC_TRACE_DECL(scdc_args_data_va_t *args_data_va = static_cast<scdc_args_data_va_t *>(data);)

  SCDC_TRACE("scdc_args_set_va: data: '" << args_data_va << "' (arg: '" << ((args_data_va)?args_data_va->arg:-1) << "', ap: '" << ((args_data_va)?args_data_va->ap:0) << "'), type: '" << type << "', v: '" << v << "', arg_ref: '" << arg_ref << "'");

  return SCDC_ARG_REF_NULL;
}


void scdc_args_free_va(void *data, scdcint_t type, void *v, scdc_arg_ref_t arg_ref)
{
  SCDC_TRACE_DECL(scdc_args_data_va_t *args_data_va = static_cast<scdc_args_data_va_t *>(data);)

  SCDC_TRACE("scdc_args_free_va: data: '" << args_data_va << "' (arg: '" << ((args_data_va)?args_data_va->arg:-1) << "', ap: '" << ((args_data_va)?args_data_va->ap:0) << "'), type: '" << type << "', v: '" << v << "', arg_ref: '" << arg_ref << "'");
}


static void scdc_args_va_init(scdc_args_t *args, va_list *ap)
{
  scdc_args_data_va_t *args_data_va = new scdc_args_data_va_t;

  args_data_va->arg = 0;
  args_data_va->ap = ap;

  args->data = args_data_va;
  args->get = scdc_args_get_va;
  args->set = scdc_args_set_va;
  args->free = scdc_args_free_va;
}


static void scdc_args_va_release(scdc_args_t *args)
{
  scdc_args_data_va_t *args_data_va = static_cast<scdc_args_data_va_t *>(args->data);

  delete args_data_va;
}


struct _scdc_context_data_t
{
  _scdc_context_data_t() { };

  ~_scdc_context_data_t() { };

#if SCDC_DEBUG
  set<scdc_dataset_t> datasets;
#endif
  scdc_dataprov_pool dataprovs;
  scdc_nodeport_pool nodeports;
  scdc_nodeconn_pool nodeconns;

  scdc_dataprov_t dp_config;
  scdc_nodeport_t np_direct;
};


static scdcint_t scdc_main_config_hook_config(void *dataprov, const char *cmd, const char *param, const char *val, scdcint_t val_size, char **res, scdcint_t *res_size);

const static scdc_dataprov_hook_t scdc_main_config_hook = {
  0, 0,  /* open, close */
  scdc_main_config_hook_config,  /* config */
  0, 0,  /* dataset_open, dataset_close */
  0, 0,  /* dataset_open_read_state, dataset_close_write_state */
  0,     /* dataset_cmd */
};


class scdc_context
{
  public:
    scdc_log log;
    _scdc_context_data_t *data;

    scdc_context()
      :log(
#ifdef SCDC_LOGFILE
        "log_filepath", SCDC_LOGFILE
#else
        "log_FILE", stdout, stderr
#endif
        ), data(0)
    {
    }

    ~scdc_context()
    {
      if (data) release();
    }

    bool init(const char *conf, scdc_args_t *args)
    {
      SCDC_TRACE("scdc_context: init: '" << conf << "'");

      SCDC_TRACE("scdc_context: init: data: " << data);

      if (data) return false;

      bool no_config = false;
      bool no_direct = false;

      stringlist confs(':', conf);

      while (confs.size() > 0)
      {
        string c = confs.front_pop();

        if (c == "no_config") no_config = true;
        else if (c == "no_direct") no_direct = true;
        else if (c.size() > 0)
        {
          SCDC_TRACE("unknown parameter '" << c << "'");
        }
      }

      data = new _scdc_context_data_t;

      scdc_nodeport_pool::init();
      scdc_nodeconn_pool::init();

      data->nodeconns.set_nodeport_pool(&data->nodeports);

      if (!no_config)
      {
        data->dp_config = scdc_dataprov_open("", "config", &scdc_main_config_hook);

      } else data->dp_config = SCDC_DATAPROV_NULL;

      if (!no_direct)
      {
        data->np_direct = scdc_nodeport_open("direct", "");
        scdc_nodeport_start(data->np_direct, SCDC_NODEPORT_START_ASYNC_UNTIL_CANCEL);

      } else data->np_direct = SCDC_NODEPORT_NULL;

      return true;
    }

    void release()
    {
      SCDC_TRACE("scdc_context: release");

      if (!data) return;

#if SCDC_DEBUG
      for (set<scdc_dataset_t>::iterator i = data->datasets.begin(); i != data->datasets.end(); ++i)
      {
        SCDC_FAIL("scdc_release_intern: dateset '" << *i << "' was not closed");
      }
#endif

      data->nodeconns.close_all();

      scdc_nodeport_stop(data->np_direct);
      scdc_nodeport_close(data->np_direct);

      data->nodeports.close_all();

      scdc_dataprov_close(data->dp_config);

      data->dataprovs.close_all();

      scdc_nodeport_pool::release();
      scdc_nodeconn_pool::release();

      delete data; data = 0;

      SCDC_TRACE("scdc_context: release: return");
    }
};


scdc_context scdc_main_context;
scdc_log *scdc_main_context_log = &scdc_main_context.log;


static scdcint_t scdc_main_config_hook_config(void *dataprov, const char *cmd, const char *param, const char *val, scdcint_t val_size, char **res, scdcint_t *res_size)
{
  SCDC_TRACE("scdc_main_config_hook_config: cmd: '" << cmd << "', param: '" << param << "', val: " << string(val, val_size) << ", result size: " << *res_size);

  string c(cmd), p(param);

  bool done = false;
  bool ret = true;

  scdc_config_result result;

  if (p == "")
  {
    if (c == "info")
    {
      done = true;

      ret = scdc_dataprov_config::info(result, "libscdc configuration");

    } else if (c == "ls")
    {
      done = true;

      ret = scdc_dataprov_config::ls(result, "dataprovs")
         && scdc_dataprov_config::ls(result, "nodeports")
         && scdc_dataprov_config::ls(result, "nodeconns")
#if NODECONN_CACHE_CONNECTIONS
         && scdc_dataprov_config::ls(result, "nodeconn_cache_connections")
#endif
         && true;

    } else done = false;

  } else
  {
    if (done);
    if (c == "info")
    {
      done = true;

      if (p == "dataprovs") ret = scdc_dataprov_config::info(result, param, "data providers");
      else if (p == "nodeports") ret = scdc_dataprov_config::info(result, param, "node ports");
      else if (p == "nodeports") ret = scdc_dataprov_config::info(result, param, "node connections");
#if NODECONN_CACHE_CONNECTIONS
      else if (p == "nodeconn_cache_connections") ret = scdc_dataprov_config::info(result, param, "number of node connections to cache (integer)");
#endif
      else done = false;

/*    } else if (c == "ls")
    {*/
    } else if (c == "put")
    {
      done = true;

/*      if (p == "dataprovs")  ret = scdc_dataprov_config::put<>(result, param);
      else if (p == "nodeports")  ret = scdc_dataprov_config::put<>(result, param);
      else if (p == "nodeconns")  ret = scdc_dataprov_config::put<>(result, param);*/
      if (p == "");
#if NODECONN_CACHE_CONNECTIONS
      else if (p == "nodeconn_cache_connections") ret = scdc_dataprov_config::put<scdcint_t>(result, param, val, data->nodeconns.cache_connections);
#endif
      else done = false;

    } else if (c == "get")
    {
      done = true;

      if (p == "dataprovs")  ret = scdc_dataprov_config::get<size_t>(result, param, scdc_main_context.data->dataprovs.size());
      else if (p == "nodeports")  ret = scdc_dataprov_config::get<size_t>(result, param, scdc_main_context.data->nodeports.size());
      else if (p == "nodeconns")  ret = scdc_dataprov_config::get<size_t>(result, param, scdc_main_context.data->nodeconns.size());
#if NODECONN_CACHE_CONNECTIONS
      else if (p == "nodeconn_cache_connections") ss << scdc_main_context.data->nodeconns.cache_connections;
#endif
      else done = false;
    }
  }

  if (!done || (done && !ret))
  {
    SCDC_FAIL("scdc_main_config_hook_config: command '" << cmd << "' failed");
    return SCDC_FAILURE;
  }

  if (res && res_size)
  {
    strncpy(*res, result.c_str(), *res_size);
    *res_size = result.size();
  }

  return ret;
}


scdcint_t scdc_init_intern(const char *conf, scdc_args_t *args)
{
  SCDC_TRACE("scdc_init_intern: conf: '" << conf << "'");

  scdcint_t ret = (scdc_main_context.init(conf, args))?SCDC_SUCCESS:SCDC_FAILURE;

  SCDC_TRACE("scdc_init_intern: return: '" << ret << "'");

  return ret;
}


scdcint_t scdc_init(const char *conf, ...)
{
  SCDC_TRACE("scdc_init: conf: '" << conf << "'");

  scdc_args_t args;

  va_list ap;
  va_start(ap, conf);

  scdc_args_va_init(&args, &ap);

  scdcint_t ret = scdc_init_intern(conf, &args);

  scdc_args_va_release(&args);

  va_end(ap);

  SCDC_TRACE("scdc_init: return '" << ret << "'");

  return ret;
}


void scdc_release_intern()
{
  SCDC_TRACE("scdc_release_intern:");

  scdc_main_context.release();

  SCDC_TRACE("scdc_release_intern: return");
}


void scdc_release()
{
  SCDC_TRACE("scdc_release:");

  scdc_release_intern();

  SCDC_TRACE("scdc_release: return");

  scdc_log_release_intern();
}


void scdc_main_log_init(const char *conf, ...)
{
  va_list ap;
  va_start(ap, conf);

  SCDC_MAIN_LOG()->default_init_intern(conf, ap);

  va_end(ap);
}


scdcint_t scdc_log_init_intern(const char *conf, scdc_args_t *args)
{
  SCDC_TRACE("scdc_log_init_intern: conf: '" << conf << "'");

  bool ret = SCDC_MAIN_LOG()->init(conf, args);

  SCDC_TRACE("scdc_log_init_intern: return");

  return (ret)?SCDC_SUCCESS:SCDC_FAILURE;
}


scdcint_t scdc_log_init(const char *conf, ...)
{
  SCDC_TRACE("scdc_log_init: conf: '" << conf << "'");

  scdc_args_t args;

  va_list ap;
  va_start(ap, conf);

  scdc_args_va_init(&args, &ap);

  scdcint_t ret = scdc_log_init_intern(conf, &args);

  scdc_args_va_release(&args);

  va_end(ap);

  SCDC_TRACE("scdc_log_init: return '" << ret << "'");

  return ret;
}


void scdc_log_release_intern()
{
  SCDC_TRACE("scdc_log_release_intern:");

  SCDC_MAIN_LOG()->release();

  SCDC_TRACE("scdc_log_release_intern: return");
}


void scdc_log_release()
{
  SCDC_TRACE("scdc_log_release:");

  scdc_log_release_intern();
  
  SCDC_TRACE("scdc_log_release: return");
}


scdc_dataprov_t scdc_dataprov_open_intern(const char *base_path, const char *conf, scdc_args_t *args)
{
  scdc_args xargs(args);

  scdc_dataprov *dataprov = scdc_main_context.data->dataprovs.open(base_path, conf, &xargs);

  if (!dataprov) return SCDC_DATAPROV_NULL;

  return static_cast<scdc_dataprov_t>(dataprov);
}


scdc_dataprov_t scdc_dataprov_open(const char *base_path, const char *conf, ...)
{
  SCDC_TRACE("scdc_dataprov_open:");

  va_list ap;
  va_start(ap, conf);

  scdc_args_t args;

  scdc_args_va_init(&args, &ap);

  scdc_dataprov_t dataprov = scdc_dataprov_open_intern(base_path, conf, &args);

  scdc_args_va_release(&args);
  va_end(ap);

  SCDC_TRACE("scdc_dataprov_open: return '" << dataprov << "'");

  return dataprov;
}


void scdc_dataprov_close(scdc_dataprov_t dataprov)
{
  SCDC_TRACE("scdc_dataprov_close: dataprov: '" << dataprov << "'");

  if (dataprov == SCDC_DATAPROV_NULL) return;

  scdc_main_context.data->dataprovs.close(static_cast<scdc_dataprov *>(dataprov));

  SCDC_TRACE("scdc_dataprov_close: return");
}


scdc_nodeport_t scdc_nodeport_open_intern(const char *conf, scdc_args_t *args)
{
  scdc_args xargs(args);

  scdc_nodeport *nodeport = scdc_main_context.data->nodeports.open(conf, &xargs);

  if (!nodeport) return SCDC_NODEPORT_NULL;

  return static_cast<scdc_nodeport_t>(nodeport);
}


scdc_nodeport_t scdc_nodeport_open(const char *conf, ...)
{
  SCDC_TRACE("scdc_nodeport_open: conf: '" << conf << "'");

  va_list ap;
  va_start(ap, conf);

  scdc_args_t args;

  scdc_args_va_init(&args, &ap);

  scdc_nodeport_t nodeport = scdc_nodeport_open_intern(conf, &args);

  scdc_args_va_release(&args);
  va_end(ap);

  SCDC_TRACE("scdc_nodeport_open: return '" << nodeport << "'");

  return nodeport;
}


void scdc_nodeport_close(scdc_nodeport_t nodeport)
{
  SCDC_TRACE("scdc_nodeport_close: nodeport: '" << nodeport << "'");

  if (nodeport == SCDC_NODEPORT_NULL) return;

  scdc_main_context.data->nodeports.close(static_cast<scdc_nodeport *>(nodeport));

  SCDC_TRACE("scdc_nodeport_close: return");
}


scdcint_t scdc_nodeport_start(scdc_nodeport_t nodeport, scdcint_t mode)
{
  SCDC_TRACE("scdc_nodeport_start: nodeport: '" << nodeport << "', mode: '" << mode << "'");

  if (nodeport == SCDC_NODEPORT_NULL) return SCDC_FAILURE;

  static_cast<scdc_nodeport *>(nodeport)->set_dataprovs(&scdc_main_context.data->dataprovs);

  static_cast<scdc_nodeport *>(nodeport)->start(mode);

  SCDC_TRACE("scdc_nodeport_start: return");

  return SCDC_SUCCESS;
}


scdcint_t scdc_nodeport_stop(scdc_nodeport_t nodeport)
{
  SCDC_TRACE("scdc_nodeport_stop: nodeport: '" << nodeport << "'");

  if (nodeport == SCDC_NODEPORT_NULL) return SCDC_FAILURE;

  static_cast<scdc_nodeport *>(nodeport)->stop();

  static_cast<scdc_nodeport *>(nodeport)->set_dataprovs(0);

  SCDC_TRACE("scdc_nodeport_stop: return");

  return SCDC_SUCCESS;
}


scdcint_t scdc_nodeport_cancel(scdc_nodeport_t nodeport, scdcint_t interrupt)
{
  SCDC_TRACE("scdc_nodeport_cancel: nodeport: '" << nodeport << "'");

  if (nodeport == SCDC_NODEPORT_NULL) return SCDC_FAILURE;

  static_cast<scdc_nodeport *>(nodeport)->cancel(interrupt);

  SCDC_TRACE("scdc_nodeport_cancel: return");

  return SCDC_SUCCESS;
}


#define SCDC_MAIN_NODEPORT_AUTHORITY_SIZE_MAX  1024
static char scdc_main_nodeport_authority[SCDC_MAIN_NODEPORT_AUTHORITY_SIZE_MAX] = { '\0' };


const char *scdc_nodeport_authority_intern(const char *conf, scdc_args_t *args)
{
  scdc_args xargs(args);

  string authority;

  bool ret = scdc_nodeport_pool::authority(conf, &xargs, authority);

  if (!ret || authority.size() > SCDC_MAIN_NODEPORT_AUTHORITY_SIZE_MAX - 1) return 0;

  strncpy(scdc_main_nodeport_authority, authority.c_str(), SCDC_MAIN_NODEPORT_AUTHORITY_SIZE_MAX);

  return scdc_main_nodeport_authority;
}


const char *scdc_nodeport_authority(const char *conf, ...)
{
  SCDC_TRACE("scdc_nodeport_authority: conf: '" << conf << "'");

  va_list ap;
  va_start(ap, conf);

  scdc_args_t args;

  scdc_args_va_init(&args, &ap);

  const char *authority = scdc_nodeport_authority_intern(conf, &args);

  scdc_args_va_release(&args);
  va_end(ap);

  SCDC_TRACE("scdc_nodeport_authority_intern: return '" << authority << "'");

  return authority;
}


scdcint_t scdc_nodeport_supported_intern(const char *uri, scdc_args_t *args)
{
  scdc_args xargs(args);

  string supported;

  bool ret = scdc_nodeport_pool::supported(uri, &xargs);

  return (ret)?SCDC_SUCCESS:SCDC_FAILURE;
}


scdcint_t scdc_nodeport_supported(const char *uri, ...)
{
  SCDC_TRACE("scdc_nodeport_supported: uri: '" << uri << "'");

  va_list ap;
  va_start(ap, uri);

  scdc_args_t args;

  scdc_args_va_init(&args, &ap);

  scdcint_t supported = scdc_nodeport_supported_intern(uri, &args);

  scdc_args_va_release(&args);
  va_end(ap);

  SCDC_TRACE("scdc_nodeport_supported_intern: return '" << supported << "'");

  return supported;
}


struct _scdc_dataset_t
{
  _scdc_dataset_t()
    :nodeconn(0), dataset(0)
  {
    buf = new char[DEFAULT_LIBSCDC_BUF_SIZE];
    buf_size = DEFAULT_LIBSCDC_BUF_SIZE;
  }

  ~_scdc_dataset_t()
  {
    delete[] buf;
  }

  scdc_nodeconn *nodeconn;
  scdc_dataset *dataset;
  char *buf;
  scdcint_t buf_size;
};


scdc_dataset_t scdc_dataset_open_intern(const char *uri, scdc_args_t *args)
{
  SCDC_TRACE("scdc_dataset_open_intern: uri: '" << uri << "'");

  scdc_dataset_t dataset = new _scdc_dataset_t;

  string sscheme, sauthority, spath;
  split_uri(uri, -1, &sscheme, &sauthority, &spath);

  scdc_args xargs(args);

  dataset->nodeconn = scdc_main_context.data->nodeconns.open(sscheme.c_str(), sauthority.c_str(), &xargs);

  if (!dataset->nodeconn)
  {
    SCDC_TRACE("scdc_dataset_open_intern: failed: open connection failed");
    delete dataset;
    return SCDC_DATASET_NULL;
  }

  scdc_dataset_output_t output;
  scdc_dataset_output_create(&output, "buffer", dataset->buf, dataset->buf_size);

  dataset->dataset = dataset->nodeconn->get_compcoup()->dataset_open(spath.c_str(), spath.size(), &output);

  scdc_dataset_output_destroy(&output);

  if (!dataset->dataset)
  {
    SCDC_FAIL("scdc_dataset_open_intern: failed");
    scdc_main_context.data->nodeconns.close(dataset->nodeconn);
    delete dataset;
    return SCDC_DATASET_NULL;
  }

#if SCDC_DEBUG
  scdc_main_context.data->datasets.insert(dataset);
#endif

  SCDC_TRACE("scdc_dataset_open_intern: return '" << dataset << "'");

  SCDC_INFO("opening dataset " << dataset << " with uri '" << uri << "'");

  return dataset;
}


scdc_dataset_t scdc_dataset_open(const char *uri, ...)
{
  SCDC_TRACE("scdc_dataset_open: uri: '" << uri << "'");

  scdc_args_t args;

  va_list ap;
  va_start(ap, uri);

  scdc_args_va_init(&args, &ap);

  scdc_dataset_t dataset = scdc_dataset_open_intern(uri, &args);

  scdc_args_va_release(&args);

  va_end(ap);

  SCDC_TRACE("scdc_dataset_open: return '" << dataset << "'");

  return dataset;
}


void scdc_dataset_close(scdc_dataset_t dataset)
{
  SCDC_TRACE("scdc_dataset_close: '" << dataset << "'");

  if (dataset == SCDC_DATASET_NULL) return;

  SCDC_INFO("closing dataset " << dataset);

#if SCDC_DEBUG
  if (scdc_main_context.data->datasets.erase(dataset) != 1)
  {
    SCDC_FAIL("scdc_dataset_close: no valid dataset handle given");
    return;
  }
#endif

  scdc_dataset_output_t output;
  scdc_dataset_output_create(&output, "buffer", dataset->buf, dataset->buf_size);

  dataset->nodeconn->get_compcoup()->dataset_close(static_cast<scdc_dataset *>(dataset->dataset), &output);

  scdc_dataset_output_destroy(&output);

  scdc_main_context.data->nodeconns.close(dataset->nodeconn);

  delete dataset;
}


scdcint_t scdc_dataset_cmd_intern(scdc_dataset_t dataset, const char *cmd, scdc_dataset_input_t *input, scdc_dataset_output_t *output, scdc_args_t *args)
{
  SCDC_TRACE("scdc_dataset_cmd_intern: dataset: '" << dataset << "', cmd = '" << string(cmd) << "'" << ", input = " << static_cast<void *>(input) << ", output = " << static_cast<void *>(output));

  scdcint_t cmd_size = strlen(cmd);

  string adhoc_cmd;

  if (dataset == SCDC_DATASET_NULL)
  {
    dataset = new _scdc_dataset_t;
    dataset->dataset = 0;

    cmd_size = strlen(cmd);

    string suri, scmd, sparams;
    split_cmdline(cmd, cmd_size, &scmd, &suri, &sparams);

    if (suri.size() <= 0)
    {
      SCDC_DATASET_OUTPUT_PRINTF(output, "no URI given");
      SCDC_FAIL("scdc_dataset_cmd_intern: no URI given!");
      delete dataset;
      return SCDC_FAILURE;
    }

    string sscheme, sauthority, spath;
    split_uri(suri.c_str(), suri.size(), &sscheme, &sauthority, &spath);

    scdc_args xargs(args);

    dataset->nodeconn = scdc_main_context.data->nodeconns.open(sscheme.c_str(), sauthority.c_str(), &xargs);

    if (!dataset->nodeconn)
    {
      SCDC_DATASET_OUTPUT_PRINTF(output, "open connection failed");
      SCDC_FAIL("scdc_dataset_cmd_intern: failed: open connection failed!");
      delete dataset;
      return SCDC_FAILURE;
    }

    join_uri(0, 0, spath.c_str(), suri);
    suri = ":" + suri;

    join_cmdline(scmd.c_str(), suri.c_str(), sparams.c_str(), adhoc_cmd);

    cmd = adhoc_cmd.c_str();
    cmd_size = adhoc_cmd.size();
  }

  SCDC_TRACE("scdc_dataset_cmd_intern: adhoc command: '" << adhoc_cmd << "'");

  scdc_dataset_input_t input_null = *SCDC_DATASET_INPUT_NONE;
  if (!input) input = &input_null;

  scdc_dataset_input_t output_null = *SCDC_DATASET_OUTPUT_ENDL;
  if (!output) output = &output_null;

  /* backup original output */
  scdc_dataset_output_t output_given = *output;
  scdc_dataset_inout_intern_t output_intern_given = (output->intern)?*output->intern:SCDC_DATASET_OUTPUT_INTERN_NULL;

  SCDC_TRACE_DATASET_INPUT(input, "scdc_dataset_cmd_intern: IN input: ");
  SCDC_TRACE_DATASET_OUTPUT(output, "scdc_dataset_cmd_intern: IN output: ");

  bool ret;

  double cmd_timing = z_time_wtime();

  if (dataset->dataset) ret = dataset->dataset->do_cmd(cmd, cmd_size, input, output);
  else ret = dataset->nodeconn->get_compcoup()->dataset_cmd(cmd, cmd_size, input, output);

  cmd_timing = z_time_wtime() - cmd_timing;

  SCDC_TRACE("scdc_dataset_cmd_intern: timing: " << cmd_timing);

  SCDC_TRACE_DATASET_INPUT(input, "scdc_dataset_cmd_intern: OUT input: ");
  SCDC_TRACE_DATASET_OUTPUT(output, "scdc_dataset_cmd_intern: OUT output: ");

  /* if there was an output next given, but the resulting cmd output is different */
  if (output_given.next && (output_given.next != output->next || output_given.data != output->data || output_given.intern != output->intern))
  {
    SCDC_TRACE("scdc_dataset_cmd_intern: redirecting cmd output to given output");
    scdc_dataset_output_redirect(output, "to:outputsink", &output_given);

    /* restore original output */
    *output = output_given;

    if (output->intern) *output->intern = output_intern_given;
  }

  if (ret)
  {
    SCDC_TRACE("scdc_dataset_cmd_intern: successful");

/*    SCDC_INFO("scdc_dataset_cmd: timing: " << cmd_timing << ", input: " << ((input)?input->total_size:0) / cmd_timing * 1e-6 << " MB/s, output: " << ((output)?output->total_size:0) / cmd_timing * 1e-6 << " MB/s");*/

  } else SCDC_TRACE("scdc_dataset_cmd_intern: failed: '" << SCDC_DATASET_OUTPUT_STR(output) << "'");

  if (!dataset->dataset)
  {
    scdc_main_context.data->nodeconns.close(dataset->nodeconn);

    delete dataset;
    dataset = SCDC_DATASET_NULL;
  }

  if (ret) SCDC_INFO("using dataset " << dataset << " to execute command '" << cmd << "'");

  return (ret)?SCDC_SUCCESS:SCDC_FAILURE;
}


scdcint_t scdc_dataset_cmd(scdc_dataset_t dataset, const char *cmd, scdc_dataset_input_t *input, scdc_dataset_output_t *output, ...)
{
  SCDC_TRACE("scdc_dataset_cmd: dataset: '" << dataset << "', cmd = '" << string(cmd) << "'" << ", input = " << static_cast<void *>(input) << ", output = " << static_cast<void *>(output));

  scdc_args_t args;

  va_list ap;
  va_start(ap, output);

  scdc_args_va_init(&args, &ap);

  scdcint_t ret = scdc_dataset_cmd_intern(dataset, cmd, input, output, &args);

  scdc_args_va_release(&args);

  va_end(ap);

  SCDC_TRACE("scdc_dataset_cmd: return '" << ret << "'");

  return ret;
}


#undef SCDC_LOG_PREFIX
