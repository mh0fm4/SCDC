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


#ifndef __DATAPROV_STORE_TCC__
#define __DATAPROV_STORE_TCC__


#include "z_pack.h"

#if SCDC_TRACE_NOT
# define DATAPROV_STORE_TCC_SCDC_TRACE_NOT_BACKUP  1
#else
# define DATAPROV_STORE_TCC_SCDC_TRACE_NOT_BACKUP  0
#endif /* SCDC_TRACE_NOT */
#undef SCDC_TRACE_NOT
#include "log_unset.hh"

#define SCDC_TRACE_NOT  !SCDC_TRACE_DATAPROV_STORE

#include "config.hh"
#include "common.hh"
#include "log.hh"
#include "dataprov_common.hh"
#include "dataprov_store.hh"


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
bool scdc_dataset_store<STORE_HANDLER>::do_cmd_cd(const std::string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output, scdc_result &result)
{
  SCDC_TRACE_F("params: '" << params << "'");

  scdc_dataprov_store<STORE_HANDLER> *dp =  static_cast<scdc_dataprov_store<STORE_HANDLER> *>(dataprov);

  bool ret = false;

  admin = false;

  if (entry != STORE_HANDLER::entry_null) dp->entry_close(store, entry);
  entry = STORE_HANDLER::entry_null;
  if (store != STORE_HANDLER::store_null) dp->store_close(store);
  store = STORE_HANDLER::store_null;

  const std::string path = params;

  if (path == "ADMIN")
  {
    admin = true;

  } else if (path.empty())
  {
    // select no store

  } else
  {
    store = dp->store_open(path.c_str());
    if (store == STORE_HANDLER::store_null)
    {
      result = "opening store '" + path + "' failed";
      SCDC_FAIL_F(result);
      goto do_return;
    }
  }

  ret = scdc_dataset::do_cmd_cd(params, input, output, result);

do_return:
  SCDC_TRACE_F("return: " << ret);

  return ret;
}


template<class STORE_HANDLER>
bool scdc_dataset_store<STORE_HANDLER>::do_cmd_ls(const std::string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output, scdc_result &result)
{
  SCDC_TRACE_F("params: '" << params << "'");

  scdc_dataprov_store<STORE_HANDLER> *dp =  static_cast<scdc_dataprov_store<STORE_HANDLER> *>(dataprov);

  bool ret = false;

  if (admin || store == STORE_HANDLER::store_null)
  {
    if (!dp->ls_stores(result))
    {
      result = "listing stores failed: " + result;
      SCDC_FAIL_F(result);
      goto do_return;
    }

  } else
  {
    if (!dp->ls_entries(store, result))
    {
      result = "listing entries failed: " + result;
      SCDC_FAIL_F(result);
      goto do_return;
    }
  } 

  ret = true;

do_return:
  SCDC_TRACE_F("return: " << ret);

  return ret;
}


template<class STORE_HANDLER>
bool scdc_dataset_store<STORE_HANDLER>::do_cmd_info(const std::string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output, scdc_result &result)
{
  SCDC_TRACE_F("params: '" << params << "'");

  scdc_dataprov_store<STORE_HANDLER> *dp =  static_cast<scdc_dataprov_store<STORE_HANDLER> *>(dataprov);

  bool ret = false;

  const std::string path = params;

  if (admin || store == STORE_HANDLER::store_null)
  {
    if (!dp->info_store(path.c_str(), result))
    {
      result = "getting info of store '" + path + "' failed: " + result;
      SCDC_FAIL_F(result);
      goto do_return;
    }

  } else
  {
    if (!dp->info_entry(store, path.c_str(), result))
    {
      result = "getting info of entry '" + path + "' failed: " + result;
      SCDC_FAIL_F(result);
      goto do_return;
    }
  }

  ret = true;

do_return:
  SCDC_TRACE_F("return: " << ret);

  return ret;
}


