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


#ifndef __DATAPROV_STORE_STUB_HH__
#define __DATAPROV_STORE_STUB_HH__


#include <string>

#include "dataprov_store.hh"


class scdc_dataprov_store_stub_handler
{
  public:
    static const char * const type;

    typedef void *entry_t;
    typedef void *store_t;

    static entry_t open_entry(const char *path);
    static void close_entry(entry_t entry);
};


typedef scdc_dataprov_store<scdc_dataprov_store_stub_handler> scdc_dataprov_store_stub;


#endif /* __DATAPROV_STORE_STUB_HH__ */
