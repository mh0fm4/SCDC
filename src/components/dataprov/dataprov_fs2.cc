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


#include <cstdio>
#include <cerrno>
#include <cstring>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sstream>

#include "z_pack.h"

#define SCDC_TRACE_NOT  !SCDC_TRACE_DATAPROV_FS

#include "config.hh"
#include "common.hh"
#include "log.hh"
#include "dataset_inout.h"
#include "dataprov_fs2.hh"


#define SCDC_DATASET_FS_LEAVE_PWD_OPEN  1


using namespace std;


#define SCDC_LOG_PREFIX  "dataset-fs: "


class scdc_dataset_fs2: public scdc_dataset
{
  public:
    scdc_dataset_fs2(scdc_dataprov *dataprov_)
      :scdc_dataset(dataprov_), pwd_file(false)
#if SCDC_DATASET_FS_LEAVE_PWD_OPEN
      , pwd_FILE(0), pwd_FILE_mode(0)
#endif
    { }


    ~scdc_dataset_fs2()
    {
#if SCDC_DATASET_FS_LEAVE_PWD_OPEN
      close_pwd_file();
#endif
    }


    bool do_cmd_cd(const string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output, scdc_result &result)
    {
      SCDC_TRACE("do_cmd_cd: params: '" << params << "'");

#if SCDC_DATASET_FS_LEAVE_PWD_OPEN
      close_pwd_file();
#endif

      bool ret = scdc_dataset::do_cmd_cd(params, input, output, result);

      SCDC_TRACE("do_cmd_cd: return: " << ret);

      return ret;
    }


  protected:
    bool pwd_file;
#if SCDC_DATASET_FS_LEAVE_PWD_OPEN
    FILE *pwd_FILE;
    string pwd_FILE_path;
    int pwd_FILE_mode;
#endif


    scdc_dataprov_fs2 *dataprov_fs2()
    {
      return static_cast<scdc_dataprov_fs2 *>(dataprov);
    }


#if SCDC_DATASET_FS_LEAVE_PWD_OPEN
    void close_pwd_file()
    {
      if (pwd_FILE) fclose(pwd_FILE);

      pwd_FILE = 0;
      pwd_FILE_path.clear();
      pwd_FILE_mode = 0;
    }
#endif


    FILE *open_file(const string &path, bool read, bool write)
    {
      int mode = (read?1:0) + (write?2:0);

#if SCDC_DATASET_FS_LEAVE_PWD_OPEN
      if (pwd_FILE && pwd_FILE_path == path)
      {
        if (mode == 0 || mode == pwd_FILE_mode || pwd_FILE_mode == 3) return pwd_FILE;

        mode |= pwd_FILE_mode;
      }
#endif

#if SCDC_DATASET_FS_LEAVE_PWD_OPEN
      close_pwd_file();
#endif

      const char *m = 0;

      switch (mode)
      {
        case 0: m = "r"; mode = 1; break;
        case 1: m = "r"; break;
        case 2: if (z_fs_exists(path.c_str())) { m = "r+"; mode = 3; } else m = "w"; break;
        case 3: m = (z_fs_exists(path.c_str()))?"r+":"w+"; break;
      }

      FILE *file = fopen(path.c_str(), m);

#if SCDC_DATASET_FS_LEAVE_PWD_OPEN
      if (file)
      {
        pwd_FILE = file;
        pwd_FILE_path = path;
        pwd_FILE_mode = mode;
      }
#endif

      return file;
    }


    void close_file(FILE *file)
    {
#if SCDC_DATASET_FS_LEAVE_PWD_OPEN
#else
      fclose(file);
#endif
    }
};


#undef SCDC_LOG_PREFIX


#define SCDC_LOG_PREFIX  "dataprov-fs: "


bool scdc_dataprov_fs2::open(const char *conf, scdc_args *args, scdc_result &result)
{
  SCDC_TRACE("open: conf: '" << conf << "'");

  bool ret = true;

  const char *r;
  if (!args->get<const char *>(SCDC_ARGS_TYPE_CSTR, &r))
  {
    SCDC_ERROR("open: getting root directory");
    ret = false;
    goto do_quit;

  } else if (!z_fs_is_directory(r))
  {
    SCDC_ERROR("open: given root directory '" << r << "' is not a directory");
    ret = false;
    goto do_quit;
  }

  if (!scdc_dataprov::open(conf, args, result))
  {
    SCDC_FAIL("open: opening base");
    ret = false;

  } else
  {
    set_root(r);
  }

do_quit:
  return ret;
}


