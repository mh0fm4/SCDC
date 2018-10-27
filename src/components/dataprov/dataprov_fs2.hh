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


#ifndef __DATAPROV_FS2_HH__
#define __DATAPROV_FS2_HH__


#include <string>

#include "dataprov.hh"


class scdc_dataprov_fs2: public scdc_dataprov
{
  public:
    scdc_dataprov_fs2(std::string type_):scdc_dataprov(type_) { }

    virtual bool open(const char *conf, scdc_args *args, scdc_result &result);
    virtual bool close(scdc_result &result);

    template<class DATASET_FS>
    scdc_dataset *dataset_open(std::string &path, scdc_result &result);
    virtual bool dataset_close(scdc_dataset *dataset, scdc_result &result);

    virtual bool config_do_cmd_param(const std::string &cmd, const std::string &param, std::string val, scdc_config_result &result, bool &done);

    void set_root(const std::string &root_);

    bool make_pwd_path(const std::string &pwd, const std::string &sub, std::string &pwd_path);
    void make_abs_path(const std::string &pwd_path, std::string &abs_path);

    std::string get_abs_path(const std::string &rel_path = "");

    void do_full_path(const char *path, std::string &full_path);
    bool undo_full_path(const char *full_path, std::string &path);

  protected:
    std::string root;
};


class scdc_dataprov_fs2_access: public scdc_dataprov_fs2
{
  public:
    scdc_dataprov_fs2_access();

    virtual bool open(const char *conf, scdc_args *args, scdc_result &result);

    virtual scdc_dataset *dataset_open(std::string &path, scdc_result &result);

#if 0
    virtual bool config_do_cmd_param(const std::string &cmd, const std::string &param, std::string val, scdc_config_result &result, bool &done);
#endif
};


class scdc_dataprov_fs2_store: public scdc_dataprov_fs2
{
  public:
    scdc_dataprov_fs2_store();

    virtual bool open(const char *conf, scdc_args *args, scdc_result &result);

    virtual scdc_dataset *dataset_open(std::string &path, scdc_result &result);

#if 0
    virtual bool config_do_cmd_param(const std::string &cmd, const std::string &param, std::string val, scdc_config_result &result, bool &done);
#endif
};


#endif /* __DATAPROV_FS2_HH__ */
