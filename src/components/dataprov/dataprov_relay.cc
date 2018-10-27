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

#define SCDC_TRACE_NOT  !SCDC_TRACE_DATAPROV_RELAY

#include "config.hh"
#include "common.hh"
#include "log.hh"
#include "dataset.hh"
#include "dataset_inout.h"
#include "dataprov_relay.hh"

#include "scdc.h"


using namespace std;


#define SCDC_LOG_PREFIX  "dataset-relay: "


class scdc_dataset_relay: public scdc_dataset
{
  public:
    scdc_dataset_relay(scdc_dataprov *dataprov_, scdc_dataset_t dataset_)
      :scdc_dataset(dataprov_), dataset(dataset_) { };


    void set_dataset(scdc_dataset_t dataset_) { dataset = dataset_; }
    scdc_dataset_t get_dataset() { return dataset; }


    virtual bool do_cmd(const char *cmd, scdcint_t cmd_size, scdc_dataset_input_t *input, scdc_dataset_output_t *output, scdc_result &result)
    {
      SCDC_TRACE("do_cmd: '" << string(cmd, cmd_size) << "'");

      scdcint_t ret = scdc_dataset_cmd(dataset, string(cmd, cmd_size).c_str(), input, output);

      SCDC_TRACE("do_cmd: return: '" << ret << "'");

      return (ret == SCDC_SUCCESS);
    }


  private:
    scdc_dataset_t dataset;
};


#undef SCDC_LOG_PREFIX

#define SCDC_LOG_PREFIX  "dataprov-relay: "


scdc_dataprov_relay::scdc_dataprov_relay()
  :scdc_dataprov_register("relay")
{
}


scdc_dataprov_relay::scdc_dataprov_relay(const std::string &type_)
  :scdc_dataprov_register(type_)
{
}


scdc_dataset *scdc_dataprov_relay::dataset_open(std::string &path, scdc_result &result)
{
  SCDC_TRACE("dataset_open: path: '" << path << "'");

  scdc_dataset_relay *dataset_relay = 0;
  scdc_dataset_t ds = SCDC_DATASET_NULL;

  relay_t::iterator r;
  for (r = relay.begin(); r != relay.end(); ++r)
  {
    const string &relay_path = r->first;

    /* if the given path does not contain the relay_path as a prefix, then skip */
    if (path.substr(0, relay_path.size()) != relay_path) continue;

    /* remove the relay_path prefix from the full path */
    string p = path.substr(relay_path.size());

    /* if the full path without the relay_path prefix does not continue with a slash, then skip */
    if (!p.empty() && p[0] != '/') continue;

    /* remove leading slashs */
    p = ltrim(p, "/");

    const string &dst_path = r->second;

    string uri = dst_path + "/" + p;

    ds = scdc_dataset_open(uri.c_str());

    if (ds == SCDC_DATASET_NULL)
    {
      result = "opening remote dataset '" + uri + "' failed";
      SCDC_FAIL(__func__ << ": scdc_dataset_open failed: " << result);
    }

    break;
  }

  if (r == relay.end()) result = "no matching relay found";

  if (ds == SCDC_DATASET_NULL)
  {
    SCDC_FAIL(__func__ << ": failed: " << result);
    goto do_return;
  }

  /* unset path to prevent a further cd command */
  path.clear();

  dataset_relay = new scdc_dataset_relay(this, ds);

do_return:
  SCDC_TRACE("dataset_open: return: " << dataset_relay);

  return dataset_relay;
}


bool scdc_dataprov_relay::dataset_close(scdc_dataset *dataset, scdc_result &result)
{
  SCDC_TRACE("dataset_close: dataset: " << dataset);

  bool ret = true;

  scdc_dataset_relay *dataset_relay = static_cast<scdc_dataset_relay *>(dataset);

  scdc_dataset_t ds = dataset_relay->get_dataset();

  scdc_dataset_close(ds);

  delete dataset;

  SCDC_TRACE("dataset_close: return: " << ret);

  return ret;
}


