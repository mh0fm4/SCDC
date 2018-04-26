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


#include "z_pack.h"

#if SCDC_TRACE_NOT
# define SCDC_TRACE_NOT_BACKUP  1
#else
# define SCDC_TRACE_NOT_BACKUP  0
#endif /* SCDC_TRACE_NOT */
#undef SCDC_TRACE_NOT
#include "log_unset.hh"

#define SCDC_TRACE_NOT  !SCDC_TRACE_DATAPROV_STORE

#include "config.hh"
#include "common.hh"
#include "log.hh"
#include "dataprov_store.hh"


using namespace std;


#define SCDC_LOG_PREFIX  "dataset-store: "

template<class STORE_HANDLER>
scdc_dataset_store<STORE_HANDLER>::scdc_dataset_store(scdc_dataprov *dataprov_)
  :scdc_dataset(dataprov_), store(STORE_HANDLER::store_null), entry(STORE_HANDLER::entry_null), admin(false)
{
  do_cmd_get_next_data.dp = static_cast<scdc_dataprov_store<STORE_HANDLER> *>(dataprov_);
  do_cmd_get_next_data.store = STORE_HANDLER::store_null;
  do_cmd_get_next_data.entry = STORE_HANDLER::entry_null;
  do_cmd_get_next_data.pos = -1;
  do_cmd_get_next_data.size = -1;
  do_cmd_get_next_data.buf = 0;
  do_cmd_get_next_data.buf_size = -1;
}


template<class STORE_HANDLER>
scdc_dataset_store<STORE_HANDLER>::~scdc_dataset_store()
{
  scdc_dataprov_store<STORE_HANDLER> *dp =  static_cast<scdc_dataprov_store<STORE_HANDLER> *>(dataprov);

  if (entry != STORE_HANDLER::entry_null) dp->entry_close(store, entry);
  if (store != STORE_HANDLER::store_null) dp->store_close(store);

  ::operator delete(do_cmd_get_next_data.buf);
}


template<class STORE_HANDLER>
bool scdc_dataset_store<STORE_HANDLER>::do_cmd_info(const std::string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output)
{
  SCDC_TRACE(__func__ << ": params: '" << params << "'");

  scdc_dataprov_store<STORE_HANDLER> *dp =  static_cast<scdc_dataprov_store<STORE_HANDLER> *>(dataprov);

  bool ret = false;
  std::string result;

  if (admin || store == STORE_HANDLER::store_null)
  {
    std::string path = params;

    if (!dp->info_store(path.c_str(), result))
    {
      SCDC_DATASET_OUTPUT_PRINTF(output, "getting info of store '%s' failed: '%s'", path.c_str(), result.c_str());
      goto do_return;
    }

  } else
  {
    std::string path = params;

    if (!dp->info_entry(store, path.c_str(), result))
    {
      SCDC_DATASET_OUTPUT_PRINTF(output, "getting info of entry '%s' failed: '%s'", path.c_str(), result.c_str());
      goto do_return;
    }
  } 

  ret = true;
  SCDC_DATASET_OUTPUT_PRINTF(output, result.c_str());

do_return:
  SCDC_TRACE(__func__ << ": return: " << ret);

  return ret;
}


template<class STORE_HANDLER>
bool scdc_dataset_store<STORE_HANDLER>::do_cmd_cd(const std::string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output)
{
  SCDC_TRACE(__func__ << ": params: '" << params << "'");

  scdc_dataprov_store<STORE_HANDLER> *dp =  static_cast<scdc_dataprov_store<STORE_HANDLER> *>(dataprov);

  bool ret = false;

  admin = false;

  if (store != STORE_HANDLER::store_null) dp->store_close(store);
  store = STORE_HANDLER::store_null;

  if (params == "ADMIN")
  {
    admin = true;
    ret = true;
    goto do_return;
  }

  if (params.empty())
  {
    ret = true;
    goto do_return;
  }

  store = dp->store_open(params.c_str());
  if (store != STORE_HANDLER::store_null)
  {
    ret = true;
    goto do_return;
  }

  ret = scdc_dataset::do_cmd_cd(params, input, output);

do_return:
  SCDC_TRACE(__func__ << ": return: " << ret);

  return ret;
}