template<class STORE_HANDLER>
bool scdc_dataset_store<STORE_HANDLER>::do_cmd_put(const std::string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output, scdc_result &result)
{
  SCDC_TRACE_F("params: '" << params << "'");

  scdc_dataprov_store<STORE_HANDLER> *dp =  static_cast<scdc_dataprov_store<STORE_HANDLER> *>(dataprov);

  bool ret = false;

  if (admin || store == STORE_HANDLER::store_null)
  {
    const std::string path = params;

    if (!dp->mk_store(path.c_str()))
    {
      result = "making store '" + path + "' failed";
      SCDC_FAIL_F(result);
      goto do_return;
    }

  } else
  {
    std::string p = params;
    const std::string path = string_pop_front(p);
    const std::string pos_size = string_pop_front(p);

    if (entry == STORE_HANDLER::entry_null) entry = dp->entry_open(store, path.c_str(), false, true, true);
    else entry = dp->entry_reopen(store, path.c_str(), false, true, true, entry);

    if (entry == STORE_HANDLER::entry_null)
    {
      result = "opening entry '" + path + "' failed";
      SCDC_FAIL_F(result);
      goto do_return;
    }

    scdcint_t pos = -1;
    scdcint_t size = -1;
    if (!pos_size.empty()) parse_pos_size(pos_size, pos, size);

    if (size != 0)
    while (1)
    {
      scdcint_t s = SCDC_DATASET_INOUT_BUF_CURRENT(input);
      if (size >= 0) s = std::min(s, size);

      scdcint_t n = dp->entry_write_at(store, entry, SCDC_DATASET_INOUT_BUF_PTR(input), s, pos);

      if (n < 0)
      {
        result = "writing to entry '" + path + "' failed";
        SCDC_FAIL_F(result);
        goto do_return;
      }

      if (n < s)
      {
        result = "writing all data to entry '" + path + "' failed";
        SCDC_FAIL_F(result);
        goto do_return;
      }

      if (pos >= 0) pos += n;
      if (size >= 0) size -= n;

      if (size == 0 || !input->next) break;

      scdc_result_t res = SCDC_RESULT_INIT_EMPTY;

      if (input->next(input, &res) != SCDC_SUCCESS)
      {
        result = std::string("getting input data failed: ") + SCDC_RESULT_STR(&res);
        SCDC_FAIL_F(result);
        goto do_return;
      }
    }
  }

  ret = true;

do_return:
  SCDC_TRACE_F("return: " << ret);

  return ret;
}


template<class STORE_HANDLER>
static scdcint_t scdc_dataset_store_do_cmd_get_next(scdc_dataset_inout_t *inout, scdc_result_t *result)
{
  scdcint_t ret = SCDC_FAILURE;
  scdc_result res;

  scdc_dataset_store_do_cmd_get_next_data_t<STORE_HANDLER> *do_cmd_get_next_data = static_cast<scdc_dataset_store_do_cmd_get_next_data_t<STORE_HANDLER> *>(inout->data);

  scdcint_t s = do_cmd_get_next_data->size;
  scdcint_t n = -1;
  scdc_buf_t buf;

  if (STORE_HANDLER::HAVE_entry_read_access_at && do_cmd_get_next_data->dp->entry_read_access_at(do_cmd_get_next_data->store, do_cmd_get_next_data->entry, s, do_cmd_get_next_data->pos, buf))
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

    if (s < 0 || s > SCDC_DATASET_INOUT_BUF_SIZE(inout)) s = SCDC_DATASET_INOUT_BUF_SIZE(inout);
    
    n = do_cmd_get_next_data->dp->entry_read_at(do_cmd_get_next_data->store, do_cmd_get_next_data->entry, SCDC_DATASET_INOUT_BUF_PTR(inout), s, do_cmd_get_next_data->pos);
  }

  if (n < 0)
  {
    scdc_result res = "reading from entry '<unknown>' failed";
    SCDC_FAIL_F(res);
    goto do_return;
  }

  SCDC_DATASET_INOUT_BUF_SET_C(inout, n);

  if (do_cmd_get_next_data->pos >= 0) do_cmd_get_next_data->pos += n;
  if (do_cmd_get_next_data->size >= 0) do_cmd_get_next_data->size -= n;

  /* if (requested == -1 and read > 0) or (requested >= 0 and buf < requested and read == buf) then continue get else stop */
  inout->next = ((do_cmd_get_next_data->size < 0 && n > 0) || (do_cmd_get_next_data->size >= 0 && s < do_cmd_get_next_data->size && n == s))?scdc_dataset_store_do_cmd_get_next<STORE_HANDLER>:0;

  ret = SCDC_SUCCESS;

