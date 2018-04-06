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

#define SCDC_TRACE_NOT  !SCDC_TRACE_DATAPROV_STORE_STUB

#include "config.hh"
#include "common.hh"
#include "log.hh"
#include "dataprov_store.hh"
#include "dataprov_store.tcc"
#include "dataprov_store_stub.hh"


using namespace std;


template class scdc_dataprov_store<scdc_dataprov_store_stub_handler>;


#define SCDC_LOG_PREFIX  "dataprov-store-stub-handler: "

const char * const scdc_dataprov_store_stub_handler::type = "stub";


scdc_dataprov_store_stub_handler::entry_t scdc_dataprov_store_stub_handler::open_entry(const char *path)
{
  SCDC_TRACE("open_entry: path: '" << path << "'");
  return 0;
}


void scdc_dataprov_store_stub_handler::close_entry(scdc_dataprov_store_stub_handler::entry_t entry)
{
  SCDC_TRACE("open_entry: entry: '" << entry << "'");
}

#undef SCDC_LOG_PREFIX
