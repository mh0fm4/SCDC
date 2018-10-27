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


#include <cstdarg>
#include <string>
#include <sstream>
#include <set>

#define SCDC_TRACE_NOT  !SCDC_TRACE_LIBSCDC

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


const scdcint_t SCDC_USE_ZLIB = USE_ZLIB;
const scdcint_t SCDC_USE_MYSQL = USE_MYSQL;
const scdcint_t SCDC_USE_MPI = USE_MPI;
const scdcint_t SCDC_USE_WEBDAV = USE_WEBDAV;
const scdcint_t SCDC_USE_NFS = USE_NFS;


#define SCDC_LOG_PREFIX  "libscdc: "


typedef struct _scdc_args_data_va_t
{
  scdcint_t arg;
  va_list *ap;
  
} scdc_args_data_va_t;


static void *scdc_dataprov_hook_open_intern(void *intern_data, const char *conf, scdc_args_t *args, scdcint_t *ret)
{
  SCDC_TRACE(__func__ << ": intern_data: " << intern_data << ", conf: '" << conf << "', args: " << args);

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

  SCDC_TRACE(__func__ << ": data: '" << args_data_va << "' (arg: '" << args_data_va->arg << "', ap: '" << args_data_va->ap << "'), type: '" << type << "', v: " << v);

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

  SCDC_TRACE(__func__ << ": error: unknown type '" << type << "'");

  return SCDC_ARG_REF_NULL;
}


static scdc_arg_ref_t scdc_args_set_va(void *data, scdcint_t type, void *v, scdc_arg_ref_t arg_ref)
{
  SCDC_TRACE_DECL(scdc_args_data_va_t *args_data_va = static_cast<scdc_args_data_va_t *>(data);)

  SCDC_TRACE(__func__ << ": data: '" << args_data_va << "' (arg: '" << ((args_data_va)?args_data_va->arg:-1) << "', ap: '" << ((args_data_va)?args_data_va->ap:0) << "'), type: '" << type << "', v: '" << v << "', arg_ref: '" << arg_ref << "'");

  return SCDC_ARG_REF_NULL;
}


void scdc_args_free_va(void *data, scdcint_t type, void *v, scdc_arg_ref_t arg_ref)
{
  SCDC_TRACE_DECL(scdc_args_data_va_t *args_data_va = static_cast<scdc_args_data_va_t *>(data);)

  SCDC_TRACE(__func__ << ": data: '" << args_data_va << "' (arg: '" << ((args_data_va)?args_data_va->arg:-1) << "', ap: '" << ((args_data_va)?args_data_va->ap:0) << "'), type: '" << type << "', v: '" << v << "', arg_ref: '" << arg_ref << "'");
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

  scdc_result_t *result;

#if SCDC_DEBUG
  set<scdc_dataset_t> datasets;
#endif
  scdc_dataprov_pool dataprovs;
  scdc_nodeport_pool nodeports;
  scdc_nodeconn_pool nodeconns;

  scdc_dataprov_t dp_config;
  scdc_nodeport_t np_direct;
};


static scdcint_t scdc_main_config_hook_config(void *dataprov, const char *cmd, const char *param, const char *val, scdcint_t val_size, scdc_result_t *result);

const static scdc_dataprov_hook_t scdc_main_config_hook = {
  0, 0,  /* open, close */
  scdc_main_config_hook_config,  /* config */
  0, 0,  /* dataset_open, dataset_close */
  0, 0,  /* dataset_open_read_state, dataset_close_write_state */
  0,     /* dataset_cmd */
};


static bool scdc_main_context_destroyed = false;

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
      scdc_main_context_destroyed = true;
    }

    bool init(const char *conf, scdc_args_t *args)
    {
      SCDC_TRACE("scdc-context: " << __func__ << ": '" << conf << "'");

      SCDC_TRACE("scdc-context: " << __func__ << ": data: " << data);

      if (data) return false;

      scdcint_t result_size = SCDC_RESULT_STR_MIN_SIZE;
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
          SCDC_TRACE("scdc-context: " << __func__ << ": unknown parameter '" << c << "'");
        }
      }

      data = new _scdc_context_data_t;

      data->result = static_cast<scdc_result_t *>(::operator new(SCDC_RESULT_EXTENT(result_size)));
      SCDC_RESULT_SET_SIZE(data->result, result_size);
      SCDC_RESULT_SET_EMPTY(data->result);

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
      SCDC_TRACE("scdc-context: " << __func__);

      if (!data) return;

