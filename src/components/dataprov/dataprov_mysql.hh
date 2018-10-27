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


#ifndef __DATAPROV_MYSQL_HH__
#define __DATAPROV_MYSQL_HH__


#include <pthread.h>

#include "dataprov.hh"


struct scdc_dataprov_mysql_data_t;


class scdc_dataprov_mysql_store: public scdc_dataprov
{
  public:
    scdc_dataprov_mysql_store();

    virtual bool open(const char *conf, scdc_args *args, scdc_result &result);
    virtual bool close(scdc_result &result);

    virtual scdc_dataset *dataset_open(std::string &path, scdc_result &result);
    virtual bool dataset_close(scdc_dataset *dataset, scdc_result &result);

    scdc_dataprov_mysql_data_t *aquire_access();
    void release_access(scdc_dataprov_mysql_data_t *mysql_data);

  protected:
    scdc_dataprov_mysql_data_t *mysql_data;
};


#endif /* __DATAPROV_MYSQL_HH__ */
