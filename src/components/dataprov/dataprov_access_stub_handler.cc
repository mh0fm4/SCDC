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

#define SCDC_TRACE_NOT  !SCDC_TRACE_DATAPROV_ACCESS_STUB

#include "config.hh"
#include "common.hh"
#include "log.hh"
#include "dataprov_access_stub_handler.hh"


using namespace std;


#define SCDC_LOG_PREFIX  "dataprov-access-stub-handler: "

const char * const scdc_dataprov_access_stub_handler::type = "stub";


bool scdc_dataprov_access_stub_handler::open_config_conf(const std::string &conf, scdc_args *args, bool &done)
{
  SCDC_TRACE_F("conf: '" << conf << "'");

  bool ret = true;
  done = true;

  SCDC_TRACE_F("return: " << ret);

  return ret;
}


bool scdc_dataprov_access_stub_handler::open_conf(std::string &conf, scdc_args *args, scdc_result &result)
{
  SCDC_TRACE_F("conf: '" << conf << "'");

  bool ret = true;

  SCDC_TRACE_F("return: " << ret << ", result: '" << result << "'");

  return ret;
}


bool scdc_dataprov_access_stub_handler::open(scdc_result &result)
{
  SCDC_TRACE_F("conf: '" << conf << "'");

  bool ret = true;

  SCDC_TRACE_F("return: " << ret);

  return ret;
}


bool scdc_dataprov_access_stub_handler::close(scdc_result &result)
{
  SCDC_TRACE(__func__ << ":");

  bool ret = true;

  SCDC_TRACE(__func__ << ": return: " << ret);

  return ret;
}


const scdc_dataprov_access_stub_handler::dir_t scdc_dataprov_access_stub_handler::dir_null = 0;
const scdc_dataprov_access_stub_handler::file_t scdc_dataprov_access_stub_handler::file_null = 0;

#define dir_valid  (scdc_dataprov_access_stub_handler::dir_t) 1
#define file_valid  (scdc_dataprov_access_stub_handler::file_t) 1


scdc_dataprov_access_stub_handler::dir_t scdc_dataprov_access_stub_handler::dir_open(const char *path)
{
  SCDC_TRACE(__func__ << ": path: '" << path << "'");

  dir_t dir = dir_valid;

  SCDC_TRACE(__func__ << ": return: " << dir);

  return dir;
}


void scdc_dataprov_access_stub_handler::dir_close(dir_t dir)
{
  SCDC_TRACE(__func__ << ": dir: '" << dir << "'");
}


bool scdc_dataprov_access_stub_handler::dir_path(dir_t dir, std::string &path)
{
  SCDC_TRACE(__func__ << ": dir: '" << dir << "'");

  bool ret = true;
  path = "";

  SCDC_TRACE(__func__ << ": return: " << ret << ", path: '" << path << "'");

  return ret;
}


bool scdc_dataprov_access_stub_handler::ls_entries(dir_t dir, std::string &result)
{
  SCDC_TRACE(__func__ << ": dir: '" << dir << "'");

  bool ret = true;
  result = "no entries in stub";

  SCDC_TRACE(__func__ << ": return: " << ret << ", result: '" << result << "'");

  return ret;
}


bool scdc_dataprov_access_stub_handler::info_entry(dir_t dir, const char *path, std::string &result)
{
  SCDC_TRACE(__func__ << ": dir: '" << dir << "', path: '" << path << "'");

  bool ret = true;
  result = "no info in stub for entry '" + std::string(path) + "'";

  SCDC_TRACE(__func__ << ": return: " << ret << ", result: '" << result << "'");

  return ret;
}


bool scdc_dataprov_access_stub_handler::rm_entry(dir_t dir, const char *path)
{
  SCDC_TRACE(__func__ << ": dir: '" << dir << "', path: '" << path << "'");

  bool ret = true;

  SCDC_TRACE(__func__ << ": return: " << ret);

  return ret;
}


bool scdc_dataprov_access_stub_handler::mk_dir(dir_t dir, const char *path)
{
  SCDC_TRACE(__func__ << ": dir: '" << dir << "', path: '" << path << "'");

  bool ret = true;

  SCDC_TRACE(__func__ << ": return: " << ret);

  return ret;
}


scdc_dataprov_access_stub_handler::file_t scdc_dataprov_access_stub_handler::file_open(dir_t dir, const char *path, bool read, bool write, bool create)
{
  SCDC_TRACE(__func__ << ": dir: '" << dir << "', path: '" << path << "', read: " << read << ", write: " << write << ", create: " << create);

  file_t file = file_valid;

  SCDC_TRACE(__func__ << ": return: " << file);

  return file;
}


scdc_dataprov_access_stub_handler::file_t scdc_dataprov_access_stub_handler::file_reopen(dir_t dir, const char *path, bool read, bool write, bool create, file_t file)
{
  SCDC_TRACE(__func__ << ": dir: '" << dir << "', path: '" << path << "', read: " << read << ", write: " << write << ", create: " << create << ", file: " << file);

  file = file_valid;

  SCDC_TRACE(__func__ << ": return: " << file);

  return file;
}


void scdc_dataprov_access_stub_handler::file_close(dir_t dir, file_t file)
{
  SCDC_TRACE(__func__ << ": dir: '" << dir << "', file: '" << file << "'");
}


bool scdc_dataprov_access_stub_handler::file_match(dir_t dir, file_t file, const char *path)
{
  SCDC_TRACE(__func__ << ": dir: '" << dir << "', file: '" << file << "', path: '" << path << "'");

  bool ret = (file != file_null);

  SCDC_TRACE(__func__ << ": return: " << ret);

  return ret;
}


bool scdc_dataprov_access_stub_handler::file_read_access_at(dir_t dir, file_t file, scdcint_t size, scdcint_t pos, scdc_buf_t &buf)
{
  SCDC_TRACE(__func__ << ": dir: '" << dir << "', file: '" << file << "', size: " << size << ", pos: " << pos);

  bool ret = false;

  SCDC_TRACE(__func__ << ": return: " << ret);

  return ret;
}


scdcint_t scdc_dataprov_access_stub_handler::file_read_at(dir_t dir, file_t file, void *ptr, scdcint_t size, scdcint_t pos)
{
  SCDC_TRACE(__func__ << ": dir: '" << dir << "', file: '" << file << "', ptr: " << ptr << ", size: " << size << ", pos: " << pos);

  scdcint_t ret = 0;

  SCDC_TRACE(__func__ << ": return: " << ret);

  return ret;
}


scdcint_t scdc_dataprov_access_stub_handler::file_write_at(dir_t dir, file_t file, const void *ptr, scdcint_t size, scdcint_t pos)
{
  SCDC_TRACE(__func__ << ": dir: '" << dir << "', file: '" << file << "', ptr: " << ptr << ", size: " << size << ", pos: " << pos);

  scdcint_t ret = max(scdcint_const(0), size);

  SCDC_TRACE(__func__ << ": return: " << ret);

  return ret;
}

#undef SCDC_LOG_PREFIX
