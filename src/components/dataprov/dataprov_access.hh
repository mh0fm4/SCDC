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


#ifndef __DATAPROV_ACCESS_HH__
#define __DATAPROV_ACCESS_HH__


#include <string>

#include "dataprov.hh"


template <class ACCESS_HANDLER>
class scdc_dataprov_access;

template<class ACCESS_HANDLER>
struct scdc_dataset_access_do_cmd_get_next_data_t
{
  scdc_dataprov_access<ACCESS_HANDLER> *dp;
  typename ACCESS_HANDLER::dir_t dir;
  typename ACCESS_HANDLER::file_t file;
  scdcint_t pos;
  scdcint_t size;
  void *buf;
  scdcint_t buf_size;
};


template <class ACCESS_HANDLER>
class scdc_dataset_access: public scdc_dataset
{
  public:
    typename ACCESS_HANDLER::dir_t dir;
    typename ACCESS_HANDLER::file_t file;

    scdc_dataset_access(scdc_dataprov *dataprov_);
    ~scdc_dataset_access();

  public:
    bool do_cmd_cd(const std::string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output, scdc_result &result);
    bool do_cmd_ls(const std::string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output, scdc_result &result);
    bool do_cmd_info(const std::string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output, scdc_result &result);
    bool do_cmd_mkd(const std::string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output, scdc_result &result);
    bool do_cmd_put(const std::string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output, scdc_result &result);
    bool do_cmd_get(const std::string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output, scdc_result &result);
    bool do_cmd_rm(const std::string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output, scdc_result &result);
    bool do_cmd_sync(const std::string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output, scdc_result &result);

  private:
    scdc_dataset_access_do_cmd_get_next_data_t<ACCESS_HANDLER> do_cmd_get_next_data;
};


template <class ACCESS_HANDLER>
class scdc_dataprov_access: public scdc_dataprov, public ACCESS_HANDLER
{
  public:
    scdc_dataprov_access():scdc_dataprov(std::string("access_") + ACCESS_HANDLER::type) { }

    virtual bool open_config_conf(const std::string &conf, scdc_args *args, bool &done);

    virtual bool open(const char *conf, scdc_args *args, scdc_result &result);
    virtual bool close(scdc_result &result);

    scdc_dataset *dataset_open(std::string &path, scdc_result &result);
    virtual bool dataset_close(scdc_dataset *dataset, scdc_result &result);

    virtual bool config_do_cmd_param(const std::string &cmd, const std::string &param, std::string val, scdc_config_result &result, bool &done);
};


#endif /* __DATAPROV_ACCESS_HH__ */