bool scdc_dataprov_fs2::close(scdc_result &result)
{
  SCDC_TRACE("close:");

  bool ret = scdc_dataprov::close(result);

  SCDC_TRACE("close: return: " << ret);

  return ret;
}


template<class DATASET_FS>
scdc_dataset *scdc_dataprov_fs2::dataset_open(std::string &path, scdc_result &result)
{
  SCDC_TRACE("dataset_open: path: '" << path << "'");

  DATASET_FS *dataset_fs2 = new DATASET_FS(this);

  SCDC_TRACE("dataset_open: return: '" << dataset_fs2 << "'");

  return dataset_fs2;
}


bool scdc_dataprov_fs2::dataset_close(scdc_dataset *dataset, scdc_result &result)
{
  SCDC_TRACE("dataset_close: dataset: " << dataset);

  bool ret = true;

  delete dataset;

  SCDC_TRACE("dataset_close: return: " << ret);

  return ret;
}


bool scdc_dataprov_fs2::config_do_cmd_param(const std::string &cmd, const std::string &param, std::string val, scdc_config_result &result, bool &done)
{
  SCDC_TRACE("config_do_cmd_param: cmd: '" << cmd << "', param: '" << param << "', val: '" << val << "', result: '" << result << "'");

  done = true;
  bool ret = true;

  if (param == "")
  {
    ret = scdc_dataprov::config_do_cmd_param(cmd, param, val, result, done);
    if (!ret) goto do_quit;

    done = true;

    if (cmd == "info")
    {
      ret = scdc_dataprov_config::info(result, "free/used");

    } else if (cmd == "ls")
    {
      ret = scdc_dataprov_config::ls(result, "root");

    } else done = false;

  } else
  {
    ret = scdc_dataprov::config_do_cmd_param(cmd, param, val, result, done);
    if (!ret) goto do_quit;

    if (done);
    else if (cmd == "info")
    {
      done = true;

      if (param == "root") ret = scdc_dataprov_config::info(result, param, "root directory within the local file system (string)");
      else done = false;

    } else if (cmd == "get")
    {
      done = true;

      if (param == "root") ret = scdc_dataprov_config::get<string>(result, "root", root);
      else done = false;

    } else done = false;
  }

do_quit:
  if (done && !ret)
  {
    SCDC_FAIL("config_do_cmd_param: cmd '" << cmd << "' failed");
    return false;
  }

  return ret;
}


bool scdc_dataprov_fs2::make_pwd_path(const std::string &pwd, const std::string &sub, std::string &pwd_path)
{
  std::string p;

  if (!sub.empty())
  {
    if (sub[0] != '/') p = pwd + "/";
    p += sub;

  } else p = pwd;

  normalize_path(p.c_str(), pwd_path);

  if (!pwd_path.empty() && pwd_path[0] == '/') pwd_path = pwd_path.erase(0, 1);

  return (pwd_path.compare("..") != 0);
}


void scdc_dataprov_fs2::make_abs_path(const std::string &pwd_path, std::string &abs_path)
{
  abs_path = root + pwd_path;
}


std::string scdc_dataprov_fs2::get_abs_path(const std::string &rel_path)
{
  return root + rel_path;
}


void scdc_dataprov_fs2::do_full_path(const char *path, std::string &full_path)
{
  full_path = get_abs_path((path)?path:"");
}


bool scdc_dataprov_fs2::undo_full_path(const char *full_path, std::string &path)
{
  string norm_full_path;

  normalize_path(full_path, norm_full_path);

  norm_full_path += "/";

  if (strncmp(norm_full_path.c_str(), root.c_str(), root.size()) != 0) return false;

  path = norm_full_path.c_str() + root.size();

  /* remove trailing '/' */
  while (path.size() > 1 && *(path.end() - 1) == '/') path.erase(path.size() - 1, 1);

  return true;
}


void scdc_dataprov_fs2::set_root(const std::string &root_)
{
  root = root_;

  if (!root.empty() && *(root.end() - 1) != '/') root += '/';

  SCDC_TRACE("set_root: root: '" << root << "'");
}


#undef SCDC_LOG_PREFIX


#define SCDC_LOG_PREFIX  "dataset-fs-access: "