#if SCDC_DEBUG
      for (set<scdc_dataset_t>::iterator i = data->datasets.begin(); i != data->datasets.end(); ++i)
      {
        SCDC_FAIL("scdc-context: " << __func__ << ": dataset '" << *i << "' was not closed");
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

      ::operator delete(data->result);

      delete data; data = 0;

      SCDC_TRACE("scdc-context: " << __func__ << ": return");
    }
};


static scdc_context scdc_main_context;
scdc_log *scdc_main_context_log = &scdc_main_context.log;


static scdcint_t scdc_main_config_hook_config(void *dataprov, const char *cmd, const char *param, const char *val, scdcint_t val_size, scdc_result_t *result)
{
  SCDC_TRACE(__func__ << ": cmd: '" << cmd << "', param: '" << param << "', val: " << string(val, val_size));

  string c(cmd), p(param);

  bool done = false;
  bool ret = true;

  scdc_config_result res;

  if (p == "")
  {
    if (c == "info")
    {
      done = true;

      ret = scdc_dataprov_config::info(res, "libscdc configuration");

    } else if (c == "ls")
    {
      done = true;

      ret = scdc_dataprov_config::ls(res, "dataprovs")
         && scdc_dataprov_config::ls(res, "nodeports")
         && scdc_dataprov_config::ls(res, "nodeconns")
#if NODECONN_CACHE_CONNECTIONS
         && scdc_dataprov_config::ls(res, "nodeconn_cache_connections")
#endif
         && true;

    } else done = false;

  } else
  {
    if (done);
    if (c == "info")
    {
      done = true;

      if (p == "dataprovs") ret = scdc_dataprov_config::info(res, param, "data providers");
      else if (p == "nodeports") ret = scdc_dataprov_config::info(res, param, "node ports");
      else if (p == "nodeports") ret = scdc_dataprov_config::info(res, param, "node connections");
#if NODECONN_CACHE_CONNECTIONS
      else if (p == "nodeconn_cache_connections") ret = scdc_dataprov_config::info(res, param, "number of node connections to cache (integer)");
#endif
      else done = false;

/*    } else if (c == "ls")
    {*/
    } else if (c == "put")
    {
      done = true;

/*      if (p == "dataprovs")  ret = scdc_dataprov_config::put<>(res, param);
      else if (p == "nodeports")  ret = scdc_dataprov_config::put<>(res, param);
      else if (p == "nodeconns")  ret = scdc_dataprov_config::put<>(res, param);*/
      if (p == "");
#if NODECONN_CACHE_CONNECTIONS
      else if (p == "nodeconn_cache_connections") ret = scdc_dataprov_config::put<scdcint_t>(res, param, val, data->nodeconns.cache_connections);
#endif
      else done = false;

    } else if (c == "get")
    {
      done = true;

      if (p == "dataprovs")  ret = scdc_dataprov_config::get<size_t>(res, param, scdc_main_context.data->dataprovs.size());
      else if (p == "nodeports")  ret = scdc_dataprov_config::get<size_t>(res, param, scdc_main_context.data->nodeports.size());
      else if (p == "nodeconns")  ret = scdc_dataprov_config::get<size_t>(res, param, scdc_main_context.data->nodeconns.size());
#if NODECONN_CACHE_CONNECTIONS
      else if (p == "nodeconn_cache_connections") ss << scdc_main_context.data->nodeconns.cache_connections;
#endif
      else done = false;
    }
  }

  if (!done || (done && !ret))
  {
    SCDC_FAIL(__func__ << ": command '" << cmd << "' failed");
    return SCDC_FAILURE;
  }

  if (result) SCDC_RESULT_SET_STR(result, res.c_str());

  return ret;
}


scdcint_t scdc_init_intern(const char *conf, scdc_args_t *args)
{
  SCDC_TRACE(__func__ << ": conf: '" << conf << "'");

  scdcint_t ret = (scdc_main_context.init(conf, args))?SCDC_SUCCESS:SCDC_FAILURE;

  SCDC_TRACE(__func__ << ": return: '" << ret << "'");

  return ret;
}


