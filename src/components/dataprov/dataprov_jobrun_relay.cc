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
#include <typeinfo>

#define SCDC_TRACE_NOT  !SCDC_TRACE_DATAPROV_JOBRUN_RELAY

#include "config.hh"
#include "common.hh"
#include "log.hh"
#include "dataset.hh"
#include "dataset_inout.h"
#include "dataprov_jobrun.hh"
#include "dataprov_jobrun_relay.hh"

#include "scdc.h"


using namespace std;


#define SCDC_LOG_PREFIX  "dataset-jobrun-relay: "


class scdc_dataset_jobrun_relay: public scdc_dataset
{
  public:
    scdc_dataset_jobrun_relay(scdc_dataprov *dataprov_)
      :scdc_dataset(dataprov_), remote_dataset(SCDC_DATASET_NULL) { };


    void set_remote_dataset(scdc_dataset_t remote_dataset_) { remote_dataset = remote_dataset_; }
    scdc_dataset_t get_remote_dataset() { return remote_dataset; }


    scdc_dataprov_jobrun_relay *dataprov_jobrun_relay()
    {
      return static_cast<scdc_dataprov_jobrun_relay *>(dataprov);
    }


    bool remote_dataset_open(string jobid, bool cd)
    {
      string remote;

      if (!dataprov_jobrun_relay()->sched_job_get_relay(jobid, remote))
      {
        SCDC_ERROR("remote_dataset_open: getting relay address of job '" << jobid << "' failed");
        return false;
      }

      if (cd) remote += "/" + jobid;

      remote_dataset = scdc_dataset_open(remote.c_str());

      if (remote_dataset == SCDC_DATASET_NULL)
      {
        SCDC_FAIL("remote_dataset_open: opening remote dataset '" << remote << "' failed");
        return false;
      }

      return true;
    }


    void remote_dataset_close()
    {
      if (remote_dataset != SCDC_DATASET_NULL) 
      {
        scdc_dataset_close(remote_dataset);
        remote_dataset = SCDC_DATASET_NULL;
      }
    }


    bool do_cmd_cd(const std::string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output)
    {
      string jobid = safe_jobid(params);

      if (jobid == pwd) return true;

      remote_dataset_close();

      set_pwd("");

      if (jobid.size() == 0) return true;

      if (!remote_dataset_open(jobid, true)) return false;

      set_pwd(jobid);

      return true;
    }


/*    bool do_cmd_pwd(const std::string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output)
    {
      return scdc_dataset::do_cmd_pwd(params, input, output);
    }*/


    bool do_cmd_ls(const std::string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output)
    {
      string p = params;
      string jobid = pwd;
      bool have_remote = (remote_dataset != SCDC_DATASET_NULL);

      if (jobid.empty())
      {
        stringlist plist(' ', p);

        jobid = safe_jobid(plist.front_pop());

        p += plist.offset();

        if (jobid.size() == 0) return dataprov_jobrun_relay()->do_cmd_ls(p, input, output);

        remote_dataset_open(jobid, false);
      }

      if (remote_dataset == SCDC_DATASET_NULL)
      {
        SCDC_FAIL("do_cmd_ls: no remote dataset available");
        SCDC_DATASET_OUTPUT_PRINTF(output, "no remote dataset available");
        return false;
      }

      string cmd = "ls ";
      if (!have_remote) cmd += jobid + " ";
      cmd += p;

      if (scdc_dataset_cmd(remote_dataset, cmd.c_str(), input, output) != SCDC_SUCCESS)
      {
        SCDC_FAIL("do_cmd_ls: listing command failed: " << SCDC_DATASET_OUTPUT_STR(output));
        SCDC_DATASET_OUTPUT_PRINTF(output, "listing command failed");
        return false;
      }

      if (!have_remote) remote_dataset_close();

      return true;
    }


