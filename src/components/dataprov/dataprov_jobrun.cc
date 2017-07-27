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
#include <cstdlib>
#include <string>
#include <sstream>
#include <algorithm>
#include <errno.h>

#include "z_pack.h"
#include "rapidxml.hpp"

#define SCDC_TRACE_NOT  !SCDC_TRACE_DATAPROV_JOBRUN

#include "config.hh"
#include "common.hh"
#include "log.hh"
#include "dataset.hh"
#include "dataset_inout.h"
#include "dataprov_jobrun.hh"

#include "scdc.h"


#define USE_SYSTEM_PID  0
#define LOCK_LIBSCDC    1


using namespace std;


#define SCDC_LOG_PREFIX  "dataset-jobrun: "

class scdc_dataset_jobrun: public scdc_dataset
{
  public:
    scdc_dataset_jobrun(scdc_dataprov *dataprov_)
      :scdc_dataset(dataprov_) { };


    scdc_dataprov_jobrun *dataprov_jobrun()
    {
      return static_cast<scdc_dataprov_jobrun *>(dataprov);
    }


    void set_jobid(const string &jobid)
    {
      set_pwd(jobid);
    }


    string get_jobid()
    {
      return pwd;
    }


    bool has_jobid()
    {
      return (pwd.size() > 0);
    }


    virtual bool create_job_inout(const string &jobid, scdc_dataprov_jobrun_job_params_t &job_params) = 0;

    bool create_job(const string &jobid, scdc_dataprov_jobrun_job_params_t &job_params)
    {
      SCDC_TRACE(__func__ << ": jobid: '" << jobid << "', nt: " << job_params.nt_min << "-" << job_params.nt_max << "', np: " << job_params.np_min << "-" << job_params.np_max);

      bool ret = true;

      ret = ret && create_job_inout(jobid, job_params);

      ret = ret && dataprov_jobrun()->job_create(jobid, job_params);

      return ret;
    }


    virtual bool destroy_job_inout() = 0;

    bool destroy_job(const string &jobid)
    {
      SCDC_TRACE(__func__ << ": jobid: '" << jobid << "'");

      bool ret = true;

      ret = ret && dataprov_jobrun()->job_destroy(jobid);

      ret = ret && destroy_job_inout();

      return ret;
    }


    virtual bool update_job_inout(const string &jobid, scdc_dataprov_jobrun_job_params_t &job_params) = 0;

    bool update_job(const string &jobid, scdc_dataprov_jobrun_job_params_t &job_params)
    {
      SCDC_TRACE(__func__ << ": jobid: '" << jobid << "', nt: " << job_params.nt_min << "-" << job_params.nt_max << "', np: " << job_params.np_min << "-" << job_params.np_max);

      bool ret = true;

      ret = ret && update_job_inout(jobid, job_params);

      ret = ret && dataprov_jobrun()->job_update(jobid, job_params);

      return ret;
    }


    void read_job_params(std::string &params, bool read_optional, std::string &jobid, scdcint_t &np_min, scdcint_t &np_max)
    {
      stringlist pl(' ', params);
      string jobparams = pl.front_pop();

      if (read_optional && jobparams[0] != ':') return;

      params = pl.conflate();

      stringlist jpl(':', jobparams);

      if (!jpl.front_pop(jobid)) return;

      jobid = safe_jobid(jobid);

      string s;

      if (!jpl.front_pop(s)) return;

      sscanf(s.c_str(), "%" scdcint_fmt, &np_min);

      if (!jpl.front_pop(s)) np_max = np_min;
      else sscanf(s.c_str(), "%" scdcint_fmt, &np_max);
    }


    void read_job_params2(std::string &params, std::string &jobid, scdc_dataprov_jobrun_job_params_t &job_params)
    {
      stringlist pl(' ', params);
      string jobparams = pl.front_pop();

      /* if there exisits a jobid (i.e., a job is selected) and the first param does not contain a ':', then there are no job params to read */
      if (jobid.size() > 0 && jobparams.find(':') == string::npos) return;

      params = pl.conflate();

      stringlist jpl(':', jobparams);

      /* if there is no jobid (i.e. not even an empty jobid), then select an empty jobid and quit */
      if (!jpl.front_pop(jobid))
      {
        jobid.clear();
        return;
      }

      jobid = safe_jobid(jobid);

      /* FIXME: Liste von job_params_proc (= $TPN$ analog zu $NPT$) als notwendige Resourcen, Auswahl durch Range 'min-max' und Alternativen '...|...' (insb. für Nodenames) */

      string s, t;

      if (!jpl.front_pop(s)) return;

      size_t p;
      scdc_dataprov_jobrun_job_params_proc_t job_params_proc;

      while (s.size() > 0)
      {
        /* nt */
        p = s.find('-');
        sscanf(s.substr(0, p).c_str(), "%" scdcint_fmt, &job_params_proc.nt_max);
        if (p != string::npos)
        {
          job_params_proc.nt_min = job_params_proc.nt_max;
          sscanf(s.substr(0, p + 1).c_str(), "%" scdcint_fmt, &job_params_proc.nt_max);
        }

        if (!jpl.front_pop(s)) break;

        /* np */
        p = s.find('-');
        sscanf(s.substr(0, p).c_str(), "%" scdcint_fmt, &job_params_proc.np_max);
        if (p != string::npos)
        {
          job_params_proc.nt_min = job_params_proc.nt_max;
          sscanf(s.substr(0, p + 1).c_str(), "%" scdcint_fmt, &job_params_proc.np_max);
        }

        if (!jpl.front_pop(s)) break;

        /* node */
        p = s.find(',');
        job_params_proc.node = s.substr(0, p);

        if (p != string::npos)
        {
          s = s.substr(p);
          job_params = job_params_proc;
          job_params_proc = scdc_dataprov_jobrun_job_params_proc_t();
        }
      }

      job_params = job_params_proc;
    }


    bool select_job(std::string &params, bool ignore_if_selected, bool fail_if_unselect, string &jobid)
    {
      jobid = get_jobid();

      /* if there is a job selected and should not be unselected */
      if (has_jobid() && ignore_if_selected) return true;

      scdc_dataprov_jobrun_job_params_t job_params;
      read_job_params2(params, jobid, job_params);

      /* if there is no jobid */
      if (jobid.size() == 0)
      {
        /* if unselect is not allowed */
        if (fail_if_unselect) return false;

      } else
      {
        /* if the job does not exist */
        if (!dataprov_jobrun()->is_job(jobid)) return false;
      }

      undo_select_jobid = get_jobid();

      set_jobid(jobid);

      return true;
    }


    bool undo_select_job()
    {
      /* if there is a jobid and the job does not exist */
      if (undo_select_jobid.size() > 0 && !dataprov_jobrun()->is_job(undo_select_jobid)) return false;

      set_jobid(undo_select_jobid);

      return true;
    }


    bool do_cmd_cd(const std::string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output)
    {
      string jobid;
      string p = params;

      if (!select_job(p, false, false, jobid))
      {
        SCDC_FAIL("do_cmd_cd: selecting job '" << jobid.c_str() << "' failed");
        SCDC_DATASET_OUTPUT_PRINTF(output, "selecting job '%s' failed", jobid.c_str());
        return false;
      }

      return true;
    }


    bool do_cmd_pwd(const std::string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output)
    {
      return scdc_dataset::do_cmd_pwd(params, input, output);
    }


    virtual bool do_cmd_ls_inout(const std::string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output) = 0;

    bool do_cmd_ls(const std::string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output)
    {
      if (!has_jobid() && dataprov_jobrun()->do_cmd_ls(params, input, output)) return true;

      string jobid;
      string p = params;

      if (!select_job(p, true, true, jobid))
      {
        SCDC_FAIL("do_cmd_ls: selecting job '" << jobid.c_str() << "' failed");
        SCDC_DATASET_OUTPUT_PRINTF(output, "selecting job '%s' failed", jobid.c_str());
        return false;
      }

      bool ret = do_cmd_ls_inout(p, input, output);

      undo_select_job();

      return ret;
    }


    bool do_cmd_info(const std::string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output)
    {
      if (!has_jobid() && dataprov_jobrun()->do_cmd_info(params, input, output)) return true;

      string jobid;
      string p = params;

      if (!select_job(p, true, true, jobid))
      {
        SCDC_FAIL("do_cmd_info: selecting job '" << jobid.c_str() << "' failed");
        SCDC_DATASET_OUTPUT_PRINTF(output, "selecting job '%s' failed", jobid.c_str());
        return false;
      }

      string state;
      dataprov_jobrun()->get_job_state(jobid, state);
      SCDC_DATASET_OUTPUT_PRINTF(output, state.c_str());

      undo_select_job();

      return true;
    }


