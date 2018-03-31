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


bool scdc_compcoup::dataset_cmd(const char *cmd, scdcint_t cmd_size, scdc_dataset_input_t *input, scdc_dataset_output_t *output)
{
  SCDC_TRACE("dataset_cmd: '" << string(cmd, cmd_size) << "'");

  string scmd, suri, sparams;
  split_cmdline(cmd, cmd_size, &scmd, &suri, &sparams);

  const char *path = suri.c_str();
  scdcint_t path_size = suri.size();

  if (path[0] == ':')
  {
    ++path;
    --path_size;
  }

  scdc_dataset *dataset = dataprovs->dataset_open(path, path_size, output);

  if (!dataset)
  {
    SCDC_TRACE("dataset_cmd: failed: '" << SCDC_DATASET_OUTPUT_STR(output) << "'");

    return false;
  }

  SCDC_TRACE("dataset_cmd: dataset: '" << dataset << "'");

  string cmdline;
  join_cmdline(scmd.c_str(), 0, sparams.c_str(), cmdline);

  bool ret = dataset->do_cmd(cmdline.c_str(), cmdline.size(), input, output);

  if (ret) SCDC_TRACE("dataset_cmd: successful");
  else SCDC_TRACE("dataset_cmd: failed: '" << SCDC_DATASET_OUTPUT_STR(output) << "'");

  dataprovs->dataset_close(dataset, output);

  return ret;
}


void scdc_compcoup::set_compression(const char *compression)
{
}


void scdc_compcoup::handshake()
{
  SCDC_TRACE("handshake:");
}
