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


template class scdc_dataprov_store<scdc_dataprov_store_mem_handler>;


#define SCDC_LOG_PREFIX  "dataprov-store-mem-handler: "

const char * const scdc_dataprov_store_mem_handler::type = "mem";
const scdc_dataprov_store_mem_handler::store_t scdc_dataprov_store_mem_handler::store_null = 0;
const scdc_dataprov_store_mem_handler::entry_t scdc_dataprov_store_mem_handler::entry_null = 0;


scdc_dataprov_store_mem_handler::store_t scdc_dataprov_store_mem_handler::add_store(const char *path)
{
  SCDC_TRACE(__func__ << ": path: '" << path << "'");

  store_t store = new store_data_t;
  store->name = path;

  return store;
}


void scdc_dataprov_store_mem_handler::del_store(store_t store)
{
  SCDC_TRACE(__func__ << ": store: '" << store << "'");

  clear_entries(store);

  delete store;
}


scdc_dataprov_store_mem_handler::store_t scdc_dataprov_store_mem_handler::find_store(const char *path)
{
  SCDC_TRACE(__func__ << ": path: '" << path << "'");

  store_t ret = store_null;

  if (!path || path[0] == '\0') goto do_return;

  for (stores_t::const_iterator i = stores.begin(); ret == store_null && i != stores.end(); ++i)
  {
    if ((*i)->name == path) ret = *i;
  }

do_return:

  SCDC_TRACE(__func__ << ": return: " << ret);

  return ret;
}


void scdc_dataprov_store_mem_handler::clear_stores()
{
  SCDC_TRACE(__func__ << ":");

  for (stores_t::iterator i = stores.begin(); i != stores.end(); ++i) del_store(*i);

  stores.clear();
}


bool scdc_dataprov_store_mem_handler::ls_stores(std::string &result)
{
  SCDC_TRACE(__func__ << ":");

  bool ret = false;

  char s[16];
  sprintf(s, "%" scdcint_fmt "|", scdcint_cast(stores.size()));

  result = s;

  for (stores_t::const_iterator i = stores.begin(); i != stores.end(); ++i)
  {
    result += (*i)->name;
    result += "|";
  }

  ret = true;

  return ret;
}


bool scdc_dataprov_store_mem_handler::info_store(const char *path, std::string &result)
{
  SCDC_TRACE(__func__ << ": path: '" << path << "'");

  bool ret = true;

  char s[256];

  if (!path || path[0] == '\0')
  {
    sprintf(s, "stores: %" scdcint_fmt, scdcint_t(stores.size()));

  } else
  {
    store_t store = find_store(path);

    if (store != store_null) sprintf(s, "store '%s': entries: %" scdcint_fmt, path, scdcint_cast(store->entries.size()));
    else sprintf(s, "store '%s': not found", path);
  }

  ret = true;
  result = s;

  return ret;
}


bool scdc_dataprov_store_mem_handler::mk_store(const char *path)
{
  SCDC_TRACE(__func__ << ": path: '" << path << "'");

  bool ret = false;
  store_t store = store_null;

  if (!path || path[0] == '\0') goto do_return;

  if (find_store(path) != store_null) goto do_return;

  store = add_store(path);

  if (store == store_null) goto do_return;

  stores.push_back(store);

  ret = true;

do_return:

  return ret;
}


bool scdc_dataprov_store_mem_handler::rm_store(const char *path)
{
  SCDC_TRACE(__func__ << ": path: '" << path << "'");

  bool ret = false;

  if (!path || path[0] == '\0') goto do_return;

  for (stores_t::iterator i = stores.begin(); i != stores.end(); ++i)
  {
    if ((*i)->name == path)
    {
      del_store(*i);

      stores.erase(i);

      ret = true;
      break;
    }
  }

do_return:

  return ret;
}


scdc_dataprov_store_mem_handler::store_t scdc_dataprov_store_mem_handler::store_open(const char *path)
{
  SCDC_TRACE(__func__ << ": path: '" << path << "'");

  store_t store = find_store(path);

  SCDC_TRACE(__func__ << ": return: " << store);

  return store;
}


