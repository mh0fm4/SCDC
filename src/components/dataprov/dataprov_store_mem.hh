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


#ifndef __DATAPROV_STORE_MEM_HH__
#define __DATAPROV_STORE_MEM_HH__


#include <string>
#include <list>

#include "dataprov_store.hh"


class scdc_dataprov_store_mem_handler
{
  public:
    static const char * const type;

    bool open_config_conf(const std::string &conf, scdc_args *args, bool &done);

    bool open_conf(std::string &conf, scdc_args *args, scdc_result &result);
    bool open(scdc_result &result);
    bool close(scdc_result &result);

    struct entry_data_t;
    typedef entry_data_t *entry_t;
    static const entry_t entry_null;

    struct store_data_t;
    typedef store_data_t *store_t;
    static const store_t store_null;

    bool ls_stores(std::string &result);
    bool info_store(const char *path, std::string &result);
    bool mk_store(const char *path);
    bool rm_store(const char *path);

    store_t store_open(const char *path);
    void store_close(store_t store);

    bool ls_entries(store_t store, std::string &result);
    bool info_entry(store_t store, const char *path, std::string &result);
    bool rm_entry(store_t store, const char *path);

    entry_t entry_open(store_t store, const char *path, bool read, bool write, bool create);
    entry_t entry_reopen(store_t store, const char *path, bool read, bool write, bool create, entry_t entry);
    void entry_close(store_t store, entry_t entry);
    bool entry_match(store_t store, entry_t entry, const char *path);

    static const bool HAVE_entry_read_access_at = true;
    bool entry_read_access_at(store_t store, entry_t entry, scdcint_t size, scdcint_t pos, scdc_buf_t &buf);
    scdcint_t entry_read_at(store_t store, entry_t entry, void *ptr, scdcint_t size, scdcint_t pos);
    scdcint_t entry_write_at(store_t store, entry_t entry, const void *ptr, scdcint_t size, scdcint_t pos);

  private:
    typedef std::list<entry_t> entries_t;
    typedef std::list<store_t> stores_t;
    stores_t stores;

    store_t add_store(const char *path);
    void del_store(store_t store);
    store_t find_store(const char *path);
    void clear_stores();

    entry_t add_entry(store_t store, const char *path);
    void del_entry(store_t store, entry_t entry);
    entry_t find_entry(store_t store, const char *path);
    void clear_entries(store_t store);
};


typedef scdc_dataprov_store<scdc_dataprov_store_mem_handler> scdc_dataprov_store_mem;


#endif /* __DATAPROV_STORE_MEM_HH__ */
