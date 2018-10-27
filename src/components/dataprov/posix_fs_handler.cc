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


#define SCDC_TRACE_NOT  !SCDC_TRACE_POSIX_FS

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>

#include "z_pack.h"

#include "config.hh"
#include "common.hh"
#include "log.hh"
#include "args.hh"
#include "result.hh"

#include "posix_fs_handler.hh"


#define SCDC_LOG_PREFIX  "posix-fs-handler: "


const char * const scdc_posix_fs_handler::type = "fs";

const scdc_posix_fs_handler::dir_t scdc_posix_fs_handler::dir_null = 0;
const scdc_posix_fs_handler::file_t scdc_posix_fs_handler::file_null = -1;


bool scdc_posix_fs_handler::open_config_conf(const std::string &conf, scdc_args *args, bool &done)
{
  SCDC_TRACE_F("conf: '" << conf << "'");

  bool ret = false;
  done = false;

  SCDC_TRACE_F("return: " << ret << ", done: " << done);

  return ret;
}


bool scdc_posix_fs_handler::open_conf(std::string &conf, scdc_args *args, scdc_result &result)
{
  SCDC_TRACE_F("conf: '" << conf << "'");

  bool ret = false;

  const char *r;
  if (!args->get<const char *>(SCDC_ARGS_TYPE_CSTR, &r))
  {
    result = "getting root directory failed";
    SCDC_ERROR_F(result);
    goto do_return;
  }

  root_dir = r;
  if (*root_dir.rbegin() != '/') root_dir += '/';

  ret = true;

do_return:
  SCDC_TRACE_F("return: " << ret << ", result: '" << result << "'");

  return ret;
}


bool scdc_posix_fs_handler::open(scdc_result &result)
{
  SCDC_TRACE_F("");

  bool ret = false;

  if (!dir_exists(""))
  {
    result = "root directory '" + root_dir + "' does not exist or is not a directory";
    SCDC_ERROR_F(result);
    goto do_return;
  }

  ret = true;

do_return:
  SCDC_TRACE_F("return: " << ret << ", result: '" << result << "'");

  return ret;
}


bool scdc_posix_fs_handler::close(scdc_result &result)
{
  SCDC_TRACE_F("");

  bool ret = true;

  SCDC_TRACE_F("return: " << ret << ", result: '" << result << "'");

  return ret;
}


bool scdc_posix_fs_handler::dir_exists(const char *path)
{
  std::string p = root_dir + path;
  SCDC_TRACE_F("path: '" << path << "' -> '" << p << "'");

  return z_fs_is_directory(p.c_str());
}


bool scdc_posix_fs_handler::dir_mk(const char *path)
{
  std::string p = root_dir + path;
  SCDC_TRACE_F("path: '" << path << "' -> '" << p << "'");

  return z_fs_mkdir(p.c_str());
}


bool scdc_posix_fs_handler::dir_rm(const char *path)
{
  std::string p = root_dir + path;
  SCDC_TRACE_F("path: '" << path << "' -> '" << p << "'");

  return z_fs_rm_r(p.c_str());
}


scdc_posix_fs_handler::dir_t scdc_posix_fs_handler::dir_open(const char *path)
{
  std::string p = root_dir + path;
  SCDC_TRACE_F("path: '" << path << "' -> '" << p << "'");

  return ::opendir(p.c_str());
}


bool scdc_posix_fs_handler::dir_close(dir_t dir)
{
#if 0
  if (dir == dir_null) return true;
#endif

  return (::closedir(dir) == 0);
}


scdc_posix_fs_handler::dir_t scdc_posix_fs_handler::dir_rewind(dir_t dir, const char *path)
{
  std::string p = root_dir + path;
  SCDC_TRACE_F("dir: " << dir << ", path: '" << path << "' -> '" << p << "'");

  ::rewinddir(dir);
  return dir;
}


static bool dirent_is_dir(struct dirent *ent)
{
  return (ent->d_type == DT_DIR && strcmp(ent->d_name, ".") != 0 && strcmp(ent->d_name, "..") != 0);
}


static bool dirent_is_file(struct dirent *ent)
{
  return (ent->d_type == DT_REG);
}