class scdc_dataset_fs2_access: public scdc_dataset_fs2
{
  public:
    scdc_dataset_fs2_access(scdc_dataprov *dataprov_)
      :scdc_dataset_fs2(dataprov_) { }


    bool do_cmd_info(const string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output, scdc_result &result)
    {
      SCDC_TRACE("do_cmd_info: '" << params << "'");

      return scdc_dataset::do_cmd_info(params, input, output, result);
    }


    bool do_cmd_cd(const string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output, scdc_result &result)
    {
      SCDC_TRACE("do_cmd_cd: params: '" << params << "'");

      bool ret = false;

      const string sub = (params.empty())?"/":params;
      string pwd_path, abs_path;

      if (!dataprov_fs2()->make_pwd_path(pwd, sub, pwd_path))
      {
        result = "invalid path";
        SCDC_FAIL(__func__ << ": make path '" << pwd << "' + '" << sub << "' failed: " << result);
        goto do_return;
      }

      dataprov_fs2()->make_abs_path(pwd_path, abs_path);
      SCDC_TRACE("do_cmd_cd: pwd_path: '" << pwd_path << "', abs_path: '" << abs_path << "'");

      z_fs_stat_t stat;

      if (!z_fs_stat(abs_path.c_str(), &stat))
      {
        result = "path does not exist";
        SCDC_FAIL(__func__ << ": checking path '" << abs_path << "' failed: " << result);
        goto do_return;
      }

      pwd_file = stat.is_file;
      SCDC_TRACE("do_cmd_cd: pwd_file: " << pwd_file);

      ret = scdc_dataset_fs2::do_cmd_cd(pwd_path, input, output, result);

do_return:
      SCDC_TRACE(__func__ << ": return: " << ret);

      return ret;
    }


    bool do_cmd_ls(const string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output, scdc_result &result)
    {
      SCDC_TRACE("do_cmd_ls: params: '" << params << "'");

      bool ret = false;

      const string sub = params;
      string pwd_path, abs_path;

      if (!dataprov_fs2()->make_pwd_path(pwd, sub, pwd_path))
      {
        result = "invalid path";
        SCDC_FAIL(__func__ << ": make path '" << pwd << "' + '" << sub << "' failed: " << result);
        goto do_return;
      }

      dataprov_fs2()->make_abs_path(pwd_path, abs_path);
      SCDC_TRACE(__func__ << ": pwd_path: '" << pwd_path << "', abs_path: '" << abs_path << "'");

      if (scdc_dataset_output_redirect(output, "from:fslist", abs_path.c_str()) != SCDC_SUCCESS)
      {
        result = "listing failed";
        SCDC_FAIL(__func__ << ": listing file or directory '" << pwd_path << "' failed: " << result);
        goto do_return;
      }

      ret = true;

do_return:
      SCDC_TRACE("do_cmd_ls: return: " << ret);

      return ret;
    }


    bool do_cmd_put(const string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output, scdc_result &result)
    {
      SCDC_TRACE("do_cmd_put: params: '" << params << "'");

      bool ret = false;

      stringlist psl(' ', params);

      const string sub = (!pwd_file)?psl.front_pop():"";
      string pwd_path, abs_path;

      if (!dataprov_fs2()->make_pwd_path(pwd, sub, pwd_path))
      {
        result = "invalid path";
        SCDC_FAIL(__func__ << ": make path '" << pwd << "' + '" << sub << "' failed: " << result);
        goto do_return;
      }

      dataprov_fs2()->make_abs_path(pwd_path, abs_path);
      SCDC_TRACE(__func__ << ": pwd_path: '" << pwd_path << "', abs_path: '" << abs_path << "'");

      if (input && strcmp(input->format, "fstar") == 0)
      {
        SCDC_TRACE(__func__ << ": fstar input to directory");

        string conf = "to:fs";

        if (scdc_dataset_input_redirect(input, conf.c_str(), abs_path.c_str()) != SCDC_SUCCESS)
        {
          result = "input redirection failed";
          SCDC_FAIL(__func__ << ": redirecting input to directory '" << abs_path << "' failed: " << result);

        } else ret = true;

      } else
      {
        string p = psl.front_pop();

        if (!p.empty() && p[0] == 'd')
        {
          SCDC_TRACE(__func__ << ": create directory");

          if (!z_fs_mkdir(abs_path.c_str()))
          {
            result = "directory creation failed";
            SCDC_FAIL(__func__ << ": creating directory '" << abs_path << "' failed: " << result);

          } else ret = true;

        } else
        {
          SCDC_TRACE(__func__ << ": plain input to file");

          size_t i = 0;

          if (!p.empty() && p[0] == 'f')
          {
            ++i;

            if (p.size() > 1 && p[1] == ':') ++i;
          }

          string conf = "to:streamdup:" + p.substr(i);

          FILE *file = open_file(abs_path, false, true);

          if (!file || scdc_dataset_input_redirect(input, conf.c_str(), file) != SCDC_SUCCESS)
          {
            result = "input redirection failed";
            SCDC_FAIL(__func__ << ": redirecting input to file '" << abs_path << "' failed: " << result);

          } else ret = true;

          close_file(file);
        }
      }

do_return:
      SCDC_TRACE(__func__ << ": return: " << ret);

      return ret;
    }


