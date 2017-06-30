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


#ifndef __DATAPROV_REGISTER_HH__
#define __DATAPROV_REGISTER_HH__


#include <string>
#include <map>

#include "dataprov.hh"


struct scdc_dataprov_register_entry_t
{
  std::string type, state;

  scdc_dataprov_register_entry_t() { }

  scdc_dataprov_register_entry_t(const std::string &type_, const std::string &state_)
    :type(type_), state(state_) { }  
};


class scdc_dataprov_register: public scdc_dataprov
{
  public:
    scdc_dataprov_register();
    scdc_dataprov_register(const std::string &type_);

/*    virtual bool open(const char *conf, scdc_args *args);
    virtual void close();*/

    virtual scdc_dataset *dataset_open(const char *path, scdcint_t path_size, scdc_dataset_output_t *output);
    virtual void dataset_close(scdc_dataset *dataset, scdc_dataset_output_t *output);

    virtual bool dataset_cmds_do_cmd(scdc_dataset *dataset, const char *cmd, const char *params, scdc_dataset_input_t *input, scdc_dataset_output_t *output);

  protected:
    typedef std::map<std::string, scdc_dataprov_register_entry_t> register_t;
    register_t reg;

    bool reg_put(const std::string &url);
    bool reg_get(const std::string &url);
    bool reg_info(const std::string &url, std::string &r);
    bool reg_rm(const std::string &url);
    bool reg_ls(const std::string &type, std::string &r);
    bool reg_update(const std::string &);
};


#endif /* __DATAPROV_REGISTER_HH__ */