    bool do_cmd_info(const std::string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output)
    {
      string p = params;
      string jobid = pwd;
      bool have_remote = (remote_dataset != SCDC_DATASET_NULL);

      if (jobid.empty())
      {
        stringlist plist(' ', p);

        jobid = safe_jobid(plist.front_pop().c_str());

        p += plist.offset();

        if (jobid.empty()) return dataprov_jobrun_relay()->do_cmd_info(p, input, output);

        remote_dataset_open(jobid, false);
      }

      if (remote_dataset == SCDC_DATASET_NULL)
      {
        SCDC_FAIL("do_cmd_info: no remote dataset available");
        SCDC_DATASET_OUTPUT_PRINTF(output, "no remote dataset available");
        return false;
      }

      string cmd = "info ";
      if (!have_remote) cmd += jobid + " ";
      cmd += p;

      if (scdc_dataset_cmd(remote_dataset, cmd.c_str(), input, output) != SCDC_SUCCESS)
      {
        SCDC_FAIL("do_cmd_info: info command failed: " << SCDC_DATASET_OUTPUT_STR(output));
        SCDC_DATASET_OUTPUT_PRINTF(output, "info command failed");
        return false;
      }

      if (!have_remote) remote_dataset_close();

      return true;
    }


    bool do_cmd_put(const std::string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output)
    {
      string p = params;
      string jobid = pwd;
      bool have_remote = (remote_dataset != SCDC_DATASET_NULL);

      if (jobid.empty())
      {
        stringlist plist(' ', p);

        jobid = safe_jobid(plist.front_pop().c_str());

        p += plist.offset();

        if (jobid.empty()) return dataprov_jobrun_relay()->do_cmd_put(p, input, output);

        if (!dataprov_jobrun_relay()->sched_job_add(jobid))
        {
          SCDC_FAIL("do_cmd_put: adding job failed");
          SCDC_DATASET_OUTPUT_PRINTF(output, "adding job failed");
          return false;
        }

        remote_dataset_open(jobid, false);
      }

      if (remote_dataset == SCDC_DATASET_NULL)
      {
        SCDC_FAIL("do_cmd_put: no remote dataset available");
        SCDC_DATASET_OUTPUT_PRINTF(output, "no remote dataset available");
        return false;
      }

      string cmd = "put ";
      if (!have_remote) cmd += jobid + " ";
      cmd += p;

      if (scdc_dataset_cmd(remote_dataset, cmd.c_str(), input, output) != SCDC_SUCCESS)
      {
        SCDC_FAIL("do_cmd_put: put command failed: " << SCDC_DATASET_OUTPUT_STR(output));
        SCDC_DATASET_OUTPUT_PRINTF(output, "put command failed");
        return false;
      }

      if (!have_remote) remote_dataset_close();

      return true;
    }


    bool do_cmd_get(const std::string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output)
    {
      string p = params;
      string jobid = pwd;
      bool have_remote = (remote_dataset != SCDC_DATASET_NULL);

      if (jobid.empty())
      {
        stringlist plist(' ', p);

        jobid = safe_jobid(plist.front_pop().c_str());

        p += plist.offset();

        if (jobid.empty()) return dataprov_jobrun_relay()->do_cmd_get(p, input, output);

        remote_dataset_open(jobid, false);
      }

      if (remote_dataset == SCDC_DATASET_NULL)
      {
        SCDC_FAIL("do_cmd_get: no remote dataset available");
        SCDC_DATASET_OUTPUT_PRINTF(output, "no remote dataset available");
        return false;
      }

      string cmd = "get ";
      if (!have_remote) cmd += jobid + " ";
      cmd += p;

      if (scdc_dataset_cmd(remote_dataset, cmd.c_str(), input, output) != SCDC_SUCCESS)
      {
        SCDC_FAIL("do_cmd_get: get command failed: " << SCDC_DATASET_OUTPUT_STR(output));
        SCDC_DATASET_OUTPUT_PRINTF(output, "get command failed");
        return false;
      }

      if (!have_remote) remote_dataset_close();

      return true;
    }


