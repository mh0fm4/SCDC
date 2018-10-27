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


#ifndef __DATAPROV_STORE_HH__
#define __DATAPROV_STORE_HH__


#include <string>

#include "dataprov.hh"


template <class STORE_HANDLER>
class scdc_dataprov_store;

template<class STORE_HANDLER>
struct scdc_dataset_store_do_cmd_get_next_data_t
{
  scdc_dataprov_store<STORE_HANDLER> *dp;
  typename STORE_HANDLER::store_t store;
  typename STORE_HANDLER::entry_t entry;
  scdcint_t pos;
  scdcint_t size;
  void *buf;
  scdcint_t buf_size;
};


template <class STORE_HANDLER>
class scdc_dataset_store: public scdc_dataset
{
  public:
    typename STORE_HANDLER::store_t store;
    typename STORE_HANDLER::entry_t entry;

    scdc_dataset_store(scdc_dataprov *dataprov_);
    ~scdc_dataset_store();

    bool do_cmd_cd(const std::string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output, scdc_result &result);
    bool do_cmd_ls(const std::string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output, scdc_result &result);
    bool do_cmd_info(const std::string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output, scdc_result &result);
    bool do_cmd_put(const std::string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output, scdc_result &result);
    bool do_cmd_get(const std::string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output, scdc_result &result);
    bool do_cmd_rm(const std::string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output, scdc_result &result);

  private:
    bool admin;
    scdc_dataset_store_do_cmd_get_next_data_t<STORE_HANDLER> do_cmd_get_next_data;
};


template <class STORE_HANDLER>
class scdc_dataprov_store: public scdc_dataprov, public STORE_HANDLER
{
  public:
    scdc_dataprov_store():scdc_dataprov(std::string("store_") + STORE_HANDLER::type) { }

    virtual bool open_config_conf(const std::string &conf, scdc_args *args, bool &done);

    virtual bool open(const char *conf, scdc_args *args, scdc_result &result);
    virtual bool close(scdc_result &result);

    scdc_dataset *dataset_open(std::string &path, scdc_result &result);
    virtual bool dataset_close(scdc_dataset *dataset, scdc_result &result);

    virtual bool config_do_cmd_param(const std::string &cmd, const std::string &param, std::string val, scdc_config_result &result, bool &done);
};


#endif /* __DATAPROV_STORE_HH__ */
