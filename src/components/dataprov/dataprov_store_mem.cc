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


#include <cstdlib>

#include "z_pack.h"

#define SCDC_TRACE_NOT  !SCDC_TRACE_DATAPROV_STORE_MEM

#include "config.hh"
#include "common.hh"
#include "log.hh"
#include "dataprov_store.hh"
#include "dataprov_store.tcc"
#include "dataprov_store_mem.hh"


using namespace std;


#define SCDC_LOG_PREFIX  "dataprov-store-mem-handler: "


/* instantiate the template class in this compile unit */
template class scdc_dataprov_store<scdc_dataprov_store_mem_handler>;


const char * const scdc_dataprov_store_mem_handler::type = "mem";


bool scdc_dataprov_store_mem_handler::open_config_conf(const std::string &conf, scdc_args *args, bool &done)
{
  SCDC_TRACE_F("conf: '" << conf << "'");

  bool ret = false;
  done = false;

  SCDC_TRACE_F("return: " << ret);

  return ret;
}


bool scdc_dataprov_store_mem_handler::open_conf(std::string &conf, scdc_args *args, scdc_result &result)
{
  SCDC_TRACE_F("conf: '" << conf << "'");

  bool ret = true;

  SCDC_TRACE_F("return: " << ret << ", result: '" << result << "'");

  return ret;
}


bool scdc_dataprov_store_mem_handler::open(scdc_result &result)
{
  SCDC_TRACE_F("conf: '" << conf << "'");

  bool ret = true;

  SCDC_TRACE_F("return: " << ret << ", result: '" << result << "'");

  return ret;
}


bool scdc_dataprov_store_mem_handler::close(scdc_result &result)
{
  SCDC_TRACE_F();

  bool ret = true;

  clear_stores();

  SCDC_TRACE_F("return: " << ret);

  return ret;
}


struct scdc_dataprov_store_mem_handler::entry_data_t
{
  std::string name;
  scdc_buf_t buf;
};
const scdc_dataprov_store_mem_handler::entry_t scdc_dataprov_store_mem_handler::entry_null = 0;

struct scdc_dataprov_store_mem_handler::store_data_t
{
  std::string name;
  scdc_dataprov_store_mem_handler::entries_t entries;
};
const scdc_dataprov_store_mem_handler::store_t scdc_dataprov_store_mem_handler::store_null = 0;


bool scdc_dataprov_store_mem_handler::ls_stores(std::string &result)
{
  SCDC_TRACE_F();

  bool ret = false;

  result = zxx::to_string(stores.size()) + "|";

  for (stores_t::const_iterator i = stores.begin(); i != stores.end(); ++i)
  {
    result += (*i)->name + "|";
  }

  ret = true;

  SCDC_TRACE_F("return: " << ret << ", result: " << result);

  return ret;
}


bool scdc_dataprov_store_mem_handler::info_store(const char *path, std::string &result)
{
  SCDC_TRACE_F("path: '" << path << "'");

  bool ret = true;

  if (!path || path[0] == '\0')
  {
    result = "stores: " + zxx::to_string(stores.size());

  } else
  {
    store_t store = find_store(path);

    if (store == store_null)
    {
      result = string("store '") + path + "' not found";
      SCDC_FAIL_F(result);
      goto do_return;
    }

    result = string("store '") + path + "', entries: " + zxx::to_string(store->entries.size());
  }

  ret = true;

do_return:
  SCDC_TRACE_F("return: " << ret << ", result: " << result);

  return ret;
}


bool scdc_dataprov_store_mem_handler::mk_store(const char *path)
{
  SCDC_TRACE_F("path: '" << path << "'");

  bool ret = false;
  store_t store = store_null;

  if (!path || path[0] == '\0') goto do_return;

  if (find_store(path) != store_null) goto do_return;

  store = add_store(path);

  if (store == store_null)
  {
    SCDC_FAIL_F("adding store '" << path << "' failed");
    goto do_return;
  } 

  stores.push_back(store);

  ret = true;

do_return:
  SCDC_TRACE_F("return: " << ret);

  return ret;
}


bool scdc_dataprov_store_mem_handler::rm_store(const char *path)
{
  SCDC_TRACE_F("path: '" << path << "'");

  bool ret = false;

  if (!path || path[0] == '\0') goto do_return;

  for (stores_t::iterator i = stores.begin(); i != stores.end(); ++i)
  {
    if ((*i)->name != path) continue;

    del_store(*i);

    stores.erase(i);

    ret = true;
  }

  if (!ret)
  {
    SCDC_FAIL_F("removing store '" << path << "' failed");
  }

do_return:
  SCDC_TRACE_F("return: " << ret);

  return ret;
}


