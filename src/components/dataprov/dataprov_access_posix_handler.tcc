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


#ifndef __DATAPROV_ACCESS_POSIX_HANDLER_TCC__
#define __DATAPROV_ACCESS_POSIX_HANDLER_TCC__

#include <vector>
#include <fcntl.h>

#include "z_pack.h"

#if SCDC_TRACE_NOT
# define DATAPROV_ACCESS_POSIX_HANDLER_TCC_SCDC_TRACE_NOT_BACKUP  1
#else
# define DATAPROV_ACCESS_POSIX_HANDLER_TCC_SCDC_TRACE_NOT_BACKUP  0
#endif /* SCDC_TRACE_NOT */
#undef SCDC_TRACE_NOT
#include "log_unset.hh"

#define SCDC_TRACE_NOT  !SCDC_TRACE_DATAPROV_ACCESS_POSIX

#include "config.hh"
#include "common.hh"
#include "log.hh"
#include "dataprov_access_posix_handler.hh"


#define SCDC_LOG_PREFIX  "dataprov-access-posix-handler: "


template<class POSIX_HANDLER>
const char * const scdc_dataprov_access_posix_handler<POSIX_HANDLER>::type = POSIX_HANDLER::type;


template<class POSIX_HANDLER>
struct scdc_dataprov_access_posix_handler<POSIX_HANDLER>::file_data_t
{
  std::string name;
  typename POSIX_HANDLER::file_t file;
};


template<class POSIX_HANDLER>
const typename scdc_dataprov_access_posix_handler<POSIX_HANDLER>::dir_t scdc_dataprov_access_posix_handler<POSIX_HANDLER>::dir_null = 0;
template<class POSIX_HANDLER>
const typename scdc_dataprov_access_posix_handler<POSIX_HANDLER>::file_t scdc_dataprov_access_posix_handler<POSIX_HANDLER>::file_null = 0;


#define dir_valid  (scdc_dataprov_access_posix_handler<POSIX_HANDLER>::dir_t) 1
#define file_valid  (scdc_dataprov_access_posix_handler<POSIX_HANDLER>::file_t) 1


template<class POSIX_HANDLER>
bool scdc_dataprov_access_posix_handler<POSIX_HANDLER>::open_config_conf(const std::string &conf, scdc_args *args, bool &done)
{
  SCDC_TRACE_F("conf: '" << conf << "'");

  bool ret = POSIX_HANDLER::open_config_conf(conf, args, done);

  SCDC_TRACE_F("return: " << ret << ", done: " << done);

  return ret;
}


template<class POSIX_HANDLER>
bool scdc_dataprov_access_posix_handler<POSIX_HANDLER>::open_conf(std::string &conf, scdc_args *args, scdc_result &result)
{
  SCDC_TRACE_F("conf: '" << conf << "'");

  bool ret = POSIX_HANDLER::open_conf(conf, args, result);

  SCDC_TRACE_F("return: " << ret << ", result: '" << result << "'");

  return ret;
}


template<class POSIX_HANDLER>
bool scdc_dataprov_access_posix_handler<POSIX_HANDLER>::open(scdc_result &result)
{
  SCDC_TRACE_F("");

  bool ret = POSIX_HANDLER::open(result);

  SCDC_TRACE_F("return: " << ret << ", result: '" << result << "'");

  return ret;
}


template<class POSIX_HANDLER>
bool scdc_dataprov_access_posix_handler<POSIX_HANDLER>::close(scdc_result &result)
{
  SCDC_TRACE_F("");

  bool ret = POSIX_HANDLER::close(result);

  SCDC_TRACE_F("return: " << ret << ", result: '" << result << "'");

  return ret;
}


template<class POSIX_HANDLER>
typename scdc_dataprov_access_posix_handler<POSIX_HANDLER>::dir_t scdc_dataprov_access_posix_handler<POSIX_HANDLER>::dir_open(const char *path)
{
  SCDC_TRACE_F("path: '" << path << "'");

  dir_t dir = dir_null;

  std::string p;
  make_path(path, "", p);
  SCDC_TRACE_F("p: '" << p << "'");

  if (!POSIX_HANDLER::dir_exists(p.c_str()))
  {
    SCDC_FAIL_F("path '" << path << "' does not exist");
    goto do_return;
  }

  dir = new std::string(path);

do_return:

  SCDC_TRACE_F("return: " << dir);

  return dir;
}


