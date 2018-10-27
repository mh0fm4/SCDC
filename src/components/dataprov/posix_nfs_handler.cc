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


#define SCDC_TRACE_NOT  !SCDC_TRACE_POSIX_NFS

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "config.hh"
#include "common.hh"
#include "log.hh"
#include "args.hh"
#include "result.hh"

#include "posix_nfs_handler.hh"


using namespace std;


#define SCDC_LOG_PREFIX  "posix-nfs-handler: "


const char * const scdc_posix_nfs_handler::type = "nfs";

const scdc_posix_nfs_handler::dir_t scdc_posix_nfs_handler::dir_null = 0;
const scdc_posix_nfs_handler::file_t scdc_posix_nfs_handler::file_null = 0;


#define SCDC_LOG_PREFIX  "posix-nfs-handler: "


bool scdc_posix_nfs_handler::open_config_conf(const std::string &conf, scdc_args *args, bool &done)
{
  SCDC_TRACE_F("conf: '" << conf << "'");

  bool ret = false;
  done = false;

  SCDC_TRACE_F("return: " << ret << ", done: " << done);

  return ret;
}


bool scdc_posix_nfs_handler::open_conf(std::string &conf, scdc_args *args, scdc_result &result)
{
  SCDC_TRACE_F("conf: '" << conf << "'");

  bool ret = false;

  const char *r;
  if (!args->get<const char *>(SCDC_ARGS_TYPE_CSTR, &r))
  {
    result = "getting NFS export URL failed";
    SCDC_ERROR_F(result);
    goto do_return;
  }

  export_url = r;
  if (*export_url.rbegin() != '/') export_url += '/';

  ret = true;

do_return:
  SCDC_TRACE_F("return: " << ret << ", result: '" << result << "'");

  return ret;
}


bool scdc_posix_nfs_handler::open(scdc_result &result)
{
  SCDC_TRACE_F("");

  bool ret = false;
  struct nfs_url *parsed_url = 0;
  string export_path;

  nfs_context = nfs_init_context();
  if (!nfs_context)
  {
    result = "creating NFS context failed";
    SCDC_ERROR_F(result);
    goto do_return;
  }

  parsed_url = nfs_parse_url_full(nfs_context, export_url.c_str());
  if (!parsed_url)
  {
    result = "parsing URL '" + export_url + "' failed: " + nfs_get_error(nfs_context);
    SCDC_ERROR_F(result);
    goto do_return;
  }

  SCDC_TRACE_F("parsed URL: server: '" << parsed_url->server << "', path: '" << parsed_url->path << "', file: '" << parsed_url->file << "'");

  export_path = parsed_url->path;
  export_path += parsed_url->file;

  if (nfs_mount(nfs_context, parsed_url->server, export_path.c_str()) != 0)
  {
    result = "mouting NFS export '" + export_path + "' failed: " + nfs_get_error(nfs_context);
    SCDC_ERROR_F(result);
    goto do_return;
  }

  ret = true;

do_return:
  if (parsed_url) nfs_destroy_url(parsed_url);

  if (!ret)
  {
    nfs_destroy_context(nfs_context);
    nfs_context = 0;
  }

  SCDC_TRACE_F("return: " << ret << ", result: '" << result << "'");

  return ret;
}


bool scdc_posix_nfs_handler::close(scdc_result &result)
{
  SCDC_TRACE_F("");

  bool ret = true;

  nfs_destroy_context(nfs_context);
  nfs_context = 0;

  SCDC_TRACE_F("return: " << ret << ", result: '" << result << "'");

  return ret;
}


bool scdc_posix_nfs_handler::dir_exists(const char *path)
{
  string p = path;
  if (*p.crbegin() == '/') p.resize(p.size() - 1);
  SCDC_TRACE_F("path: '" << path << "' -> '" << p << "'");

  last_result.clear();

  struct nfs_stat_64 st;
  bool ret = (nfs_stat64(nfs_context, p.c_str(), &st) == 0);
  if (!ret)
  {
    last_result = nfs_get_error(nfs_context);
    SCDC_ERROR_F(last_result);
    goto do_return;
  }

  ret = S_ISDIR(st.nfs_mode);

do_return:
  SCDC_TRACE_F("return: " << ret << ",  last_result: '" << last_result << "'");

  return ret;
}