template<class STORE_HANDLER>
bool scdc_dataset_store<STORE_HANDLER>::do_cmd_ls(const std::string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output)
{
  SCDC_TRACE(__func__ << ": params: '" << params << "'");

  scdc_dataprov_store<STORE_HANDLER> *dp =  static_cast<scdc_dataprov_store<STORE_HANDLER> *>(dataprov);

  bool ret = false;
  std::string result;

  if (admin || store == STORE_HANDLER::store_null)
  {
    if (!dp->ls_stores(result))
    {
      SCDC_DATASET_OUTPUT_PRINTF(output, "listing stores failed: '%s'", result.c_str());
      goto do_return;
    }

  } else
  {
    if (!dp->ls_entries(store, result))
    {
      SCDC_DATASET_OUTPUT_PRINTF(output, "listing entries failed: '%s'", result.c_str());
      goto do_return;
    }
  } 

  ret = true;
  SCDC_DATASET_OUTPUT_PRINTF(output, result.c_str());

do_return:
  SCDC_TRACE(__func__ << ": return: " << ret);

  return ret;
}


template<class STORE_HANDLER>
bool scdc_dataset_store<STORE_HANDLER>::do_cmd_put(const std::string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output)
{
  SCDC_TRACE(__func__ << ": params: '" << params << "'");

  scdc_dataprov_store<STORE_HANDLER> *dp =  static_cast<scdc_dataprov_store<STORE_HANDLER> *>(dataprov);

  bool ret = false;

  if (admin || store == STORE_HANDLER::store_null)
  {
    std::string path = params;

    if (!dp->mk_store(path.c_str()))
    {
      SCDC_DATASET_OUTPUT_PRINTF(output, "making store '%s' failed", path.c_str());
      goto do_return;
    }

  } else
  {
    std::string path = params;
    entry = dp->entry_reopen(store, path.c_str(), false, true, true, entry);

    scdcint_t pos = -1;
    scdcint_t size = -1;

    if (size != 0)
    while (1)
    {
      scdcint_t s = SCDC_DATASET_INOUT_BUF_CURRENT(input);
      if (size >= 0) s = std::min(s, size);

      scdcint_t n = dp->entry_write_at(store, entry, SCDC_DATASET_INOUT_BUF_PTR(input), s, pos);

      if (n < 0)
      {
        SCDC_DATASET_OUTPUT_PRINTF(output, "writing to entry '%s' failed", path.c_str());
        goto do_return;
      }

      if (n < s)
      {
        SCDC_DATASET_OUTPUT_PRINTF(output, "writing all data to entry '%s' failed", path.c_str());
        goto do_return;
      }

      if (pos >= 0) pos += n;
      if (size >= 0) size -= n;

      if (size == 0 || !input->next) break;

      if (input->next(input) != SCDC_SUCCESS)
      {
        SCDC_DATASET_OUTPUT_PRINTF(output, "getting input data failed");
        goto do_return;
      }
    }
  }

  ret = true;

do_return:
  SCDC_TRACE(__func__ << ": return: " << ret);

  return ret;
}


template<class STORE_HANDLER>
static scdcint_t scdc_dataset_store_do_cmd_get_next(scdc_dataset_inout_t *inout)
{
  scdcint_t ret = SCDC_FAILURE;

  scdc_dataset_store_do_cmd_get_next_data_t<STORE_HANDLER> *do_cmd_get_next_data = static_cast<scdc_dataset_store_do_cmd_get_next_data_t<STORE_HANDLER> *>(inout->data);

  scdcint_t s = do_cmd_get_next_data->buf_size;
  if (do_cmd_get_next_data->size >= 0) s = std::min(s, do_cmd_get_next_data->size);

  scdcint_t n = -1;
  scdc_buf_t buf;

  if (do_cmd_get_next_data->dp->entry_read_access_at(do_cmd_get_next_data->store, do_cmd_get_next_data->entry, s, do_cmd_get_next_data->pos, buf))
  {
    SCDC_DATASET_INOUT_BUF_SET_P(inout, buf.ptr);
    SCDC_DATASET_INOUT_BUF_SET_S(inout, buf.size);
    n = buf.current;
    
  } else
  {
    if (!SCDC_DATASET_INOUT_BUF_PTR(inout))
    {
      if (!do_cmd_get_next_data->buf)
      {
        do_cmd_get_next_data->buf_size = 1024;
        do_cmd_get_next_data->buf = ::operator new(do_cmd_get_next_data->buf_size);
      }
      
      SCDC_DATASET_INOUT_BUF_SET(inout, do_cmd_get_next_data->buf, do_cmd_get_next_data->buf_size, 0);
    }
    
    n = do_cmd_get_next_data->dp->entry_read_at(do_cmd_get_next_data->store, do_cmd_get_next_data->entry, SCDC_DATASET_INOUT_BUF_PTR(inout), s, do_cmd_get_next_data->pos);
  }

  if (n < 0)
  {
    SCDC_DATASET_OUTPUT_PRINTF(inout, "reading from entry '%s' failed", "<unknown>");
    goto do_return;
  }

  SCDC_DATASET_INOUT_BUF_SET_C(inout, n);

  if (do_cmd_get_next_data->pos >= 0) do_cmd_get_next_data->pos += n;
  if (do_cmd_get_next_data->size >= 0) do_cmd_get_next_data->size -= n;

  inout->next = (n < s)?scdc_dataset_store_do_cmd_get_next<STORE_HANDLER>:0;

  ret = SCDC_SUCCESS;

do_return:

  return ret;
}


