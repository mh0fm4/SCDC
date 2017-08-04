/*
 *  Copyright (C) 2014, 2015, 2016, 2017 Michael Hofmann
 *  Copyright (C) 2017 Thomas Weber
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


#ifndef __DATAPROV_NFS_HH__
#define __DATAPROV_NFS_HH__


#include <string>

#include "dataprov.hh"


class scdc_dataprov_nfs_store: public scdc_dataprov
{
  public:
    scdc_dataprov_nfs_store();

    virtual bool open(const char *conf, scdc_args *args);
    virtual void close();

    virtual scdc_dataset *dataset_open(const char *path, scdcint_t path_size, scdc_dataset_output_t *output);
    virtual void dataset_close(scdc_dataset *dataset, scdc_dataset_output_t *output);

  friend class scdc_dataset_nfs_store;

  protected:
    struct nfs_context *_nfs_context;

    const char *_libnfs_err(void);
};


#endif /* __DATAPROV_NFS_HH__ */