    virtual bool do_cmd_put_inout(const std::string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output) = 0;

    bool do_cmd_put(const std::string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output)
    {
      if (!has_jobid() && dataprov_jobrun()->do_cmd_put(params, input, output)) return true;

      string jobid = get_jobid();

      string p = params;
      scdc_dataprov_jobrun_job_params_t job_params;
      read_job_params2(p, jobid, job_params);

      if (jobid.size() > 0)
      {
        if (!dataprov_jobrun()->is_job(jobid))
        {
          if (!create_job(jobid, job_params))
          {
            SCDC_FAIL("do_cmd_put: creating job '" << jobid << "' failed");
            SCDC_DATASET_OUTPUT_PRINTF(output, "creating job '%s' failed", jobid.c_str());
            return false;
          }

        } else
        {
          if (!update_job(jobid, job_params))
          {
            SCDC_FAIL("do_cmd_put: updating job '" << jobid << "' failed");
            SCDC_DATASET_OUTPUT_PRINTF(output, "updating job '%s' failed", jobid.c_str());
            return false;
          }
        }
      }

      p = jobid + " " + p;

      if (!select_job(p, true, true, jobid))
      {
        SCDC_FAIL("do_cmd_put: selecting job '" << jobid << "' failed");
        SCDC_DATASET_OUTPUT_PRINTF(output, "selecting job '%s' failed", jobid.c_str());
        return false;
      }

      SCDC_ASSERT(dataprov_jobrun()->is_job(jobid));

      bool ret = true;

      ret = ret && do_cmd_put_inout(p, input, output);

      ret = ret && dataprov_jobrun()->job_run(jobid, p, dataprov_jobrun()->local_res);

      undo_select_job();

      return ret;
    }


    virtual bool do_cmd_get_inout(const std::string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output) = 0;

    bool do_cmd_get(const std::string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output)
    {
      if (!has_jobid() && dataprov_jobrun()->do_cmd_get(params, input, output)) return true;

      string jobid;
      string p = params;

      if (!select_job(p, true, true, jobid))
      {
        SCDC_FAIL("do_cmd_put: selecting job '" << jobid.c_str() << "' failed");
        SCDC_DATASET_OUTPUT_PRINTF(output, "selecting job '%s' failed", jobid.c_str());
        return false;
      }

      SCDC_ASSERT(dataprov_jobrun()->is_job(jobid));

      dataprov_jobrun()->job_wait_finished(jobid);

      bool ret = true;

      ret = ret && do_cmd_get_inout(p, input, output);

      undo_select_job();

      return ret;
    }


    virtual bool do_cmd_rm_inout(const std::string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output) = 0;

    bool do_cmd_rm(const std::string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output)
    {
      if (!has_jobid() && dataprov_jobrun()->do_cmd_rm(params, input, output)) return true;

      string jobid;
      string p = params;

      if (!select_job(p, true, true, jobid))
      {
        SCDC_FAIL("do_cmd_put: selecting job '" << jobid.c_str() << "' failed");
        SCDC_DATASET_OUTPUT_PRINTF(output, "selecting job '%s' failed", jobid.c_str());
        return false;
      }

      SCDC_ASSERT(dataprov_jobrun()->is_job(jobid));

      dataprov_jobrun()->job_wait_running(jobid);

      bool ret = true;

      ret = ret && do_cmd_rm_inout(p, input, output);

      ret = ret && destroy_job(jobid);

      undo_select_job();

      set_jobid("");

      return ret;
    }

  private:
    string undo_select_jobid;
};

#undef SCDC_LOG_PREFIX


#define SCDC_LOG_PREFIX  "dataprov-jobrun-system: "

class scdc_dataset_jobrun_system: public scdc_dataset_jobrun
{
  public:
    scdc_dataset_jobrun_system(scdc_dataprov *dataprov_)
      :scdc_dataset_jobrun(dataprov_) { };


    scdc_dataprov_jobrun_system *dataprov_jobrun_system()
    {
      return static_cast<scdc_dataprov_jobrun_system *>(dataprov);
    }


    string workdir(const string &jobid)
    {
      return dataprov_jobrun_system()->get_workdir(jobid);
    }


    string workdir()
    {
      return workdir(get_jobid());
    }


    bool create_job_inout(const string &jobid, scdc_dataprov_jobrun_job_params_t &job_params)
    {
      if (!z_fs_mkdir(workdir(jobid).c_str()))
      {
        SCDC_FAIL("create_job_inout: creating directory failed!");
        return false;
      }

      return true;
    }

    
    bool destroy_job_inout()
    {
      if (!z_fs_rm_r(workdir().c_str()))
      {
        SCDC_FAIL("destroy_job_inout: deleting directory failed!");
        return false;
      }

      return true;
    }


    bool update_job_inout(const string &jobid, scdc_dataprov_jobrun_job_params_t &job_params)
    {
      return true;
    }

    
    bool do_cmd_ls_inout(const std::string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output)
    {
      bool ret = (scdc_dataset_output_redirect(output, "from:fslist", workdir().c_str()) == SCDC_SUCCESS);

      return ret;
    }


    bool do_cmd_put_inout(const std::string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output)
    {
      bool ret = true;

      if (input)
      {
        if (string(input->format) == "fstar")
        {
          SCDC_TRACE(__func__ << ": input to 'fs' in '" << workdir() << "'");
          ret = (scdc_dataset_input_redirect(input, "to:fs", workdir().c_str()) == SCDC_SUCCESS);

        } else
        {
          SCDC_TRACE(__func__ << ": input to 'file' in '" << workdir() << "'");
          ret = (scdc_dataset_input_redirect(input, "to:file", workdir().c_str()) == SCDC_SUCCESS);
        }
      }

      return ret;
    }


    bool do_cmd_get_inout(const std::string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output)
    {
      bool ret = true;

      if (output)
      {
        SCDC_TRACE(__func__ << ": output from 'fs' in '" << workdir() << "': '" << params << "'");

        ret = (scdc_dataset_output_redirect(output, "from:fs", workdir().c_str(), params.c_str()) == SCDC_SUCCESS);
      }

      return ret;
    }


    bool do_cmd_rm_inout(const std::string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output)
    {
      return true;
    }
};

#undef SCDC_LOG_PREFIX


#define SCDC_LOG_PREFIX  "dataprov-jobrun-handler: "

class scdc_dataset_jobrun_handler: public scdc_dataset_jobrun
{
  public:
    scdc_dataset_jobrun_handler(scdc_dataprov *dataprov_)
      :scdc_dataset_jobrun(dataprov_) { };


    scdc_dataprov_jobrun_handler *dataprov_jobrun_handler()
    {
      return static_cast<scdc_dataprov_jobrun_handler *>(dataprov);
    }


    bool create_job_inout(const string &jobid, scdc_dataprov_jobrun_job_params_t &job_params)
    {
      return dataprov_jobrun_handler()->job_do_cmd(jobid, "add", "", 0, 0);
    }


    bool destroy_job_inout()
    {
      return dataprov_jobrun_handler()->job_do_cmd(get_jobid(), "del", "", 0, 0);
    }


    bool update_job_inout(const string &jobid, scdc_dataprov_jobrun_job_params_t &job_params)
    {
      return true;
    }


    bool do_cmd_ls_inout(const std::string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output)
    {
      return dataprov_jobrun_handler()->job_do_cmd(get_jobid(), "ls", params, input, output);
    }


    bool do_cmd_put_inout(const std::string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output)
    {
      return dataprov_jobrun_handler()->job_do_cmd(get_jobid(), "put", params, input, output);
    }


    bool do_cmd_get_inout(const std::string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output)
    {
      return dataprov_jobrun_handler()->job_do_cmd(get_jobid(), "get", params, input, output);
    }


    bool do_cmd_rm_inout(const std::string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output)
    {
      return dataprov_jobrun_handler()->job_do_cmd(get_jobid(), "rm", params, input, output);
    }
};

#undef SCDC_LOG_PREFIX


#define SCDC_LOG_PREFIX  "dataprov-jobrun-sched: "

