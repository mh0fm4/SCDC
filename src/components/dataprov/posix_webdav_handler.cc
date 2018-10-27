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


#define SCDC_TRACE_NOT  !SCDC_TRACE_POSIX_WEBDAV

#include <sys/stat.h>

#include "config.hh"
#include "common.hh"
#include "log.hh"
#include "args.hh"
#include "result.hh"

#include "posix_webdav_handler.hh"


using namespace std;
using namespace Davix;


#define SCDC_LOG_PREFIX  "posix-webdav-handler: "


const char * const scdc_posix_webdav_handler::type = "fs";

const scdc_posix_webdav_handler::dir_t scdc_posix_webdav_handler::dir_null = 0;
const scdc_posix_webdav_handler::file_t scdc_posix_webdav_handler::file_null = 0;


/* default file/directory mode: user/group/others can read/write */
#define MODE_DEFAULT  S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH


bool scdc_posix_webdav_handler::open_config_conf(const string &conf, scdc_args *args, bool &done)
{
  SCDC_TRACE_F("conf: '" << conf << "'");

  bool ret = false;
  done = false;

  if (conf == "username")
  {
    done = true;
    const char *u;
    if (!args->get<const char *>(SCDC_ARGS_TYPE_CSTR, &u))
    {
      SCDC_ERROR_F("getting username failed");
      goto do_return;
    }
    ret = true;
    username = u;
    SCDC_TRACE_F("username: '" << username << "'");

  } else if (conf == "password")
  {
    done = true;
    const char *p;
    if (!args->get<const char *>(SCDC_ARGS_TYPE_CSTR, &p))
    {
      SCDC_ERROR_F("getting username failed");
      goto do_return;
    }
    ret = true;
    password = p;
    SCDC_TRACE_F("password: '" << password << "'");

  } else if (conf == "no100continue")
  {
    done = true;
    no100continue = true;
    ret = true;

  } else if (conf == "100continue")
  {
    done = true;
    no100continue = false;
    ret = true;
  }

do_return:
  SCDC_TRACE_F("return: " << ret << ", done: " << done);

  return ret;
}


bool scdc_posix_webdav_handler::open_conf(string &conf, scdc_args *args, scdc_result &result)
{
  SCDC_TRACE_F("conf: '" << conf << "'");

  bool ret = false;

  const char *r;
  if (!args->get<const char *>(SCDC_ARGS_TYPE_CSTR, &r))
  {
    result = "getting root URL directory failed";
    SCDC_ERROR_F(result);
    goto do_return;
  }

  root_url = r;
  if (*root_url.rbegin() != '/') root_url += '/';

  SCDC_TRACE_F("root_url: '" << root_url << "'");

  ret = true;

do_return:
  SCDC_TRACE_F("return: " << ret << ", result: '" << result << "'");

  return ret;
}


bool scdc_posix_webdav_handler::open(scdc_result &result)
{
  SCDC_TRACE_F("");

  bool ret = false;

  davix_request_params.setClientLoginPassword(username, password);
  davix_request_params.setProtocol(RequestProtocol::Webdav);
  davix_request_params.set100ContinueSupport(!no100continue);

  if (!dir_exists(""))
  {
    result = "root URL '" + root_url + "' does not exist or is not a directory";
    SCDC_ERROR_F(result);
    goto do_return;
  }

  ret = true;

do_return:
  SCDC_TRACE_F("return: " << ret << ", result: '" << result << "'");

  return ret;
}


bool scdc_posix_webdav_handler::close(scdc_result &result)
{
  SCDC_TRACE_F("");

  bool ret = true;

  SCDC_TRACE_F("return: " << ret << ", result: '" << result << "'");

  return ret;
}


