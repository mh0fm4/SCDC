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


#ifndef __DATASET_HH__
#define __DATASET_HH__


#include "data.hh"


class scdc_dataprov;

class scdc_dataset
{
  public:
    scdc_dataset(scdc_dataprov *dataprov_)
      :dataprov(dataprov_), config(false) { }
    virtual ~scdc_dataset() { }

/*    void set_dataprov(scdc_dataprov *dataprov_) { dataprov = dataprov_; }*/
    scdc_dataprov *get_dataprov() { return dataprov; }

    void set_pwd(const std::string &pwd_) { pwd = pwd_; }
    std::string &get_pwd() { return pwd; }

    virtual bool do_cmd(const char *cmd, scdcint_t cmd_size, scdc_dataset_input_t *input, scdc_dataset_output_t *output);

    virtual bool do_cmd_info(const std::string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output);
    virtual bool do_cmd_cd(const std::string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output);
    virtual bool do_cmd_pwd(const std::string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output);

  protected:
    scdc_dataprov *dataprov;
    bool config;
    std::string pwd;
};


#endif /* __DATASET_HH__ */