scdcint_t scdc_init(const char *conf, ...)
{
  SCDC_TRACE(__func__ << ": conf: '" << conf << "'");

  scdc_args_t args;

  va_list ap;
  va_start(ap, conf);

  scdc_args_va_init(&args, &ap);

  scdcint_t ret = scdc_init_intern(conf, &args);

  scdc_args_va_release(&args);

  va_end(ap);

  SCDC_TRACE(__func__ << ": return '" << ret << "'");

  return ret;
}


void scdc_release_intern()
{
  SCDC_TRACE(__func__ << ":");

  scdc_main_context.release();

  SCDC_TRACE(__func__ << ": return");
}


void scdc_release()
{
  SCDC_TRACE(__func__ << ":");

  scdc_release_intern();

  SCDC_TRACE(__func__ << ": return");

  scdc_log_release_intern();
}


const char *scdc_last_result()
{
  SCDC_TRACE(__func__ << ":");

  const char *result = SCDC_RESULT_STR(scdc_main_context.data->result);

  SCDC_TRACE(__func__ << ": return: " << static_cast<void *>(result));

  return result;
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
  SCDC_TRACE(__func__ << ": conf: '" << conf << "'");

  bool ret = SCDC_MAIN_LOG()->init(conf, args);

  SCDC_TRACE(__func__ << ": return");

  return (ret)?SCDC_SUCCESS:SCDC_FAILURE;
}


scdcint_t scdc_log_init(const char *conf, ...)
{
  SCDC_TRACE(__func__ << ": conf: '" << conf << "'");

  scdc_args_t args;

  va_list ap;
  va_start(ap, conf);

  scdc_args_va_init(&args, &ap);

  scdcint_t ret = scdc_log_init_intern(conf, &args);

  scdc_args_va_release(&args);

  va_end(ap);

  SCDC_TRACE(__func__ << ": return '" << ret << "'");

  return ret;
}


void scdc_log_release_intern()
{
  SCDC_TRACE(__func__ << ":");

  SCDC_MAIN_LOG()->release();

  SCDC_TRACE(__func__ << ": return");
}


void scdc_log_release()
{
  SCDC_TRACE(__func__ << ":");

  scdc_log_release_intern();
  
  SCDC_TRACE(__func__ << ": return");
}


scdc_dataprov_t scdc_dataprov_open_intern(const char *base_path, const char *conf, scdc_args_t *args)
{
  scdc_args xargs(args);

  scdc_result result;

  scdc_dataprov *dataprov = scdc_main_context.data->dataprovs.open(base_path, conf, &xargs, result);

  SCDC_RESULT_SET_STR(scdc_main_context.data->result, result.c_str());

  if (!dataprov) return SCDC_DATAPROV_NULL;

  return static_cast<scdc_dataprov_t>(dataprov);
}


scdc_dataprov_t scdc_dataprov_open(const char *base_path, const char *conf, ...)
{
  SCDC_TRACE(__func__ << ":");

  va_list ap;
  va_start(ap, conf);

  scdc_args_t args;

  scdc_args_va_init(&args, &ap);

  scdc_dataprov_t dataprov = scdc_dataprov_open_intern(base_path, conf, &args);

  scdc_args_va_release(&args);
  va_end(ap);

  SCDC_TRACE(__func__ << ": return '" << dataprov << "'");

  return dataprov;
}


void scdc_dataprov_close(scdc_dataprov_t dataprov)
{
  SCDC_TRACE(__func__ << ": dataprov: '" << dataprov << "'");

  if (dataprov == SCDC_DATAPROV_NULL) return;

  scdc_result result;

  if (!scdc_main_context_destroyed) scdc_main_context.data->dataprovs.close(static_cast<scdc_dataprov *>(dataprov), result);

  SCDC_RESULT_SET_STR(scdc_main_context.data->result, result.c_str());

  SCDC_TRACE(__func__ << ": return");
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
  SCDC_TRACE(__func__ << ": conf: '" << conf << "'");

  va_list ap;
  va_start(ap, conf);

  scdc_args_t args;

  scdc_args_va_init(&args, &ap);

  scdc_nodeport_t nodeport = scdc_nodeport_open_intern(conf, &args);

  scdc_args_va_release(&args);
  va_end(ap);

  SCDC_TRACE(__func__ << ": return '" << nodeport << "'");

  return nodeport;
}