do_return:
  if (result) SCDC_RESULT_SET_STR(result, res.c_str());

  return ret;
}


template<class STORE_HANDLER>
bool scdc_dataset_store<STORE_HANDLER>::do_cmd_get(const std::string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output, scdc_result &result)
{
  SCDC_TRACE_F("params: '" << params << "'");

  scdc_dataprov_store<STORE_HANDLER> *dp =  static_cast<scdc_dataprov_store<STORE_HANDLER> *>(dataprov);

  bool ret = false;

  if (admin || store == STORE_HANDLER::store_null)
  {
    result = "command not available for a store";
    goto do_return;

  } else
  {
    std::string p = params;
    const std::string path = string_pop_front(p);
    const std::string pos_size = string_pop_front(p);

    if (entry == STORE_HANDLER::entry_null) entry = dp->entry_open(store, path.c_str(), true, false, false);
    else entry = dp->entry_reopen(store, path.c_str(), true, false, false, entry);

    if (entry == STORE_HANDLER::entry_null)
    {
      result = "opening entry '" + path + "' failed";
      SCDC_FAIL_F(result);
      goto do_return;
    }

    do_cmd_get_next_data.store = store;
    do_cmd_get_next_data.entry = entry;
    do_cmd_get_next_data.pos = -1;
    do_cmd_get_next_data.size = -1;
    if (!pos_size.empty()) parse_pos_size(pos_size, do_cmd_get_next_data.pos, do_cmd_get_next_data.size);

    scdc_dataset_output_t output_local;

    output_local.data = &do_cmd_get_next_data;
    output_local.next = scdc_dataset_store_do_cmd_get_next<STORE_HANDLER>;

    if (do_cmd_get_next_data.size != 0)
    while (output_local.next)
    {
      SCDC_DATASET_INOUT_BUF_ASSIGN(&output_local, output);

      scdc_result_t res = SCDC_RESULT_INIT_EMPTY;

      if (output_local.next(&output_local, &res) != SCDC_SUCCESS)
      {
        result = std::string("getting output data failed: ") + SCDC_RESULT_STR(&res);
        goto do_return;
      }

      SCDC_DATASET_INOUT_BUF_ASSIGN(output, &output_local);

      if (do_cmd_get_next_data.size == 0 || !output->next) break;

      SCDC_RESULT_SET_EMPTY(&res);

      if (output->next(output, &res) != SCDC_SUCCESS)
      {
        result = std::string("setting output data failed: ") + SCDC_RESULT_STR(&res);
        goto do_return;
      }
    }

    output->data = output_local.data;
    output->next = output_local.next;
  }

  ret = true;

do_return:
  SCDC_TRACE_F("return: " << ret);

  return ret;
}


template<class STORE_HANDLER>
bool scdc_dataset_store<STORE_HANDLER>::do_cmd_rm(const std::string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output, scdc_result &result)
{
  SCDC_TRACE_F("params: '" << params << "'");

  scdc_dataprov_store<STORE_HANDLER> *dp =  static_cast<scdc_dataprov_store<STORE_HANDLER> *>(dataprov);

  bool ret = false;

  const std::string path = params;

  if (admin || store == STORE_HANDLER::store_null)
  {
    if (!dp->rm_store(path.c_str()))
    {
      result = "removing store '" + path + "' failed";
      goto do_return;
    }

  } else
  {
    if (entry != STORE_HANDLER::entry_null && dp->entry_match(store, entry, path.c_str()))
    {
      dp->entry_close(store, entry);
      entry = STORE_HANDLER::entry_null;
    }

    if (!dp->rm_entry(store, path.c_str()))
    {
      result = "removing entry '" + path + "' failed";
      goto do_return;
    }
  }

  ret = true;

do_return:
  SCDC_TRACE_F("return: " << ret);

  return ret;
}

#undef SCDC_LOG_PREFIX


#define SCDC_LOG_PREFIX  "dataprov-store: "

template<class STORE_HANDLER>
bool scdc_dataprov_store<STORE_HANDLER>::open_config_conf(const std::string &conf, scdc_args *args, bool &done)
{
  SCDC_TRACE_F("conf: '" << conf << "'");

  bool ret = STORE_HANDLER::open_config_conf(conf, args, done);

  SCDC_TRACE_F("ret: '" << ret << "'");

  return ret;
}


