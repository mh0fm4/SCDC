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


#ifndef __DATAPROV_HOOK_HH__
#define __DATAPROV_HOOK_HH__


#include "dataprov.hh"


class scdc_dataprov_hook: public scdc_dataprov
{
  public:
    scdc_dataprov_hook();

    virtual bool open(const char *conf, scdc_args *args, scdc_result &result);
    virtual bool close(scdc_result &result);

    virtual scdc_dataset *dataset_open(std::string &path, scdc_result &result);
    virtual bool dataset_close(scdc_dataset *dataset, scdc_result &result);

    virtual scdc_dataset *dataset_open_read_state(scdc_data *incoming, scdc_result &result);
    virtual bool dataset_close_write_state(scdc_dataset *dataset, scdc_data *outgoing, scdc_result &result);

    virtual bool config_do_cmd_param(const std::string &cmd, const std::string &param, std::string val, scdc_config_result &result, bool &done);

    virtual bool dataset_cmds_do_cmd(scdc_dataset *dataset, const std::string &cmd, const std::string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output, scdc_result &result);

  public:
    scdc_args_dataprov_hook_t hook_arg;
    void *dataprov;
};


#endif /* __DATAPROV_HOOK_HH__ */
