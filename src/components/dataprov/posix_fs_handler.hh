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


#ifndef __POSIX_FS_HANDLER_HH__
#define __POSIX_FS_HANDLER_HH__


#include <dirent.h>

#include <vector>
#include <string>

#include "config.hh"
#include "result.hh"


class scdc_posix_fs_handler
{
  public:
    static const char * const type;

    typedef DIR *dir_t;
    static const dir_t dir_null;

    typedef int file_t;
    static const file_t file_null;

    bool open_config_conf(const std::string &conf, scdc_args *args, bool &done);
    bool open_conf(std::string &conf, scdc_args *args, scdc_result &result);
    bool open(scdc_result &result);
    bool close(scdc_result &result);

    bool dir_exists(const char *path);
    bool dir_mk(const char *path);
    bool dir_rm(const char *path);
    dir_t dir_open(const char *path);
    bool dir_close(dir_t dir);
    dir_t dir_rewind(dir_t dir, const char *path);
    scdcint_t dir_count_entries(dir_t dir);
    scdcint_t dir_count_dirs(dir_t dir);
    bool dir_list_dirs(dir_t dir, std::vector<std::string> &dirs);
    scdcint_t dir_count_files(dir_t dir);
    bool dir_list_files(dir_t dir, std::vector<std::string> &files);

    scdcint_t file_get_size(const char *path);
    bool file_rm(const char *path);
    file_t file_open(const char *path, int flags);
    file_t file_reopen(const char *path, int flags, file_t file, const char *old_path);
    bool file_close(file_t file);
    scdcint_t file_seek(file_t file, scdcint_t offset, int whence);
    scdcint_t file_read(file_t file, void *buf, scdcint_t count);
    scdcint_t file_write(file_t file, const void *buf, scdcint_t count);

  private:
    std::string root_dir;
};


#endif /* __POSIX_FS_HANDLER_HH__ */
