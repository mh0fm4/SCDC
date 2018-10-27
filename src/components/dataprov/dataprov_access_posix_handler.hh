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


#ifndef __DATAPROV_ACCESS_POSIX_HANDLER_HH__
#define __DATAPROV_ACCESS_POSIX_HANDLER_HH__


#include <string>

#include "dataprov.hh"


template <class POSIX_HANDLER>
class scdc_dataprov_access_posix_handler: public POSIX_HANDLER
{
  public:
    static const char * const type;

    typedef std::string *dir_t;
    static const dir_t dir_null;

    struct file_data_t;
    typedef file_data_t *file_t;
    static const file_t file_null;

    bool open_config_conf(const std::string &conf, scdc_args *args, bool &done);

    bool open_conf(std::string &conf, scdc_args *args, scdc_result &result);
    bool open(scdc_result &result);
    bool close(scdc_result &result);

    dir_t dir_open(const char *path);
    void dir_close(dir_t dir);
    bool dir_path(dir_t dir, std::string &path);

    bool ls_entries(dir_t dir, std::string &result);
    bool info_entry(dir_t dir, const char *path, std::string &result);
    bool rm_entry(dir_t dir, const char *path);

    bool mk_dir(dir_t dir, const char *path);

    file_t file_open(dir_t dir, const char *path, bool read, bool write, bool create);
    file_t file_reopen(dir_t dir, const char *path, bool read, bool write, bool create, file_t file);
    void file_close(dir_t dir, file_t file);
    bool file_match(dir_t dir, file_t file, const char *path);

    static const bool HAVE_file_read_access_at = false;
    bool file_read_access_at(dir_t dir, file_t file, scdcint_t size, scdcint_t pos, scdc_buf_t &buf);
    scdcint_t file_read_at(dir_t dir, file_t file, void *ptr, scdcint_t size, scdcint_t pos);
    scdcint_t file_write_at(dir_t dir, file_t file, const void *ptr, scdcint_t size, scdcint_t pos);

  protected:
    bool make_path(const std::string &dir_path, const std::string &entry_path, std::string &path);
    bool dir_make_path(const dir_t dir, const std::string &entry_path, std::string &path);
};


#endif /* __DATAPROV_ACCESS_POSIX_HANDLER_HH__ */