scdcint_t scdc_posix_fs_handler::dir_count_entries(dir_t dir)
{
  SCDC_TRACE_F("dir: " << dir);

  scdcint_t n = 0;
  struct dirent *ent;
  while ((ent = ::readdir(dir)) != NULL)
  {
    if (dirent_is_dir(ent) || dirent_is_file(ent)) ++n;
  }

  return n;
}


scdcint_t scdc_posix_fs_handler::dir_count_dirs(dir_t dir)
{
  SCDC_TRACE_F("dir: " << dir);

  scdcint_t n = 0;
  struct dirent *ent;
  while ((ent = ::readdir(dir)) != NULL)
  {
    if (dirent_is_dir(ent)) ++n;
  }

  return n;
}


bool scdc_posix_fs_handler::dir_list_dirs(dir_t dir, std::vector<std::string> &dirs)
{
  SCDC_TRACE_F("dir: " << dir);

  struct dirent *ent;
  while ((ent = ::readdir(dir)) != NULL)
  {
    if (dirent_is_dir(ent)) dirs.push_back(ent->d_name);
  }

  return true;
}


scdcint_t scdc_posix_fs_handler::dir_count_files(dir_t dir)
{
  SCDC_TRACE_F("dir: " << dir);

  scdcint_t n = 0;
  struct dirent *ent;
  while ((ent = ::readdir(dir)) != NULL)
  {
    if (dirent_is_file(ent)) ++n;
  }

  return n;
}


bool scdc_posix_fs_handler::dir_list_files(dir_t dir, std::vector<std::string> &files)
{
  SCDC_TRACE_F("dir: " << dir);

  struct dirent *ent;
  while ((ent = ::readdir(dir)) != NULL)
  {
    if (dirent_is_file(ent)) files.push_back(ent->d_name);
  }

  return true;
}


scdcint_t scdc_posix_fs_handler::file_get_size(const char *path)
{
  std::string p = root_dir + path;
  SCDC_TRACE_F("path: '" << path << "' -> '" << p << "'");

  return z_fs_get_file_size(p.c_str());
}


bool scdc_posix_fs_handler::file_rm(const char *path)
{
  std::string p = root_dir + path;
  SCDC_TRACE_F("path: '" << path << "' -> '" << p << "'");

  return z_fs_rm(p.c_str());
}


scdc_posix_fs_handler::file_t scdc_posix_fs_handler::file_open(const char *path, int flags)
{
  std::string p = root_dir + path;
  SCDC_TRACE_F("path: '" << path << "' -> '" << p << "', flags: " << flags);

  /* file mode: user/group/others can read/write */
  const mode_t mode = S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH;

  return ::open(p.c_str(), flags, mode);
}


scdc_posix_fs_handler::file_t scdc_posix_fs_handler::file_reopen(const char *path, int flags, file_t file, const char *old_path)
{
  std::string p = root_dir + path;
  SCDC_TRACE_F("path: '" << path << "' -> '" << p << "', flags: " << flags << ", old_path: '" << old_path << "'");

  /* FIXME: check whether path == old_path and flags == old_flags (determine with fnctl and F_GETFL) to prevent unnecessary close and open */
  if (::close(file) != 0) return file_null;

  return ::open(p.c_str(), flags);
}


bool scdc_posix_fs_handler::file_close(file_t file)
{
  SCDC_TRACE_F("file: " << file);

#if 0
  if (file == file_null) return true;
#endif

  return (::close(file) == 0);
}


scdcint_t scdc_posix_fs_handler::file_seek(file_t file, scdcint_t offset, int whence)
{
  SCDC_TRACE_F("file: " << file << ", offset: " << offset << ", whence: " << whence);

  return ::lseek(file, offset, whence);
}


scdcint_t scdc_posix_fs_handler::file_read(file_t file, void *buf, scdcint_t count)
{
  SCDC_TRACE_F("file: " << file << ", buf: " << buf << ", count: " << count);

  return ::read(file, buf, count);
}


scdcint_t scdc_posix_fs_handler::file_write(file_t file, const void *buf, scdcint_t count)
{
  SCDC_TRACE_F("file: " << file << ", buf: " << buf << ", count: " << count);

  return ::write(file, buf, count);
}


#undef SCDC_LOG_PREFIX
