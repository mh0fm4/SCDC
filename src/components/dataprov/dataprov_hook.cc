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


#include <cstring>
#include <string>

#define SCDC_TRACE_NOT  !SCDC_TRACE_DATAPROV_HOOK

#include "config.hh"
#include "common.hh"
#include "log.hh"
#include "dataset_inout.h"
#include "dataprov_hook.hh"


using namespace std;


#define SCDC_LOG_PREFIX  "dataset-hook: "


class scdc_dataset_hook: public scdc_dataset
{
  public:
    void *dataset;

    scdc_dataset_hook(scdc_dataprov *dataprov_, void *dataset_)
      :scdc_dataset(dataprov_), dataset(dataset_) { }


    bool do_cmd(const string &cmd, const string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output, scdc_result &result)
    {
      SCDC_TRACE("dataset_hook: do_cmd: cmd: '" << cmd << "'");

      scdc_dataprov_hook *dataprov_hook = static_cast<scdc_dataprov_hook *>(dataprov);

      scdc_result_t res = SCDC_RESULT_INIT_EMPTY;
      scdcint_t ret = dataprov_hook->hook_arg.hook.dataset_cmd(dataprov_hook->dataprov, dataset, cmd.c_str(), params.c_str(), input, output, &res);
      result = SCDC_RESULT_STR(&res);

      SCDC_TRACE("dataset_hook: do_cmd: return: '" << ret << "'");

      return (ret == SCDC_SUCCESS);
    }
};


#undef SCDC_LOG_PREFIX


#define SCDC_LOG_PREFIX  "dataprov-hook: "


scdc_dataprov_hook::scdc_dataprov_hook()
  :scdc_dataprov("hook"), dataprov(0)
{
}


bool scdc_dataprov_hook::open(const char *conf, scdc_args *args, scdc_result &result)
{
  SCDC_TRACE("open: '" << conf << "'");

  bool ret = true;

  args = open_args_init(args);

  if (args->get<scdc_args_dataprov_hook_t>(SCDC_ARGS_TYPE_DATAPROV_HOOK, &hook_arg, true) == SCDC_ARG_REF_NULL)
  {
    SCDC_ERROR("open: getting hook functions");
    ret = false;
    goto do_quit;
  }

  if (!scdc_dataprov::open(0, args, result))
  {
    SCDC_FAIL("open: opening base");
    ret = false;

  } else
  {
    if (hook_arg.hook_open_intern)
    {
      scdcint_t ret;

      dataprov = hook_arg.hook_open_intern(hook_arg.intern_data, conf, args->get_args(), &ret);

      if (ret != SCDC_SUCCESS)
      {
        SCDC_FAIL("open: opening data provider with hook function");
        ret = false;
        goto do_close;
      }
    }

do_close:
    if (!ret) scdc_dataprov::close(result);
  }

  SCDC_TRACE("open: dataprov: '" << dataprov << "'");

  open_args_clear();

do_quit:
  if (!ret) open_args_release();

  return ret;
}


bool scdc_dataprov_hook::close(scdc_result &result)
{
  SCDC_TRACE("close:");

  bool ret = true;

  if (hook_arg.hook.close)
  {
    if (hook_arg.hook.close(dataprov) != SCDC_SUCCESS)
    {
      SCDC_FAIL("close: closing data provider with hook");
      ret = false;
    }
  }

  open_args_release();

  ret = scdc_dataprov::close(result);

  SCDC_TRACE("close: return: " << ret);

  return ret;
}


scdc_dataset *scdc_dataprov_hook::dataset_open(std::string &path, scdc_result &result)
{
  SCDC_TRACE("dataset_open: path: '" << path << "'");

  scdc_dataset_hook *dataset_hook = 0;
  void *dataset_hook_dataset = 0;

  if (hook_arg.hook.dataset_open)
  {
    scdc_result_t res = SCDC_RESULT_INIT_EMPTY;
    dataset_hook_dataset = hook_arg.hook.dataset_open(dataprov, path.c_str(), &res);
    result = SCDC_RESULT_STR(&res);

    if (!dataset_hook_dataset)
    {
      SCDC_FAIL(__func__ << ": opening dataset with hook failed: " << result);
      goto do_return;
    }
  }

  /* unset path to prevent a further cd command */
  path.clear();

  dataset_hook = new scdc_dataset_hook(this, dataset_hook_dataset);

do_return:
  SCDC_TRACE("dataset_open: return: " << dataset_hook);

  return dataset_hook;
}


bool scdc_dataprov_hook::dataset_close(scdc_dataset *dataset, scdc_result &result)
{
  SCDC_TRACE("dataset_close: dataset: " << dataset);

  bool ret = false;

  scdc_dataset_hook *dataset_hook = static_cast<scdc_dataset_hook *>(dataset);

  if (hook_arg.hook.dataset_close)
  {
    scdc_result_t res = SCDC_RESULT_INIT_EMPTY;
    scdcint_t r = hook_arg.hook.dataset_close(dataprov, dataset_hook->dataset, &res);
    result = SCDC_RESULT_STR(&res);

    if (r != SCDC_SUCCESS)
    {
      SCDC_FAIL(__func__ << ": closing dataset with hook failed: " << result);
      goto do_return;
    }
  }

  delete dataset_hook;
  ret = true;

do_return:
  SCDC_TRACE("dataset_close: return: " << ret);

  return ret;
}