template<class STORE_HANDLER>
bool scdc_dataprov_store<STORE_HANDLER>::open(const char *conf, scdc_args *args, scdc_result &result)
{
  SCDC_TRACE_F("conf: '" << conf << "'");

  bool ret = false;
  std::string c = conf;
  args = open_args_init(args);

  if (!STORE_HANDLER::open_conf(c, args, result))
  {
    SCDC_FAIL_F("opening store config failed: " << result);
    goto do_return;
  }

  if (!scdc_dataprov::open(c.c_str(), args, result))
  {
    SCDC_FAIL_F("opening base failed: " << result);
    goto do_return;
  }

  if (!STORE_HANDLER::open(result))
  {
    SCDC_FAIL_F("opening store handler failed: " << result);
    goto do_return;
  }

  dataset_cmds_add("pwd", static_cast<dataset_cmds_do_cmd_f>(&scdc_dataset_store<STORE_HANDLER>::do_cmd_pwd));
  dataset_cmds_add("cd", static_cast<dataset_cmds_do_cmd_f>(&scdc_dataset_store<STORE_HANDLER>::do_cmd_cd));
  dataset_cmds_add("ls", static_cast<dataset_cmds_do_cmd_f>(&scdc_dataset_store<STORE_HANDLER>::do_cmd_ls));
  dataset_cmds_add("info", static_cast<dataset_cmds_do_cmd_f>(&scdc_dataset_store<STORE_HANDLER>::do_cmd_info));
  dataset_cmds_add("put", static_cast<dataset_cmds_do_cmd_f>(&scdc_dataset_store<STORE_HANDLER>::do_cmd_put));
  dataset_cmds_add("get", static_cast<dataset_cmds_do_cmd_f>(&scdc_dataset_store<STORE_HANDLER>::do_cmd_get));
  dataset_cmds_add("rm", static_cast<dataset_cmds_do_cmd_f>(&scdc_dataset_store<STORE_HANDLER>::do_cmd_rm));

  ret = true;

do_return:
  if (!ret) open_args_release();

  SCDC_TRACE_F("ret: '" << ret << "'");

  return ret;
}


template<class STORE_HANDLER>
bool scdc_dataprov_store<STORE_HANDLER>::close(scdc_result &result)
{
  SCDC_TRACE_F("");

  bool ret = scdc_dataprov::close(result) || STORE_HANDLER::close(result);

  open_args_release();

  SCDC_TRACE_F("return: " << ret);

  return ret;
}


template<class STORE_HANDLER>
scdc_dataset *scdc_dataprov_store<STORE_HANDLER>::dataset_open(std::string &path, scdc_result &result)
{
  SCDC_TRACE_F("path: '" << path << "'");

  scdc_dataset_store<STORE_HANDLER> *dataset_store = new scdc_dataset_store<STORE_HANDLER>(this);

  SCDC_TRACE_F("return: " << dataset_store);

  return dataset_store;
}


template<class STORE_HANDLER>
bool scdc_dataprov_store<STORE_HANDLER>::dataset_close(scdc_dataset *dataset, scdc_result &result)
{
  SCDC_TRACE_F("dataset: " << dataset);

  bool ret = true;

  delete dataset;

  SCDC_TRACE_F("return: " << ret);

  return ret;
}


template<class STORE_HANDLER>
bool scdc_dataprov_store<STORE_HANDLER>::config_do_cmd_param(const std::string &cmd, const std::string &param, std::string val, scdc_config_result &result, bool &done)
{
  SCDC_TRACE_F("cmd: '" << cmd << "', param: '" << param << "', val: '" << val << "', result: '" << result << "'");
  
  return false;
}

#undef SCDC_LOG_PREFIX


#undef SCDC_TRACE_NOT
#if DATAPROV_STORE_TCC_SCDC_TRACE_NOT_BACKUP
# define SCDC_TRACE_NOT  1
#else /* DATAPROV_STORE_TCC_SCDC_TRACE_NOT_BACKUP */
# define SCDC_TRACE_NOT  0
#endif /* DATAPROV_STORE_TCC_SCDC_TRACE_NOT_BACKUP */
#undef DATAPROV_STORE_TCC_SCDC_TRACE_NOT_BACKUP
#include "log_unset.hh"
#include "log_set.hh"


#endif /* __DATAPROV_STORE_TCC__ */