    bool do_cmd_get(const string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output, scdc_result &result)
    {
      SCDC_TRACE("do_cmd_get: params: '" << params << "'");

      bool ret = false;

      stringlist psl(' ', params);

      const string sub = (!pwd_file)?psl.front_pop():"";
      string pwd_path, abs_path;

      if (!dataprov_fs2()->make_pwd_path(pwd, sub, pwd_path))
      {
        result = "invalid path";
        SCDC_FAIL(__func__ << ": make path '" << pwd << "' + '" << sub << "' failed: " << result);
        goto do_return;
      }

      dataprov_fs2()->make_abs_path(pwd_path, abs_path);
      SCDC_TRACE("do_cmd_get: pwd_path: '" << pwd_path << "', abs_path: '" << abs_path << "'");

      if (z_fs_is_directory(abs_path.c_str()))
      {
        SCDC_TRACE("do_cmd_get: fstar output from directory");

        if (scdc_dataset_output_redirect(output, "from:fs", abs_path.c_str(), params.c_str()) != SCDC_SUCCESS)
        {
          result = "output redirection failed";
          SCDC_FAIL("do_cmd_get: redirecting output '" << params << "' from directory '" << abs_path.c_str() << "' failed: " << result);

        } else ret = true;

      } else
      {
        SCDC_TRACE("do_cmd_get: plain output from file");

        string conf = "from:streamdup:" + psl.front_pop();

        FILE *file = open_file(abs_path, true, false);

        if (scdc_dataset_output_redirect(output, conf.c_str(), file) != SCDC_SUCCESS)
        {
          result = "output redirection failed";
          SCDC_FAIL("do_cmd_get: redirecting output from file '" << abs_path << "' failed: " << result);

        } else ret = true;

        close_file(file);
      }

do_return:
      SCDC_TRACE("do_cmd_get: return: " << ret);

      return ret;
    }


    bool do_cmd_rm(const string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output, scdc_result &result)
    {
      SCDC_TRACE("do_cmd_rm: params: '" << params << "'");

      bool ret = false;

      const string sub = params;
      string pwd_path, abs_path;

      if (!dataprov_fs2()->make_pwd_path(pwd, sub, pwd_path))
      {
        result = "invalid path";
        SCDC_FAIL(__func__ << ": make path '" << pwd << "' + '" << sub << "' failed: " << result);
        goto do_return;
      }

      dataprov_fs2()->make_abs_path(pwd_path, abs_path);
      SCDC_TRACE("do_cmd_rm: pwd_path: '" << pwd_path << "', abs_path: '" << abs_path << "'");

      if (!z_fs_rm(abs_path.c_str()))
      {
        result = "removing failed";
        SCDC_FAIL(__func__ << ": removing file or directory '" << pwd_path << "' failed: " << result);
        goto do_return;
      }

      if (sub.empty()) do_cmd_cd("..", input, output, result);

      ret = true;

do_return:
      SCDC_TRACE("do_cmd_rm: return: " << ret);

      return ret;
    }