    bool do_cmd_rm(const std::string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output)
    {
      string p = params;
      string jobid = pwd;
      bool have_remote = (remote_dataset != SCDC_DATASET_NULL);

      if (jobid.empty())
      {
        stringlist plist(' ', p);

        jobid = safe_jobid(plist.front_pop().c_str());

        p += plist.offset();

        if (jobid.empty()) return dataprov_jobrun_relay()->do_cmd_rm(p, input, output);

        remote_dataset_open(jobid, false);
      }

      if (remote_dataset == SCDC_DATASET_NULL)
      {
        SCDC_FAIL("do_cmd_rm: no remote dataset available");
        SCDC_DATASET_OUTPUT_PRINTF(output, "no remote dataset available");
        return false;
      }

      string cmd = "rm ";
      if (!have_remote) cmd += jobid + " ";
      cmd += p;

      if (scdc_dataset_cmd(remote_dataset, cmd.c_str(), input, output) != SCDC_SUCCESS)
      {
        SCDC_FAIL("do_cmd_rm: rm command failed: " << SCDC_DATASET_OUTPUT_STR(output));
        SCDC_DATASET_OUTPUT_PRINTF(output, "rm command failed");
        return false;
      }

      if (!have_remote) remote_dataset_close();

      if (!dataprov_jobrun_relay()->sched_job_del(jobid))
      {
        SCDC_FAIL("do_cmd_put: deleting job failed");
        SCDC_DATASET_OUTPUT_PRINTF(output, "deleting job failed");
        return false;
      }

      do_cmd_cd("", 0, 0);

      return true;
    }


  private:
    scdc_dataset_t remote_dataset;
};


#undef SCDC_LOG_PREFIX

#define SCDC_LOG_PREFIX  "dataprov-jobrun-relay: "


scdc_dataprov_jobrun_relay::scdc_dataprov_jobrun_relay()
  :scdc_dataprov_relay("jobrun_relay")
{
  max_parallel_jobs = 0;
  sched_last = sched.end();
  sched_last_jobs = 0;
}


bool scdc_dataprov_jobrun_relay::open(const char *conf, scdc_args *args)
{
  SCDC_TRACE("open: '" << conf << "'");

  bool ret = true;

  if (!scdc_dataprov_relay::open(conf, args))
  {
    SCDC_FAIL("open: opening base");
    ret = false;

  } else
  {
    dataset_cmds_add("cd", static_cast<dataset_cmds_do_cmd_f>(&scdc_dataset_jobrun_relay::do_cmd_cd));
    dataset_cmds_add("pwd", static_cast<dataset_cmds_do_cmd_f>(&scdc_dataset_jobrun_relay::do_cmd_pwd));
    dataset_cmds_add("ls", static_cast<dataset_cmds_do_cmd_f>(&scdc_dataset_jobrun_relay::do_cmd_ls));
    dataset_cmds_add("info", static_cast<dataset_cmds_do_cmd_f>(&scdc_dataset_jobrun_relay::do_cmd_info));
    dataset_cmds_add("put", static_cast<dataset_cmds_do_cmd_f>(&scdc_dataset_jobrun_relay::do_cmd_put));
    dataset_cmds_add("get", static_cast<dataset_cmds_do_cmd_f>(&scdc_dataset_jobrun_relay::do_cmd_get));
    dataset_cmds_add("rm", static_cast<dataset_cmds_do_cmd_f>(&scdc_dataset_jobrun_relay::do_cmd_rm));
  }

  return ret;
}


void scdc_dataprov_jobrun_relay::close()
{
  SCDC_TRACE("close:");

  scdc_dataprov_relay::close();
}


scdc_dataset *scdc_dataprov_jobrun_relay::dataset_open(const char *path, scdcint_t path_size, scdc_dataset_output_t *output)
{
  SCDC_TRACE("dataset_open: '" << string(path, path_size) << "'");

  scdc_dataset *dataset = 0;
  
  if (config_open(path, path_size, output, &dataset)) return dataset;

  string jobid(path, path_size);

  if (jobid == "" || sched_job_exists(jobid))
  {
    SCDC_TRACE("dataset_open: local dataset");
  
    /* stay local and change to sub */
    scdc_dataset_jobrun_relay *dataset_jobrun_relay = new scdc_dataset_jobrun_relay(this);

    dataset_jobrun_relay->do_cmd_cd(jobid.c_str(), 0, output);

    dataset = dataset_jobrun_relay;

  } else
  {
    SCDC_TRACE("dataset_open: relay dataset");

    dataset = scdc_dataprov_relay::dataset_open(path, path_size, output);
  }

  SCDC_TRACE("dataset_open: return: '" << dataset << "'");

  return dataset;
}