template<class STORE_HANDLER>
bool scdc_dataset_store<STORE_HANDLER>::do_cmd_get(const std::string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output)
{
  SCDC_TRACE(__func__ << ": params: '" << params << "'");

  scdc_dataprov_store<STORE_HANDLER> *dp =  static_cast<scdc_dataprov_store<STORE_HANDLER> *>(dataprov);

  bool ret = false;

  if (admin || store == STORE_HANDLER::store_null)
  {
    SCDC_DATASET_OUTPUT_PRINTF(output, "command not available for a store");
    goto do_return;

  } else
  {
    std::string path = params;
    entry = dp->entry_reopen(store, path.c_str(), false, true, true, entry);

    do_cmd_get_next_data.store = store;
    do_cmd_get_next_data.entry = entry;
    do_cmd_get_next_data.pos = -1;
    do_cmd_get_next_data.size = -1;

    scdc_dataset_output_t output_local;

    output_local.data = &do_cmd_get_next_data;
    output_local.next = scdc_dataset_store_do_cmd_get_next<STORE_HANDLER>;

    if (do_cmd_get_next_data.size != 0)
    while (output_local.next)
    {
      SCDC_DATASET_INOUT_BUF_ASSIGN(&output_local, output);

      if (output_local.next(&output_local) != SCDC_SUCCESS)
      {
        SCDC_DATASET_OUTPUT_PRINTF(output, "getting output data failed: %.*s", (int) SCDC_DATASET_INOUT_BUF_CURRENT(&output_local), SCDC_DATASET_INOUT_BUF_PTR(&output_local));
        goto do_return;
      }

      SCDC_DATASET_INOUT_BUF_ASSIGN(output, &output_local);

      if (do_cmd_get_next_data.size == 0 || !output->next) break;

      if (output->next(output) != SCDC_SUCCESS)
      {
        SCDC_DATASET_OUTPUT_PRINTF(output, "setting output data failed");
        goto do_return;
      }
    }

    output->data = output_local.data;
    output->next = output_local.next;
  }

  ret = true;

do_return:
  SCDC_TRACE(__func__ << ": return: " << ret);

  return ret;
}


template<class STORE_HANDLER>
bool scdc_dataset_store<STORE_HANDLER>::do_cmd_rm(const std::string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output)
{
  SCDC_TRACE(__func__ << ": params: '" << params << "'");

  scdc_dataprov_store<STORE_HANDLER> *dp =  static_cast<scdc_dataprov_store<STORE_HANDLER> *>(dataprov);

  bool ret = false;

  if (admin || store == STORE_HANDLER::store_null)
  {
    std::string path = params;

    if (!dp->rm_store(path.c_str()))
    {
      SCDC_DATASET_OUTPUT_PRINTF(output, "removing store '%s' failed", path.c_str());
      goto do_return;
    }

  } else
  {
    std::string path = params;

    if (dp->entry_match(store, entry, path.c_str()))
    {
      dp->entry_close(store, entry);
      entry = STORE_HANDLER::entry_null;
    }

    if (!dp->rm_entry(store, path.c_str()))
    {
      SCDC_DATASET_OUTPUT_PRINTF(output, "removing entry '%s' failed", path.c_str());
      goto do_return;
    }
  }

  ret = true;

do_return:
  SCDC_TRACE(__func__ << ": return: " << ret);

  return ret;
}