bool scdc_posix_nfs_handler::dir_mk(const char *path)
{
  string p = path;
  if (*p.crbegin() == '/') p.resize(p.size() - 1);
  SCDC_TRACE_F("path: '" << path << "' -> '" << p << "'");

  last_result.clear();

  bool ret = (nfs_mkdir(nfs_context, p.c_str()) == 0);
  if (!ret)
  {
    last_result = nfs_get_error(nfs_context);
    SCDC_ERROR_F(last_result);
  }

  SCDC_TRACE_F("return: " << ret << ",  last_result: '" << last_result << "'");

  return ret;
}


bool scdc_posix_nfs_handler::dir_rm(const char *path)
{
  string p = path;
  if (*p.crbegin() == '/') p.resize(p.size() - 1);
  SCDC_TRACE_F("path: '" << path << "' -> '" << p << "'");

  last_result.clear();

  bool ret = dir_rm_r(p.c_str());
  if (!ret)
  {
    SCDC_ERROR_F(last_result);
  }

  SCDC_TRACE_F("return: " << ret << ",  last_result: '" << last_result << "'");

  return ret;
}


scdc_posix_nfs_handler::dir_t scdc_posix_nfs_handler::dir_open(const char *path)
{
  string p = path;
  if (*p.crbegin() == '/') p.resize(p.size() - 1);
  SCDC_TRACE_F("path: '" << path << "' -> '" << p << "'");

  last_result.clear();

  struct nfsdir *dir = dir_null;
  if (nfs_opendir(nfs_context, path, &dir) != 0)
  if (!dir)
  {
    last_result = nfs_get_error(nfs_context);
    SCDC_ERROR_F(last_result);
  }

  SCDC_TRACE_F("return: " << dir << ",  last_result: '" << last_result << "'");

  return dir;
}


bool scdc_posix_nfs_handler::dir_close(dir_t dir)
{
  SCDC_TRACE_F("dir: " << dir);

  last_result.clear();

#if 0
  if (dir == dir_null) return true;
#endif

  bool ret = true;
  nfs_closedir(nfs_context, dir);

  SCDC_TRACE_F("return: " << ret);

  return ret;
}


scdc_posix_nfs_handler::dir_t scdc_posix_nfs_handler::dir_rewind(dir_t dir, const char *path)
{
  SCDC_TRACE_F("dir: " << dir << ", path: '" << path << "'");

  last_result.clear();

  nfs_rewinddir(nfs_context, dir);

  SCDC_TRACE_F("return: " << dir);

  return dir;
}


static bool dirent_is_dir(struct nfsdirent *ent)
{
  return (ent->type == NF3DIR && strcmp(ent->name, ".") != 0 && strcmp(ent->name, "..") != 0);
}


static bool dirent_is_file(struct nfsdirent *ent)
{
  return (ent->type == NF3REG);
}


scdcint_t scdc_posix_nfs_handler::dir_count_entries(dir_t dir)
{
  SCDC_TRACE_F("dir: " << dir);

  last_result.clear();

  scdcint_t n = 0;
  struct nfsdirent *ent;
  while ((ent = nfs_readdir(nfs_context, dir)) != NULL)
  {
    if (dirent_is_dir(ent) || dirent_is_file(ent)) ++n;
  }

  SCDC_TRACE_F("return: " << n);

  return n;
}


scdcint_t scdc_posix_nfs_handler::dir_count_dirs(dir_t dir)
{
  SCDC_TRACE_F("dir: " << dir);

  last_result.clear();

  scdcint_t n = 0;
  struct nfsdirent *ent;
  while ((ent = nfs_readdir(nfs_context, dir)) != NULL)
  {
    if (dirent_is_dir(ent)) ++n;
  }

  SCDC_TRACE_F("return: " << n);

  return n;
}


