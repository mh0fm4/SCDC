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


#include <cstdio>
#include <typeinfo>

#define SCDC_TRACE_NOT  !SCDC_TRACE_DATAPROV

#include "config.hh"
#include "common.hh"
#include "log.hh"
#include "dataprov.hh"


using namespace std;


#define SCDC_DATAPROV_CONFIG_STR  "CONFIG"


#define SCDC_LOG_PREFIX  "dataset-config: "


class scdc_dataset_config: public scdc_dataset
{
  public:
    scdc_dataset_config(scdc_dataprov *dataprov_)
      :scdc_dataset(dataprov_) { config = true; }
};


#undef SCDC_LOG_PREFIX

#define SCDC_LOG_PREFIX  "dataprov: "


bool scdc_dataprov::open_config_conf(const std::string &conf, scdc_args *args, bool &done)
{
  done = false;
  return true;
}


bool scdc_dataprov::open_config(std::string &conf, scdc_args *args)
{
  bool done;
  stringlist cl(conf);

  while (cl.size() > 0)
  {
    const string &c = cl.front_pop();

    if (!open_config_conf(c, args, done))
    {
      SCDC_FAIL("open_config: processing configuration '" << c << "' failed");
      return false;
    }

    if (!done) SCDC_INFO("open_config: ignoring unknown configuration '" << c << "'");
  }

  return true;
}


bool scdc_dataprov::open(const char *conf, scdc_args *args)
{
  SCDC_TRACE("open: conf: '" << (conf?conf:"") << "'");

  bool ret = true;

  if (conf)
  {
    string confs(conf);

    if (!open_config(confs, args))
    {
      SCDC_FAIL("open: processing configuration failed");
      ret = false;
    }
  }

  return ret;
}


void scdc_dataprov::close()
{
  SCDC_TRACE("close:");
}


scdc_dataset *scdc_dataprov::dataset_open_read_state(scdc_data *incoming, scdc_dataset_output_t *output)
{
  SCDC_TRACE("dataset_open_read_state:");

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

  scdc_dataset *dataset = 0;

  string dataset_str(buf, buf_size);
  sscanf(dataset_str.c_str(), "%p", &dataset);

  incoming->inc_read_pos(buf_size + 1);

  if (!dataset)
  {
    SCDC_DATASET_OUTPUT_PRINTF(output, "dataset not available");
    SCDC_FAIL("dataset_open_read_state: dataset not available");
    return 0;
  }

  return dataset;
}


void scdc_dataprov::dataset_close_write_state(scdc_dataset *dataset, scdc_data *outgoing, scdc_dataset_output_t *output)
{
  SCDC_TRACE("dataset_close_write_state: '" << dataset << "'");

  char *buf = outgoing->get_write_pos_buf();
  scdcint_t buf_size = outgoing->get_write_pos_buf_size();

  scdcint_t n = snprintf(buf, buf_size, "%p|", dataset);

  SCDC_ASSERT(n <= buf_size);

  if (n > buf_size) n = buf_size;

  outgoing->inc_write_pos(n);
}


bool scdc_dataprov::config_open(const char *path, scdcint_t path_size, scdc_dataset_output_t *output, scdc_dataset **dataset)
{
  SCDC_TRACE("config_open: path: '" << string(path, path_size) << "', output: '" << output << "'");

  SCDC_DATASET_OUTPUT_CLEAR(output);

  bool ret = false;
  *dataset = 0;

  stringlist pl('/', path, path_size);

  if (pl.front_pop() == SCDC_DATAPROV_CONFIG_STR)
  {
    ret = true;

    *dataset = new scdc_dataset_config(this);

    string p = pl.conflate();

    if (p.size() > 0)
    {
      string cd_cmd = "cd " + p;
    
      if (!(*dataset)->do_cmd(cd_cmd.c_str(), cd_cmd.size(), 0, output))
      {
        SCDC_FAIL("selecting config parameter '" << p << "' failed");
        SCDC_DATASET_OUTPUT_PRINTF(output, "selecting config parameter '%s' failed", p.c_str());
        delete *dataset;
        *dataset = 0;
      }
    }
  }

  SCDC_TRACE("config_open: dataset: '" << *dataset << "'");
  SCDC_TRACE("config_open: return: '" << ret << "'");

  return ret;
}


bool scdc_dataprov::config_close(scdc_dataset *dataset, scdc_dataset_output_t *output)
{
  SCDC_TRACE("config_close: dataset: '" << dataset << "'");

  bool ret = false;

  if (typeid(*dataset) == typeid(scdc_dataset_config))
  {
    delete dataset;
    ret = true;
  }

  SCDC_TRACE("config_close: return: '" << ret << "'");

  return ret;
}