#undef SCDC_LOG_PREFIX

#define SCDC_LOG_PREFIX  "dataprov-store: "

template<class STORE_HANDLER>
bool scdc_dataprov_store<STORE_HANDLER>::open(const char *conf, scdc_args *args)
{
  SCDC_TRACE("open: conf: '" << conf << "'");

  bool ret = true;

  if (!scdc_dataprov::open(conf, args))
  {
    SCDC_FAIL("open: opening base");
    ret = false;
    goto do_return;
  }

  dataset_cmds_add("pwd", static_cast<dataset_cmds_do_cmd_f>(&scdc_dataset_store<STORE_HANDLER>::do_cmd_pwd));
  dataset_cmds_add("info", static_cast<dataset_cmds_do_cmd_f>(&scdc_dataset_store<STORE_HANDLER>::do_cmd_info));
  dataset_cmds_add("cd", static_cast<dataset_cmds_do_cmd_f>(&scdc_dataset_store<STORE_HANDLER>::do_cmd_cd));
  dataset_cmds_add("ls", static_cast<dataset_cmds_do_cmd_f>(&scdc_dataset_store<STORE_HANDLER>::do_cmd_ls));
  dataset_cmds_add("put", static_cast<dataset_cmds_do_cmd_f>(&scdc_dataset_store<STORE_HANDLER>::do_cmd_put));
  dataset_cmds_add("get", static_cast<dataset_cmds_do_cmd_f>(&scdc_dataset_store<STORE_HANDLER>::do_cmd_get));
  dataset_cmds_add("rm", static_cast<dataset_cmds_do_cmd_f>(&scdc_dataset_store<STORE_HANDLER>::do_cmd_rm));

do_return:
  SCDC_TRACE("open: ret: '" << ret << "'");

  return ret;
}


template<class STORE_HANDLER>
void scdc_dataprov_store<STORE_HANDLER>::close()
{
  SCDC_TRACE("close:");
}


template<class STORE_HANDLER>
scdc_dataset *scdc_dataprov_store<STORE_HANDLER>::dataset_open(const char *path, scdcint_t path_size, scdc_dataset_output_t *output)
{
  SCDC_TRACE("dataset_open: '" << std::string(path, path_size) << "'");

  scdc_dataset *dataset = 0;

  if (config_open(path, path_size, output, &dataset)) return dataset;
  
  scdc_dataset_store<STORE_HANDLER> *dataset_store = new scdc_dataset_store<STORE_HANDLER>(this);

  if (path && !dataset_store->do_cmd_cd(std::string(path, path_size), 0, output))
  {
    SCDC_FAIL("dataset_open: do_cmd_cd: failed: '" << SCDC_DATASET_OUTPUT_STR(output) << "'");
    delete dataset_store;
    return 0;
  }

  SCDC_TRACE("dataset_open: return: '" << dataset_store << "'");

  return dataset_store;
}


template<class STORE_HANDLER>
void scdc_dataprov_store<STORE_HANDLER>::dataset_close(scdc_dataset *dataset, scdc_dataset_output_t *output)
{
  SCDC_TRACE("dataset_close: '" << dataset << "'");

  if (config_close(dataset, output)) return;

  scdc_dataset_store<STORE_HANDLER> *dataset_store = static_cast<scdc_dataset_store<STORE_HANDLER> *>(dataset);

  delete dataset_store;

  SCDC_TRACE("dataset_close: return");
}


template<class STORE_HANDLER>
bool scdc_dataprov_store<STORE_HANDLER>::config_do_cmd_param(const std::string &cmd, const std::string &param, std::string val, scdc_config_result &result, bool &done)
{
  SCDC_TRACE("config_do_cmd_param: cmd: '" << cmd << "', param: '" << param << "', val: '" << val << "', result: '" << result << "'");
  
  return false;
}

#undef SCDC_LOG_PREFIX


#undef SCDC_TRACE_NOT
#if SCDC_TRACE_NOT_BACKUP
# define SCDC_TRACE_NOT  1
#else /* SCDC_TRACE_NOT_BACKUP */
# define SCDC_TRACE_NOT  0
#endif /* SCDC_TRACE_NOT_BACKUP */
#undef SCDC_TRACE_NOT_BACKUP
#include "log_unset.hh"
#include "log_set.hh"