scdc_dataprov_store_mem_handler::store_t scdc_dataprov_store_mem_handler::store_open(const char *path)
{
  SCDC_TRACE_F("path: '" << path << "'");

  store_t store = find_store(path);

  SCDC_TRACE_F("return: " << store);

  return store;
}


void scdc_dataprov_store_mem_handler::store_close(store_t store)
{
  SCDC_TRACE_F("store: '" << store << "'");
}


bool scdc_dataprov_store_mem_handler::ls_entries(store_t store, std::string &result)
{
  SCDC_TRACE_F("store: '" << store << "'");

  bool ret = false;

  result = zxx::to_string(store->entries.size()) + "|";

  for (entries_t::const_iterator i = store->entries.begin(); i != store->entries.end(); ++i)
  {
    result += (*i)->name + ":" + zxx::to_string((*i)->buf.current) + "|";
  }

  ret = true;

  SCDC_TRACE_F("return: " << ret << ", result: " << result);

  return ret;
}


bool scdc_dataprov_store_mem_handler::info_entry(store_t store, const char *path, std::string &result)
{
  SCDC_TRACE_F("store: '" << store << "', path: '%s'" << path);

  bool ret = true;

  if (!path || path[0] == '\0')
  {
    result = "entries: " + zxx::to_string(store->entries.size());

  } else
  {
    entry_t entry = find_entry(store, path);

    if (entry == entry_null)
    {
      result = string("entry '") + path + "' not found";
      SCDC_FAIL_F(result);
      goto do_return;
    }

    result = string("entry '") + path + "', size: " + zxx::to_string(entry->buf.current);
  }

  ret = true;

do_return:
  SCDC_TRACE_F("return: " << ret << ", result: " << result);

  return ret;
}


bool scdc_dataprov_store_mem_handler::rm_entry(store_t store, const char *path)
{
  SCDC_TRACE_F("store: '" << store << "', path: '" << path << "'");

  bool ret = false;

  if (!path || path[0] == '\0') goto do_return;

  for (entries_t::iterator i = store->entries.begin(); i != store->entries.end(); ++i)
  {
    if ((*i)->name != path) continue;

    del_entry(store, *i);

    store->entries.erase(i);

    ret = true;
  }

  if (!ret)
  {
    SCDC_FAIL_F("removing entry '" << path << "' failed");
  }

do_return:
  SCDC_TRACE_F("return: " << ret);

  return ret;
}


scdc_dataprov_store_mem_handler::entry_t scdc_dataprov_store_mem_handler::entry_open(store_t store, const char *path, bool read, bool write, bool create)
{
  SCDC_TRACE_F("path: '" << path << "', read: " << read << ", write: " << write << ", create: " << create);

  entry_t entry = entry_null;

  if (!path || path[0] == '\0') goto do_return;

  entry = find_entry(store, path);

  if (entry == entry_null)
  {
    if (!create)
    {
      SCDC_FAIL_F("opening entry '" << path << "' failed");
      goto do_return;
    }

    entry = add_entry(store, path);

    store->entries.push_back(entry);
  }

do_return:
  SCDC_TRACE_F("return: " << entry);

  return entry;
}


scdc_dataprov_store_mem_handler::entry_t scdc_dataprov_store_mem_handler::entry_reopen(store_t store, const char *path, bool read, bool write, bool create, entry_t entry)
{
  SCDC_TRACE_F("path: '" << path << "', read: " << read << ", write: " << write << ", create: " << create << ", entry: " << entry);

  if (entry->name == path) goto do_return;

  entry_close(store, entry);

  entry = entry_open(store, path, read, write, create);

do_return:
  SCDC_TRACE_F("return: " << entry);

  return entry;
}


void scdc_dataprov_store_mem_handler::entry_close(store_t store, entry_t entry)
{
  SCDC_TRACE_F("entry: '" << entry << "'");
}


bool scdc_dataprov_store_mem_handler::entry_match(store_t store, entry_t entry, const char *path)
{
  SCDC_TRACE_F("entry: '" << entry << "', path: '" << path << "'");

  bool ret = (entry->name == path);

  SCDC_TRACE_F("return: " << ret);

  return ret;
}


bool scdc_dataprov_store_mem_handler::entry_read_access_at(store_t store, entry_t entry, scdcint_t size, scdcint_t pos, scdc_buf_t &buf)
{
  SCDC_TRACE_F("store: '" << store << "', entry: '" << entry << "', size: " << size << ", pos: " << pos);

  bool ret = false;

  if (!entry->buf.ptr) goto do_return;

  if (pos < 0) pos = 0;

  if (pos > entry->buf.size) goto do_return;

  if (size < 0 || size > entry->buf.size - pos) size = entry->buf.size - pos;

  buf.ptr = static_cast<char *>(entry->buf.ptr) + pos;
  buf.size = entry->buf.size - pos;
  buf.current = size;

do_return:
  SCDC_TRACE_F("return: " << ret);

  return ret;
}