void scdc_nodeport_close(scdc_nodeport_t nodeport)
{
  SCDC_TRACE(__func__ << ": nodeport: '" << nodeport << "'");

  if (nodeport == SCDC_NODEPORT_NULL) return;

  if (!scdc_main_context_destroyed) scdc_main_context.data->nodeports.close(static_cast<scdc_nodeport *>(nodeport));

  SCDC_TRACE(__func__ << ": return");
}


scdcint_t scdc_nodeport_start(scdc_nodeport_t nodeport, scdcint_t mode)
{
  SCDC_TRACE(__func__ << ": nodeport: '" << nodeport << "', mode: '" << mode << "'");

  if (nodeport == SCDC_NODEPORT_NULL) return SCDC_FAILURE;

  static_cast<scdc_nodeport *>(nodeport)->set_dataprovs(&scdc_main_context.data->dataprovs);

  static_cast<scdc_nodeport *>(nodeport)->start(mode);

  SCDC_TRACE(__func__ << ": return");

  return SCDC_SUCCESS;
}


scdcint_t scdc_nodeport_stop(scdc_nodeport_t nodeport)
{
  SCDC_TRACE(__func__ << ": nodeport: '" << nodeport << "'");

  if (nodeport == SCDC_NODEPORT_NULL) return SCDC_FAILURE;

  static_cast<scdc_nodeport *>(nodeport)->stop();

  static_cast<scdc_nodeport *>(nodeport)->set_dataprovs(0);

  SCDC_TRACE(__func__ << ": return");

  return SCDC_SUCCESS;
}