class scdc_dataprov_jobrun_sched
{
  public:
    struct {
      std::string id;
      scdcint_t cores, free_cores;

    } node_data;

    scdc_dataprov_jobrun *dataprov_jobrun;
    scdcint_t method;

    scdc_dataprov_jobrun_sched():method(0) { };

    /* single node scheduling */

    void node_update(stringlist &sl)
    {
      std::string nodeid = sl.front_pop();

      /* update node information */
      scdc_dataprov_jobrun_node_t node;
      dataprov_jobrun->get_node(nodeid, node);

      if (nodeid == node_data.id)
      {
        node_data.free_cores += node.cores - node_data.cores;
        node_data.cores = node.cores;

      } else
      {
        node_data.cores = node_data.free_cores = node.cores;

        node_data.id = nodeid;
      }
    }

    void node_job_started(const std::string &jobid, const scdc_dataprov_jobrun_res_t &jobres)
    {
      for (scdc_dataprov_jobrun_res_t::const_iterator i = jobres.begin(); i != jobres.end(); ++i) node_data.free_cores -= i->np * i->nt;
    }

    void node_job_stopped(const std::string &jobid, const scdc_dataprov_jobrun_res_t &jobres)
    {
      for (scdc_dataprov_jobrun_res_t::const_iterator i = jobres.begin(); i != jobres.end(); ++i) node_data.free_cores += i->np * i->nt;
    }

    bool node_job_get_runable(string &jobid, scdc_dataprov_jobrun_job_t &job, scdc_dataprov_jobrun_res_t &res)
    {
      jobid = dataprov_jobrun->sched_jobs_wait.front();

      dataprov_jobrun->get_job(jobid, job);

      SCDC_TRACE(__func__ << ": jobid: '" << jobid << "', jobcmds: '" << job.jobcmds << "', np: " << job.params.np_min << "-" << job.params.np_max);
      SCDC_TRACE(__func__ << ": node: '" << node_data.id << "', free cores: " << node_data.free_cores << " of " << node_data.cores);

      if (node_data.free_cores < job.params.np_min * job.params.nt_min) return false;

      dataprov_jobrun->sched_jobs_wait.pop_front();

      scdc_dataprov_jobrun_res_proc_t res_proc;
      res_proc.node = node_data.id;
      res_proc.np = job.params.np_min;
      res_proc.nt = job.params.nt_min;

      res.push_back(res_proc);

      return true;
    }

    /* general scheduling */

    void update(const std::string &conf)
    {
      stringlist sl(' ', conf);

      std::string m = sl.front_pop();

      if (m == "default" || m == "node")
      {
        method = 0;
        node_update(sl);

      } else if (m == "sched")
      {
        method = 1;
      }
    }

    void job_started(const scdcint_t m, const std::string &jobid, const scdc_dataprov_jobrun_res_t &jobres)
    {
      switch (m)
      {
        case 0: return node_job_started(jobid, jobres);
/*        case 1: return sched_get_runable(jobid, res);*/
      }
    }

    void job_stopped(const scdcint_t m, const std::string &jobid, const scdc_dataprov_jobrun_res_t &jobres)
    {
      switch (m)
      {
        case 0: return node_job_stopped(jobid, jobres);
/*        case 1: return sched_get_runable(jobid, jobres);*/
      }
    }

    bool job_get_runable(string &jobid, scdc_dataprov_jobrun_job_t &job, scdc_dataprov_jobrun_res_t &jobres, scdcint_t &m)
    {
      switch (method)
      {
        case 0: m = 0; return node_job_get_runable(jobid, job, jobres);
/*        case 1: m = 1; return sched_job_get_runable(jobid, job, jobres);*/
      }

      return false;
    }
};

#undef SCDC_LOG_PREFIX


#define SCDC_LOG_PREFIX  "dataprov-jobrun: "

scdc_dataprov_jobrun::scdc_dataprov_jobrun(const std::string &type_)
  :scdc_dataprov(type_),
  total_cores(0),
  max_parallel_jobs(0),
  runjobs(true),
  sched_mode("online"), sched_method("default local"),
  sched_run(false)
{
  scdc_dataprov_jobrun_node_t node(1, 1, 1.0);
  add_node("local", node);

  scdc_dataprov_jobrun_res_proc_t local_proc;
  local_proc.node = "local";
  local_proc.np = 1;
  local_proc.nt = 1;

  local_res.push_back(local_proc);
}


bool scdc_dataprov_jobrun::open_config_conf(const std::string &conf, scdc_args *args, bool &done)
{
  done = true;
  bool ret = true;

  if (conf == "cmd" || conf == "jobcmd")
  {
    const char *s;

    if (args->get<const char *>(SCDC_ARGS_TYPE_CSTR, &s) == SCDC_ARG_REF_NULL)
    {
      SCDC_ERROR(__func__ << ": getting parameter for job command");
      ret = false;

    } else
    {
      jobcmd = s;
    }

  } else if (conf == "nonode")
  {
    del_node("local");

  } else
  {
    done = false;
    ret = scdc_dataprov::open_config_conf(conf, args, done);
  }

  return ret;
}


bool scdc_dataprov_jobrun::open(const char *conf, scdc_args *args)
{
  SCDC_TRACE(__func__ << ": '" << conf << "'");

  if (!scdc_dataprov::open(conf, args))
  {
    SCDC_FAIL("open: opening base");
    return false;
  }

  dataset_cmds_add("cd", static_cast<dataset_cmds_do_cmd_f>(&scdc_dataset_jobrun::do_cmd_cd));
  dataset_cmds_add("pwd", static_cast<dataset_cmds_do_cmd_f>(&scdc_dataset_jobrun::do_cmd_pwd));
  dataset_cmds_add("ls", static_cast<dataset_cmds_do_cmd_f>(&scdc_dataset_jobrun::do_cmd_ls));
  dataset_cmds_add("info", static_cast<dataset_cmds_do_cmd_f>(&scdc_dataset_jobrun::do_cmd_info));
  dataset_cmds_add("put", static_cast<dataset_cmds_do_cmd_f>(&scdc_dataset_jobrun::do_cmd_put));
  dataset_cmds_add("get", static_cast<dataset_cmds_do_cmd_f>(&scdc_dataset_jobrun::do_cmd_get));
  dataset_cmds_add("rm", static_cast<dataset_cmds_do_cmd_f>(&scdc_dataset_jobrun::do_cmd_rm));

#if SCDC_LOCK
  pthread_mutex_init(&libscdc_lock, NULL);
#endif

  sched_init();

  return true;
}


void scdc_dataprov_jobrun::close()
{
  SCDC_TRACE(__func__ << ":");

  sched_release();

#if SCDC_LOCK
  pthread_mutex_destroy(&libscdc_lock);
#endif

  scdc_dataprov::close();
}


template<class DATASET_JOBRUN>
scdc_dataset *scdc_dataprov_jobrun::dataset_open(const char *path, scdcint_t path_size, scdc_dataset_output_t *output)
{
  SCDC_TRACE(__func__ << ": '" << string(path, path_size) << "'");

  scdc_dataset *dataset = 0;
  
  if (config_open(path, path_size, output, &dataset)) return dataset;

  DATASET_JOBRUN *dataset_jobrun = 0;

  dataset_jobrun = new DATASET_JOBRUN(this);

  string s(path, path_size);
  dataset_jobrun->do_cmd_cd(ltrim(s, "/").c_str(), 0, output);

  SCDC_TRACE(__func__ << ": return: '" << dataset_jobrun << "'");

  return dataset_jobrun;
}


void scdc_dataprov_jobrun::dataset_close(scdc_dataset *dataset, scdc_dataset_output_t *output)
{
  SCDC_TRACE(__func__ << ": '" << dataset << "'");

  if (config_close(dataset, output)) return;

  delete dataset;

  SCDC_TRACE(__func__ << ": return");
}