bool scdc_posix_webdav_handler::dir_exists(const char *path)
{
  string url = root_url + path;
  SCDC_TRACE_F("path: '" << path << "' -> 'url: " << url << "'");

  last_result.clear();
  DavixError *daverr = NULL;

  StatInfo si;
  bool ret = (davix_posix.stat64(&davix_request_params, url, &si, &daverr) == 0);
  if (!ret)
  {
    set_last_result(daverr);
    SCDC_ERROR_F(last_result);
    goto do_return;
  }

  ret = S_ISDIR(si.mode);

do_return:

  SCDC_TRACE_F("return: " << ret << ",  last_result: '" << last_result << "'");

  return ret;
}


bool scdc_posix_webdav_handler::dir_mk(const char *path)
{
  string url = root_url + path;
  SCDC_TRACE_F("path: '" << path << "' -> '" << url << "'");

  last_result.clear();
  DavixError *daverr = NULL;

  bool ret = (davix_posix.mkdir(&davix_request_params, url, MODE_DEFAULT, &daverr) == 0);
  if (!ret)
  {
    set_last_result(daverr);
    SCDC_ERROR_F(last_result);
  }

  SCDC_TRACE_F("return: " << ret << ",  last_result: '" << last_result << "'");

  return ret;
}


bool scdc_posix_webdav_handler::dir_rm(const char *path)
{
  string url = root_url + path;
  SCDC_TRACE_F("path: '" << path << "' -> '" << url << "'");

  last_result.clear();
  DavixError *daverr = NULL;

  bool ret = (davix_posix.rmdir(&davix_request_params, url, &daverr) == 0);
  if (!ret)
  {
    set_last_result(daverr);
    SCDC_ERROR_F(last_result);
  }

  SCDC_TRACE_F("return: " << ret << ",  last_result: '" << last_result << "'");

  return ret;
}


scdc_posix_webdav_handler::dir_t scdc_posix_webdav_handler::dir_open(const char *path)
{
  string url = root_url + path;
  SCDC_TRACE_F("path: '" << path << "' -> 'url: " << url << "'");

  last_result.clear();
  DavixError *daverr = NULL;

  DAVIX_DIR *dir = davix_posix.opendir(&davix_request_params, url.c_str(), &daverr);
  if (!dir)
  {
    set_last_result(daverr);
    SCDC_ERROR_F(last_result);
  }

  SCDC_TRACE_F("return: " << dir << ",  last_result: '" << last_result << "'");

  return dir;
}


bool scdc_posix_webdav_handler::dir_close(dir_t dir)
{
#if 0
  if (dir == dir_null) return true;
#endif

  last_result.clear();
  DavixError *daverr = NULL;

  bool ret = (davix_posix.closedir(dir, &daverr) == 0);
  if (!ret)
  {
    set_last_result(daverr);
    SCDC_ERROR_F(last_result);
  }

  SCDC_TRACE_F("return: " << dir << ",  last_result: '" << last_result << "'");

  return ret;
}


scdc_posix_webdav_handler::dir_t scdc_posix_webdav_handler::dir_rewind(dir_t dir, const char *path)
{
  string url = root_url + path;
  SCDC_TRACE_F("dir: " << dir << ", path: '" << path << "' -> 'url: " << url << "'");

  last_result.clear();
  DavixError *daverr = NULL;

  if (davix_posix.closedir(dir, &daverr) != 0)
  {
    dir = dir_null;
    set_last_result(daverr);
    SCDC_ERROR_F(last_result);
    goto do_return;
  }

  dir = davix_posix.opendir(&davix_request_params, url.c_str(), &daverr);
  if (!dir)
  {
    set_last_result(daverr);
    SCDC_ERROR_F(last_result);
  }

do_return:

  SCDC_TRACE_F("return: " << dir << ",  last_result: '" << last_result << "'");

  return dir;
}


static bool dirent_is_dir(struct dirent *ent)
{
  return (ent->d_type == DT_DIR);
}


static bool dirent_is_file(struct dirent *ent)
{
  return (ent->d_type == DT_REG);
}


