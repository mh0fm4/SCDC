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


#include <cstdio>
#include <sstream>

#define SCDC_TRACE_NOT  !SCDC_TRACE_DATAPROV_REGISTER

#include "z_pack.h"

#include "config.hh"
#include "common.hh"
#include "log.hh"
#include "dataset.hh"
#include "dataset_inout.h"
#include "dataprov_register.hh"

#include "scdc.h"


using namespace std;


#define SCDC_LOG_PREFIX  "dataset-register: "


/*class scdc_dataset_register: public scdc_dataset
{
};*/

typedef scdc_dataset scdc_dataset_register;


#undef SCDC_LOG_PREFIX

#define SCDC_LOG_PREFIX  "dataprov-register: "


scdc_dataprov_register::scdc_dataprov_register()
  :scdc_dataprov("register")
{
}


scdc_dataprov_register::scdc_dataprov_register(const std::string &type_)
  :scdc_dataprov(type_)
{
}


scdc_dataset *scdc_dataprov_register::dataset_open(std::string &path, scdc_result &result)
{
  SCDC_TRACE("dataset_open: path: '" << path << "'");

  scdc_dataset_register *dataset_register = new scdc_dataset_register(this);

  SCDC_TRACE("dataset_open: return: " << dataset_register);

  return dataset_register;
}


bool scdc_dataprov_register::dataset_close(scdc_dataset *dataset, scdc_result &result)
{
  SCDC_TRACE("dataset_close: dataset: " << dataset);

  bool ret = true;

  delete dataset;

  SCDC_TRACE("dataset_close: return: " << ret);

  return ret;
}


bool scdc_dataprov_register::dataset_cmds_do_cmd(scdc_dataset *dataset, const char *cmd, const char *params, scdc_dataset_input_t *input, scdc_dataset_output_t *output, scdc_result &result)
{
  SCDC_TRACE("dataset_cmds_do_cmd: cmd: '" << cmd << "', params: '" << params << "'");

  string c(cmd), p(params), r;
  bool ret;

  if (c == "put")
  {
    ret = reg_put(p);

    if (ret) reg_update(p);

  } else if (c == "get")
  {
    ret = reg_get(p);

  } else if (c == "info")
  {
    ret = reg_info(p, r);

  } else if (c == "rm")
  {
    ret = reg_rm(p);

  } else if (c == "ls")
  {
    ret = reg_ls(p, r);

  } else 
  {
    return scdc_dataprov::dataset_cmds_do_cmd(dataset, cmd, params, input, output, result);
  }

  result = r;

  return ret;
}


bool scdc_dataprov_register::reg_put(const std::string &url)
{
  SCDC_TRACE("reg_put: url: '" << url << "'");

  pair<register_t::iterator, bool> ret = reg.insert(register_t::value_type(url, scdc_dataprov_register_entry_t()));

  if (!ret.second)
  {
    SCDC_FAIL("reg_put: inserting register entry '" << url << "' failed");
    return false;
  }

  return true;
}


bool scdc_dataprov_register::reg_get(const std::string &url)
{
  SCDC_TRACE("reg_get: url: '" << url << "'");

  return (reg.find(url) != reg.end());
}


bool scdc_dataprov_register::reg_info(const std::string &url, string &r)
{
  SCDC_TRACE("reg_info: url: '" << url << "'");

  stringstream ss;

  if (url == "")
  {
    ss << reg.size() << " url(s)";

  } else
  {
    register_t::iterator i = reg.find(url);

    if (i == reg.end()) return false;

    ss << i->second.type << ":" << i->second.state;
  }

  r = ss.str();

  return true;
}


bool scdc_dataprov_register::reg_rm(const std::string &url)
{
  SCDC_TRACE("reg_rm: url: '" << url << "'");

  if (reg.erase(url) != 1)
  {
    SCDC_FAIL("relay_rm: erasing relay entry '" << url << "' failed");
    return false;
  }

  return true;
}


bool scdc_dataprov_register::reg_ls(const std::string &type, string &r)
{
  SCDC_TRACE("reg_ls: type: '" << type << "'");

  r = "";

  for (register_t::iterator i = reg.begin(); i != reg.end(); ++i)
  {
    if (type == "" || i->second.type == type) r += i->first + ",";
  }

  if (r.size() > 0) r.resize(r.size() - 1);

  return true;
}


bool scdc_dataprov_register::reg_update(const std::string &url)
{
  SCDC_TRACE("reg_update: url: '" << url << "'");

  register_t::iterator i = reg.find(url);

  if (i == reg.end()) return false;

  i->second.type = "";
  i->second.state = "";

  string cmd = url + "/CONFIG info";

  bool ret = scdc_dataset_cmd(SCDC_DATASET_NULL, cmd.c_str(), 0, 0);
  string result = scdc_last_result();

  if (ret)
  {
    string::size_type p = result.find('|');

    i->second.type = result.substr(0, p);
    if (p != string::npos) i->second.state = result.substr(p + 1, string::npos);
  }

  SCDC_TRACE("reg_update: type: '" << i->second.type << "', state: '" << i->second.state << "'");

  return ret;
}


#undef SCDC_LOG_PREFIX
