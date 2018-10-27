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
#include <cstring>
#include <algorithm>

#define SCDC_TRACE_NOT  !SCDC_TRACE_DATAPROV_POOL

#include "config.hh"
#include "common.hh"
#include "log.hh"
#include "dataset_inout.h"
#include "dataprov.hh"
#include "dataprov_access_stub.hh"
#include "dataprov_store_stub.hh"
#include "dataprov_store_mem.hh"
#include "dataprov_fs.hh"
#include "dataprov_fs2.hh"
#if USE_MYSQL
# include "dataprov_mysql.hh"
#endif
#if USE_WEBDAV
# include "dataprov_webdav.hh"
# include "dataprov_webdav2.hh"
#endif
#if USE_NFS
# include "dataprov_nfs.hh"
# include "dataprov_nfs2.hh"
#endif
#include "dataprov_bench.hh"
#include "dataprov_hook.hh"
#include "dataprov_register.hh"
#include "dataprov_relay.hh"
#include "dataprov_jobrun.hh"
#include "dataprov_jobrun_relay.hh"
#include "dataprov_pool.hh"


using namespace std;


#define SCDC_LOG_PREFIX  "dataprov-pool: "


scdc_dataprov *scdc_dataprov_pool::open(const char *base_path, const char *conf, scdc_args *args, scdc_result &result)
{
  SCDC_TRACE("open: base_path: '" << base_path << "', conf: '" << conf << "'");

  stringlist confs(':', conf);
  string dp_type = confs.front_pop();

  if (dp_type == "fs") // deprecated
  {
    string s = confs.front();
    if (s == "access")
    {
      confs.front_pop();
      dp_type = "fs_access";

    } else if (s == "store")
    {
      confs.front_pop();
      dp_type = "fs_store";

    } else dp_type = "fs_access";

  } else if (dp_type == "mysql")
  {
    string s = confs.front();
    if (s == "store")
    {
      confs.front_pop();
      dp_type = "mysql_store";

    } else dp_type = "mysql_store";

  } else if (dp_type == "webdav")
  {
    string s = confs.front();
    if (s == "access")
    {
      confs.front_pop();
      dp_type = "webdav_access";

    } else if (s == "store")
    {
      confs.front_pop();
      dp_type = "webdav_store";

    } else dp_type = "webdav_access";

  } else if (dp_type == "nfs")
  {
    string s = confs.front();
    if (s == "access")
    {
      confs.front_pop();
      dp_type = "nfs_access";

    } else if (s == "store")
    {
      confs.front_pop();
      dp_type = "nfs_store";

    } else dp_type = "nfs_access";

  } else if (dp_type == "access")
  {
    string s = confs.front();
    if (s == "stub")
    {
      confs.front_pop();
      dp_type = "access_stub";

    } else if (s == "fs")
    {
      confs.front_pop();
      dp_type = "access_fs";

    } else if (s == "nfs")
    {
      confs.front_pop();
      dp_type = "access_nfs";

    } else if (s == "webdav")
    {
      confs.front_pop();
      dp_type = "access_webdav";

    } else if (s == "none")
    {
      confs.front_pop();
      dp_type = "access_none";

    } else dp_type = "access_none";

  } else if (dp_type == "store")
  {
    string s = confs.front();
    if (s == "stub")
    {
      confs.front_pop();
      dp_type = "store_stub";

    } else if (s == "mem")
    {
      confs.front_pop();
      dp_type = "store_mem";

    } else if (s == "fs")
    {
      confs.front_pop();
      dp_type = "store_fs";

    } else if (s == "nfs")
    {
      confs.front_pop();
      dp_type = "store_nfs";

    } else if (s == "webdav")
    {
      confs.front_pop();
      dp_type = "store_webdav";

    } else if (s == "mysql")
    {
      confs.front_pop();
      dp_type = "store_mysql";

    } else if (s == "none")
    {
      confs.front_pop();
      dp_type = "store_none";

    } else dp_type = "store_none";

  } else if (dp_type == "jobrun")
  {
    string s = confs.front();
    if (s == "system")
    {
      confs.front_pop();
      dp_type = "jobrun_system";

    } else if (s == "handler")
    {
      confs.front_pop();
      dp_type = "jobrun_handler";

    } else if (s == "relay")
    {
      confs.front_pop();
      dp_type = "jobrun_relay";

    } else dp_type = "jobrun_system";
  }

  scdc_dataprov *dataprov = 0;

  if (dp_type == "access_stub") dataprov = new scdc_dataprov_access_stub();
  else if (dp_type == "access_fs" || dp_type == "fs_access") dataprov = new scdc_dataprov_access_fs();
  else if (dp_type == "store_stub") dataprov = new scdc_dataprov_store_stub();
  else if (dp_type == "store_mem") dataprov = new scdc_dataprov_store_mem();
  else if (dp_type == "store_fs" || dp_type == "fs_store") dataprov = new scdc_dataprov_store_fs();
  else if (dp_type == "fs2_access") dataprov = new scdc_dataprov_fs2_access(); // deprecated
  else if (dp_type == "fs2_store") dataprov = new scdc_dataprov_fs2_store(); // deprecated
#if USE_MYSQL
  else if (dp_type == "mysql_store" || dp_type == "store_mysql") dataprov = new scdc_dataprov_mysql_store();
#endif
#if USE_WEBDAV
  else if (dp_type == "webdav_access" || dp_type == "access_webdav") dataprov = new scdc_dataprov_access_webdav();
  else if (dp_type == "webdav_store" || dp_type == "store_webdav") dataprov = new scdc_dataprov_store_webdav();
  else if (dp_type == "webdav2_access") dataprov = new scdc_dataprov_webdav2_access();
#endif
#if USE_NFS
  else if (dp_type == "nfs_access" || dp_type == "access_nfs") dataprov = new scdc_dataprov_access_nfs();
  else if (dp_type == "nfs_store" || dp_type == "store_nfs") dataprov = new scdc_dataprov_store_nfs();
  else if (dp_type == "nfs2_store") dataprov = new scdc_dataprov_nfs2_store();
#endif
  else if (dp_type == "bench" || dp_type == "gen") dataprov = new scdc_dataprov_bench();
  else if (dp_type == "hook" || dp_type == "config") dataprov = new scdc_dataprov_hook();
  else if (dp_type == "register") dataprov = new scdc_dataprov_register();
  else if (dp_type == "relay") dataprov = new scdc_dataprov_relay();
  else if (dp_type == "jobrun_system") dataprov = new scdc_dataprov_jobrun_system();
  else if (dp_type == "jobrun_handler") dataprov = new scdc_dataprov_jobrun_handler();
  else if (dp_type == "jobrun_relay") dataprov = new scdc_dataprov_jobrun_relay();
  else SCDC_ERROR("unknown dataprov '" << dp_type << "'");

  if (!dataprov) return 0;

  /* use the given config string as type instead of the implementation defined type (i.e., "fs:store" and "store:fs" use the same implementation, but different types) */
  dataprov->set_type(dp_type);

  if (!dataprov->open(confs.conflate().c_str(), args, result))
  {
    delete dataprov;
    return 0;
  }

  pair<iterator, bool> ret = insert(value_type(trim(base_path, "/"), dataprov));

  if (!ret.second)
  {
    delete dataprov;
    return 0;
  }

  return dataprov;
}