bool scdc_posix_nfs_handler::dir_list_dirs(dir_t dir, std::vector<std::string> &dirs)
{
  SCDC_TRACE_F("dir: " << dir);

  last_result.clear();
  bool ret = false;

  struct nfsdirent *ent;
  while ((ent = nfs_readdir(nfs_context, dir)) != NULL)
  {
    if (dirent_is_dir(ent)) dirs.push_back(ent->name);
  }

  ret = true;

  SCDC_TRACE_F("return: " << ret);

  return ret;
}


scdcint_t scdc_posix_nfs_handler::dir_count_files(dir_t dir)
{
  SCDC_TRACE_F("dir: " << dir);

  last_result.clear();

  scdcint_t n = 0;
  struct nfsdirent *ent;
  while ((ent = nfs_readdir(nfs_context, dir)) != NULL)
  {
    if (dirent_is_file(ent)) ++n;
  }

  SCDC_TRACE_F("return: " << n);

  return n;
}


bool scdc_posix_nfs_handler::dir_list_files(dir_t dir, std::vector<std::string> &files)
{
  SCDC_TRACE_F("dir: " << dir);

  last_result.clear();
  bool ret = false;

  struct nfsdirent *ent;
  while ((ent = nfs_readdir(nfs_context, dir)) != NULL)
  {
    if (dirent_is_file(ent)) files.push_back(ent->name);
  }

  ret = true;

  SCDC_TRACE_F("return: " << ret);

  return ret;
}


scdcint_t scdc_posix_nfs_handler::file_get_size(const char *path)
{
  SCDC_TRACE_F("path: '" << path << "'");

  last_result.clear();
  scdcint_t size = -1;

  struct nfs_stat_64 st;
  if (nfs_stat64(nfs_context, path, &st) != 0)
  {
    last_result = nfs_get_error(nfs_context);
    SCDC_ERROR_F(last_result);
    goto do_return;
  }

  size = st.nfs_size;

do_return:
  SCDC_TRACE_F("return: " << size << ",  last_result: '" << last_result << "'");

  return size;
}


bool scdc_posix_nfs_handler::file_rm(const char *path)
{
  SCDC_TRACE_F("path: '" << path << "'");

  last_result.clear();

  bool ret = (nfs_unlink(nfs_context, path) == 0);
  if (!ret)
  {
    last_result = nfs_get_error(nfs_context);
    SCDC_ERROR_F(last_result);
  }

  SCDC_TRACE_F("return: " << ret << ",  last_result: '" << last_result << "'");

  return ret;
}


scdc_posix_nfs_handler::file_t scdc_posix_nfs_handler::file_open(const char *path, int flags)
{
  SCDC_TRACE_F("path: '" << path << ", flags: " << flags);

  /* file mode: user/group/others can read/write */
  const mode_t mode = S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH;

  last_result.clear();

  struct nfsfh *file = file_null;
  int r = 0;

  if (flags & O_CREAT) r = nfs_create(nfs_context, path, flags, mode, &file);  /* if file exists, create performs open */
  else r = nfs_open(nfs_context, path, flags, &file);
  if (r < 0)
  {
    last_result = nfs_get_error(nfs_context);
    SCDC_ERROR_F(last_result);
  }

  SCDC_TRACE_F("return: " << file << ", last_result: '" << last_result << "'");

  return file;
}


scdc_posix_nfs_handler::file_t scdc_posix_nfs_handler::file_reopen(const char *path, int flags, file_t file, const char *old_path)
{
  SCDC_TRACE_F("path: '" << path << "', flags: " << flags << ", old_path: '" << old_path << "'");

  last_result.clear();

  bool ret = (nfs_close(nfs_context, file) == 0);
  if (!ret)
  {
    file = file_null;
    last_result = nfs_get_error(nfs_context);
    SCDC_ERROR_F(last_result);
    goto do_return;
  }

  if (nfs_open(nfs_context, path, flags, &file) < 0)
  {
    last_result = nfs_get_error(nfs_context);
    SCDC_ERROR_F(last_result);
  }

do_return:
  SCDC_TRACE_F("return: " << file << ",  last_result: '" << last_result << "'");

  return file;
}


