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
#include <cstdarg>
#include <iostream>
#include <typeinfo>

#define SCDC_TRACE_NOT  !SCDC_TRACE_DATASET

#include "config.hh"
#include "log.hh"
#include "common.hh"
#include "dataset.hh"
#include "dataset_inout.h"
#include "dataprov.hh"


using namespace std;


#define SCDC_LOG_PREFIX  "dataset: "


bool scdc_dataset::do_cmd(const char *cmd, scdcint_t cmd_size, scdc_dataset_input_t *input, scdc_dataset_output_t *output)
{
  SCDC_TRACE("do_cmd: '" << string(cmd, cmd_size) << "'");

  string scmd, suri, sparams, params;

  split_cmdline(cmd, cmd_size, &scmd, &suri, &sparams);

  join_cmdline(0, suri.c_str(), sparams.c_str(), params);

  SCDC_TRACE("do_cmd: pwd: '" << pwd << "'");
  SCDC_TRACE("do_cmd: cmd: '" << scmd << "'");
  SCDC_TRACE("do_cmd: params: '" << params << "'");

  bool ret;

  if (config)
  {
    if (scmd == "cd")
    {
      if (params == "") ret = true;
      else ret = dataprov->config_do_cmd("info", params.c_str(), 0, 0);

      if (ret) ret = scdc_dataset::do_cmd_cd(params.c_str(), input, output);

    } else if (scmd == "pwd")
    {
      ret = scdc_dataset::do_cmd_pwd(params.c_str(), input, output);

    } else
    {
      if (pwd.size() > 0) params = pwd + " " + params;

      ret = dataprov->config_do_cmd(scmd.c_str(), params.c_str(), input, output);
    }

  } else ret = dataprov->dataset_cmds_do_cmd(this, scmd.c_str(), params.c_str(), input, output);

  SCDC_TRACE("do_cmd: return: '" << ret << "'");

  return ret;
}


bool scdc_dataset::do_cmd_info(const std::string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output)
{
  SCDC_TRACE("do_cmd_info: params: '" << params << "'");

  SCDC_INFO("dataprov: '" << dataprov << "'");
  SCDC_INFO("config: '" << config << "'");
  SCDC_INFO("pwd: '" << pwd << "'");

  SCDC_DATASET_OUTPUT_CLEAR(output);

  return true;
}


bool scdc_dataset::do_cmd_cd(const std::string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output)
{
  SCDC_TRACE("do_cmd_cd: '" << params << "'");

  set_pwd(params);

  SCDC_DATASET_OUTPUT_CLEAR(output);

  return true;
}


bool scdc_dataset::do_cmd_pwd(const std::string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output)
{
  SCDC_TRACE("do_cmd_pwd: params: '" << params << "'");

  SCDC_DATASET_OUTPUT_PRINTF(output, pwd.c_str());

  return true;
}


#undef SCDC_LOG_PREFIX