bool scdc_dataprov_pool::close(scdc_dataprov *dataprov, scdc_result &result)
{
  bool ret = false;

  for (iterator i = begin(); i != end(); ++i)
  {
    if (i->second != dataprov) continue;

    erase(i);

    ret = dataprov->close(result);

    delete dataprov;

    dataprov = 0;

    break;
  }

  return ret;
}


void scdc_dataprov_pool::close_all()
{
  SCDC_TRACE("close_all:");

  scdc_result result;

  while (!empty()) close(begin()->second, result);

  SCDC_TRACE("close_all: return");
}


bool scdc_dataprov_pool::exists(scdc_dataprov *dataprov)
{
  for (iterator i = begin(); i != end(); ++i)
  {
    if (i->second == dataprov) return true;
  }

  return false;
}


scdc_dataset *scdc_dataprov_pool::dataset_open(const std::string &path, scdc_result &result)
{
  SCDC_TRACE(__func__ << ": '" << path << "'");

  string p = path.substr(0, path.find('/'));

  iterator i = find(p);

  if (i == end())
  {
    SCDC_FAIL("dataset_open: failed: no matching provider found");
    result = "no matching provider found";
    return 0;
  }

  scdc_dataprov *dataprov = i->second;

  p = ltrim(path.substr(p.size()), "/");

  scdc_dataset *dataset = dataprov->dataset_path_open(p, result);

  if (!dataset)
  {
    SCDC_FAIL(__func__ << ": opening dataset failed: " << result);
  }

  SCDC_TRACE(__func__ << ": return: " << dataset);

  return dataset;
}


bool scdc_dataprov_pool::dataset_close(scdc_dataset *dataset, scdc_result &result)
{
  SCDC_TRACE(__func__ << ": dataset: " << dataset);

  bool ret = dataset->get_dataprov()->dataset_path_close(dataset, result);

  SCDC_TRACE(__func__ << ": return: " << ret);

  return ret;
}


scdc_dataset *scdc_dataprov_pool::dataset_open_read_state(scdc_data *incoming, scdc_result &result)
{
  SCDC_TRACE("dataset_open_read_state:");

  char *buf;
  scdcint_t buf_size;
  const char *end;

  while (1)
  {
    buf = incoming->get_read_pos_buf();
    buf_size = incoming->get_read_pos_buf_size();
    end = memchr(buf, '|', buf_size);

    if (end) break;

    incoming->do_next();
  }

  scdc_dataprov *dataprov = 0;

  string dataprov_str(buf, end - buf);
  sscanf(dataprov_str.c_str(), "%p", &dataprov);

  incoming->inc_read_pos(end - buf + 1);

  if (!exists(dataprov))
  {
    result = "data provider not available";
    SCDC_FAIL(__func__ << ": " << result);
    return 0;
  }

  return dataprov->dataset_open_read_state(incoming, result);
}


bool scdc_dataprov_pool::dataset_close_write_state(scdc_dataset *dataset, scdc_data *outgoing, scdc_result &result)
{
  SCDC_TRACE("dataset_close_write_state: dataset: '" << dataset << "'");

  scdc_dataprov *dataprov = dataset->get_dataprov();

  char *buf = outgoing->get_write_pos_buf();
  scdcint_t buf_size = outgoing->get_write_pos_buf_size();

  scdcint_t n = snprintf(buf, buf_size, "%p|", dataprov);

  SCDC_ASSERT(n <= buf_size);

  if (n > buf_size) n = buf_size;

  outgoing->inc_write_pos(n);

  bool ret = dataprov->dataset_close_write_state(dataset, outgoing, result);

  SCDC_TRACE("dataset_close_write_state: return: " << ret);

  return ret;
}


#undef SCDC_LOG_PREFIX
