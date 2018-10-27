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


#ifndef __DATAPROV_STORE_POSIX_HANDLER_TCC__
#define __DATAPROV_STORE_POSIX_HANDLER_TCC__

#include <vector>
#include <fcntl.h>

#include "z_pack.h"

#if SCDC_TRACE_NOT
# define DATAPROV_STORE_POSIX_HANDLER_TCC_SCDC_TRACE_NOT_BACKUP  1
#else
# define DATAPROV_STORE_POSIX_HANDLER_TCC_SCDC_TRACE_NOT_BACKUP  0
#endif /* SCDC_TRACE_NOT */
#undef SCDC_TRACE_NOT
#include "log_unset.hh"

#define SCDC_TRACE_NOT  !SCDC_TRACE_DATAPROV_STORE_POSIX

#include "config.hh"
#include "common.hh"
#include "log.hh"
#include "dataprov_store_posix_handler.hh"

#include "dataprov_access_posix_handler.tcc"


#define SCDC_LOG_PREFIX  "dataprov-store-posix-handler: "


template<class POSIX_HANDLER>
const typename scdc_dataprov_store_posix_handler<POSIX_HANDLER>::store_t scdc_dataprov_store_posix_handler<POSIX_HANDLER>::store_null = scdc_dataprov_access_posix_handler<POSIX_HANDLER>::dir_null;
template<class POSIX_HANDLER>
const typename scdc_dataprov_store_posix_handler<POSIX_HANDLER>::entry_t scdc_dataprov_store_posix_handler<POSIX_HANDLER>::entry_null = scdc_dataprov_access_posix_handler<POSIX_HANDLER>::file_null;


template<class POSIX_HANDLER>
bool scdc_dataprov_store_posix_handler<POSIX_HANDLER>::open_config_conf(const std::string &conf, scdc_args *args, bool &done)
{
  SCDC_TRACE_F("conf: '" << conf << "'");

  bool ret = scdc_dataprov_access_posix_handler<POSIX_HANDLER>::open_config_conf(conf, args, done);

  SCDC_TRACE_F("return: " << ret);

  return ret;
}


template<class POSIX_HANDLER>
bool scdc_dataprov_store_posix_handler<POSIX_HANDLER>::open_conf(std::string &conf, scdc_args *args, scdc_result &result)
{
  SCDC_TRACE_F("conf: '" << conf << "'");

  bool ret = scdc_dataprov_access_posix_handler<POSIX_HANDLER>::open_conf(conf, args, result);

  SCDC_TRACE_F("return: " << ret << ", result: '" << result << "'");

  return ret;
}


template<class POSIX_HANDLER>
bool scdc_dataprov_store_posix_handler<POSIX_HANDLER>::open(scdc_result &result)
{
  SCDC_TRACE_F("");

  bool ret = scdc_dataprov_access_posix_handler<POSIX_HANDLER>::open(result);

  SCDC_TRACE_F("return: " << ret << ", result: '" << result << "'");

  return ret;
}


template<class POSIX_HANDLER>
bool scdc_dataprov_store_posix_handler<POSIX_HANDLER>::close(scdc_result &result)
{
  SCDC_TRACE_F("");

  bool ret = scdc_dataprov_access_posix_handler<POSIX_HANDLER>::close(result);

  SCDC_TRACE_F("return: " << ret << ", result: '" << result << "'");

  return ret;
}


template<class POSIX_HANDLER>
bool scdc_dataprov_store_posix_handler<POSIX_HANDLER>::ls_stores(std::string &result)
{
  SCDC_TRACE_F("");

  bool ret = false;

  std::string p;
  scdc_dataprov_access_posix_handler<POSIX_HANDLER>::make_path("", "", p);
  SCDC_TRACE_F("p: '" << p << "'");

  {
    typename POSIX_HANDLER::dir_t dir = POSIX_HANDLER::dir_open(p.c_str());
    if (!dir)
    {
      result = "open stores directory '" + p + "' failed";
      SCDC_FAIL_F(result);
      goto do_return;
    }

    std::vector<std::string> stores;
    POSIX_HANDLER::dir_list_dirs(dir, stores);

    POSIX_HANDLER::dir_close(dir);

    result = zxx::to_string(stores.size()) + "|";

    for (std::vector<std::string>::const_iterator i = stores.begin(); i != stores.end(); ++i) result += *i + "|";
  }

  ret = true;

do_return:
  SCDC_TRACE_F("return: " << ret << ", result: " << result);

  return ret;
}


