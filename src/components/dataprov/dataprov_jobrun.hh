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


#ifndef __DATAPROV_JOBRUN_HH__
#define __DATAPROV_JOBRUN_HH__


#include <string>
#include <map>
#include <vector>
#include <pthread.h>

#include "dataprov.hh"


static inline std::string &safe_jobid(std::string &jobid)
{
  return trim(jobid);
}


static inline std::string safe_jobid(const std::string &jobid)
{
  return trim(jobid);
}


struct scdc_dataprov_jobrun_node_t
{
  scdcint_t cores, max_parallel_jobs;
  double performance;

  scdc_dataprov_jobrun_node_t() { }

  scdc_dataprov_jobrun_node_t(scdcint_t cores_, scdcint_t max_parallel_jobs_, double performance_)
    :cores(cores_), max_parallel_jobs(max_parallel_jobs_), performance(performance_) { }
};


struct scdc_dataprov_jobrun_job_params_proc_t
{
  scdcint_t nt_min, nt_max;
  scdcint_t np_min, np_max;
  std::string node;

  scdc_dataprov_jobrun_job_params_proc_t():nt_min(0), nt_max(0), np_min(0), np_max(0) { }
};


/*typedef std::vector<scdc_dataprov_jobrun_job_params_proc_t> scdc_dataprov_jobrun_params_t;*/
typedef scdc_dataprov_jobrun_job_params_proc_t scdc_dataprov_jobrun_job_params_t;


struct scdc_dataprov_jobrun_job_t
{
  scdc_dataprov_jobrun_job_params_t params;

  std::string jobcmds;
  bool waiting;

  scdc_dataprov_jobrun_job_t():waiting(true) { }
};


struct scdc_dataprov_jobrun_res_proc_t
{
  std::string node;
  scdcint_t np, nt;
};


typedef std::vector<scdc_dataprov_jobrun_res_proc_t> scdc_dataprov_jobrun_res_t;


class scdc_dataprov_jobrun_sched;


class scdc_dataprov_jobrun: public scdc_dataprov
{
  public:
    scdc_dataprov_jobrun(const std::string &type_);

    virtual bool open_config_conf(const std::string &conf, scdc_args *args, bool &done);

    virtual bool open(const char *conf, scdc_args *args);
    virtual void close();

    template<class DATASET_JOBRUN>
    scdc_dataset *dataset_open(const char *path, scdcint_t path_size, scdc_dataset_output_t *output);
    virtual void dataset_close(scdc_dataset *dataset, scdc_dataset_output_t *output);

    virtual bool config_do_cmd_param(const std::string &cmd, const std::string &param, std::string val, scdc_config_result &result, bool &done);

    bool do_cmd_ls(const std::string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output);
    bool do_cmd_info(const std::string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output);
    bool do_cmd_put(const std::string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output);
    bool do_cmd_get(const std::string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output);
    bool do_cmd_rm(const std::string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output);

    bool add_node(const std::string &nodeid, scdc_dataprov_jobrun_node_t &node);
    bool del_node(const std::string &nodeid);
    bool get_node(const std::string &nodeid, scdc_dataprov_jobrun_node_t &node);
    bool set_node(const std::string &nodeid, scdcint_t cores, scdcint_t max_parallel_jobs, double performance);
    bool nodes_config(const std::string &cmd, scdcint_t param, std::string &val, scdc_config_result &result);

    bool add_job(const std::string &jobid, scdc_dataprov_jobrun_job_t &job);
    bool del_job(const std::string &jobid);
    bool is_job(const std::string &jobid);
    bool set_job(const std::string &jobid, scdc_dataprov_jobrun_job_t &job);
    bool set_job_params(scdc_dataprov_jobrun_job_t &job, scdc_dataprov_jobrun_job_params_t &job_params);
    bool set_job_params(const std::string &jobid, scdc_dataprov_jobrun_job_params_t &job_params);
    bool get_job(const std::string &jobid, scdc_dataprov_jobrun_job_t &job);
    bool get_job_state(const std::string &jobid, std::string &state, bool sched_locked = false);

