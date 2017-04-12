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
#include <cstdarg>
#include <string>

#include "config.hh"
#include "common.hh"
#include "log.hh"
#include "dataprov_nfs.hh"


using namespace std;


#define SCDC_LOG_PREFIX  "dataset-nfs-store: "


class scdc_dataset_nfs_store: public scdc_dataset
{
  public:
    scdc_dataset_nfs_store(scdc_dataprov *dataprov_)
      :scdc_dataset(dataprov_) { };


    bool do_cmd_info(const string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output)
    {
      SCDC_TRACE("do_cmd_info: '" << params << "'");

      SCDC_DATASET_OUTPUT_CLEAR(output);

      return false;
    }


    bool do_cmd_cd(const string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output)
    {
      SCDC_TRACE("do_cmd_cd: '" << params << "'");

      SCDC_DATASET_OUTPUT_CLEAR(output);

      return false;
    }


    bool do_cmd_ls(const string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output)
    {
      SCDC_TRACE("do_cmd_ls: '" << params << "'");

      SCDC_DATASET_OUTPUT_CLEAR(output);

      return false;
    }


    bool do_cmd_put(const string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output)
    {
      SCDC_TRACE("do_cmd_put: '" << params << "'");

      SCDC_DATASET_OUTPUT_CLEAR(output);

      return false;
    }


    bool do_cmd_get(const string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output)
    {
      SCDC_TRACE("do_cmd_get: '" << params << "'");

      SCDC_DATASET_OUTPUT_CLEAR(output);

      return false;
    }


    bool do_cmd_rm(const string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output)
    {
      SCDC_TRACE("do_cmd_rm: '" << params << "'");

      SCDC_DATASET_OUTPUT_CLEAR(output);

      return false;
    }
};


#undef SCDC_LOG_PREFIX


#define SCDC_LOG_PREFIX  "dataprov-nfs: "


scdc_dataprov_nfs_store::scdc_dataprov_nfs_store()
  :scdc_dataprov("nfs")
{
}


bool scdc_dataprov_nfs_store::open(const char *conf, scdc_args *args)
{
  SCDC_TRACE("open: conf: '" << conf << "'");

  bool ret = true;

  if (!scdc_dataprov::open(conf, args))
  {
    SCDC_FAIL("open: opening base");
    ret = false;

  } else
  {
    dataset_cmds_add("pwd", static_cast<dataset_cmds_do_cmd_f>(&scdc_dataset_nfs_store::do_cmd_pwd));
    dataset_cmds_add("info", static_cast<dataset_cmds_do_cmd_f>(&scdc_dataset_nfs_store::do_cmd_info));
    dataset_cmds_add("cd", static_cast<dataset_cmds_do_cmd_f>(&scdc_dataset_nfs_store::do_cmd_cd));
    dataset_cmds_add("ls", static_cast<dataset_cmds_do_cmd_f>(&scdc_dataset_nfs_store::do_cmd_ls));
    dataset_cmds_add("put", static_cast<dataset_cmds_do_cmd_f>(&scdc_dataset_nfs_store::do_cmd_put));
    dataset_cmds_add("get", static_cast<dataset_cmds_do_cmd_f>(&scdc_dataset_nfs_store::do_cmd_get));
    dataset_cmds_add("rm", static_cast<dataset_cmds_do_cmd_f>(&scdc_dataset_nfs_store::do_cmd_rm));

do_close:
    if (!ret) scdc_dataprov::close();
  }

do_quit:
  return ret;
}


void scdc_dataprov_nfs_store::close()
{
  SCDC_TRACE("close:");

  scdc_dataprov::close();
}


scdc_dataset *scdc_dataprov_nfs_store::dataset_open(const char *path, scdcint_t path_size, scdc_dataset_output_t *output)
{
  SCDC_TRACE("dataset_open: '" << string(path, path_size) << "'");

  scdc_dataset *dataset = 0;
  
  if (config_open(path, path_size, output, &dataset)) return dataset;

  scdc_dataset_nfs_store *dataset_nfs = new scdc_dataset_nfs_store(this);

  if (path && !dataset_nfs->do_cmd_cd(string(path, path_size).c_str(), NULL, output))
  {
    SCDC_FAIL("dataset_open: do_cmd_cd: failed: '" << SCDC_DATASET_OUTPUT_STR(output) << "'");
    delete dataset_nfs;
    return 0;
  }

  SCDC_TRACE("dataset_open: return: '" << dataset_nfs << "'");

  return dataset_nfs;
}


void scdc_dataprov_nfs_store::dataset_close(scdc_dataset *dataset, scdc_dataset_output_t *output)
{
  SCDC_TRACE("dataset_close: '" << dataset << "'");

  if (config_close(dataset, output)) return;

  delete dataset;

  SCDC_TRACE("dataset_close: return");
}


#undef SCDC_LOG_PREFIX