void scdc_dataprov_store_mem_handler::store_close(store_t store)
{
  SCDC_TRACE(__func__ << ": store: '" << store << "'");
}


bool scdc_dataprov_store_mem_handler::store_match(store_t store, const char *path)
{
  SCDC_TRACE(__func__ << ": path: '" << path << "'");

  bool ret = (store != store_null && store->name == path);

  SCDC_TRACE(__func__ << ": return: " << ret);

  return ret;
}


scdc_dataprov_store_mem_handler::entry_t scdc_dataprov_store_mem_handler::add_entry(store_t store, const char *path)
{
  SCDC_TRACE(__func__ << ": store: '" << store << "', path: '" << path << "'");

  entry_t entry = new entry_data_t;
  entry->name = path;
  entry->buf.ptr = 0;
  entry->buf.size = entry->buf.current = 0;

  return entry;
}


void scdc_dataprov_store_mem_handler::del_entry(store_t store, entry_t entry)
{
  SCDC_TRACE(__func__ << ": store: '" << store << "', entry: '" << entry << "'");

  if (entry->buf.ptr) free(entry->buf.ptr);

  delete entry;
}


scdc_dataprov_store_mem_handler::entry_t scdc_dataprov_store_mem_handler::find_entry(store_t store, const char *path)
{
  SCDC_TRACE(__func__ << ": store: '" << store << "', path: '" << path << "'");

  entry_t ret = entry_null;

  if (!path || path[0] == '\0') goto do_return;

  for (entries_t::const_iterator i = store->entries.begin(); ret == entry_null && i != store->entries.end(); ++i)
  {
    if ((*i)->name == path) ret = *i;
  }

do_return:

  SCDC_TRACE(__func__ << ": return: " << ret);

  return ret;
}


void scdc_dataprov_store_mem_handler::clear_entries(store_t store)
{
  SCDC_TRACE(__func__ << ": store: '" << store << "'");

  for (entries_t::iterator i = store->entries.begin(); i != store->entries.end(); ++i) del_entry(store, *i);

  store->entries.clear();
}


bool scdc_dataprov_store_mem_handler::ls_entries(store_t store, std::string &result)
{
  SCDC_TRACE(__func__ << ": store: '" << store << "'");

  SCDC_TRACE(__func__ << ":");

  bool ret = false;

  char s[256];
  sprintf(s, "%" scdcint_fmt "|", scdcint_cast(store->entries.size()));

  result = s;

  for (entries_t::const_iterator i = store->entries.begin(); i != store->entries.end(); ++i)
  {
    sprintf(s, "%s:%" scdcint_fmt "|", (*i)->name.c_str(), (*i)->buf.current);
    result += s;
  }

  ret = true;

  return ret;
}


bool scdc_dataprov_store_mem_handler::info_entry(store_t store, const char *path, std::string &result)
{
  SCDC_TRACE(__func__ << ": store: '" << store << "', path: '%s'" << path);

  bool ret = true;

  char s[256];

  if (!path || path[0] == '\0')
  {
    sprintf(s, "entries: %" scdcint_fmt, scdcint_t(store->entries.size()));

  } else
  {
    entry_t entry = find_entry(store, path);

    if (entry != entry_null) sprintf(s, "entry '%s': size: %" scdcint_fmt, path, scdcint_cast(entry->buf.current));
    else sprintf(s, "entry '%s': not found", path);
  }

  ret = true;
  result = s;

  return ret;
}


bool scdc_dataprov_store_mem_handler::rm_entry(store_t store, const char *path)
{
  SCDC_TRACE(__func__ << ": store: '" << store << "', path: '" << path << "'");

  bool ret = false;

  if (!path || path[0] == '\0') goto do_return;

  for (entries_t::iterator i = store->entries.begin(); i != store->entries.end(); ++i)
  {
    if ((*i)->name == path)
    {
      del_entry(store, *i);

      store->entries.erase(i);

      ret = true;
      break;
    }
  }

do_return:

  return ret;
}