    virtual bool job_do_run(const std::string &jobid, const std::string &cmd, const std::string &params, const scdc_dataprov_jobrun_res_t &res) = 0;

    bool job_create(const std::string &jobid, scdc_dataprov_jobrun_job_params_t &job_params);
    bool job_destroy(const std::string &jobid);
    bool job_update(const std::string &jobid, scdc_dataprov_jobrun_job_params_t &job_params);
    bool job_run(const std::string &jobid, const std::string &jobcmds, const scdc_dataprov_jobrun_res_t &res);
    bool job_schedule(const std::string &jobid);
    bool job_wait_running(const std::string &jobid);
    bool job_wait_finished(const std::string &jobid);

    void sched_init();
    void sched_release();
    bool sched_change();

  public:
    std::string jobcmd;
    typedef std::map<std::string, scdc_dataprov_jobrun_node_t> nodes_t;
    nodes_t nodes;
    scdcint_t total_cores, max_parallel_jobs;
    bool runjobs;
    std::string sched_mode, sched_method;

    scdc_dataprov_jobrun_res_t local_res;

    typedef std::map<std::string, scdc_dataprov_jobrun_job_t> jobs_t;
    jobs_t jobs;

    pthread_mutex_t libscdc_lock;

    scdc_dataprov_jobrun_sched *sched;
    bool sched_run;
    pthread_mutex_t sched_lock;
    pthread_cond_t sched_cond, sched_cond_finished;
    void *sched_run_routine_arg;
    pthread_t sched_run_thread;
    typedef std::list<std::string> sched_jobs_wait_t;
    sched_jobs_wait_t sched_jobs_wait;
    typedef std::map<std::string, pthread_t> sched_jobs_run_t;
    sched_jobs_run_t sched_jobs_run;
};


class scdc_dataprov_jobrun_system: public scdc_dataprov_jobrun
{
  public:
    scdc_dataprov_jobrun_system();

    virtual bool open_config_conf(const std::string &conf, scdc_args *args, bool &done);

    virtual bool open(const char *conf, scdc_args *args);
    virtual void close();

    virtual bool config_do_cmd_param(const std::string &cmd, const std::string &param, std::string val, scdc_config_result &result, bool &done);

    virtual scdc_dataset *dataset_open(const char *path, scdcint_t path_size, scdc_dataset_output_t *output);

    virtual bool job_do_run(const std::string &jobid, const std::string &cmd, const std::string &params, const scdc_dataprov_jobrun_res_t &res);

    std::string get_workdir(const std::string &jobid);

  public:
    std::string workdir;
    bool xterm, xterm_hold, Xvfb, show_output;
    scdcint_t sleep_before_run, sleep_after_run;
};


class scdc_dataprov_jobrun_remote: public scdc_dataprov_jobrun
{
  public:
    scdc_dataprov_jobrun_remote();
};



class scdc_dataprov_jobrun_handler: public scdc_dataprov_jobrun
{
  public:
    scdc_dataprov_jobrun_handler();

    virtual bool open(const char *conf, scdc_args *args);
    virtual void close();

    virtual bool config_do_cmd_param(const std::string &cmd, const std::string &param, std::string val, scdc_config_result &result, bool &done);

    virtual scdc_dataset *dataset_open(const char *path, scdcint_t path_size, scdc_dataset_output_t *output);

    virtual bool job_do_run(const std::string &jobid, const std::string &cmd, const std::string &params, const scdc_dataprov_jobrun_res_t &res);

    bool job_do_cmd(const std::string &jobid, const std::string &cmd, const std::string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output);

  public:
    scdc_dataprov_jobrun_handler_args_t handler_args;
};


#endif /* __DATAPROV_JOBRUN_HH__ */