bool scdc_dataprov_jobrun::config_do_cmd_param(const std::string &cmd, const std::string &param, std::string val, scdc_config_result &result, bool &done)
{
  SCDC_TRACE(__func__ << ": cmd: '" << cmd << "', param: '" << param << "', val: '" << val << "', result: '" << result << "'");

  done = true;
  bool ret = true;

  if (param == "")
  {
    ret = scdc_dataprov::config_do_cmd_param(cmd, "", val, result, done);
    if (!ret) goto do_quit;

    if (cmd == "info")
    {
      done = true;

    } else if (cmd == "ls")
    {
      done = true;

      ret = scdc_dataprov_config::ls(result, "jobcmd")
         && scdc_dataprov_config::ls(result, "nodes")
         && scdc_dataprov_config::ls(result, "cores")
         && scdc_dataprov_config::ls(result, "max_parallel_jobs")
         && scdc_dataprov_config::ls(result, "performance")
         && scdc_dataprov_config::ls(result, "runjobs")
         && scdc_dataprov_config::ls(result, "sched_mode")
         && scdc_dataprov_config::ls(result, "sched_method");

    } else done = false;

  } else
  {
    ret = scdc_dataprov::config_do_cmd_param(cmd, param, val, result, done);
    if (!ret) goto do_quit;

    if (done);
    else if (cmd == "info")
    {
      done = true;

      if (param == "jobcmd")                 ret = scdc_dataprov_config::info(result, param, "command for running a job (string)");
      else if (param == "nodes")             ret = scdc_dataprov_config::info(result, param, "compute nodes for executing jobs (string list)");
      else if (param == "cores")             ret = scdc_dataprov_config::info(result, param, "number of cores of a node or in total (int)");
      else if (param == "max_parallel_jobs") ret = scdc_dataprov_config::info(result, param, "maximum number of parallel jobs of a node or in total (int)");
      else if (param == "performance")       ret = scdc_dataprov_config::info(result, param, "performance factor of a node (double)");
      else if (param == "runjobs")           ret = scdc_dataprov_config::info(result, param, "whether jobs should be run (bool)");
      else if (param == "sched_mode")        ret = scdc_dataprov_config::info(result, param, "how scheduling is invoked (string)");
      else if (param == "sched_method")      ret = scdc_dataprov_config::info(result, param, "which scheduling method is used (string)");
      else done = false;

    } else if (cmd == "put")
    {
      done = true;

      if (param == "jobcmd")                 ret = scdc_dataprov_config::put<string>(result, param, val, jobcmd);
      else if (param == "nodes")             ret = nodes_config(cmd, 0, val, result) && sched_change();
      else if (param == "cores")             ret = nodes_config(cmd, 1, val, result) && sched_change();
      else if (param == "max_parallel_jobs") ret = nodes_config(cmd, 2, val, result) && sched_change();
      else if (param == "performance")       ret = nodes_config(cmd, 3, val, result) && sched_change();
      else if (param == "runjobs")           ret = scdc_dataprov_config::put<bool>(result, param, val, runjobs) && sched_change();
      else if (param == "sched_mode")        ret = scdc_dataprov_config::put<string>(result, param, val, sched_mode) && sched_change();
      else if (param == "sched_method")      ret = scdc_dataprov_config::put<string>(result, param, val, sched_method) && sched_change();
      else done = false;

    } else if (cmd == "get")
    {
      done = true;

      if (param == "jobcmd")                 ret = scdc_dataprov_config::get<string>(result, param, jobcmd);
      else if (param == "nodes")             ret = nodes_config(cmd, 0, val, result);
      else if (param == "cores")             ret = nodes_config(cmd, 1, val, result);
      else if (param == "max_parallel_jobs") ret = nodes_config(cmd, 2, val, result);
      else if (param == "performance")       ret = nodes_config(cmd, 3, val, result);
      else if (param == "runjobs")           ret = scdc_dataprov_config::get<bool>(result, param, runjobs);
      else if (param == "sched_mode")        ret = scdc_dataprov_config::get<string>(result, param, sched_mode);
      else if (param == "sched_method")      ret = scdc_dataprov_config::get<string>(result, param, sched_method);
      else done = false;

    } else if (cmd == "rm")
    {
      done = true;

      if (param == "nodes") ret = nodes_config(cmd, 0, val, result);
      else done = false;

    } else done = false;
  }

do_quit:
  if (done && !ret)
  {
    SCDC_FAIL("config_do_cmd_param: command '" << cmd << "' failed");
    return false;
  }

  return ret;
}


bool scdc_dataprov_jobrun::do_cmd_ls(const std::string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output)
{
  stringstream ss;

  pthread_mutex_lock(&sched_lock);
  for (jobs_t::iterator i = jobs.begin(); i != jobs.end(); ++i)
  {
    string state;

    get_job_state(i->first, state, true);

    ss << i->first << ":" << i->second.params.nt_min << "-" << i->second.params.nt_max << ":" << i->second.params.np_min << "-" << i->second.params.np_max << ":" << state << ",";
  }
  pthread_mutex_unlock(&sched_lock);

  string s = ss.str();

  if (s.size() > 0) s.resize(s.size() - 1);

  SCDC_DATASET_OUTPUT_PRINTF(output, s.c_str());

  return true;
}


bool scdc_dataprov_jobrun::do_cmd_info(const std::string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output)
{
  stringstream ss;

  pthread_mutex_lock(&sched_lock);
  ss << jobs.size() << ":" << sched_jobs_wait.size() << ":" << sched_jobs_run.size();
  pthread_mutex_unlock(&sched_lock);

  SCDC_DATASET_OUTPUT_PRINTF(output, ss.str().c_str());

  return true;
}


bool scdc_dataprov_jobrun::do_cmd_put(const std::string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output)
{
  SCDC_TRACE(__func__ << ": job: '" << params << "'");

  return false;
}


bool scdc_dataprov_jobrun::do_cmd_get(const std::string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output)
{
  SCDC_TRACE(__func__ << ": job: '" << params << "'");

  return false;
}


bool scdc_dataprov_jobrun::do_cmd_rm(const std::string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output)
{
  SCDC_TRACE(__func__ << ": job: '" << params << "'");

  return false;
}


bool scdc_dataprov_jobrun::add_node(const std::string &nodeid, scdc_dataprov_jobrun_node_t &node)
{
  SCDC_TRACE(__func__ << ": nodeid: '" << nodeid << "', cores: " << node.cores << ", max_parallel_jobs: " << node.max_parallel_jobs << ", performance: " << node.performance);

  pair<nodes_t::iterator, bool> r = nodes.insert(nodes_t::value_type(nodeid, node));

  if (!r.second)
  {
    SCDC_FAIL("add_node: adding node failed");
    return false;
  }

  total_cores += node.cores;
  max_parallel_jobs += node.max_parallel_jobs;

  return true;
}


bool scdc_dataprov_jobrun::del_node(const std::string &nodeid)
{
  SCDC_TRACE(__func__ << ": nodeid: '" << nodeid << "'");

  nodes_t::iterator i = nodes.find(nodeid);

  scdc_dataprov_jobrun_node_t node;

  if (i != nodes.end()) node = i->second;
  else node.cores = node.max_parallel_jobs = 0;

  scdcint_t r = nodes.erase(nodeid);

  if (r <= 0)
  {
    SCDC_FAIL("del_node: deleting node failed");
    return false;
  }

  total_cores -= node.cores;
  max_parallel_jobs -= node.max_parallel_jobs;

  return true;
}


bool scdc_dataprov_jobrun::get_node(const std::string &nodeid, scdc_dataprov_jobrun_node_t &node)
{
  SCDC_TRACE(__func__ << ": nodeid: '" << nodeid << "'");

  nodes_t::iterator i = nodes.find(nodeid);

  if (i == nodes.end())
  {
    SCDC_FAIL("get_node: getting node failed");
    return false;
  }

  node = i->second;

  return true;
}


bool scdc_dataprov_jobrun::set_node(const std::string &nodeid, scdcint_t cores, scdcint_t max_parallel_jobs, double performance)
{
  SCDC_TRACE(__func__ << ": nodeid: '" << nodeid << "', cores: '" << cores << "', max_parallel_jobs: '" << max_parallel_jobs << "', performance: '" << performance << "'");

  nodes_t::iterator i = nodes.find(nodeid);

  if (i == nodes.end())
  {
    SCDC_FAIL("set_node: setting node failed");
    return false;
  }

  if (cores >= 0) i->second.cores = cores;

  if (max_parallel_jobs >= 0) i->second.max_parallel_jobs = max_parallel_jobs;

  if (performance >= 0) i->second.performance = performance;

  return true;
}


