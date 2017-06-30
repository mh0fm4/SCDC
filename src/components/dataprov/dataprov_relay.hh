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


#ifndef __DATAPROV_RELAY_HH__
#define __DATAPROV_RELAY_HH__


#include <string>
#include <map>

#include "dataprov_register.hh"


class scdc_dataprov_relay: public scdc_dataprov_register
{
  public:
    scdc_dataprov_relay();
    scdc_dataprov_relay(const std::string &type_);

/*    virtual bool open(const char *conf, scdc_args *args);
    virtual void close();*/

    virtual scdc_dataset *dataset_open(const char *path, scdcint_t path_size, scdc_dataset_output_t *output);
    virtual void dataset_close(scdc_dataset *dataset, scdc_dataset_output_t *output);

    virtual bool config_do_cmd_param(const std::string &cmd, const std::string &param, std::string val, scdc_config_result &result, bool &done);

    virtual bool dataset_cmds_do_cmd(scdc_dataset *dataset, const std::string &cmd, const std::string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output);

  protected:
    typedef std::map<std::string, std::string> relay_t;
    relay_t relay;

    virtual bool relay_put(const std::string &path, const std::string &url, std::string &r);
    virtual bool relay_get(const std::string &path, std::string &r);
    virtual bool relay_info(const std::string &path, std::string &r);
    virtual bool relay_rm(const std::string &path);
    virtual bool relay_ls(const std::string &type, std::string &r);
    virtual bool relay_update(const std::string &path);
};


#endif /* __DATAPROV_RELAY_HH__ */
