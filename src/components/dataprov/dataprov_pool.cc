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


#include <cstdio>
#include <cstring>
#include <algorithm>

#define SCDC_TRACE_NOT  !SCDC_TRACE_DATAPROV_POOL

#include "config.hh"
#include "common.hh"
#include "log.hh"
#include "dataset_inout.h"
#include "dataprov.hh"
#include "dataprov_fs.hh"
#include "dataprov_bench.hh"
#include "dataprov_hook.hh"
#include "dataprov_mysql.hh"
#include "dataprov_webdav.hh"
#include "dataprov_nfs.hh"
#include "dataprov_register.hh"
#include "dataprov_relay.hh"
#include "dataprov_jobrun.hh"
#include "dataprov_jobrun_relay.hh"
#include "dataprov_pool.hh"


using namespace std;


#define SCDC_LOG_PREFIX  "dataprov-pool: "


scdc_dataprov *scdc_dataprov_pool::open(const char *base_path, const char *conf, scdc_args *args)
{
  SCDC_TRACE("open: base_path: '" << base_path << "', conf: '" << conf << "'");

  stringlist confs(':', conf);
  string dp_type = confs.front_pop();

  if (dp_type == "fs")
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
    if (s == "store")
    {
      confs.front_pop();
      dp_type = "webdav_store";

    } else dp_type = "webdav_store";

  } else if (dp_type == "nfs")
  {
    string s = confs.front();
    if (s == "store")
    {
      confs.front_pop();
      dp_type = "nfs_store";

    } else dp_type = "nfs_store";

  } else if (dp_type == "store")
  {
    string s = confs.front();
    if (s == "fs")
    {
      confs.front_pop();
      dp_type = "store_fs";

    } else if (s == "mysql")
    {
      confs.front_pop();
      dp_type = "store_mysql";

    } else if (s == "webdav")
    {
      confs.front_pop();
      dp_type = "store_webdav";

    } else if (s == "nfs")
    {
      confs.front_pop();
      dp_type = "store_nfs";

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

  if (dp_type == "fs_access") dataprov = new scdc_dataprov_fs_access();
  else if (dp_type == "fs_store" || dp_type == "store_fs") dataprov = new scdc_dataprov_fs_store();
  else if (dp_type == "bench" || dp_type == "gen") dataprov = new scdc_dataprov_bench();
  else if (dp_type == "hook" || dp_type == "config") dataprov = new scdc_dataprov_hook();
#if USE_MYSQL
  else if (dp_type == "mysql_store" || dp_type == "store_mysql") dataprov = new scdc_dataprov_mysql_store();
#endif
  else if (dp_type == "webdav_store" || dp_type == "store_webdav") dataprov = new scdc_dataprov_webdav_store();
  else if (dp_type == "nfs_store" || dp_type == "store_nfs") dataprov = new scdc_dataprov_nfs_store();
  else if (dp_type == "register") dataprov = new scdc_dataprov_register();
  else if (dp_type == "relay") dataprov = new scdc_dataprov_relay();
  else if (dp_type == "jobrun_system") dataprov = new scdc_dataprov_jobrun_system();
  else if (dp_type == "jobrun_handler") dataprov = new scdc_dataprov_jobrun_handler();
  else if (dp_type == "jobrun_relay") dataprov = new scdc_dataprov_jobrun_relay();
  else SCDC_ERROR("unknown dataprov '" << dp_type << "'");

  if (!dataprov) return 0;

  /* use the given config string as type instead of the implementation defined type (i.e., "fs:store" and "store:fs" use the same implementation, but different types) */
  dataprov->set_type(dp_type);

  if (!dataprov->open(confs.conflate().c_str(), args))
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


void scdc_dataprov_pool::close(scdc_dataprov *dataprov)
{
  for (iterator i = begin(); i != end(); ++i)
  {
    if (i->second != dataprov) continue;

    erase(i);

    dataprov->close();

    delete dataprov;

    dataprov = 0;

    break;
  }
}


void scdc_dataprov_pool::close_all()
{
  SCDC_TRACE("close_all:");

  while (!empty()) close(begin()->second);

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


scdc_dataset *scdc_dataprov_pool::dataset_open(const char *path, scdcint_t path_size, scdc_dataset_output_t *output)
{
  SCDC_TRACE("dataset_open: '" << string(path, path_size) << "'");

  string p(path, path_size);

  p = p.substr(0, p.find('/'));

  iterator i = find(p);

  if (i == end())
  {
    SCDC_FAIL("dataset_open: failed: no matching provider found");
    SCDC_DATASET_OUTPUT_PRINTF(output, "no matching provider found");
    return 0;
  }

  scdc_dataprov *dataprov = i->second;

  path += p.size();
  path_size -= p.size();

  /* skip trailing '/' */
  while (path_size > 0 && path[0] == '/')
  {
    ++path;
    --path_size;
  }

  scdc_dataset *dataset = dataprov->dataset_open(path, path_size, output);

  if (!dataset)
  {
    SCDC_FAIL("dataset_open: failed: '" << SCDC_DATASET_OUTPUT_STR(output) << "'");
    return 0;
  }

  SCDC_TRACE("dataset_open: '" << dataset << "'");

  return dataset;
}


void scdc_dataprov_pool::dataset_close(scdc_dataset *dataset, scdc_dataset_output_t *output)
{
  SCDC_TRACE("dataset_close: dataset: '" << dataset << "'");

  dataset->get_dataprov()->dataset_close(dataset, output);
}


scdc_dataset *scdc_dataprov_pool::dataset_open_read_state(scdc_data *incoming, scdc_dataset_output_t *output)
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
    SCDC_DATASET_OUTPUT_PRINTF(output, "data provider not available");
    SCDC_FAIL("dataset_open_read_state: data provider not available");
    return 0;
  }

  return dataprov->dataset_open_read_state(incoming, output);
}


void scdc_dataprov_pool::dataset_close_write_state(scdc_dataset *dataset, scdc_data *outgoing, scdc_dataset_output_t *output)
{
  SCDC_TRACE("dataset_close_write_state: dataset: '" << dataset << "'");

  scdc_dataprov *dataprov = dataset->get_dataprov();

  char *buf = outgoing->get_write_pos_buf();
  scdcint_t buf_size = outgoing->get_write_pos_buf_size();

  scdcint_t n = snprintf(buf, buf_size, "%p|", dataprov);

  SCDC_ASSERT(n <= buf_size);

  if (n > buf_size) n = buf_size;

  outgoing->inc_write_pos(n);

  dataprov->dataset_close_write_state(dataset, outgoing, output);
}


#undef SCDC_LOG_PREFIX