bool scdc_posix_nfs_handler::file_close(file_t file)
{
  SCDC_TRACE_F("file: " << file);

  last_result.clear();

#if 0
  if (file == file_null) return true;
#endif

  bool ret = (nfs_close(nfs_context, file) == 0);
  if (!ret)
  {
    last_result = nfs_get_error(nfs_context);
    SCDC_ERROR_F(last_result);
    goto do_return;
  }

do_return:
  SCDC_TRACE_F("return: " << file << ", last_result: '" << last_result << "'");

  return ret;
}


scdcint_t scdc_posix_nfs_handler::file_seek(file_t file, scdcint_t offset, int whence)
{
  SCDC_TRACE_F("file: " << file << ", offset: " << offset << ", whence: " << whence);

  last_result.clear();

  uint64_t pos = nfs_lseek(nfs_context, file, offset, whence, &pos); 
  if (pos < 0)
  {
    last_result = nfs_get_error(nfs_context);
    SCDC_ERROR_F(last_result);
  }

  SCDC_TRACE_F("return: " << pos << ", last_result: '" << last_result << "'");

  return pos;
}


scdcint_t scdc_posix_nfs_handler::file_read(file_t file, void *buf, scdcint_t count)
{
  SCDC_TRACE_F("file: " << file << ", buf: " << buf << ", count: " << count);

  last_result.clear();

  scdcint_t n = nfs_read(nfs_context, file, count, buf);
  if (n < 0)
  {
    last_result = nfs_get_error(nfs_context);
    SCDC_ERROR_F(last_result);
  }

  SCDC_TRACE_F("return: " << n << ", last_result: '" << last_result << "'");

  return n;
}


scdcint_t scdc_posix_nfs_handler::file_write(file_t file, const void *buf, scdcint_t count)
{
  SCDC_TRACE_F("file: " << file << ", buf: " << buf << ", count: " << count);

  last_result.clear();

  scdcint_t n = nfs_write(nfs_context, file, count, buf);
  if (n < 0)
  {
    last_result = nfs_get_error(nfs_context);
    SCDC_ERROR_F(last_result);
  }

  SCDC_TRACE_F("return: " << n << ", last_result: '" << last_result << "'");

  return n;
}


bool scdc_posix_nfs_handler::dir_rm_r(const char *path)
{
  string p = path;
  if (*p.crbegin() == '/') p.resize(p.size() - 1);
  SCDC_TRACE_F("path: '" << path << "' -> '" << p << "'");

  bool ret = true;

  struct nfsdir *dir = 0;
  if (nfs_opendir(nfs_context, p.c_str(), &dir) != 0) ret = false;

  struct nfsdirent *ent;
  while (ret && (ent = nfs_readdir(nfs_context, dir)))
  {
    string e = p + "/" + ent->name;

    if (dirent_is_dir(ent))
    {
      if (!dir_rm_r(e.c_str())) ret = false;
    }

    if (dirent_is_file(ent))
    {
      if (nfs_unlink(nfs_context, e.c_str()) != 0)
      {
        ret = false;
        last_result = nfs_get_error(nfs_context);
      }
    }
  }

  if (dir) nfs_closedir(nfs_context, dir);

  if (!ret) goto do_return;

  if (nfs_rmdir(nfs_context, p.c_str()) != 0)
  {
    ret = false;
    last_result = nfs_get_error(nfs_context);
  }

do_return:
  SCDC_TRACE_F("return: " << ret << ", last_result: '" << last_result << "'");

  return ret;
}


#undef SCDC_LOG_PREFIX