template<class POSIX_HANDLER>
bool scdc_dataprov_store_posix_handler<POSIX_HANDLER>::info_store(const char *path, std::string &result)
{
  SCDC_TRACE_F("path: '" << path << "'");

  bool ret = true;

  std::string p;
  scdc_dataprov_access_posix_handler<POSIX_HANDLER>::make_path(path, "", p);
  SCDC_TRACE_F("p: '" << p << "'");

  typename POSIX_HANDLER::dir_t dir = POSIX_HANDLER::dir_open(p.c_str());

  if (!path || path[0] == '\0')
  {
    if (!dir)
    {
      result = "open stores directory '" + p + "' failed";
      SCDC_FAIL_F(result);
      goto do_return;
    }

    result = "stores: " + zxx::to_string(POSIX_HANDLER::dir_count_dirs(dir));

  } else
  {
    if (!dir)
    {
      result = std::string("store '") + path + "' not found";
      SCDC_FAIL_F(result);
      goto do_return;
    }

    result = std::string("store '") + path + "', entries: " + zxx::to_string(POSIX_HANDLER::dir_count_files(dir));
  }

  POSIX_HANDLER::dir_close(dir);

  ret = true;

do_return:
  SCDC_TRACE_F("return: " << ret << ", result: " << result);

  return ret;
}


template<class POSIX_HANDLER>
bool scdc_dataprov_store_posix_handler<POSIX_HANDLER>::mk_store(const char *path)
{
  SCDC_TRACE_F("path: '" << path << "'");

  bool ret = false;

  if (!path || path[0] == '\0') goto do_return;

  {
    std::string p;
    scdc_dataprov_access_posix_handler<POSIX_HANDLER>::make_path(path, "", p);
    SCDC_TRACE_F("p: '" << p << "'");

    if (!POSIX_HANDLER::dir_mk(p.c_str()))
    {
      SCDC_FAIL_F("creating store directory '" << p << "' failed");
      goto do_return;
    }
  }

  ret = true;

do_return:
  SCDC_TRACE_F("return: " << ret);

  return ret;
}


template<class POSIX_HANDLER>
bool scdc_dataprov_store_posix_handler<POSIX_HANDLER>::rm_store(const char *path)
{
  SCDC_TRACE_F("path: '" << path << "'");

  bool ret = false;

  if (!path || path[0] == '\0') goto do_return;

  {
    std::string p;
    scdc_dataprov_access_posix_handler<POSIX_HANDLER>::make_path(path, "", p);
    SCDC_TRACE_F("p: '" << p << "'");

    if (!POSIX_HANDLER::dir_exists(p.c_str()))
    {
      SCDC_FAIL_F("store directory '" << p << "' does not exist");
      goto do_return;
    } 

    if (!POSIX_HANDLER::dir_rm(p.c_str()))
    {
      SCDC_FAIL_F("removing store directory '" << p << "' failed");
      goto do_return;
    }
  }

  ret = true;

do_return:
  SCDC_TRACE_F("return: " << ret);

  return ret;
}


template<class POSIX_HANDLER>
typename scdc_dataprov_store_posix_handler<POSIX_HANDLER>::store_t scdc_dataprov_store_posix_handler<POSIX_HANDLER>::store_open(const char *path)
{
  SCDC_TRACE_F("path: '" << path << "'");

  store_t store = scdc_dataprov_access_posix_handler<POSIX_HANDLER>::dir_open(path);

  SCDC_TRACE_F("return: " << store);

  return store;
}


template<class POSIX_HANDLER>
void scdc_dataprov_store_posix_handler<POSIX_HANDLER>::store_close(store_t store)
{
  SCDC_TRACE_F("store: '" << store << "'");

  scdc_dataprov_access_posix_handler<POSIX_HANDLER>::dir_close(store);

  SCDC_TRACE_F("return:");
}


