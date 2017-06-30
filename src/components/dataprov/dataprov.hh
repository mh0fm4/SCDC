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


#ifndef __DATAPROV_HH__
#define __DATAPROV_HH__


#include <string>
#include <map>

#include "config.hh"
#include "args.hh"
#include "dataset.hh"
#include "dataset_inout.h"
#include "dataprov_config.hh"


class scdc_dataprov
{
  public:
    scdc_dataprov(const std::string &type_):type(type_), open_args(0), open_args_refcount(0) { }
    virtual ~scdc_dataprov() { }

    virtual bool open_config_conf(const std::string &conf, scdc_args *args, bool &done);
    bool open_config(std::string &conf, scdc_args *args);

    virtual bool open(const char *conf, scdc_args *args);
    virtual void close();

    virtual scdc_dataset *dataset_open(const char *path, scdcint_t path_size, scdc_dataset_output_t *output) = 0;
    virtual void dataset_close(scdc_dataset *dataset, scdc_dataset_output_t *output) = 0;

    virtual scdc_dataset *dataset_open_read_state(scdc_data *incoming, scdc_dataset_output_t *output);
    virtual void dataset_close_write_state(scdc_dataset *dataset, scdc_data *outgoing, scdc_dataset_output_t *output);

    bool config_open(const char *path, scdcint_t path_size, scdc_dataset_output_t *output, scdc_dataset **dataset);
    bool config_close(scdc_dataset *dataset, scdc_dataset_output_t *output);

    virtual bool config_do_cmd(const std::string &cmd, const std::string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output);
    bool config_do_cmd_param_base(const std::string &cmd, const std::string &param, std::string val, scdc_config_result &result, bool &done);
    virtual bool config_do_cmd_param(const std::string &cmd, const std::string &param, std::string val, scdc_config_result &result, bool &done);

    typedef bool (scdc_dataset::*dataset_cmds_do_cmd_f)(const std::string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output);
    typedef std::map<std::string, dataset_cmds_do_cmd_f> dataset_cmds_t;

    void dataset_cmds_add(const std::string &cmd, dataset_cmds_do_cmd_f do_cmd) { dataset_cmds.insert(dataset_cmds_t::value_type(std::string(cmd), do_cmd)); }
    void dataset_cmds_del(const std::string &cmd) { dataset_cmds.erase(std::string(cmd)); };
    virtual bool dataset_cmds_do_cmd(scdc_dataset *dataset, const std::string &cmd, const std::string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output);

    void set_type(const std::string &type_) { type = type_; }

  private:
    std::string type;
    dataset_cmds_t dataset_cmds;

  protected:
    scdc_args *open_args;
    scdcint_t open_args_refcount;

    scdc_args *open_args_init(scdc_args *args) { if (!open_args) open_args = new scdc_args(args); ++open_args_refcount; return open_args; }
    void open_args_clear() { open_args->clear_args_data(); }
    void open_args_release() { --open_args_refcount; if (open_args_refcount == 0) { delete open_args; open_args = 0; } }
};


#endif /* __DATAPROV_HH__ */