void scdc_dataprov_jobrun_relay::dataset_close(scdc_dataset *dataset, scdc_dataset_output_t *output)
{
  SCDC_TRACE("dataset_close: '" << dataset << "'");

  if (config_close(dataset, output)) return;

  if (typeid(*dataset) == typeid(scdc_dataset_jobrun_relay))
  {
    SCDC_TRACE("dataset_close: local dataset");
    delete dataset;

  } else
  {
    SCDC_TRACE("dataset_close: relay dataset");
    scdc_dataprov_relay::dataset_close(dataset, output);
  }

  SCDC_TRACE("dataset_close: return");
}


bool scdc_dataprov_jobrun_relay::config_do_cmd_param(const std::string &cmd, const std::string &param, std::string val, scdc_config_result &result, bool &done)
{
  SCDC_TRACE("config_do_cmd_param: cmd: '" << cmd << "', param: '" << param << "', val: '" << val << "', result: '" << result << "'");

  string c(cmd), p(param);

  done = true;
  bool ret = true;

  if (p == "")
  {
    ret = scdc_dataprov_relay::config_do_cmd_param(cmd, param, val, result, done);
    if (!ret) goto do_quit;

    if (c == "info")
    {
    } else if (c == "ls")
    {
      ret = scdc_dataprov_config::ls(result, "max_parallel_jobs");

    } else done = false;

  } else
  {
    ret = scdc_dataprov_relay::config_do_cmd_param(cmd, param, val, result, done);
    if (!ret) goto do_quit;

    if (done);
    else if (p == "max_parallel_jobs")
    {
      done = true;

      if (c == "info") ret = scdc_dataprov_config::ls(result, "maximum number of parallel jobs executed at once");
      else if (c == "get") ret = scdc_dataprov_config::get<scdcint_t>(result, "max_parallel_jobs", max_parallel_jobs);
      else done = false;
    }
  }

do_quit:
  if (done && !ret)
  {
    SCDC_FAIL("config_do_cmd_param: cmd '" << cmd << "' failed");
    return false;
  }

  return ret;
}


bool scdc_dataprov_jobrun_relay::do_cmd_ls(const std::string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output)
{
  SCDC_TRACE("do_cmd_ls: params: '" << params << "'");

  string s;

  for (sched_jobs_t::iterator i = sched_jobs.begin(); i != sched_jobs.end(); ++i)
  {
    s += i->first + ":unknown:" + i->second + ",";
  }

  if (s.size() > 0) s.resize(s.size() - 1);

  SCDC_DATASET_OUTPUT_PRINTF(output, s.c_str());

  return true;
}


bool scdc_dataprov_jobrun_relay::do_cmd_info(const std::string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output)
{
  SCDC_TRACE("do_cmd_info: params: '" << params << "'");

  stringstream ss;

  ss << sched_jobs.size() << ":-1:-1";

  SCDC_DATASET_OUTPUT_PRINTF(output, ss.str().c_str());

  return true;
}


bool scdc_dataprov_jobrun_relay::do_cmd_put(const std::string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output)
{
  SCDC_TRACE("do_cmd_put: params: '" << params << "'");

  return false;
}


bool scdc_dataprov_jobrun_relay::do_cmd_get(const std::string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output)
{
  SCDC_TRACE("do_cmd_get: params: '" << params << "'");

  return false;
}


bool scdc_dataprov_jobrun_relay::do_cmd_rm(const std::string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output)
{
  SCDC_TRACE("do_cmd_rm: params: '" << params << "'");

  return false;
}