scdcint_t scdc_dataprov_store_mem_handler::entry_read_at(store_t store, entry_t entry, void *ptr, scdcint_t size, scdcint_t pos)
{
  SCDC_TRACE_F("store: '" << store << "', entry: '" << entry << "', ptr: " << ptr << ", size: " << size << ", pos: " << pos);

  scdcint_t n = -1;

  if (!entry->buf.ptr) goto do_return;

  if (pos < 0) pos = 0;

  if (pos > entry->buf.size) goto do_return;

  if (size < 0 || size > entry->buf.size - pos) size = entry->buf.size - pos;

  memcpy(ptr, static_cast<char *>(entry->buf.ptr) + pos, size);
  n = size;

do_return:
  SCDC_TRACE_F("return: " << n);

  return n;
}


scdcint_t scdc_dataprov_store_mem_handler::entry_write_at(store_t store, entry_t entry, const void *ptr, scdcint_t size, scdcint_t pos)
{
  SCDC_TRACE_F("store: '" << store << "', entry: '" << entry << "', ptr: " << ptr << ", size: " << size << ", pos: " << pos);

  scdcint_t n = -1;

  if (size < 0) goto do_return;

  if (pos < 0) pos = entry->buf.current;

  if (pos + size > entry->buf.size)
  {
    scdcint_t new_size = pos + size;
    void *new_ptr = realloc(entry->buf.ptr, new_size);

    if (!new_ptr) goto do_return;

    entry->buf.ptr = new_ptr;
    entry->buf.size = new_size;
  }

  memcpy(static_cast<char *>(entry->buf.ptr) + pos, ptr, size);
  entry->buf.current = pos + size;
  n = size;

do_return:
  SCDC_TRACE_F("return: " << n);

  return n;
}


scdc_dataprov_store_mem_handler::store_t scdc_dataprov_store_mem_handler::add_store(const char *path)
{
  SCDC_TRACE_F("path: '" << path << "'");

  store_t store = new store_data_t;
  store->name = path;

  SCDC_TRACE_F("return: store: " << store);

  return store;
}


void scdc_dataprov_store_mem_handler::del_store(store_t store)
{
  SCDC_TRACE_F("store: '" << store << "'");

  clear_entries(store);

  delete store;

  SCDC_TRACE_F("return:");
}


scdc_dataprov_store_mem_handler::store_t scdc_dataprov_store_mem_handler::find_store(const char *path)
{
  SCDC_TRACE_F("path: '" << path << "'");

  store_t ret = store_null;

  if (!path || path[0] == '\0') goto do_return;

  for (stores_t::const_iterator i = stores.begin(); ret == store_null && i != stores.end(); ++i)
  {
    if ((*i)->name == path) ret = *i;
  }

do_return:
  SCDC_TRACE_F("return: " << ret);

  return ret;
}


void scdc_dataprov_store_mem_handler::clear_stores()
{
  SCDC_TRACE_F();

  for (stores_t::iterator i = stores.begin(); i != stores.end(); ++i) del_store(*i);

  stores.clear();

  SCDC_TRACE_F("return:");
}


scdc_dataprov_store_mem_handler::entry_t scdc_dataprov_store_mem_handler::add_entry(store_t store, const char *path)
{
  SCDC_TRACE_F("store: '" << store << "', path: '" << path << "'");

  entry_t entry = new entry_data_t;
  entry->name = path;
  entry->buf.ptr = 0;
  entry->buf.size = entry->buf.current = 0;

  SCDC_TRACE_F("return: entry: " << entry);

  return entry;
}


void scdc_dataprov_store_mem_handler::del_entry(store_t store, entry_t entry)
{
  SCDC_TRACE_F("store: '" << store << "', entry: '" << entry << "'");

  if (entry->buf.ptr) free(entry->buf.ptr);

  delete entry;

  SCDC_TRACE_F("return:");
}


scdc_dataprov_store_mem_handler::entry_t scdc_dataprov_store_mem_handler::find_entry(store_t store, const char *path)
{
  SCDC_TRACE_F("store: '" << store << "', path: '" << path << "'");

  entry_t ret = entry_null;

  if (!path || path[0] == '\0') goto do_return;

  for (entries_t::const_iterator i = store->entries.begin(); ret == entry_null && i != store->entries.end(); ++i)
  {
    if ((*i)->name == path) ret = *i;
  }

do_return:
  SCDC_TRACE_F("return: " << ret);

  return ret;
}


void scdc_dataprov_store_mem_handler::clear_entries(store_t store)
{
  SCDC_TRACE_F("store: '" << store << "'");

  for (entries_t::iterator i = store->entries.begin(); i != store->entries.end(); ++i) del_entry(store, *i);

  store->entries.clear();

  SCDC_TRACE_F("return:");
}

#undef SCDC_LOG_PREFIX