template<class POSIX_HANDLER>
void scdc_dataprov_access_posix_handler<POSIX_HANDLER>::dir_close(dir_t dir)
{
  SCDC_TRACE_F("dir: '" << dir << "'");

  delete dir;

  SCDC_TRACE_F("return:");
}


template<class POSIX_HANDLER>
bool scdc_dataprov_access_posix_handler<POSIX_HANDLER>::dir_path(dir_t dir, std::string &path)
{
  SCDC_TRACE_F("dir: '" << dir << "'");

  bool ret = false;

  if (dir == dir_null) goto do_return;

  path = *dir;
  ret = true;

do_return:
  SCDC_TRACE_F("return: " << ret << ", path: '" << path << "'");

  return ret;
}


template<class POSIX_HANDLER>
bool scdc_dataprov_access_posix_handler<POSIX_HANDLER>::ls_entries(dir_t dir, std::string &result)
{
  SCDC_TRACE_F("dir: '" << dir << "'");

  bool ret = false;

  std::string p;
  dir_make_path(dir, "", p);
  SCDC_TRACE_F("p: '" << p << "'");

  {
    typename POSIX_HANDLER::dir_t pdir = POSIX_HANDLER::dir_open(p.c_str());
    if (pdir == POSIX_HANDLER::dir_null)
    {
      result = "open directory '" + p + "' failed";
      SCDC_FAIL_F(result);
      goto do_return;
    }

    std::vector<std::string> dirs, files;
    POSIX_HANDLER::dir_list_dirs(pdir, dirs);
    pdir = POSIX_HANDLER::dir_rewind(pdir, p.c_str());
    POSIX_HANDLER::dir_list_files(pdir, files);

    POSIX_HANDLER::dir_close(pdir);

    result = zxx::to_string(dirs.size() + files.size()) + "|";

    for (std::vector<std::string>::const_iterator i = dirs.begin(); i != dirs.end(); ++i) result += "d:" + *i + "|";
    for (std::vector<std::string>::const_iterator i = files.begin(); i != files.end(); ++i) result += "f:" + *i + "|";
  }

  ret = true;

do_return:
  SCDC_TRACE_F("return: " << ret << ", result: " << result);

  return ret;
}


