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


#define SCDC_TRACE_NOT  !SCDC_TRACE_DATAPROV_NFS

#include "config.hh"
#include "common.hh"
#include "log.hh"
#include "dataprov_nfs.hh"

#include "dataprov_access.tcc"
#include "dataprov_access_posix_handler.tcc"
#include "dataprov_store.tcc"
#include "dataprov_store_posix_handler.tcc"
#include "posix_nfs_handler.hh"


using namespace std;


/* instantiate the template classes in this compile unit */
template class scdc_dataprov_access_posix_handler<scdc_posix_nfs_handler>;
template class scdc_dataprov_access<scdc_dataprov_access_posix_handler<scdc_posix_nfs_handler> >;
template class scdc_dataprov_store_posix_handler<scdc_posix_nfs_handler>;
template class scdc_dataprov_store<scdc_dataprov_store_posix_handler<scdc_posix_nfs_handler> >;