bool scdc_dataprov_relay::config_do_cmd_param(const std::string &cmd, const std::string &param, std::string val, scdc_config_result &result, bool &done)
{
  SCDC_TRACE("config_do_cmd_param: cmd: '" << cmd << "', param: '" << param << "', val: '" << val << "', result: '" << result << "'");

  done = true;
  bool ret = true;

  if (param == "")
  {
    ret = scdc_dataprov_register::config_do_cmd_param(cmd, param, val, result, done);
    if (!ret) goto do_quit;

    done = true;

    if (cmd == "info")
    {
    } else if (cmd == "ls")
    {
      ret = scdc_dataprov_config::ls(result, "relay");

    } else done = false;

  } else
  {
    ret = scdc_dataprov_register::config_do_cmd_param(cmd, param, val, result, done);
    if (!ret) goto do_quit;

    if (done);
    else if (param == "relay")
    {
      done = true;

      if (cmd == "put")
      {
        string path, url, r;

        ret = scdc_dataprov_config::put<string>(result, "relay", val, path)
           && scdc_dataprov_config::put<string>(result, "relay", val, url);

        if (ret && (!relay_put(path, url, r) || !relay_update(path))) ret = scdc_dataprov_config::fail(result, "relay", "adding path '" + path + "' with url '" + url + "' failed");

      } else if (cmd == "get")
      {
        string r;

        if (relay_get(val, r)) ret = scdc_dataprov_config::get(result, "relay", r);
        else ret = scdc_dataprov_config::fail(result, "relay", "getting relay '" + val + "' failed");

      } else if (cmd == "info")
      {
        string r;

        if (relay_info(val, r)) ret = scdc_dataprov_config::info(result, "relay", r);
        else ret = scdc_dataprov_config::fail(result, "relay", "info of relay '" + val + "' failed");

      } else if (cmd == "rm")
      {
        if (relay_rm(val)) ret = scdc_dataprov_config::rm(result, "relay");
        else ret = scdc_dataprov_config::fail(result, "relay", "removing relay '" + val + "' failed");

      } else if (cmd == "ls")
      {
        string r;

        if (relay_ls(val, r)) ret = scdc_dataprov_config::ls(result, "relay", r);
        else ret = scdc_dataprov_config::fail(result, "relay", "listing relay '" + val + "' failed");
      }

    } else done = false;
  }

do_quit:
  if (done && !ret)
  {
    SCDC_FAIL("config_do_cmd_param: cmd '" << cmd << "' failed");
    return false;
  }

  return ret;
}


bool scdc_dataprov_relay::dataset_cmds_do_cmd(scdc_dataset *dataset, const std::string &cmd, const std::string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output, scdc_result &result)
{
  SCDC_TRACE("dataset_cmds_do_cmd: cmd: '" << cmd << "', params: '" << params << "'");

  bool ret = scdc_dataprov::dataset_cmds_do_cmd(dataset, cmd, params, input, output, result);

  SCDC_TRACE("dataset_cmds_do_cmd: return: '" << ret << "'");

  return ret;
}


bool scdc_dataprov_relay::relay_put(const std::string &path, const std::string &url, std::string &r)
{
  SCDC_TRACE("relay_put: path: '" << path << "', url: '" << url << "'");

  scdcint_t i = 0;

  r = path;

  while (relay.find(r) != relay.end())
  {
    char s[64];
    sprintf(s, "_%" scdcint_fmt, i);

    r = path + s;

    ++i;
  }

  pair<relay_t::iterator, bool> ret = relay.insert(relay_t::value_type(r, url));

  if (!ret.second)
  {
    SCDC_FAIL("relay_put: inserting relay entry '" << r << "' failed");
    return false;
  }

  reg_put(url);

  return true;
}


bool scdc_dataprov_relay::relay_get(const std::string &path, std::string &r)
{
  SCDC_TRACE("relay_get: path: '" << path << "'");

  relay_t::iterator i = relay.find(path);

  if (i == relay.end()) return false;

  r = i->second;

  return true;
}


bool scdc_dataprov_relay::relay_info(const std::string &path, std::string &r)
{
  SCDC_TRACE("relay_info: path: '" << path << "'");

  stringstream ss;

  if (path == "")
  {
    ss << relay.size() << " path(s)";

  } else
  {
    relay_t::iterator i = relay.find(path);

    if (i == relay.end()) return false;

    if (!reg_info(i->second, r)) return false;

    ss << r;
  }

  r = ss.str();

  return true;
}


bool scdc_dataprov_relay::relay_rm(const std::string &path)
{
  SCDC_TRACE("relay_rm: path: '" << path << "'");

  relay_t::iterator i = relay.find(path);

  if (i == relay.end() || !reg_rm(i->second) || relay.erase(path) != 1)
  {
    SCDC_FAIL("relay_rm: erasing relay entry '" << path << "' failed");
    return false;
  }

  return true;
}


bool scdc_dataprov_relay::relay_ls(const std::string &type, std::string &r)
{
  SCDC_TRACE("relay_ls: type: '" << type << "'");

  r = "";

  for (relay_t::iterator i = relay.begin(); i != relay.end(); ++i)
  {
    if (type != "")
    {
      register_t::iterator j = reg.find(i->second);

      if (j == reg.end() || j->second.type != type) continue;
    }

    r += i->first + ",";
  }

  if (r.size() > 0) r.resize(r.size() - 1);

  return true;
}


bool scdc_dataprov_relay::relay_update(const std::string &path)
{
  SCDC_TRACE("relay_update: path: '" << path << "'");

  relay_t::iterator i = relay.find(path);

  if (i == relay.end() || !reg_update(i->second))
  {
    SCDC_FAIL("relay_update: updating relay entry '" << path << "' failed");
    return false;
  }

  return true;
}


#undef SCDC_LOG_PREFIX