bool scdc_dataprov_jobrun::nodes_config(const std::string &cmd, scdcint_t param, std::string &val, scdc_config_result &result)
{
  SCDC_TRACE(__func__ << ": cmd: '" << cmd << "', param: '" << param << "', val: '" << val << "', result: '" << result << "'");

  bool ret = false;

  if (param == 0) /* nodes */
  {
    if (cmd == "put")
    {
      stringlist sl(':', val);

      string nodeid, s;
      scdc_dataprov_jobrun_node_t node(1, 1, 1.0);

      sl.front_pop(nodeid);

      if (sl.front_pop(s) && s != "") stringstream(s) >> node.cores;

      if (sl.front_pop(s) && s != "") stringstream(s) >> node.performance;

      if (sl.front_pop(s) && s != "") stringstream(s) >> node.max_parallel_jobs;
      else node.max_parallel_jobs = node.cores;

      ret = add_node(nodeid, node);

    } else if (cmd == "get")
    {
      nodes_t::iterator nodes_begin, nodes_end;

      if (val == "") { nodes_begin = nodes.begin(); nodes_end = nodes.end(); }
      else
      {
        nodes_begin = nodes_end = nodes.find(val);
        ++nodes_end;
      }

      for (nodes_t::iterator i = nodes_begin; i != nodes_end; ++i)
      {
        char s[64], *ps = s;
        sprintf(s, "%s:%" scdcint_fmt ":%f:%" scdcint_fmt, i->first.c_str(), i->second.cores, i->second.performance, i->second.max_parallel_jobs);
        ret = scdc_dataprov_config::get<char *>(result, "nodes", ps);
      }
      
    } else if (cmd == "rm")
    {
      ret = del_node(val);
    }

  } else if (param == 1) /* cores */
  {
    if (cmd == "put")
    {
      string nodeid;
      scdcint_t cores;

      ret = scdc_dataprov_config::put<string>(result, "cores", val, nodeid)
         && scdc_dataprov_config::put<scdcint_t>(result, "cores", val, cores)
         && set_node(nodeid, cores, -1, -1);

    } else if (cmd == "get")
    {
      scdc_dataprov_jobrun_node_t node;

      if (val == "") ret = scdc_dataprov_config::get<scdcint_t>(result, "cores", total_cores);
      else
      {
        if (get_node(val, node)) ret = scdc_dataprov_config::get<scdcint_t>(result, "cores", node.cores);
        else ret = scdc_dataprov_config::fail(result, "cores", "getting node " + val + " failed");
      }
    }

  } else if (param == 2) /* max_parallel_jobs */
  {
    if (cmd == "put")
    {
      string nodeid;
      scdcint_t max_parallel_jobs;

      ret = scdc_dataprov_config::put<string>(result, "max_parallel_jobs", val, nodeid)
         && scdc_dataprov_config::put<scdcint_t>(result, "max_parallel_jobs", val, max_parallel_jobs)
         && set_node(nodeid, -1, max_parallel_jobs, -1);

    } else if (cmd == "get")
    {
      scdc_dataprov_jobrun_node_t node;

      if (val == "") ret = scdc_dataprov_config::get<scdcint_t>(result, "max_parallel_jobs", max_parallel_jobs);
      else
      {
        if (get_node(val, node)) ret = scdc_dataprov_config::get<scdcint_t>(result, "max_parallel_jobs", node.max_parallel_jobs);
        else ret = scdc_dataprov_config::fail(result, "max_parallel_jobs", "getting node " + val + " failed");
      }
    }

  } else if (param == 3) /* performance */
  {
    if (cmd == "put")
    {
      string nodeid;
      double performance;

      ret = scdc_dataprov_config::put<string>(result, "performance", val, nodeid)
         && scdc_dataprov_config::put<double>(result, "performance", val, performance)
         && set_node(nodeid, -1, -1, performance);

    } else if (cmd == "get")
    {
      scdc_dataprov_jobrun_node_t node;

      if (get_node(val, node)) ret = scdc_dataprov_config::get<double>(result, "performance", node.performance);
      else ret = scdc_dataprov_config::fail(result, "performance", "getting node " + val + " failed");
    }
  }

  return ret;
}


bool scdc_dataprov_jobrun::add_job(const std::string &jobid, scdc_dataprov_jobrun_job_t &job)
{
  SCDC_INFO("adding job '" << jobid << "'");

  pair<jobs_t::iterator, bool> r = jobs.insert(jobs_t::value_type(jobid, job));

  return r.second;
}


bool scdc_dataprov_jobrun::del_job(const std::string &jobid)
{
  SCDC_INFO("removing job '" << jobid << "'");

  return (jobs.erase(jobid) > 0);
}


bool scdc_dataprov_jobrun::is_job(const std::string &jobid)
{
  return (jobs.find(jobid) != jobs.end());
}


bool scdc_dataprov_jobrun::set_job(const std::string &jobid, scdc_dataprov_jobrun_job_t &job)
{
  jobs_t::iterator i = jobs.find(jobid);

  if (i == jobs.end()) return false;

  i->second = job;

  return true;
}


bool scdc_dataprov_jobrun::set_job_params(scdc_dataprov_jobrun_job_t &job, scdc_dataprov_jobrun_job_params_t &job_params)
{
  job.params = job_params;

  if (job.params.nt_max <= 0) job.params.nt_max = 1;
  if (job.params.nt_min <= 0) job.params.nt_min = job.params.nt_max;

  if (job.params.np_max <= 0) job.params.np_max = 1;
  if (job.params.np_min <= 0) job.params.np_min = job.params.np_max;

  return true;
}


bool scdc_dataprov_jobrun::set_job_params(const std::string &jobid, scdc_dataprov_jobrun_job_params_t &job_params)
{
  jobs_t::iterator i = jobs.find(jobid);

  if (i == jobs.end()) return false;

  if (!set_job_params(i->second, job_params)) return false;

  return true;
}


bool scdc_dataprov_jobrun::get_job(const std::string &jobid, scdc_dataprov_jobrun_job_t &job)
{
  jobs_t::iterator i = jobs.find(jobid);

  if (i == jobs.end()) return false;

  job = i->second;

  return true;
}


bool scdc_dataprov_jobrun::get_job_state(const std::string &jobid, std::string &state, bool sched_locked)
{
  if (!is_job(jobid)) state = "unknown";
  else
  {
    /* waiting | running | done */

    if (!sched_locked) pthread_mutex_lock(&sched_lock);

    if (find(sched_jobs_wait.begin(), sched_jobs_wait.end(), jobid) != sched_jobs_wait.end()) state = "waiting";
    else if (sched_jobs_run.find(jobid) != sched_jobs_run.end()) state = "running";
    else state = "done";

    if (!sched_locked) pthread_mutex_unlock(&sched_lock);
  }

  return true;
}


static void string_replace_all(string &str, const std::string &from, const std::string &to)
{
  size_t pos = 0;
  size_t from_n = from.size();
  size_t to_n = to.size();

  while ((pos = str.find(from, pos)) != string::npos)
  {
    str.replace(pos, from_n, to);
    pos += to_n;
  }
}


static void build_cmd(const std::string &format, const char *run_time, const std::string &jobid, const std::string &params, const scdc_dataprov_jobrun_res_t &res, std::string &cmd)
{
  cmd = format;

  SCDC_TRACE(__func__ << ": format: '" << format << "'");

  /* replace $PARAMS$ first so that all following placeholders are also replaced in the params string */
  string_replace_all(cmd, "$PARAMS$", params);

  string_replace_all(cmd, "$RUN_TIME$", run_time);
  string_replace_all(cmd, "$JOBID$", jobid);

  stringstream nodes, nprocs, nthrds, npt, tpn;

  /* - $NODES$ = '<node0>,<node1>,...',
     - $NT$ bzw. $NTHRDS$ = '<nt0>,<nt1>,...',
     - $NP$ bzw. $NPROCS$ = '<np0>,<np1>,...',
     - $NPT$ = '<node0>:<np0>:<nt0>,<node1>:<np1>:<nt1>,...'
     - $TPN$ = '<nt0>:<np0>:<node0>,<nt1>:<np1>:<node1>,...' */

  for (scdc_dataprov_jobrun_res_t::const_iterator i = res.begin(); i != res.end(); ++i)
  {
    if (i != res.begin())
    {
      nodes << ",";
      nprocs << ",";
      nthrds << ",";
      npt << ",";
      tpn << ",";
    }

    nodes << i->node;
    nprocs << i->np;
    nthrds << i->nt;
    npt << i->node << ":" << i->np << ":" << i->nt;
    tpn << i->nt << ":" << i->np << ":" << i->node;
  }

  SCDC_TRACE(__func__ << ": nodes: '" << nodes.str() << "'");
  SCDC_TRACE(__func__ << ": nprocs: '" << nprocs.str() << "'");
  SCDC_TRACE(__func__ << ": nthrds: '" << nthrds.str() << "'");
  SCDC_TRACE(__func__ << ": npt: '" << npt.str() << "'");
  SCDC_TRACE(__func__ << ": tpn: '" << tpn.str() << "'");

  string_replace_all(cmd, "$NODES$", nodes.str());
  string_replace_all(cmd, "$NP$", nprocs.str());
  string_replace_all(cmd, "$NPROCS$", nprocs.str());
  string_replace_all(cmd, "$NT$", nthrds.str());
  string_replace_all(cmd, "$NTHRDS$", nthrds.str());
  string_replace_all(cmd, "$NPT$", npt.str());
  string_replace_all(cmd, "$TPN$", tpn.str());
}


