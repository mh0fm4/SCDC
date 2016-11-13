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


#ifndef __DATAPROV_JOBRUN_RELAY_HH__
#define __DATAPROV_JOBRUN_RELAY_HH__


#include <string>
#include <map>

#include "dataprov.hh"
#include "dataprov_relay.hh"


struct scdc_dataprov_jobrun_relay_sched_t
{
  scdcint_t max_parallel_jobs;

  scdc_dataprov_jobrun_relay_sched_t() { }

  scdc_dataprov_jobrun_relay_sched_t(scdcint_t max_parallel_jobs_)
    :max_parallel_jobs(max_parallel_jobs_) { }
};


class scdc_dataprov_jobrun_relay: public scdc_dataprov_relay
{
  public:
    scdc_dataprov_jobrun_relay();

    virtual bool open(const char *conf, scdc_args *args);
    virtual void close();

    virtual scdc_dataset *dataset_open(const char *path, scdcint_t path_size, scdc_dataset_output_t *output);
    virtual void dataset_close(scdc_dataset *dataset, scdc_dataset_output_t *output);

    virtual bool config_do_cmd_param(const std::string &cmd, const std::string &param, std::string val, scdc_config_result &result, bool &done);

    bool do_cmd_ls(const std::string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output);
    bool do_cmd_info(const std::string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output);
    bool do_cmd_put(const std::string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output);
    bool do_cmd_get(const std::string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output);
    bool do_cmd_rm(const std::string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output);

    bool sched_job_add(const std::string &jobid);
    bool sched_job_del(const std::string &jobid);
    bool sched_job_exists(const std::string &jobid);
    bool sched_job_get_relay(const std::string &jobid, std::string &relay);

  private:
    typedef std::map<std::string, scdc_dataprov_jobrun_relay_sched_t> sched_t;
    sched_t sched;

    typedef std::map<std::string, std::string> sched_jobs_t;
    sched_jobs_t sched_jobs;

    scdcint_t max_parallel_jobs;
    sched_t::iterator sched_last;
    scdcint_t sched_last_jobs;

    virtual bool relay_put(const std::string &path, const std::string &url, std::string &r);
    virtual bool relay_rm(const std::string &path);
};


#endif /* __DATAPROV_JOBRUN_RELAY_HH__ */