bool scdc_dataprov_jobrun_relay::sched_job_add(const std::string &jobid)
{
  /* last sched exists, but execeeds its max. jobs? */
  if (sched_last != sched.end() && sched_last_jobs >= sched_last->second.max_parallel_jobs)
  {
    /* select next relay */
    ++sched_last;
    sched_last_jobs = 0;
  }

  /* last sched does not exist? */
  if (sched_last == sched.end())
  {
    /* restart with first */
    sched_last = sched.begin();
    sched_last_jobs = 0;
  }

  if (sched_last == sched.end()) return false;

  relay_t::iterator i = relay.find(sched_last->first);

  if (i == relay.end()) return false;

  ++sched_last_jobs;

  string rel = i->second;

  return sched_jobs.insert(sched_jobs_t::value_type(jobid, rel)).second;
}


bool scdc_dataprov_jobrun_relay::sched_job_del(const std::string &jobid)
{
  return (sched_jobs.erase(jobid) == 1);
}


bool scdc_dataprov_jobrun_relay::sched_job_exists(const std::string &jobid)
{
  return (sched_jobs.find(jobid) != sched_jobs.end());
}


bool scdc_dataprov_jobrun_relay::sched_job_get_relay(const std::string &jobid, std::string &relay)
{
  sched_jobs_t::iterator i = sched_jobs.find(jobid);

  if (i == sched_jobs.end()) return false;

  relay = i->second;

  return true;
}


bool scdc_dataprov_jobrun_relay::relay_put(const std::string &path, const std::string &url, std::string &r)
{
  SCDC_TRACE("relay_put: path: '" << path << "', url: '" << url << "'");

  scdc_dataset_input_t *input = NULL;
  scdc_dataset_output_t output_, *output = &output_;

  output = scdc_dataset_output_create(output, "alloc");

  string cmd = url + "/CONFIG get type,max_parallel_jobs";

  bool ret = scdc_dataset_cmd(SCDC_DATASET_NULL, cmd.c_str(), input, output);

  if (ret)
  {
    string s = SCDC_DATASET_OUTPUT_STR(output);
    stringlist sl(',', s.c_str(), s.size());

    if (sl.front_pop() != "jobrun")
    {
      SCDC_FAIL("relay_put: url '" << url << "' is not a jobrun datprovider");
      r = "not a jobrun dataprovider";
      ret = false;

    } else
    {
      ret = scdc_dataprov_relay::relay_put(path, url, r);

      if (ret)
      {
        stringstream ss(sl.front_pop());

        scdcint_t p;

        ss >> p;

        sched.insert(sched_t::value_type(path, scdc_dataprov_jobrun_relay_sched_t(p)));
        max_parallel_jobs += p;

        SCDC_TRACE("relay_put: schedule " << p << " jobs for path '" << path << "'");
        SCDC_TRACE("relay_put: max_parallel_jobs: " << max_parallel_jobs);
        SCDC_INFO("adding relay to '" << url << "' for path '" << path << "' with " << p << " max. parallel jobs (total max. parallel jobs: " << max_parallel_jobs << ")");
      }
    }

  } else
  {
    SCDC_FAIL("relay_put: access to data provider '" << url << "' failed: " << SCDC_DATASET_OUTPUT_STR(output));
    r = "access to data provider failed: " + SCDC_DATASET_OUTPUT_STR(output);
  }

  scdc_dataset_output_destroy(output);

  return ret;
}


bool scdc_dataprov_jobrun_relay::relay_rm(const std::string &path)
{
  SCDC_TRACE("relay_rm: path: '" << path << "'");

  bool ret = scdc_dataprov_relay::relay_rm(path);

  sched_t::iterator i = sched.find(path);

  if (ret && i != sched.end())
  {
    max_parallel_jobs -= i->second.max_parallel_jobs;

    if (sched_last == i)
    {
      ++sched_last;
      sched_last_jobs = 0;
    }

    sched.erase(i);

    SCDC_TRACE("relay_put: max_parallel_jobs: " << max_parallel_jobs);
    SCDC_INFO("removing relay for path '" << path << "' (total max. parallel jobs: " << max_parallel_jobs << ")");
  }

  return ret;
}


#undef SCDC_LOG_PREFIX
