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


#define SCDC_TRACE_NOT  !SCDC_TRACE_COMPCOUP

#include "config.hh"
#include "log.hh"
#include "common.hh"
#include "dataset_inout.h"
#include "compcoup.hh"


using namespace std;


#define SCDC_LOG_PREFIX  "compcoup: "


bool scdc_compcoup::dataset_cmd(const std::string &cmd, scdc_dataset_input_t *input, scdc_dataset_output_t *output, scdc_result &result)
{
  SCDC_TRACE_F("cmd: '" << cmd << "'");

  bool ret = false;
  string res;

  string scmd, suri, sparams;
  split_cmdline(cmd.c_str(), cmd.size(), &scmd, &suri, &sparams);
  suri = ltrim(suri, ":");

  scdc_dataset *dataset = dataprovs->dataset_open(suri, res);

  if (!dataset)
  {
    result = "opening dataset '" + suri + "' failed: " + res;
    SCDC_FAIL_F(result);
    goto do_return;
  }

  SCDC_TRACE_F("dataset: " << dataset);

  {
    string cmdline;
    join_cmdline(scmd.c_str(), 0, sparams.c_str(), cmdline);

    ret = dataset->do_cmd(cmdline, input, output, result);

    if (!ret)
    {
      result = "command failed: " + result;
      SCDC_FAIL_F(result);
    }

    SCDC_TRACE_F("command " << (ret?"successful":"failed") << ", result: '" << result << "'");
  }

  if (!dataprovs->dataset_close(dataset, res))
  {
    result = "closing dataset failed: " + res + ", previous command " + (ret?"successful: ":"failed: ") + result;
    SCDC_FAIL_F(result);
    goto do_return;
  }

do_return:
  SCDC_TRACE_F("return: " << ret << ", result: '" + result + "'");

  return ret;
}


void scdc_compcoup::set_compression(const char *compression)
{
}


void scdc_compcoup::handshake()
{
  SCDC_TRACE("handshake:");
}