template<class POSIX_HANDLER>
bool scdc_dataprov_access_posix_handler<POSIX_HANDLER>::info_entry(dir_t dir, const char *path, std::string &result)
{
  SCDC_TRACE_F("dir: '" << dir << "', path: '" << path << "'");

  bool ret = true;

  std::string p;
  dir_make_path(dir, path, p);
  SCDC_TRACE_F("p: '" << p << "'");

  if (POSIX_HANDLER::dir_exists(p.c_str()))
  {
    typename POSIX_HANDLER::dir_t pdir = POSIX_HANDLER::dir_open(p.c_str());
    if (pdir == POSIX_HANDLER::dir_null)
    {
      result = "open directory '" + p + "' failed";
      SCDC_FAIL_F(result);
      goto do_return;
    }

    result = "entries: " + zxx::to_string(POSIX_HANDLER::dir_count_entries(pdir));

    POSIX_HANDLER::dir_close(pdir);

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
bool scdc_dataprov_access_posix_handler<POSIX_HANDLER>::rm_entry(dir_t dir, const char *path)
{
  SCDC_TRACE_F("dir: '" << dir << "', path: '" << path << "'");

  bool ret = false;

  if (!path || path[0] == '\0') goto do_return;

  {
    std::string p;
    dir_make_path(dir, path, p);
    SCDC_TRACE_F("p: '" << p << "'");

    if (POSIX_HANDLER::dir_exists(p.c_str()))
    {
      if (!POSIX_HANDLER::dir_rm(p.c_str()))
      {
        SCDC_FAIL_F("removing directory '" << p << "' failed");
        goto do_return;
      }

    } else
    {
      if (!POSIX_HANDLER::file_rm(p.c_str()))
      {
        SCDC_FAIL_F("removing file '" << p << "' failed");
        goto do_return;
      }
    }
  }

  ret = true;

do_return:
  SCDC_TRACE_F("return: " << ret);

  return ret;
}


template<class POSIX_HANDLER>
bool scdc_dataprov_access_posix_handler<POSIX_HANDLER>::mk_dir(dir_t dir, const char *path)
{
  SCDC_TRACE_F("dir: '" << dir << "', path: '" << path << "'");

  bool ret = false;

  if (!path || path[0] == '\0') goto do_return;

  {
    std::string p;
    dir_make_path(dir, path, p);
    SCDC_TRACE_F("p: '" << p << "'");

    if (!POSIX_HANDLER::dir_mk(p.c_str()))
    {
      SCDC_FAIL_F("creating directory '" << p << "' failed");
      goto do_return;
    }
  }

  ret = true;

do_return:
  SCDC_TRACE_F("return: " << ret);

  return ret;
}


template<class POSIX_HANDLER>
typename scdc_dataprov_access_posix_handler<POSIX_HANDLER>::file_t scdc_dataprov_access_posix_handler<POSIX_HANDLER>::file_open(dir_t dir, const char *path, bool read, bool write, bool create)
{
  SCDC_TRACE_F("path: '" << path << "', read: " << read << ", write: " << write << ", create: " << create);

  file_t file = file_null;

  if (!path || path[0] == '\0') goto do_return;

  {
    std::string p;
    dir_make_path(dir, path, p);
    SCDC_TRACE_F("p: '" << p << "'");

    int flags = 0;
    if (read && write) flags = O_RDWR;
    else if (read) flags = O_RDONLY;
    else if (write) flags = O_WRONLY;
    if (create) flags |= O_CREAT;

    typename POSIX_HANDLER::file_t pfile = POSIX_HANDLER::file_open(p.c_str(), flags);
    if (pfile == POSIX_HANDLER::file_null)
    {
      SCDC_FAIL_F("opening file '" << path << "' failed");
      goto do_return;
    }

    file = new file_data_t;
    file->name = path;
    file->file = pfile;
  }

do_return:
  SCDC_TRACE_F("return: " << file);

  return file;
}


template<class POSIX_HANDLER>
typename scdc_dataprov_access_posix_handler<POSIX_HANDLER>::file_t scdc_dataprov_access_posix_handler<POSIX_HANDLER>::file_reopen(dir_t dir, const char *path, bool read, bool write, bool create, file_t file)
{
  SCDC_TRACE_F("path: '" << path << "', read: " << read << ", write: " << write << ", create: " << create << ", file: " << file);

  file_t ret_file = file_null;

  if (!path || path[0] == '\0')
  {
    file_close(dir, file);
    goto do_return;
  }

  {
    std::string p;
    dir_make_path(dir, path, p);
    SCDC_TRACE_F("p: '" << p << "'");

    int flags = 0;
    if (read && write) flags = O_RDWR;
    else if (read) flags = O_RDONLY;
    else if (write) flags = O_WRONLY;
    if (create) flags |= O_CREAT;

    typename POSIX_HANDLER::file_t pfile = POSIX_HANDLER::file_reopen(p.c_str(), flags, file->file, file->name.c_str());
    if (pfile == POSIX_HANDLER::file_null)
    {
      SCDC_FAIL_F("reopening file '" << path << "' failed");
      goto do_return;
    }

    file->name = path;
    file->file = pfile;

    ret_file = file;
  }

do_return:
  SCDC_TRACE_F("return: " << ret_file);

  return ret_file;
}


template<class POSIX_HANDLER>
void scdc_dataprov_access_posix_handler<POSIX_HANDLER>::file_close(dir_t dir, scdc_dataprov_access_posix_handler::file_t file)
{
  SCDC_TRACE_F("dir: '" << dir << "', file: '" << file << "'");

  POSIX_HANDLER::file_close(file->file);
  delete file;

  SCDC_TRACE_F("return:");
}


template<class POSIX_HANDLER>
bool scdc_dataprov_access_posix_handler<POSIX_HANDLER>::file_match(dir_t dir, file_t file, const char *path)
{
  SCDC_TRACE_F("path: '" << path << "', file: '" << file << "', path: '" << path << "'");

  bool ret = (file->name == path);

  SCDC_TRACE_F("return: " << ret);

  return ret;
}


template<class POSIX_HANDLER>
bool scdc_dataprov_access_posix_handler<POSIX_HANDLER>::file_read_access_at(dir_t dir, file_t file, scdcint_t size, scdcint_t pos, scdc_buf_t &buf)
{
  SCDC_TRACE_F("dir: '" << dir << "', file: '" << file << "', size: " << size << ", pos: " << pos);

  bool ret = false;

  SCDC_TRACE_F("return: " << ret);

  return ret;
}


template<class POSIX_HANDLER>
scdcint_t scdc_dataprov_access_posix_handler<POSIX_HANDLER>::file_read_at(dir_t dir, file_t file, void *ptr, scdcint_t size, scdcint_t pos)
{
  SCDC_TRACE_F("dir: '" << dir << "', file: '" << file << "', ptr: " << ptr << ", size: " << size << ", pos: " << pos);

  scdcint_t n = -1;

  if (pos >= 0 && POSIX_HANDLER::file_seek(file->file, pos, SEEK_SET) < 0)
  {
    SCDC_FAIL_F("seeking to position " << pos << " failed");
    goto do_return;
  }

  n = POSIX_HANDLER::file_read(file->file, ptr, size);
  if (n < 0)
  {
    SCDC_FAIL_F("reading from file failed");
    goto do_return;
  }

do_return:
  SCDC_TRACE_F("return: " << n);

  return n;
}


template<class POSIX_HANDLER>
scdcint_t scdc_dataprov_access_posix_handler<POSIX_HANDLER>::file_write_at(dir_t dir, file_t file, const void *ptr, scdcint_t size, scdcint_t pos)
{
  SCDC_TRACE_F("dir: '" << dir << "', file: '" << file << "', ptr: " << ptr << ", size: " << size << ", pos: " << pos);

  scdcint_t n = -1;

  if (pos >= 0 && POSIX_HANDLER::file_seek(file->file, pos, SEEK_SET) < 0)
  {
    SCDC_FAIL_F("seeking to position " << pos << " failed");
    goto do_return;
  }

  n = POSIX_HANDLER::file_write(file->file, ptr, size);
  if (n < 0)
  {
    SCDC_FAIL_F("writing to file failed");
    goto do_return;
  }

do_return:
  SCDC_TRACE_F("return: " << n);

  return n;
}


template<class POSIX_HANDLER>
bool scdc_dataprov_access_posix_handler<POSIX_HANDLER>::make_path(const std::string &dir_path, const std::string &entry_path, std::string &path)
{
  path.clear();

  if (!dir_path.empty()) path += dir_path + "/";

  if (!entry_path.empty()) path += entry_path;

  return true;
}


template<class POSIX_HANDLER>
bool scdc_dataprov_access_posix_handler<POSIX_HANDLER>::dir_make_path(const dir_t dir, const std::string &entry_path, std::string &path)
{
  return make_path((dir != dir_null)?*dir:"", entry_path, path);
}

#undef SCDC_LOG_PREFIX


#undef SCDC_TRACE_NOT
#if DATAPROV_ACCESS_POSIX_HANDLER_TCC_SCDC_TRACE_NOT_BACKUP
# define SCDC_TRACE_NOT  1
#else /* DATAPROV_ACCESS_POSIX_HANDLER_TCC_SCDC_TRACE_NOT_BACKUP */
# define SCDC_TRACE_NOT  0
#endif /* DATAPROV_ACCESS_POSIX_HANDLER_TCC_SCDC_TRACE_NOT_BACKUP */
#undef DATAPROV_ACCESS_POSIX_HANDLER_TCC_SCDC_TRACE_NOT_BACKUP
#include "log_unset.hh"
#include "log_set.hh"


#endif /* __DATAPROV_ACCESS_POSIX_HANDLER_TCC__ */