bool scdc_dataprov_jobrun::job_create(const std::string &jobid, scdc_dataprov_jobrun_job_params_t &job_params)
{
  scdc_dataprov_jobrun_job_t job;

  bool ret = set_job_params(job, job_params);

  if (ret)
  {
    pthread_mutex_lock(&sched_lock);
    ret = add_job(jobid, job);
    pthread_mutex_unlock(&sched_lock);
  }

  if (!ret)
  {
    SCDC_FAIL("job_create: adding job failed");
    return false;
  }

  return true;
}


bool scdc_dataprov_jobrun::job_destroy(const std::string &jobid)
{
  pthread_mutex_lock(&sched_lock);
  bool ret = del_job(jobid);
  pthread_mutex_unlock(&sched_lock);

  if (!ret)
  {
    SCDC_FAIL("job_destroy: deleting job failed");
    return false;
  }

  return true;
}


bool scdc_dataprov_jobrun::job_update(const std::string &jobid, scdc_dataprov_jobrun_job_params_t &job_params)
{
  pthread_mutex_lock(&sched_lock);
  bool ret = set_job_params(jobid, job_params);
  pthread_mutex_unlock(&sched_lock);

  if (!ret)
  {
    SCDC_FAIL("job_update: updating job failed");
    return false;
  }

  return true;
}


bool scdc_dataprov_jobrun::job_run(const std::string &jobid, const std::string &jobcmds, const scdc_dataprov_jobrun_res_t &res)
{
  SCDC_TRACE(__func__ << ": jobid: '" << jobid << "', jobcmds: '" << jobcmds << "', res: " << res.size());

  bool ret = true;

  bool record = false;
  string record_jobcmds;

  stringlist cmds(';', jobcmds);
  string s;
  while (cmds.front_pop(s))
  {
    SCDC_TRACE(__func__ << ": cmd: '" << s << "'");

    stringlist cmd(' ', s);
    string c = cmd.front_pop();
    string p = cmd.conflate();

    if (c == "jobbegin")
    {
      SCDC_TRACE(__func__ << ": begin");

      if (!record) record = true;

    } else if (c == "jobend")
    {
      SCDC_TRACE(__func__ << ": end");

      if (record)
      {
        record = false;

        scdc_dataprov_jobrun_job_t job;

        pthread_mutex_lock(&sched_lock);
        get_job(jobid, job);
        job.jobcmds = record_jobcmds;
        set_job(jobid, job);
        pthread_mutex_unlock(&sched_lock);

        job_schedule(jobid);
      }

    } else if (record)
    {
      SCDC_TRACE(__func__ << ": record: '" << s << "'");

      record_jobcmds += s + ";";

    } else if (c == "jobwait")
    {
      SCDC_TRACE(__func__ << ": wait");

      job_wait_finished(jobid);

    } else
    {
      SCDC_TRACE(__func__ << ": run:");

      ret = job_do_run(jobid, c, p, res);

      SCDC_TRACE(__func__ << ": run: " << ret);
    }
  }

  return ret;
}


bool scdc_dataprov_jobrun::job_schedule(const std::string &jobid)
{
  pthread_mutex_lock(&sched_lock);
  sched_jobs_wait.push_back(jobid);
  pthread_mutex_unlock(&sched_lock);

  pthread_cond_signal(&sched_cond);

  return true;
}


bool scdc_dataprov_jobrun::job_wait_running(const std::string &jobid)
{
  SCDC_TRACE(__func__ << ": jobid: '" << jobid << "'");

  bool wait = false;
  pthread_t thread;

  pthread_mutex_lock(&sched_lock);
  sched_jobs_run_t::iterator i = sched_jobs_run.find(jobid);
  if (i != sched_jobs_run.end())
  {
    wait = true;
    thread = i->second;
  }
  pthread_mutex_unlock(&sched_lock);

  SCDC_TRACE(__func__ << ": wait: '" << wait << "'");

  if (!wait) return false;

  pthread_join(thread, NULL);

  return true;
}


bool scdc_dataprov_jobrun::job_wait_finished(const std::string &jobid)
{
  SCDC_TRACE(__func__ << ": jobid: '" << jobid << "'");

  scdc_dataprov_jobrun_job_t job;

  pthread_mutex_lock(&sched_lock);
  get_job(jobid, job);
  while (job.waiting)
  {
    pthread_cond_wait(&sched_cond_finished, &sched_lock);
    get_job(jobid, job);
  }
  pthread_mutex_unlock(&sched_lock);

  job_wait_running(jobid);

  return true;
}


typedef struct _sched_job_arg_t
{
  scdc_dataprov_jobrun *dataprov_jobrun;
  string jobid, jobcmds;
  scdc_dataprov_jobrun_res_t jobres;
  scdcint_t m;

} sched_job_arg_t;


static void *sched_job_routine(void *arg)
{
  sched_job_arg_t *sched_job_arg = static_cast<sched_job_arg_t *>(arg);

  SCDC_TRACE(__func__ << ": jobid: '" << sched_job_arg->jobid << "'");

  sched_job_arg->dataprov_jobrun->job_run(sched_job_arg->jobid, sched_job_arg->jobcmds.c_str(), sched_job_arg->jobres);

  pthread_mutex_lock(&sched_job_arg->dataprov_jobrun->sched_lock);
  sched_job_arg->dataprov_jobrun->sched_jobs_run.erase(sched_job_arg->jobid);
  sched_job_arg->dataprov_jobrun->sched->job_stopped(sched_job_arg->m, sched_job_arg->jobid, sched_job_arg->jobres);
  pthread_mutex_unlock(&sched_job_arg->dataprov_jobrun->sched_lock);

  pthread_cond_signal(&sched_job_arg->dataprov_jobrun->sched_cond);

  SCDC_TRACE(__func__ << ": return: jobid: '" << sched_job_arg->jobid << "'");

  delete sched_job_arg;

  pthread_exit(NULL);
}


static void *sched_run_routine(void *arg)
{
  SCDC_TRACE(__func__ << ": arg: '" << arg << "'");

  scdc_dataprov_jobrun *dataprov_jobrun = static_cast<scdc_dataprov_jobrun *>(arg);

  pthread_mutex_lock(&dataprov_jobrun->sched_lock);
  while (dataprov_jobrun->sched_run)
  {
    struct timespec abstime;
    abstimeout(&abstime, 0.1, -1);

    pthread_cond_timedwait(&dataprov_jobrun->sched_cond, &dataprov_jobrun->sched_lock, &abstime);

/*    SCDC_TRACE(__func__ << ": " << time(NULL) << " TICK-TACK");*/

    if (!dataprov_jobrun->runjobs) continue;

    while (dataprov_jobrun->sched_jobs_wait.size() > 0)
    {
      /* check scheduling method */
      dataprov_jobrun->sched->update(dataprov_jobrun->sched_method);

      string jobid;
      scdc_dataprov_jobrun_job_t job;
      scdc_dataprov_jobrun_res_t jobres;
      scdcint_t m;

      if (!dataprov_jobrun->sched->job_get_runable(jobid, job, jobres, m)) break;

      sched_job_arg_t *sched_job_arg = new sched_job_arg_t;
      sched_job_arg->dataprov_jobrun = dataprov_jobrun;
      sched_job_arg->jobid = jobid;
      sched_job_arg->jobcmds = job.jobcmds;
      sched_job_arg->jobres = jobres;
      sched_job_arg->m = m;

      dataprov_jobrun->sched->job_started(m, jobid, sched_job_arg->jobres);

      pthread_t thread;
      pthread_create(&thread, NULL, sched_job_routine, sched_job_arg);

      dataprov_jobrun->sched_jobs_run.insert(scdc_dataprov_jobrun::sched_jobs_run_t::value_type(jobid, thread));

      job.waiting = false;
      dataprov_jobrun->set_job(jobid, job);

      pthread_cond_signal(&dataprov_jobrun->sched_cond_finished);
    }
  }
  pthread_mutex_unlock(&dataprov_jobrun->sched_lock);

  SCDC_TRACE(__func__ << ": return");

  pthread_exit(NULL);
}