scdcint_t scdc_posix_webdav_handler::dir_count_entries(dir_t dir)
{
  SCDC_TRACE_F("dir: " << dir);

  last_result.clear();
  DavixError *daverr = NULL;

  scdcint_t n = 0;
  struct dirent *ent;
  while ((ent = davix_posix.readdir(dir, &daverr)) != NULL)
  {
    if (dirent_is_dir(ent) || dirent_is_file(ent)) ++n;
  }

  SCDC_TRACE_F("return: " << n);

  return n;
}


scdcint_t scdc_posix_webdav_handler::dir_count_dirs(dir_t dir)
{
  SCDC_TRACE_F("dir: " << dir);

  last_result.clear();
  DavixError *daverr = NULL;

  scdcint_t n = 0;
  struct dirent *ent;
  while ((ent = davix_posix.readdir(dir, &daverr)) != NULL)
  {
    if (dirent_is_dir(ent)) ++n;
  }

  SCDC_TRACE_F("return: " << n);

  return n;
}


bool scdc_posix_webdav_handler::dir_list_dirs(dir_t dir, vector<string> &dirs)
{
  SCDC_TRACE_F("dir: " << dir);

  last_result.clear();
  bool ret = false;
  DavixError *daverr = NULL;

  struct dirent *ent;
  while ((ent = davix_posix.readdir(dir, &daverr)) != NULL)
  {
    if (dirent_is_file(ent)) dirs.push_back(ent->d_name);
  }

  if (daverr)
  {
    set_last_result(daverr);
    SCDC_ERROR_F(last_result);

  } else ret = true;

  SCDC_TRACE_F("return: " << ret);

  return ret;
}


scdcint_t scdc_posix_webdav_handler::dir_count_files(dir_t dir)
{
  SCDC_TRACE_F("dir: " << dir);

  last_result.clear();
  DavixError *daverr = NULL;

  scdcint_t n = 0;
  struct dirent *ent;
  while ((ent = davix_posix.readdir(dir, &daverr)) != NULL)
  {
    if (dirent_is_file(ent)) ++n;
  }

  SCDC_TRACE_F("return: " << n);

  return n;
}


bool scdc_posix_webdav_handler::dir_list_files(dir_t dir, vector<string> &files)
{
  SCDC_TRACE_F("dir: " << dir);

  last_result.clear();
  bool ret = false;
  DavixError *daverr = NULL;

  struct dirent *ent;
  while ((ent = davix_posix.readdir(dir, &daverr)) != NULL)
  {
    if (dirent_is_file(ent)) files.push_back(ent->d_name);
  }

  if (daverr)
  {
    set_last_result(daverr);
    SCDC_ERROR_F(last_result);

  } else ret = true;

  SCDC_TRACE_F("return: " << ret);

  return ret;
}


scdcint_t scdc_posix_webdav_handler::file_get_size(const char *path)
{
  string url = root_url + path;
  SCDC_TRACE_F("path: '" << path << "' -> 'url: " << url << "'");

  last_result.clear();
  scdcint_t size = -1;
  DavixError *daverr = NULL;

  StatInfo si;
  if (davix_posix.stat64(&davix_request_params, url, &si, &daverr) != 0)
  {
    set_last_result(daverr);
    SCDC_ERROR_F(last_result);
    goto do_return;
  }

  size = si.size;

do_return:

  SCDC_TRACE_F("return: " << size << ",  last_result: '" << last_result << "'");

  return size;
}


bool scdc_posix_webdav_handler::file_rm(const char *path)
{
  string url = root_url + path;
  SCDC_TRACE_F("path: '" << path << "' -> '" << url << "'");

  last_result.clear();
  DavixError *daverr = NULL;

  bool ret = (davix_posix.unlink(&davix_request_params, url, &daverr) == 0);
  if (!ret)
  {
    set_last_result(daverr);
    SCDC_ERROR_F(last_result);
  }

  SCDC_TRACE_F("return: " << ret << ",  last_result: '" << last_result << "'");

  return ret;
}