bool scdc_dataprov::config_do_cmd(const std::string &cmd, const std::string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output)
{
  SCDC_TRACE("config_do_cmd: cmd: '" << cmd << "', params: '" << params << "', input: '" << input << "', output: '" << output << "'");

  stringlist pvsl(',', params);

  scdc_config_result result;
  bool done = false;

  SCDC_DATASET_OUTPUT_CLEAR(output);

  if (pvsl.size() > 0)
  {
    string pv;

    while (pvsl.front_pop(pv))
    {
      SCDC_TRACE("config_do_cmd: parameter (and value): '" << pv << "'");

      stringlist pvl(' ', pv);
      string p = pvl.front_pop();

      if (p == "*")
      {
        scdc_config_result ast;
        config_do_cmd_param("ls", "", string(""), ast, done);

        if (ast.size() > 0)
        {
          if (pvsl.size() > 0) ast += "," + pvsl.conflate();
          pvsl = stringlist(',', ast);
        }

      } else
      {
        if (!config_do_cmd_param_base(cmd, p.c_str(), pvl.conflate(), result, done))
        {
          SCDC_FAIL("config command '" << cmd << "' for parameter '" << p << "' failed: " << result << "");
          SCDC_DATASET_OUTPUT_PRINTF(output, "config command for parameter '%s' failed: %s", p.c_str(), result.c_str());
          return false;
        }
      }
    }

  } else
  {
    if (!config_do_cmd_param_base(cmd, "", string(""), result, done))
    {
      SCDC_FAIL("doing config command '" << cmd << "' failed");
      SCDC_DATASET_OUTPUT_PRINTF(output, "config command failed");
      return false;
    }
  }

  SCDC_TRACE("config_do_cmd: result: '" << result << "'");

  SCDC_DATASET_OUTPUT_PRINTF(output, result.c_str());

  return true;
}


bool scdc_dataprov::config_do_cmd_param_base(const std::string &cmd, const std::string &param, std::string val, scdc_config_result &result, bool &done)
{
  SCDC_TRACE("config_do_cmd_param_base: cmd: '" << cmd << "', param: '" << param << "', val: '" << val << "', result: '" << result << "'");

/*  string c(cmd), p(param);*/
  bool ret = false;

  ret = config_do_cmd_param(cmd, param, val, result, done);

  return ret;
}


bool scdc_dataprov::config_do_cmd_param(const std::string &cmd, const std::string &param, std::string val, scdc_config_result &result, bool &done)
{
  SCDC_TRACE("config_do_cmd_param: cmd: '" << cmd << "', param: '" << param << "', val: '" << val << "', result: '" << result << "'");

  string c(cmd), p(param);
  done = true;
  bool ret = true;

  if (p == "")
  {
    if (c == "info")
    {
      ret = scdc_dataprov_config::info(result, type.c_str());

    } else if (c == "ls")
    {
      ret = scdc_dataprov_config::ls(result, "type");

    } else done = false;

  } else if (p == "type")
  {
    if (c == "info")
    {
      ret = scdc_dataprov_config::info(result, p, "type of the data provider");

    } else if (c == "get")
    {
      ret = scdc_dataprov_config::get<string>(result, p, type);

    } else done = false;

  } else done = false;

  if (done && !ret)
  {
    SCDC_FAIL("config_do_cmd_param: cmd '" << cmd << "' not implemented");
    return false;
  }

  return ret;
}


bool scdc_dataprov::dataset_cmds_do_cmd(scdc_dataset *dataset, const std::string &cmd, const std::string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output)
{
  SCDC_TRACE("dataset_cmds_do_cmd: cmd: '" << cmd << "', params: '" << params << "'");

  dataset_cmds_t::iterator i = dataset_cmds.find(string(cmd));

  if (i == dataset_cmds.end())
  {
    SCDC_FAIL("do_cmd: command '" << cmd << "' not implemented");
    SCDC_DATASET_OUTPUT_PRINTF(output, "command '%s' not implemented", cmd.c_str());
    return false;
  }

  dataset_cmds_do_cmd_f do_cmd = i->second;

  bool ret = (dataset->*do_cmd)(params, input, output);

  SCDC_TRACE("dataset_cmds_do_cmd: return: '" << ret << "'");

  return ret;
}


#undef SCDC_LOG_PREFIX