    bool do_cmd_sync(const string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output, scdc_result &result)
    {
      SCDC_TRACE("do_cmd_sync: params: '" << params << "'");

      bool ret = false;

      stringlist psl(' ', params);

      const string sub = (!pwd_file)?psl.front_pop():"";
      string pwd_path, abs_path;

      if (!dataprov_fs2()->make_pwd_path(pwd, sub, pwd_path))
      {
        result = "invalid path";
        SCDC_FAIL(__func__ << ": make path '" << pwd << "' + '" << sub << "' failed: " << result);
        goto do_return;
      }

      dataprov_fs2()->make_abs_path(pwd_path, abs_path);
      SCDC_TRACE("do_cmd_sync: pwd_path: '" << pwd_path << "', abs_path: '" << abs_path << "'");

      if (z_fs_is_directory(abs_path.c_str()))
      {
        result = "syncing not supported";
        SCDC_FAIL(__func__ << ": syncing directory '" << abs_path << "' failed: " << result);

      } else
      {
        FILE *file = open_file(abs_path, false, false);

        if (!file || fsync(fileno(file)) != 0)
        {
          result = "syncing failed";
          SCDC_FAIL(__func__ << ": syncing file '" << abs_path << "' failed: " << result);

        } else ret = true;

        close_file(file);
      }

do_return:
      SCDC_TRACE(__func__ << ": return: " << ret);

      return ret;
    }
};


#undef SCDC_LOG_PREFIX


#define SCDC_LOG_PREFIX  "dataprov-fs-access: "


scdc_dataprov_fs2_access::scdc_dataprov_fs2_access()
  :scdc_dataprov_fs2("fs:access")
{
}


bool scdc_dataprov_fs2_access::open(const char *conf, scdc_args *args, scdc_result &result)
{
  SCDC_TRACE("open: conf: '" << conf << "'");

  if (!scdc_dataprov_fs2::open(conf, args, result))
  {
    SCDC_FAIL("open: opening base");
    return false;
  }

  dataset_cmds_add("pwd", static_cast<dataset_cmds_do_cmd_f>(&scdc_dataset_fs2_access::do_cmd_pwd));
  dataset_cmds_add("info", static_cast<dataset_cmds_do_cmd_f>(&scdc_dataset_fs2_access::do_cmd_info));
  dataset_cmds_add("cd", static_cast<dataset_cmds_do_cmd_f>(&scdc_dataset_fs2_access::do_cmd_cd));
  dataset_cmds_add("ls", static_cast<dataset_cmds_do_cmd_f>(&scdc_dataset_fs2_access::do_cmd_ls));
  dataset_cmds_add("put", static_cast<dataset_cmds_do_cmd_f>(&scdc_dataset_fs2_access::do_cmd_put));
  dataset_cmds_add("get", static_cast<dataset_cmds_do_cmd_f>(&scdc_dataset_fs2_access::do_cmd_get));
  dataset_cmds_add("rm", static_cast<dataset_cmds_do_cmd_f>(&scdc_dataset_fs2_access::do_cmd_rm));
  dataset_cmds_add("sync", static_cast<dataset_cmds_do_cmd_f>(&scdc_dataset_fs2_access::do_cmd_sync));

  return true;
}


scdc_dataset *scdc_dataprov_fs2_access::dataset_open(std::string &path, scdc_result &result)
{
  return scdc_dataprov_fs2::dataset_open<scdc_dataset_fs2_access>(path, result);
}


#undef SCDC_LOG_PREFIX


#define SCDC_LOG_PREFIX  "dataset-fs-store: "


#define DIR_STORE_PREFIX  "store_v1_"
#define DIR_ENTRY_PREFIX  "entry_v1_"
#define DIR_ENTRY_SUFFIX  ""


static scdcint_t dir_count_stores(DIR *dir)
{
  struct dirent *ent;
  scdcint_t c = 0;

  while ((ent = readdir(dir)) != NULL)
  {
    if (ent->d_type == DT_DIR && has_prefix(ent->d_name, DIR_STORE_PREFIX)) ++c;
  }

  return c;
}


static scdcint_t dir_count_entries(DIR *dir)
{
  struct dirent *ent;
  scdcint_t c = 0;

  while ((ent = readdir(dir)) != NULL)
  {
    if (ent->d_type == DT_REG && has_prefix_suffix(ent->d_name, DIR_ENTRY_PREFIX, DIR_ENTRY_SUFFIX)) ++c;
  }

  return c;
}


class scdc_dataset_fs2_store: public scdc_dataset_fs2
{
  public:
    scdc_dataset_fs2_store(scdc_dataprov *dataprov_)
      :scdc_dataset_fs2(dataprov_), admin(false) { }