void scdc_dataprov_jobrun::sched_init()
{
  SCDC_TRACE(__func__ << ":");

  sched = new scdc_dataprov_jobrun_sched();
  sched->dataprov_jobrun = this;

  sched_run = true;

  pthread_mutex_init(&sched_lock, NULL);
  pthread_cond_init(&sched_cond, NULL);
  pthread_cond_init(&sched_cond_finished, NULL);

  pthread_create(&sched_run_thread, NULL, sched_run_routine, this);

  SCDC_TRACE(__func__ << ": return");
}


void scdc_dataprov_jobrun::sched_release()
{
  SCDC_TRACE(__func__ << ":");

  pthread_mutex_lock(&sched_lock);
  sched_run = false;
  pthread_mutex_unlock(&sched_lock);

#if SCHED_RUN_THREAD_CANCEL
  pthread_cancel(sched_thread);
#else
  pthread_cond_signal(&sched_cond);
#endif
  pthread_join(sched_run_thread, NULL);

  pthread_mutex_lock(&sched_lock);
  while (sched_jobs_run.size() > 0)
  {
    pthread_t thread = sched_jobs_run.begin()->second;
    pthread_mutex_unlock(&sched_lock);
    pthread_join(thread, NULL);
    pthread_mutex_lock(&sched_lock);
  }
  pthread_mutex_unlock(&sched_lock);

  pthread_cond_broadcast(&sched_cond_finished);

  pthread_cond_destroy(&sched_cond);
  pthread_cond_destroy(&sched_cond_finished);
  pthread_mutex_destroy(&sched_lock);

  delete sched;

  SCDC_TRACE(__func__ << ": return");
}


bool scdc_dataprov_jobrun::sched_change()
{
  pthread_cond_signal(&sched_cond);

  return true;
}


#undef SCDC_LOG_PREFIX

#define SCDC_LOG_PREFIX  "dataprov-jobrun-system: "

#if USE_SYSTEM_PID
#define SYSTEM_PID_CMD_MAX_SIZE  1024

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>


static pid_t system_pid(const char *cmd)
{
  pid_t pid = fork();

  if (pid != 0) return pid;

  char cmd_sh[] = "/bin/sh";
  char cmd_c[] = "-c";
  char cmd_cmd[SYSTEM_PID_CMD_MAX_SIZE];
  strncpy(cmd_cmd, cmd, SYSTEM_PID_CMD_MAX_SIZE);

  char *const argv[] = { cmd_sh, cmd_c, cmd_cmd, NULL };

  execv(cmd_sh, argv);

  SCDC_TRACE(__func__ << ": executing '/bin/sh -c " << cmd << "' failed (errno = " << errno << ": " << strerror(errno) << ")");

  exit(1);

  return -1;
}
#endif


scdc_dataprov_jobrun_system::scdc_dataprov_jobrun_system()
  :scdc_dataprov_jobrun("jobrun:system"),
  xterm(false), xterm_hold(false), Xvfb(false), show_output(false), sleep_before_run(0), sleep_after_run(0)
{
  jobcmd = "echo %run_time% %nodes%:%nprocs%";
}


bool scdc_dataprov_jobrun_system::open_config_conf(const std::string &conf, scdc_args *args, bool &done)
{
  done = true;
  bool ret = true;

  if (conf == "workdir")
  {
    const char *s;

    if (args->get<const char *>(SCDC_ARGS_TYPE_CSTR, &s) == SCDC_ARG_REF_NULL)
    {
      SCDC_ERROR(__func__ << ": getting parameter for work directory");
      ret = false;

    } else
    {
      if (!z_fs_is_directory(s))
      {
        SCDC_ERROR(__func__ << ": given work directory '" << s << "' is not a directory");
        ret = false;

      } else
      {
        workdir = s;

        if (*(workdir.end() - 1) != '/') workdir += '/';
      }
    }

  } else
  {
    done = false;
    ret = scdc_dataprov_jobrun::open_config_conf(conf, args, done);
  }

  return ret;
}


bool scdc_dataprov_jobrun_system::open(const char *conf, scdc_args *args)
{
  SCDC_TRACE(__func__ << ": conf: '" << conf << "'");

  bool ret = true;

  const char *s;

  if (args->get<const char *>(SCDC_ARGS_TYPE_CSTR, &s) == SCDC_ARG_REF_NULL)
  {
    SCDC_ERROR("open: getting parameter for job command");
    ret = false;
    goto do_quit;
  }

  jobcmd = s;

  if (!scdc_dataprov_jobrun::open(conf, args))
  {
    SCDC_FAIL("open: opening base");
    ret = false;
  }

  SCDC_TRACE(__func__ << ": jobcmd: '" << jobcmd << "', workdir: '" << workdir << "'");

do_quit:
  return ret;
}


void scdc_dataprov_jobrun_system::close()
{
  SCDC_TRACE(__func__ << ":");

  scdc_dataprov_jobrun::close();
}


scdc_dataset *scdc_dataprov_jobrun_system::dataset_open(const char *path, scdcint_t path_size, scdc_dataset_output_t *output)
{
  return scdc_dataprov_jobrun::dataset_open<scdc_dataset_jobrun_system>(path, path_size, output);
}


bool scdc_dataprov_jobrun_system::config_do_cmd_param(const std::string &cmd, const std::string &param, std::string val, scdc_config_result &result, bool &done)
{
  SCDC_TRACE(__func__ << ": cmd: '" << cmd << "', param: '" << param << "', val: '" << val << "', result: '" << result << "'");

  done = true;
  bool ret = true;

  if (param == "")
  {
    ret = scdc_dataprov_jobrun::config_do_cmd_param(cmd, "", val, result, done);
    if (!ret) goto do_quit;

    if (cmd == "info")
    {
      done = true;

      ret = scdc_dataprov_config::info(result, param, "job-system");

    } else if (cmd == "ls")
    {
      done = true;

      ret = scdc_dataprov_config::ls(result, "workdir")
         && scdc_dataprov_config::ls(result, "xterm")
         && scdc_dataprov_config::ls(result, "xterm_hold")
         && scdc_dataprov_config::ls(result, "Xvfb")
         && scdc_dataprov_config::ls(result, "show_output")
         && scdc_dataprov_config::ls(result, "sleep_before_run")
         && scdc_dataprov_config::ls(result, "sleep_after_run");

    } else done = false;

  } else
  {
    ret = scdc_dataprov_jobrun::config_do_cmd_param(cmd, param, val, result, done);
    if (!ret) goto do_quit;

    if (done);
    else if (cmd == "info")
    {
      done = true;

      if (param == "workdir")               ret = scdc_dataprov_config::info(result, param, "root of working directory for jobs (string)");
      else if (param == "xterm")            ret = scdc_dataprov_config::info(result, param, "whether a job should run in an xterm (bool)");
      else if (param == "xterm_hold")       ret = scdc_dataprov_config::info(result, param, "whether the xterm should be closed automatically (boolean)");
      else if (param == "Xvfb")             ret = scdc_dataprov_config::info(result, param, "whether a dummy X server should be started (boolean)");
      else if (param == "show_output")      ret = scdc_dataprov_config::info(result, param, "whether the command output should be shown (boolean)");
      else if (param == "sleep_before_run") ret = scdc_dataprov_config::info(result, param, "number of seconds to sleep before runing a job (integer)");
      else if (param == "sleep_after_run")  ret = scdc_dataprov_config::info(result, param, "number of seconds to sleep after runing a job (integer)");
      else done = false;

    } else if (cmd == "put")
    {
      done = true;

      if (param == "workdir")               ret = scdc_dataprov_config::put<string>(result, param, val, workdir);
      else if (param == "xterm")            ret = scdc_dataprov_config::put<bool>(result, param, val, xterm);
      else if (param == "xterm_hold")       ret = scdc_dataprov_config::put<bool>(result, param, val, xterm_hold);
      else if (param == "Xvfb")             ret = scdc_dataprov_config::put<bool>(result, param, val, Xvfb);
      else if (param == "show_output")      ret = scdc_dataprov_config::put<bool>(result, param, val, show_output);
      else if (param == "sleep_before_run") ret = scdc_dataprov_config::put<scdcint_t>(result, param, val, sleep_before_run);
      else if (param == "sleep_after_run")  ret = scdc_dataprov_config::put<scdcint_t>(result, param, val, sleep_after_run);
      else done = false;

    } else if (cmd == "get")
    {
      done = true;

      if (param == "workdir")               ret = scdc_dataprov_config::get<string>(result, param, workdir);
      else if (param == "xterm")            ret = scdc_dataprov_config::get<bool>(result, param, xterm);
      else if (param == "xterm_hold")       ret = scdc_dataprov_config::get<bool>(result, param, xterm_hold);
      else if (param == "Xvfb")             ret = scdc_dataprov_config::get<bool>(result, param, Xvfb);
      else if (param == "show_output")      ret = scdc_dataprov_config::get<bool>(result, param, show_output);
      else if (param == "sleep_before_run") ret = scdc_dataprov_config::get<scdcint_t>(result, param, sleep_before_run);
      else if (param == "sleep_after_run")  ret = scdc_dataprov_config::get<scdcint_t>(result, param, sleep_after_run);
      else done = false;

    } else done = false;
  }

do_quit:  
  if (done && !ret)
  {
    SCDC_FAIL("config_do_cmd_param: command '" << cmd << "' with param '" << param << "' failed");
    return false;
  }

  return ret;
}