template<class POSIX_HANDLER>
bool scdc_dataprov_store_posix_handler<POSIX_HANDLER>::ls_entries(store_t store, std::string &result)
{
  SCDC_TRACE_F("store: '" << store << "'");

  bool ret = false;

  std::string p;
  scdc_dataprov_access_posix_handler<POSIX_HANDLER>::dir_make_path(store, "", p);

  {
    typename POSIX_HANDLER::dir_t dir = POSIX_HANDLER::dir_open(p.c_str());
    if (!dir)
    {
      result = std::string("store '") + *store + "' not found";
      SCDC_FAIL_F(result);
      goto do_return;
    }

    std::vector<std::string> entries;
    POSIX_HANDLER::dir_list_files(dir, entries);
    POSIX_HANDLER::dir_close(dir);

    result = zxx::to_string(entries.size()) + "|";

    for (std::vector<std::string>::const_iterator i = entries.begin(); i != entries.end(); ++i)
    {
      result += *i + "|";
    }
  }

  ret = true;

do_return:
  SCDC_TRACE_F("return: " << ret << ", result: " << result);

  return ret;
}


template<class POSIX_HANDLER>
bool scdc_dataprov_store_posix_handler<POSIX_HANDLER>::info_entry(store_t store, const char *path, std::string &result)
{
  SCDC_TRACE_F("store: '" << store << "', path: '" << path << "'");

  bool ret = true;

  std::string p;
  scdc_dataprov_access_posix_handler<POSIX_HANDLER>::dir_make_path(store, path, p);

  if (!path || path[0] == '\0')
  {
    typename POSIX_HANDLER::dir_t dir = POSIX_HANDLER::dir_open(p.c_str());
    if (!dir)
    {
      result = "open stores directory '" + p + "' failed";
      SCDC_FAIL_F(result);
      goto do_return;
    }

    result = "entries: " + zxx::to_string(POSIX_HANDLER::dir_count_files(dir));

    POSIX_HANDLER::dir_close(dir);

  } else
  {
    scdcint_t s = POSIX_HANDLER::file_get_size(p.c_str());

    if (s < 0)
    {
      result = std::string("entry '") + path + "' not found";
      SCDC_FAIL_F(result);
      goto do_return;
    }

    result = std::string("entry '") + path + "', size: " + zxx::to_string(s);
  }

  ret = true;

do_return:
  SCDC_TRACE_F("return: " << ret << ", result: " << result);

  return ret;
}


template<class POSIX_HANDLER>
bool scdc_dataprov_store_posix_handler<POSIX_HANDLER>::rm_entry(store_t store, const char *path)
{
  SCDC_TRACE_F("store: '" << store << "', path: '" << path << "'");

  bool ret = false;

  if (!path || path[0] == '\0') goto do_return;

  {
    std::string p;
    scdc_dataprov_access_posix_handler<POSIX_HANDLER>::dir_make_path(store, path, p);

    if (!POSIX_HANDLER::file_rm(p.c_str()))
    {
      SCDC_FAIL_F("removing entry '" << path << "' failed");
      goto do_return;
    }
  }

  ret = true;

do_return:
  SCDC_TRACE_F("return: " << ret);

  return ret;
}


template<class POSIX_HANDLER>
typename scdc_dataprov_store_posix_handler<POSIX_HANDLER>::entry_t scdc_dataprov_store_posix_handler<POSIX_HANDLER>::entry_open(store_t store, const char *path, bool read, bool write, bool create)
{
  SCDC_TRACE_F("store: '" << store << "', path: '" << path << "', read: " << read << ", write: " << write << ", create: " << create);

  entry_t entry = scdc_dataprov_access_posix_handler<POSIX_HANDLER>::file_open(store, path, read, write, create);

  SCDC_TRACE_F("return: " << entry);

  return entry;
}