scdc_posix_webdav_handler::file_t scdc_posix_webdav_handler::file_open(const char *path, int flags)
{
  string url = root_url + path;
  SCDC_TRACE_F("path: '" << path << "' -> '" << url << "', flags: " << flags);

  last_result.clear();
  DavixError *daverr = NULL;

  DAVIX_FD *file = davix_posix.open(&davix_request_params, url, flags, &daverr);
  if (!file)
  {
    set_last_result(daverr);
    SCDC_ERROR_F(last_result);
  }

  SCDC_TRACE_F("return: " << file << ",  last_result: '" << last_result << "'");

  return file;
}


scdc_posix_webdav_handler::file_t scdc_posix_webdav_handler::file_reopen(const char *path, int flags, file_t file, const char *old_path)
{
  string url = root_url + path;
  SCDC_TRACE_F("path: '" << path << "' -> '" << url << "', flags: " << flags << ", old_path: '" << old_path << "'");

  last_result.clear();
  DavixError *daverr = NULL;

  bool ret = (davix_posix.close(file, &daverr) == 0);
  if (!ret)
  {
    file = file_null;
    set_last_result(daverr);
    SCDC_ERROR_F(last_result);
    goto do_return;
  }

  file = davix_posix.open(&davix_request_params, url, flags, &daverr);
  if (!file)
  {
    set_last_result(daverr);
    SCDC_ERROR_F(last_result);
  }

do_return:
  SCDC_TRACE_F("return: " << file << ",  last_result: '" << last_result << "'");

  return file;
}


bool scdc_posix_webdav_handler::file_close(file_t file)
{
  SCDC_TRACE_F("file: " << file);

#if 0
  if (file == file_null) return true;
#endif

  last_result.clear();
  DavixError *daverr = NULL;

  bool ret = (davix_posix.close(file, &daverr) == 0);
  if (!ret)
  {
    set_last_result(daverr);
    SCDC_ERROR_F(last_result);
  }

  SCDC_TRACE_F("return: " << ret << ",  last_result: '" << last_result << "'");

  return ret;
}


scdcint_t scdc_posix_webdav_handler::file_seek(file_t file, scdcint_t offset, int whence)
{
  SCDC_TRACE_F("file: " << file << ", offset: " << offset << ", whence: " << whence);

  last_result.clear();
  DavixError *daverr = NULL;

  scdcint_t pos = davix_posix.lseek(file, offset, whence, &daverr);
  if (pos == -1)
  {
    set_last_result(daverr);
    SCDC_ERROR_F(last_result);
  }

  SCDC_TRACE_F("return: " << pos << ",  last_result: '" << last_result << "'");

  return pos;
}


scdcint_t scdc_posix_webdav_handler::file_read(file_t file, void *buf, scdcint_t count)
{
  SCDC_TRACE_F("file: " << file << ", buf: " << buf << ", count: " << count);

  last_result.clear();
  DavixError *daverr = NULL;

  scdcint_t size = davix_posix.read(file, buf, count, &daverr);
  if (size < 0)
  {
    set_last_result(daverr);
    SCDC_ERROR_F(last_result);
  }

  SCDC_TRACE_F("return: " << size << ",  last_result: '" << last_result << "'");

  return size;
}


scdcint_t scdc_posix_webdav_handler::file_write(file_t file, const void *buf, scdcint_t count)
{
  SCDC_TRACE_F("file: " << file << ", buf: " << buf << ", count: " << count);

  last_result.clear();
  DavixError *daverr = NULL;

  scdcint_t size = davix_posix.write(file, buf, count, &daverr);
  if (size < 0)
  {
    set_last_result(daverr);
    SCDC_ERROR_F(last_result);
  }

  SCDC_TRACE_F("return: " << size << ",  last_result: '" << last_result << "'");

  return size;
}


void scdc_posix_webdav_handler::set_last_result(Davix::DavixError *daverr)
{
  last_result = daverr->getErrScope() + ": " + daverr->getErrMsg();
}


#undef SCDC_LOG_PREFIX