scdc_dataprov_store_mem_handler::entry_t scdc_dataprov_store_mem_handler::entry_open(store_t store, const char *path, bool read, bool write, bool create)
{
  SCDC_TRACE(__func__ << ": path: '" << path << "', read: " << read << ", write: " << write << ", create: " << create);

  entry_t entry = entry_null;

  if (!path || path[0] == '\0') goto do_return;

  entry = find_entry(store, path);

  if (entry == entry_null)
  {
    if (!create) goto do_return;

    entry = add_entry(store, path);

    store->entries.push_back(entry);
  }

do_return:

  SCDC_TRACE(__func__ << ": return: " << entry);

  return entry;
}


scdc_dataprov_store_mem_handler::entry_t scdc_dataprov_store_mem_handler::entry_reopen(store_t store, const char *path, bool read, bool write, bool create, entry_t entry)
{
  SCDC_TRACE(__func__ << ": path: '" << path << "', read: " << read << ", write: " << write << ", create: " << create << ", entry: " << entry);

  if (entry != entry_null) entry_close(store, entry);

  entry = entry_open(store, path, read, write, create);

  SCDC_TRACE(__func__ << ": return: " << entry);

  return entry;
}


void scdc_dataprov_store_mem_handler::entry_close(store_t store, entry_t entry)
{
  SCDC_TRACE(__func__ << ": entry: '" << entry << "'");
}


bool scdc_dataprov_store_mem_handler::entry_match(store_t store, entry_t entry, const char *path)
{
  SCDC_TRACE(__func__ << ": entry: '" << entry << "', path: '" << path << "'");

  bool ret = (entry != entry_null && entry->name == path);

  SCDC_TRACE(__func__ << ": return: " << ret);

  return ret;
}


bool scdc_dataprov_store_mem_handler::entry_read_access_at(store_t store, entry_t entry, scdcint_t size, scdcint_t pos, scdc_buf_t &buf)
{
  SCDC_TRACE(__func__ << ": store: '" << store << "', entry: '" << entry << "', size: " << size << ", pos: " << pos);

  bool ret = false;

  if (!entry->buf.ptr) goto do_return;

  if (pos < 0) pos = 0;

  if (pos > entry->buf.size) goto do_return;

  if (size < 0 || size > entry->buf.size - pos) size = entry->buf.size - pos;

  buf.ptr = static_cast<char *>(entry->buf.ptr) + pos;
  buf.size = entry->buf.size - pos;
  buf.current = size;

do_return:

  SCDC_TRACE(__func__ << ": return: " << ret);

  return ret;
}


scdcint_t scdc_dataprov_store_mem_handler::entry_read_at(store_t store, entry_t entry, void *ptr, scdcint_t size, scdcint_t pos)
{
  SCDC_TRACE(__func__ << ": store: '" << store << "', entry: '" << entry << "', ptr: " << ptr << ", size: " << size << ", pos: " << pos);

  scdcint_t n = -1;

  if (!entry->buf.ptr) goto do_return;

  if (pos < 0) pos = 0;

  if (pos > entry->buf.size) goto do_return;

  if (size < 0 || size > entry->buf.size - pos) size = entry->buf.size - pos;

  memcpy(ptr, static_cast<char *>(entry->buf.ptr) + pos, size);
  n = size;

do_return:

  SCDC_TRACE(__func__ << ": return: " << n);

  return n;
}


scdcint_t scdc_dataprov_store_mem_handler::entry_write_at(store_t store, entry_t entry, const void *ptr, scdcint_t size, scdcint_t pos)
{
  SCDC_TRACE(__func__ << ": store: '" << store << "', entry: '" << entry << "', ptr: " << ptr << ", size: " << size << ", pos: " << pos);

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

  SCDC_TRACE(__func__ << ": return: " << n);

  return n;
}

#undef SCDC_LOG_PREFIX