    bool do_cmd_info(const string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output, scdc_result &result)
    {
      SCDC_TRACE("do_cmd_info: '" << params << "'");

      scdc_dataprov_fs2 *dataprov_fs2 = static_cast<scdc_dataprov_fs2 *>(dataprov);

      string main_path;
      dataprov_fs2->do_full_path(0, main_path);

      SCDC_TRACE("do_cmd_info: main_path: '" << main_path << "'");

      DIR *dir = opendir(main_path.c_str());

      if (!dir)
      {
        result = "opening failed";
        SCDC_FAIL(__func__ << ": opening directory '" << main_path << "' failed: " << result);
        return false;
      }

      scdcint_t nstores = dir_count_stores(dir);

      closedir(dir);

      result = string("admin: ") + (admin?"yes":"no") + ", stores: " + zxx::to_string(nstores);

      return true;
    }


    bool do_cmd_cd(const string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output, scdc_result &result)
    {
      SCDC_TRACE("do_cmd_cd: '" << params << "'");

      if (params == "ADMIN")
      {
        admin = true;
        return scdc_dataset::do_cmd_cd(params, input, output, result);
      }

      scdc_dataprov_fs2 *dataprov_fs2 = static_cast<scdc_dataprov_fs2 *>(dataprov);

      if (!params.empty())
      {
        string store_path;
        dataprov_fs2->do_full_path(0, store_path);

        store_path += DIR_STORE_PREFIX + string(params) + "/";

        SCDC_TRACE("do_cmd_cd: store_path: '" << store_path << "'");

        if (!z_fs_is_directory(store_path.c_str()))
        {
          result = "store does not exist";
          SCDC_FAIL(__func__ << ": changing to store '" << params << "' failed: " << result);
          return false;
        }
      }

      admin = false;

      return scdc_dataset_fs2::do_cmd_cd(params, input, output, result);
    }


    bool do_cmd_ls(const string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output, scdc_result &result)
    {
      SCDC_TRACE("do_cmd_ls: '" << params << "'");

      SCDC_DATASET_OUTPUT_CLEAR_(output);

      scdc_dataprov_fs2 *dataprov_fs2 = static_cast<scdc_dataprov_fs2 *>(dataprov);

      if (admin || pwd.size() == 0)
      {
        SCDC_TRACE("do_cmd_ls: listing stores");

        string main_path;
        dataprov_fs2->do_full_path(0, main_path);

        SCDC_TRACE("do_cmd_ls: main_path: '" << main_path << "'");

        DIR *dir = opendir(main_path.c_str());

        if (!dir)
        {
          result = "opening directory failed";
          SCDC_FAIL(__func__ << ": opening directory '" << main_path << "' failed: << result");
          return false;
        }

        scdcint_t stores = dir_count_stores(dir);

        SCDC_DATASET_OUTPUT_PRINTF_(output, "%" scdcint_fmt "|", stores);

        rewinddir(dir);

        struct dirent *ent;

        while ((ent = readdir(dir)) != NULL)
        {
          if (ent->d_type == DT_DIR && has_prefix(ent->d_name, DIR_STORE_PREFIX))
          {
            SCDC_TRACE("store: '" << ent->d_name << "'");
            SCDC_DATASET_OUTPUT_PRINTF_APPEND_(output, "%s|", ent->d_name + strlen(DIR_STORE_PREFIX));
          }
        }

        closedir(dir);

      } else
      {
        SCDC_TRACE("do_cmd_ls: listing entries of store '" << pwd << "'");

        string store_path;
        dataprov_fs2->do_full_path(0, store_path);

        store_path += DIR_STORE_PREFIX + pwd + "/";

        SCDC_TRACE("do_cmd_ls: store_path: '" << store_path << "'");

        DIR *dir = opendir(store_path.c_str());

        if (!dir)
        {
          result = "opening directory failed";
          SCDC_FAIL(__func__ << ": opening directory '" << store_path << "' failed: " << result);
          return false;
        }

        scdcint_t entries = dir_count_entries(dir);

        SCDC_DATASET_OUTPUT_PRINTF_(output, "%" scdcint_fmt "|", entries);

        rewinddir(dir);

        struct dirent *ent;

        while ((ent = readdir(dir)) != NULL)
        {
          if (ent->d_type == DT_REG && has_prefix_suffix(ent->d_name, DIR_ENTRY_PREFIX, DIR_ENTRY_SUFFIX))
          {
            string entry_name = trim_prefix_suffix(ent->d_name, DIR_ENTRY_PREFIX, DIR_ENTRY_SUFFIX);

            /* FIXME */
            string entry_format;

            string entry_path = store_path + ent->d_name;
            scdcint_t entry_size = z_fs_get_file_size(entry_path.c_str());

            SCDC_TRACE("entry: name: '" << entry_name << "', format: '" << entry_format << "', size: " << entry_size);
            SCDC_DATASET_OUTPUT_PRINTF_APPEND_(output, "%s:%s:%" scdcint_fmt "|", entry_name.c_str(), entry_format.c_str(), entry_size);
          }
        }
      }

      return true;
    }