template<class POSIX_HANDLER>
typename scdc_dataprov_store_posix_handler<POSIX_HANDLER>::entry_t scdc_dataprov_store_posix_handler<POSIX_HANDLER>::entry_reopen(store_t store, const char *path, bool read, bool write, bool create, entry_t entry)
{
  SCDC_TRACE_F("store: '" << store << "', path: '" << path << "', read: " << read << ", write: " << write << ", create: " << create << ", entry: " << entry);

  entry_t ret_entry = scdc_dataprov_access_posix_handler<POSIX_HANDLER>::file_reopen(store, path, read, write, create, entry);

  SCDC_TRACE_F("return: " << ret_entry);

  return ret_entry;
}


template<class POSIX_HANDLER>
void scdc_dataprov_store_posix_handler<POSIX_HANDLER>::entry_close(store_t store, scdc_dataprov_store_posix_handler::entry_t entry)
{
  SCDC_TRACE_F("store: '" << store << "', entry: '" << entry << "'");

  scdc_dataprov_access_posix_handler<POSIX_HANDLER>::file_close(store, entry);

  SCDC_TRACE_F("return");
}


template<class POSIX_HANDLER>
bool scdc_dataprov_store_posix_handler<POSIX_HANDLER>::entry_match(store_t store, entry_t entry, const char *path)
{
  SCDC_TRACE_F("store: '" << store << "', entry: '" << entry << "', path: '" << path << "'");

  bool ret = scdc_dataprov_access_posix_handler<POSIX_HANDLER>::file_match(store, entry, path);

  SCDC_TRACE_F("return: " << ret);

  return ret;
}


template<class POSIX_HANDLER>
bool scdc_dataprov_store_posix_handler<POSIX_HANDLER>::entry_read_access_at(store_t store, entry_t entry, scdcint_t size, scdcint_t pos, scdc_buf_t &buf)
{
  SCDC_TRACE_F("store: '" << store << "', entry: '" << entry << "', size: " << size << ", pos: " << pos);

  bool ret = scdc_dataprov_access_posix_handler<POSIX_HANDLER>::file_read_access_at(store, entry, size, pos, buf);

  SCDC_TRACE_F("return: " << ret);

  return ret;
}


template<class POSIX_HANDLER>
scdcint_t scdc_dataprov_store_posix_handler<POSIX_HANDLER>::entry_read_at(store_t store, entry_t entry, void *ptr, scdcint_t size, scdcint_t pos)
{
  SCDC_TRACE_F("store: '" << store << "', entry: '" << entry << "', ptr: " << ptr << ", size: " << size << ", pos: " << pos);

  scdcint_t n = scdc_dataprov_access_posix_handler<POSIX_HANDLER>::file_read_at(store, entry, ptr, size, pos);

  SCDC_TRACE_F("return: " << n);

  return n;
}


template<class POSIX_HANDLER>
scdcint_t scdc_dataprov_store_posix_handler<POSIX_HANDLER>::entry_write_at(store_t store, entry_t entry, const void *ptr, scdcint_t size, scdcint_t pos)
{
  SCDC_TRACE_F("store: '" << store << "', entry: '" << entry << "', ptr: " << ptr << ", size: " << size << ", pos: " << pos);

  scdcint_t n = scdc_dataprov_access_posix_handler<POSIX_HANDLER>::file_write_at(store, entry, ptr, size, pos);

  SCDC_TRACE_F("return: " << n);

  return n;
}

#undef SCDC_LOG_PREFIX


#undef SCDC_TRACE_NOT
#if DATAPROV_STORE_POSIX_HANDLER_TCC_SCDC_TRACE_NOT_BACKUP
# define SCDC_TRACE_NOT  1
#else /* DATAPROV_STORE_POSIX_HANDLER_TCC_SCDC_TRACE_NOT_BACKUP */
# define SCDC_TRACE_NOT  0
#endif /* DATAPROV_STORE_POSIX_HANDLER_TCC_SCDC_TRACE_NOT_BACKUP */
#undef DATAPROV_STORE_POSIX_HANDLER_TCC_SCDC_TRACE_NOT_BACKUP
#include "log_unset.hh"
#include "log_set.hh"


#endif /* __DATAPROV_STORE_POSIX_HANDLER_TCC__ */