scdc_dataset *scdc_dataprov_hook::dataset_open_read_state(scdc_data *incoming, scdc_result &result)
{
  SCDC_TRACE("dataset_open_read_state:");

  if (!hook_arg.hook.dataset_open_read_state)
  {
    SCDC_TRACE("dataset_open_read_state: using fallback");
    return scdc_dataprov::dataset_open_read_state(incoming, result);
  }

  char *buf;
  scdcint_t buf_size;
  const char *end;

  while (1)
  {
    buf = incoming->get_read_pos_buf();
    buf_size = incoming->get_read_pos_buf_size();
    end = memchr(buf, '|', buf_size);

    if (end) break;

    incoming->do_next();
  }

  buf_size = end - buf;

  scdc_result_t res = SCDC_RESULT_INIT_EMPTY;
  void *dataset = hook_arg.hook.dataset_open_read_state(dataprov, buf, buf_size, &res);
  result = SCDC_RESULT_STR(&res);

  if (!dataset)
  {
    SCDC_FAIL(__func__ << ": opening dataset by reading state with hook failed: '" << result << "'");
    return 0;
  }

  incoming->inc_read_pos(end - buf + 1);

  scdc_dataset_hook *dataset_hook = new scdc_dataset_hook(this, dataset);

  SCDC_TRACE("dataset_open_read_state: dataset_hook: '" << dataset_hook << "'");

  return dataset_hook;
}


bool scdc_dataprov_hook::dataset_close_write_state(scdc_dataset *dataset, scdc_data *outgoing, scdc_result &result)
{
  SCDC_TRACE("dataset_close_write_state: dataset: '" << dataset << "'");

  if (!hook_arg.hook.dataset_close_write_state)
  {
    SCDC_TRACE("dataset_close_write_state: using fallback");
    return scdc_dataprov::dataset_close_write_state(dataset, outgoing, result);
  }

  bool ret = true;

  scdc_dataset_hook *dataset_hook = static_cast<scdc_dataset_hook *>(dataset);

  char *buf = outgoing->get_write_pos_buf();
  scdcint_t buf_size = outgoing->get_write_pos_buf_size();

  scdc_result_t res = SCDC_RESULT_INIT_EMPTY;
  scdcint_t n = hook_arg.hook.dataset_close_write_state(dataprov, dataset_hook->dataset, buf, buf_size, &res);
  result = SCDC_RESULT_STR(&res);

  if (n < 0)
  {
    SCDC_FAIL(__func__ << ": closing dataset by writing state with hook failed: '" << result << "'");
    ret = false;

  } else
  {
    outgoing->inc_write_pos(n);
  }

  buf = outgoing->get_write_pos_buf();
  buf_size = outgoing->get_write_pos_buf_size();

  SCDC_ASSERT(buf_size > 0);

  buf[0] = '|';

  outgoing->inc_write_pos(1);

  delete dataset_hook;

  SCDC_TRACE("dataset_close_write_state: return: " << ret);

  return ret;
}


bool scdc_dataprov_hook::config_do_cmd_param(const std::string &cmd, const std::string &param, std::string val, scdc_config_result &result, bool &done)
{
  SCDC_TRACE("config_do_cmd_param: cmd: '" << cmd << "', param: '" << param << "', val: '" << val << "', result: '" << result << "'");

  done = true;
  bool ret = true;

  if (param == "")
  {
    ret = scdc_dataprov::config_do_cmd_param(cmd, param, val, result, done);
    if (!ret) goto do_quit;

    done = true;

    if (hook_arg.hook.config)
    {
      scdc_result_t res = SCDC_RESULT_INIT_EMPTY;

      ret = (hook_arg.hook.config(dataprov, cmd.c_str(), param.c_str(), val.c_str(), val.size(), &res) == SCDC_SUCCESS);
      if (!ret) goto do_quit;

      done = true;

      /* base class AND hook, concat results */
      if (cmd == "info")
      {
        ret = scdc_dataprov_config::info(result, SCDC_RESULT_STR(&res));

      } else if (cmd == "ls")
      {
        ret = scdc_dataprov_config::ls(result, SCDC_RESULT_STR(&res));

      } else
      {
        ret = true;
        result = SCDC_RESULT_STR(&res);
      }
    }

  } else
  {
    ret = scdc_dataprov::config_do_cmd_param(cmd, param, val, result, done);
    if (!ret) goto do_quit;

    if (done);
    else if (hook_arg.hook.config)
    {
      scdc_result_t res = SCDC_RESULT_INIT_EMPTY;

      ret = (hook_arg.hook.config(dataprov, cmd.c_str(), param.c_str(), val.c_str(), val.size(), &res) == SCDC_SUCCESS);
      if (!ret) goto do_quit;

      done = true;

      result = SCDC_RESULT_STR(&res);

    } else done = false;
  }

do_quit:
  if (done && !ret)
  {
    SCDC_FAIL("config_do_cmd_param: cmd '" << cmd << "' failed");
    return false;
  }

  return ret;
}


bool scdc_dataprov_hook::dataset_cmds_do_cmd(scdc_dataset *dataset, const std::string &cmd, const std::string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output, scdc_result &result)
{
  SCDC_TRACE("dataset_cmds_do_cmd: cmd: '" << cmd << "', params: '" << params << "'");

  scdc_dataset_hook *dataset_hook = static_cast<scdc_dataset_hook *>(dataset);

  bool ret = dataset_hook->do_cmd(cmd, params, input, output, result);

  SCDC_TRACE("dataset_cmds_do_cmd: return: '" << ret << "'");

  return ret;
}


#undef SCDC_LOG_PREFIX