    bool do_cmd_put(const string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output, scdc_result &result)
    {
      SCDC_TRACE("do_cmd_put: '" << params << "'");

      scdc_dataprov_fs2 *dataprov_fs2 = static_cast<scdc_dataprov_fs2 *>(dataprov);

      if (params.empty())
      {
        result = "empty path";
        SCDC_FAIL(__func__ << ": writing to store failed: " << result);
        return false;

      } else if (admin)
      {
        SCDC_TRACE("do_cmd_put: creating store '" << params << "'");

        string store_path;
        dataprov_fs2->do_full_path(0, store_path);

        store_path += DIR_STORE_PREFIX + string(params) + "/";

        SCDC_TRACE("do_cmd_put: store_path: '" << store_path << "'");

        if (!z_fs_mkdir(store_path.c_str()))
        {
          result = string("creating store failed: ") + strerror(errno);
          SCDC_FAIL(__func__ << ": making store '" << store_path << "' failed: " << result);
          return false;
        }

      } else if (pwd.size() == 0)
      {
        result = "no store selected";
        SCDC_FAIL(__func__ << ": creating entry failed: " << result);
        return false;

      } else
      {
        SCDC_TRACE("do_cmd_put: creating entry '" << params << "'");

        string store_path;
        dataprov_fs2->do_full_path(0, store_path);

        store_path += DIR_STORE_PREFIX + pwd + "/";

        SCDC_TRACE("do_cmd_put: store_path: '" << store_path << "'");

        string entry_path = store_path + DIR_ENTRY_PREFIX + params + DIR_ENTRY_SUFFIX;

        SCDC_TRACE("do_cmd_put: entry_path: '" << entry_path.c_str() << "'");

        if (!input)
        {
          result = "no input available";
          SCDC_FAIL("do_cmd_put: creating entry failed: " << result);
          return false;
        }

        if (scdc_dataset_input_redirect(input, "to:file", entry_path.c_str()) != SCDC_SUCCESS)
        {
          result = " input redirection failed";
          SCDC_FAIL("do_cmd_put: creating entry failed: " << result);
          return false;
        }
      }

      return true;
    }


    bool do_cmd_get(const string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output, scdc_result &result)
    {
      SCDC_TRACE("do_cmd_get: '" << params << "'");

      scdc_dataprov_fs2 *dataprov_fs2 = static_cast<scdc_dataprov_fs2 *>(dataprov);

      if (params.empty())
      {
        result = "empty path";
        SCDC_FAIL(__func__ << ": writing to store failed: " << result);
        return false;

      } else if (pwd.empty())
      {
        result = "no store selected";
        SCDC_FAIL(__func__ << ": getting entry failed: " << result);
        return false;

      } else
      {
        SCDC_TRACE("do_cmd_get: getting entry '" << params << "'");

        string store_path;
        dataprov_fs2->do_full_path(0, store_path);

        store_path += DIR_STORE_PREFIX + pwd + "/";

        SCDC_TRACE("do_cmd_put: store_path: '" << store_path << "'");

        string entry_path = store_path + DIR_ENTRY_PREFIX + params + DIR_ENTRY_SUFFIX;

        SCDC_TRACE("do_cmd_put: entry_path: '" << entry_path.c_str() << "'");

        if (!z_fs_is_file(entry_path.c_str()))
        {
          result = "entry does not exist";
          SCDC_FAIL("do_cmd_get: getting entry " << entry_path << " failed: " << result);
          return false;
        }

        /* FIXME */
        string entry_format;

        if (scdc_dataset_output_redirect(output, "from:file", entry_path.c_str()) != SCDC_SUCCESS)
        {
          result = "output redirection failed";
          SCDC_FAIL("do_cmd_get: getting entry " << entry_path << " failed: " << result);
          return false;
        }
      }

      return true;
    }


