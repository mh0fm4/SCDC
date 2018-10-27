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


#include "z_pack.h"

#define SCDC_TRACE_NOT  !SCDC_TRACE_DATAPROV_STORE_STUB

#include "config.hh"
#include "common.hh"
#include "log.hh"
#include "dataprov_store_stub_handler.hh"


using namespace std;


#define SCDC_LOG_PREFIX  "dataprov-store-stub-handler: "

const char * const scdc_dataprov_store_stub_handler::type = "stub";


bool scdc_dataprov_store_stub_handler::open_config_conf(const std::string &conf, scdc_args *args, bool &done)
{
  SCDC_TRACE_F("conf: '" << conf << "'");

  bool ret = true;
  done = true;

  SCDC_TRACE_F("return: " << ret);

  return ret;
}


bool scdc_dataprov_store_stub_handler::open_conf(std::string &conf, scdc_args *args, scdc_result &result)
{
  SCDC_TRACE_F("conf: '" << conf << "'");

  bool ret = true;

  SCDC_TRACE_F("return: " << ret << ", result: '" << result << "'");

  return ret;
}


bool scdc_dataprov_store_stub_handler::open(scdc_result &result)
{
  SCDC_TRACE_F("conf: '" << conf << "'");

  bool ret = true;

  SCDC_TRACE_F("return: " << ret);

  return ret;
}


bool scdc_dataprov_store_stub_handler::close(scdc_result &result)
{
  SCDC_TRACE_F("");

  bool ret = true;

  SCDC_TRACE_F("return: " << ret);

  return ret;
}


const scdc_dataprov_store_stub_handler::store_t scdc_dataprov_store_stub_handler::store_null = 0;
const scdc_dataprov_store_stub_handler::entry_t scdc_dataprov_store_stub_handler::entry_null = 0;

#define store_valid  (scdc_dataprov_store_stub_handler::store_t) 1
#define entry_valid  (scdc_dataprov_store_stub_handler::entry_t) 1


bool scdc_dataprov_store_stub_handler::ls_stores(std::string &result)
{
  SCDC_TRACE_F("");

  result = "no stores in stub";

  return true;
}


bool scdc_dataprov_store_stub_handler::info_store(const char *path, std::string &result)
{
  SCDC_TRACE_F("");

  result = "store info '" + std::string(path) + "'";

  return true;
}


bool scdc_dataprov_store_stub_handler::mk_store(const char *path)
{
  SCDC_TRACE_F("path: '" << path << "'");

  return true;
}


bool scdc_dataprov_store_stub_handler::rm_store(const char *path)
{
  SCDC_TRACE_F("path: '" << path << "'");

  return true;
}


scdc_dataprov_store_stub_handler::store_t scdc_dataprov_store_stub_handler::store_open(const char *path)
{
  SCDC_TRACE_F("path: '" << path << "'");

  store_t store = store_valid;

  SCDC_TRACE_F("return: " << store);

  return store;
}


void scdc_dataprov_store_stub_handler::store_close(store_t store)
{
  SCDC_TRACE_F("store: '" << store << "'");
}


bool scdc_dataprov_store_stub_handler::ls_entries(store_t store, std::string &result)
{
  SCDC_TRACE_F("store: '" << store << "'");

  result = "no entries in stub";

  return true;
}


bool scdc_dataprov_store_stub_handler::info_entry(store_t store, const char *path, std::string &result)
{
  SCDC_TRACE_F("store: '" << store << "', path: '%s'" << path);

  result = "entry in store '" + std::string(path) + "'";

  return true;
}


bool scdc_dataprov_store_stub_handler::rm_entry(store_t store, const char *path)
{
  SCDC_TRACE_F("store: '" << store << "', path: '" << path << "'");

  bool ret = true;

  SCDC_TRACE_F("return: " << ret);

  return ret;
}


scdc_dataprov_store_stub_handler::entry_t scdc_dataprov_store_stub_handler::entry_open(store_t store, const char *path, bool read, bool write, bool create)
{
  SCDC_TRACE_F("path: '" << path << "', read: " << read << ", write: " << write << ", create: " << create);

  entry_t entry = entry_valid;

  SCDC_TRACE_F("return: " << entry);

  return entry;
}


scdc_dataprov_store_stub_handler::entry_t scdc_dataprov_store_stub_handler::entry_reopen(store_t store, const char *path, bool read, bool write, bool create, entry_t entry)
{
  SCDC_TRACE_F("path: '" << path << "', read: " << read << ", write: " << write << ", create: " << create << ", entry: " << entry);

  entry = entry_valid;

  SCDC_TRACE_F("return: " << entry);

  return entry;
}


void scdc_dataprov_store_stub_handler::entry_close(store_t store, entry_t entry)
{
  SCDC_TRACE_F("entry: '" << entry << "'");
}


bool scdc_dataprov_store_stub_handler::entry_match(store_t store, entry_t entry, const char *path)
{
  SCDC_TRACE_F("path: '" << path << "', entry: '" << entry << "'");

  bool ret = false;

  SCDC_TRACE_F("return: " << ret);

  return ret;
}


bool scdc_dataprov_store_stub_handler::entry_read_access_at(store_t store, entry_t entry, scdcint_t size, scdcint_t pos, scdc_buf_t &buf)
{
  SCDC_TRACE_F("store: '" << store << "', entry: '" << entry << "', size: " << size << ", pos: " << pos);

  bool ret = false;

  SCDC_TRACE_F("return: " << ret);

  return ret;
}


scdcint_t scdc_dataprov_store_stub_handler::entry_read_at(store_t store, entry_t entry, void *ptr, scdcint_t size, scdcint_t pos)
{
  SCDC_TRACE_F("store: '" << store << "', entry: '" << entry << "', ptr: " << ptr << ", size: " << size << ", pos: " << pos);

  scdcint_t ret = 0;

  SCDC_TRACE_F("return: " << ret);

  return ret;
}


scdcint_t scdc_dataprov_store_stub_handler::entry_write_at(store_t store, entry_t entry, const void *ptr, scdcint_t size, scdcint_t pos)
{
  SCDC_TRACE_F("store: '" << store << "', entry: '" << entry << "', ptr: " << ptr << ", size: " << size << ", pos: " << pos);

  scdcint_t ret = max(scdcint_const(0), size);

  SCDC_TRACE_F("return: " << ret);

  return ret;
}

#undef SCDC_LOG_PREFIX