bool scdc_dataprov_jobrun_system::job_do_run(const std::string &jobid, const std::string &cmd, const std::string &params, const scdc_dataprov_jobrun_res_t &res)
{
  SCDC_TRACE(__func__ << ": jobid: '" << jobid << ", cmd: '" << cmd << "', params: '" << params << "', res: " << res.size());

  bool ret = true;

  if (cmd == "scdc")
  {
    SCDC_TRACE(__func__ << ": scdc '" << params << "'");

    scdc_dataset_input_t *get_input = NULL;
    scdc_dataset_output_t get_output_, *get_output = &get_output_;

#if LOCK_LIBSCDC
    pthread_mutex_lock(&libscdc_lock);
#endif
    get_output = scdc_dataset_output_create(get_output, "fs", get_workdir(jobid).c_str());
    scdc_dataset_cmd(SCDC_DATASET_NULL, params.c_str(), get_input, get_output);
    scdc_dataset_output_destroy(get_output);
#if LOCK_LIBSCDC
    pthread_mutex_unlock(&libscdc_lock);
#endif

  } else if (cmd == "exec")
  {
    SCDC_TRACE(__func__ << ": exec '" << params << "'");

    string execmd = "cd " + get_workdir(jobid) + " && " + params;

    SCDC_TRACE(__func__ << ": execmd: '" << execmd << "'");

    system(execmd.c_str());

  } else if (cmd == "run")
  {
    SCDC_TRACE(__func__ << ": running '" << params << "'");
    SCDC_INFO("executing run of job '" << jobid << "'");

    string c;

    build_cmd(jobcmd.c_str(), "run", jobid.c_str(), params.c_str(), res, c);

    stringstream ss;
    if (xterm) ss << (xterm_hold?"xterm -hold -e \"":"xterm -e \"");
    if (sleep_before_run > 0) ss << "sleep " << sleep_before_run << "; ";
    if (Xvfb) ss << "Xvfb :77 2>/dev/null & XVFB_PID=$!; DISPLAY=:77; ";
    ss << "cd " << get_workdir(jobid) << "; "
       << "(" << c << ") 2>&1 "
       << (show_output?"| tee run.output; ":"| cat > run.output; ");
    if (Xvfb) ss << "kill ${XVFB_PID} 2>/dev/null; ";
    if (sleep_after_run > 0) ss << "sleep " << sleep_after_run << "; ";
    if (xterm) ss << "exit; \"";

    SCDC_TRACE(__func__ << ": executing '" << ss.str() << "'");
#if USE_SYSTEM_PID
    pid_t pid = system_pid(ss.str().c_str());
    waitpid(pid, NULL, 0);
#else
    system(ss.str().c_str());
#endif

  } else ret = false;

  return ret;
}


std::string scdc_dataprov_jobrun_system::get_workdir(const std::string &jobid)
{
  return workdir + jobid + "/";
}


#undef SCDC_LOG_PREFIX

#define SCDC_LOG_PREFIX  "dataprov-jobrun-handler: "


scdc_dataprov_jobrun_handler::scdc_dataprov_jobrun_handler()
  :scdc_dataprov_jobrun("jobrun:handler")
{
  jobcmd = "%run_time% %nodes%:%nprocs%";

  handler_args.handler = 0;
  handler_args.data = 0;
}


bool scdc_dataprov_jobrun_handler::open(const char *conf, scdc_args *args)
{
  SCDC_TRACE(__func__ << ": '" << conf << "'");

  bool ret = true;

  args = open_args_init(args);

  if (args->get<scdc_dataprov_jobrun_handler_args_t>(SCDC_ARGS_TYPE_DATAPROV_JOBRUN_HANDLER, &handler_args, true) == SCDC_ARG_REF_NULL)
  {
    SCDC_ERROR("open: getting parameter for jobrun handler");
    ret = false;
    goto do_quit;
  }

  if (!scdc_dataprov_jobrun::open(conf, args))
  {
    SCDC_FAIL("open: opening base");
    ret = false;
  }

  SCDC_TRACE(__func__ << ": handler: '" << handler_args.handler << "', handler_data: '" << handler_args.data << "'");

  open_args_clear();

do_quit:
  if (!ret) open_args_release();

  return ret;
}


void scdc_dataprov_jobrun_handler::close()
{
  SCDC_TRACE(__func__ << ":");

  scdc_dataprov_jobrun::close();

  open_args_release();
}


bool scdc_dataprov_jobrun_handler::config_do_cmd_param(const std::string &cmd, const std::string &param, std::string val, scdc_config_result &result, bool &done)
{
  SCDC_TRACE(__func__ << ": cmd: '" << cmd << "', param: '" << param << "', val: '" << val << "', result: '" << result << "'");

  done = true;
  bool ret = true;

  if (param == "")
  {
    ret = scdc_dataprov_jobrun::config_do_cmd_param(cmd, "", val, result, done);
    if (!ret) goto do_quit;

    if (cmd == "info")
    {
      done = true;

      ret = scdc_dataprov_config::info(result, param, "job-handler");

    } else if (cmd == "ls")
    {
      done = true;

      ret = true;

    } else done = false;
  }

do_quit:
  if (done && !ret)
  {
    SCDC_FAIL("config_do_cmd_param: command '" << cmd << "' with param '" << param << "' failed");
    return false;
  }

  return ret;
}


scdc_dataset *scdc_dataprov_jobrun_handler::dataset_open(const char *path, scdcint_t path_size, scdc_dataset_output_t *output)
{
  return scdc_dataprov_jobrun::dataset_open<scdc_dataset_jobrun_handler>(path, path_size, output);
}


bool scdc_dataprov_jobrun_handler::job_do_run(const std::string &jobid, const std::string &cmd, const std::string &params, const scdc_dataprov_jobrun_res_t &res)
{
  bool ret = true;

  if (cmd == "run")
  {
    SCDC_TRACE(__func__ << ": running job '" << jobid << "' with '" << params << "'");
    SCDC_INFO("executing run of job '" << jobid << "'");

    string c;

    build_cmd(jobcmd.c_str(), "run", jobid.c_str(), params.c_str(), res, c);

    ret = job_do_cmd(jobid, c.c_str(), params, 0, 0);

  } else ret = false;

  return ret;
}


bool scdc_dataprov_jobrun_handler::job_do_cmd(const std::string &jobid, const std::string &cmd, const std::string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output)
{
  bool ret = true;

  if (handler_args.handler)
  {
    if (handler_args.handler(handler_args.data, jobid.c_str(), cmd.c_str(), params.c_str(), input, output) != SCDC_SUCCESS)
    {
      SCDC_FAIL("job_do_cmd: jobrun handler");
      ret = false;
    }
  }

  return ret;
}


#undef SCDC_LOG_PREFIX