    bool do_cmd_rm(const string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output, scdc_result &result)
    {
      SCDC_TRACE("do_cmd_rm: '" << params << "'");

      scdc_dataprov_fs2 *dataprov_fs2 = static_cast<scdc_dataprov_fs2 *>(dataprov);

      if (params.empty())
      {
        result = "empty path";
        SCDC_FAIL(__func__ << ": writing to store failed: " << result);
        return false;

      } else if (admin)
      {
        SCDC_TRACE("do_cmd_rm: deleting store '" << params << "'");

        string store_path;
        dataprov_fs2->do_full_path(0, store_path);

        store_path += DIR_STORE_PREFIX + string(params) + "/";

        SCDC_TRACE("do_cmd_rm: store_path: '" << store_path << "'");

        if (!z_fs_is_directory(store_path.c_str()))
        {
          result = string("store does not exist");
          SCDC_FAIL("do_cmd_rm: removing store '" << store_path << "' failed: " << result);
          return false;
        }

        if (!z_fs_rm_r(store_path.c_str()))
        {
          result = string("remove failed: ") + strerror(errno);
          SCDC_FAIL("do_cmd_rm: removing store '" << store_path << "' failed: " << result);
          return false;
        }

      } else if (pwd.size() == 0)
      {
        result = "no store selected";
        SCDC_FAIL(__func__ << ": creating entry failed: " << result);
        return false;

      } else
      {
        SCDC_TRACE("do_cmd_rm: deleting entry '" << params << "'");

        string store_path;
        dataprov_fs2->do_full_path(0, store_path);

        store_path += DIR_STORE_PREFIX + pwd + "/";

        SCDC_TRACE("do_cmd_rm: store_path: '" << store_path << "'");

        string entry_path = store_path + DIR_ENTRY_PREFIX + params + DIR_ENTRY_SUFFIX;

        SCDC_TRACE("do_cmd_rm: entry_path: '" << entry_path.c_str() << "'");

        if (!z_fs_is_file(entry_path.c_str()))
        {
          result = "entry does not exist";
          SCDC_FAIL("do_cmd_rm: removing entry '" << entry_path << "' failed: " << result);
          return false;
        }

        if (remove(entry_path.c_str()) != 0)
        {
          result = string("remove failed: ") + strerror(errno);
          SCDC_FAIL("do_cmd_rm: removing entry '" << entry_path << "' failed: " << result);
          return false;
        }
      }

      return true;
    }

  private:
    bool admin;
};


#undef SCDC_LOG_PREFIX


#define SCDC_LOG_PREFIX  "dataprov-fs-store: "


scdc_dataprov_fs2_store::scdc_dataprov_fs2_store()
  :scdc_dataprov_fs2("fs:store")
{
}


bool scdc_dataprov_fs2_store::open(const char *conf, scdc_args *args, scdc_result &result)
{
  SCDC_TRACE("open: conf: '" << conf << "'");

  if (!scdc_dataprov_fs2::open(conf, args, result))
  {
    SCDC_FAIL("open: opening base");
    return false;
  }

  dataset_cmds_add("pwd", static_cast<dataset_cmds_do_cmd_f>(&scdc_dataset_fs2_store::do_cmd_pwd));
  dataset_cmds_add("info", static_cast<dataset_cmds_do_cmd_f>(&scdc_dataset_fs2_store::do_cmd_info));
  dataset_cmds_add("cd", static_cast<dataset_cmds_do_cmd_f>(&scdc_dataset_fs2_store::do_cmd_cd));
  dataset_cmds_add("ls", static_cast<dataset_cmds_do_cmd_f>(&scdc_dataset_fs2_store::do_cmd_ls));
  dataset_cmds_add("put", static_cast<dataset_cmds_do_cmd_f>(&scdc_dataset_fs2_store::do_cmd_put));
  dataset_cmds_add("get", static_cast<dataset_cmds_do_cmd_f>(&scdc_dataset_fs2_store::do_cmd_get));
  dataset_cmds_add("rm", static_cast<dataset_cmds_do_cmd_f>(&scdc_dataset_fs2_store::do_cmd_rm));

  return true;
}


scdc_dataset *scdc_dataprov_fs2_store::dataset_open(std::string &path, scdc_result &result)
{
  return scdc_dataprov_fs2::dataset_open<scdc_dataset_fs2_store>(path, result);
}


#undef SCDC_LOG_PREFIX