scdcint_t scdc_nodeport_cancel(scdc_nodeport_t nodeport, scdcint_t interrupt)
{
  SCDC_TRACE(__func__ << ": nodeport: '" << nodeport << "'");

  if (nodeport == SCDC_NODEPORT_NULL) return SCDC_FAILURE;

  static_cast<scdc_nodeport *>(nodeport)->cancel(interrupt);

  SCDC_TRACE(__func__ << ": return");

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
  SCDC_TRACE(__func__ << ": conf: '" << conf << "'");

  va_list ap;
  va_start(ap, conf);

  scdc_args_t args;

  scdc_args_va_init(&args, &ap);

  const char *authority = scdc_nodeport_authority_intern(conf, &args);

  scdc_args_va_release(&args);
  va_end(ap);

  SCDC_TRACE(__func__ << ": return '" << authority << "'");

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
  SCDC_TRACE(__func__ << ": uri: '" << uri << "'");

  va_list ap;
  va_start(ap, uri);

  scdc_args_t args;

  scdc_args_va_init(&args, &ap);

  scdcint_t supported = scdc_nodeport_supported_intern(uri, &args);

  scdc_args_va_release(&args);
  va_end(ap);

  SCDC_TRACE(__func__ << ": return '" << supported << "'");

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


static scdcint_t pointer2scdcint(void *p)
{
  union { scdcint_t i; void *p; } u = { .i = 0 };

  u.p = p;

  return u.i;
}


scdcint_t scdc_dataset_inout_next_hash(scdc_dataset_inout_t *inout)
{
  SCDC_TRACE_DATASET_INPUT(inout, __func__ << ": inout: ");

  if (!inout) return 0;

  SCDC_TRACE(__func__ << ": next: " << reinterpret_cast<void *>(inout->next));
  SCDC_TRACE(__func__ << ": data: " << static_cast<void *>(inout->data));
  SCDC_TRACE(__func__ << ": intern_data: " << static_cast<void *>(inout->intern_data));

  scdcint_t next_hash = pointer2scdcint(reinterpret_cast<void *>(inout->next)) ^ pointer2scdcint(inout->data) ^ pointer2scdcint(inout->intern_data);

  SCDC_TRACE(__func__ << ": next_hash: " << next_hash << " (" << pointer2scdcint(reinterpret_cast<void *>(inout->next)) << " + " << pointer2scdcint(inout->data) << " + " << pointer2scdcint(inout->intern_data) << ")");

  return next_hash;
}


scdc_dataset_t scdc_dataset_open_intern(const char *uri, scdc_args_t *args)
{
  SCDC_TRACE(__func__ << ": uri: '" << uri << "'");

  scdc_dataset_t dataset = new _scdc_dataset_t;

  string sscheme, sauthority, spath;
  split_uri(uri, -1, &sscheme, &sauthority, &spath);

  scdc_args xargs(args);

  dataset->nodeconn = scdc_main_context.data->nodeconns.open(sscheme.c_str(), sauthority.c_str(), &xargs);

  if (!dataset->nodeconn)
  {
    SCDC_TRACE(__func__ << ": failed: open connection failed");
    delete dataset;
    return SCDC_DATASET_NULL;
  }

  scdc_result result;
  dataset->dataset = dataset->nodeconn->dataset_open(spath, result);
  SCDC_RESULT_SET_STR(scdc_main_context.data->result, result.c_str());

  SCDC_TRACE(__func__ << ": result: '" << result << "'");

  if (!dataset->dataset)
  {
    SCDC_FAIL(__func__ << ": failed");
    scdc_main_context.data->nodeconns.close(dataset->nodeconn);
    delete dataset;
    return SCDC_DATASET_NULL;
  }

#if SCDC_DEBUG
  scdc_main_context.data->datasets.insert(dataset);
#endif

  SCDC_TRACE(__func__ << ": return '" << dataset << "'");

/*  SCDC_INFO("opening dataset " << dataset << " with uri '" << uri << "'");*/

  return dataset;
}


scdc_dataset_t scdc_dataset_open(const char *uri, ...)
{
  SCDC_TRACE(__func__ << ": uri: '" << uri << "'");

  scdc_args_t args;

  va_list ap;
  va_start(ap, uri);

  scdc_args_va_init(&args, &ap);

  scdc_dataset_t dataset = scdc_dataset_open_intern(uri, &args);

  scdc_args_va_release(&args);

  va_end(ap);

  SCDC_TRACE(__func__ << ": return '" << dataset << "'");

  return dataset;
}


void scdc_dataset_close(scdc_dataset_t dataset)
{
  SCDC_TRACE(__func__ << ": '" << dataset << "'");

  if (dataset == SCDC_DATASET_NULL) return;

/*  SCDC_INFO("closing dataset " << dataset);*/

#if SCDC_DEBUG
  if (scdc_main_context.data->datasets.erase(dataset) != 1)
  {
    SCDC_FAIL(__func__ << ": no valid dataset handle given");
    return;
  }
#endif

  scdc_result result;
  dataset->nodeconn->dataset_close(static_cast<scdc_dataset *>(dataset->dataset), result);
  SCDC_RESULT_SET_STR(scdc_main_context.data->result, result.c_str());

  SCDC_TRACE(__func__ << ": result: '" << result << "'");

  scdc_main_context.data->nodeconns.close(dataset->nodeconn);

  delete dataset;

  SCDC_TRACE(__func__ << ": return");
}


scdcint_t scdc_dataset_cmd_intern(scdc_dataset_t dataset, const char *cmd, scdc_dataset_input_t *input, scdc_dataset_output_t *output, scdc_args_t *args)
{
  SCDC_TRACE(__func__ << ": dataset: '" << dataset << "', cmd = '" << string(cmd) << "'" << ", input = " << static_cast<void *>(input) << ", output = " << static_cast<void *>(output));

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
      SCDC_RESULT_SET_STR(scdc_main_context.data->result, "no URI given");
      SCDC_FAIL(__func__ << ": " << SCDC_RESULT_STR(scdc_main_context.data->result));
      delete dataset;
      return SCDC_FAILURE;
    }

    string sscheme, sauthority, spath;
    split_uri(suri.c_str(), suri.size(), &sscheme, &sauthority, &spath);

    scdc_args xargs(args);

    dataset->nodeconn = scdc_main_context.data->nodeconns.open(sscheme.c_str(), sauthority.c_str(), &xargs);

    if (!dataset->nodeconn)
    {
      SCDC_RESULT_SET_STR(scdc_main_context.data->result, "open connection failed");
      SCDC_FAIL(__func__ << ": " << SCDC_RESULT_STR(scdc_main_context.data->result));
      delete dataset;
      return SCDC_FAILURE;
    }

    join_uri(0, 0, spath.c_str(), suri);
    suri = ":" + suri;

    join_cmdline(scmd.c_str(), suri.c_str(), sparams.c_str(), adhoc_cmd);

    cmd = adhoc_cmd.c_str();
    cmd_size = adhoc_cmd.size();
  }

  SCDC_TRACE(__func__ << ": adhoc command: '" << adhoc_cmd << "'");

  scdc_dataset_input_t input_null = *SCDC_DATASET_INPUT_NONE;
  if (!input) input = &input_null;

  scdc_dataset_output_t output_null = *SCDC_DATASET_OUTPUT_ENDL;
  if (!output) output = &output_null;

  /* backup original output */
  scdc_dataset_output_t output_given = *output;
  scdc_dataset_inout_intern_t output_intern_given = (output->intern)?*output->intern:SCDC_DATASET_OUTPUT_INTERN_NULL;

  SCDC_TRACE_DATASET_INPUT(input, "scdc_dataset_cmd_intern: IN input: ");
  SCDC_TRACE_DATASET_OUTPUT(output, "scdc_dataset_cmd_intern: IN output: ");

  scdcint_t output_next_hash_in = scdc_dataset_inout_next_hash(output);

  bool ret;
  scdc_result result;

  double cmd_timing = z_time_wtime();

  if (dataset->dataset) ret = dataset->dataset->do_cmd(string(cmd, cmd_size), input, output, result);
  else ret = dataset->nodeconn->dataset_cmd(string(cmd, cmd_size), input, output, result);

  cmd_timing = z_time_wtime() - cmd_timing;
  SCDC_TRACE(__func__ << ": timing: " << cmd_timing);

  SCDC_TRACE(__func__ << ": result: '" << result << "'");

  SCDC_RESULT_SET_STR(scdc_main_context.data->result, result.c_str());

  scdcint_t output_next_hash_out = scdc_dataset_inout_next_hash(output);

  SCDC_TRACE_DATASET_INPUT(input, "scdc_dataset_cmd_intern: OUT input: ");
  SCDC_TRACE_DATASET_OUTPUT(output, "scdc_dataset_cmd_intern: OUT output: ");

  /* if there was an output next given, but the resulting cmd output is different */
  if (output_given.next && output_next_hash_in != output_next_hash_out)
  {
    SCDC_TRACE(__func__ << ": redirecting cmd output to given output");
    scdc_dataset_output_redirect(output, "to:outputsink", &output_given);

    /* restore original output */
    *output = output_given;

    if (output->intern) *output->intern = output_intern_given;
  }

  if (ret)
  {
    SCDC_TRACE(__func__ << ": successful");

/*    SCDC_INFO("scdc_dataset_cmd: timing: " << cmd_timing << ", input: " << ((input)?input->total_size:0) / cmd_timing * 1e-6 << " MB/s, output: " << ((output)?output->total_size:0) / cmd_timing * 1e-6 << " MB/s");*/

  } else SCDC_TRACE(__func__ << ": failed: '" << SCDC_DATASET_OUTPUT_STR(output) << "'");

  if (!dataset->dataset)
  {
    scdc_main_context.data->nodeconns.close(dataset->nodeconn);

    delete dataset;
    dataset = SCDC_DATASET_NULL;
  }

/*  if (ret) SCDC_INFO("using dataset " << dataset << " to execute command '" << cmd << "'");*/

  return (ret)?SCDC_SUCCESS:SCDC_FAILURE;
}


scdcint_t scdc_dataset_cmd(scdc_dataset_t dataset, const char *cmd, scdc_dataset_input_t *input, scdc_dataset_output_t *output, ...)
{
  SCDC_TRACE(__func__ << ": dataset: '" << dataset << "', cmd = '" << string(cmd) << "'" << ", input = " << static_cast<void *>(input) << ", output = " << static_cast<void *>(output));

  scdc_args_t args;

  va_list ap;
  va_start(ap, output);

  scdc_args_va_init(&args, &ap);

  scdcint_t ret = scdc_dataset_cmd_intern(dataset, cmd, input, output, &args);

  scdc_args_va_release(&args);

  va_end(ap);

  SCDC_TRACE(__func__ << ": return '" << ret << "'");

  return ret;
}


#undef SCDC_LOG_PREFIX
