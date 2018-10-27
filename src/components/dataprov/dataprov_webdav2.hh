/*
 *  Copyright (C) 2014, 2015, 2016, 2017, 2018 Michael Hofmann
 *  Copyright (C) 2016, 2017 Eric Kunze
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


#ifndef __DATAPROV_WEBDAV2_HH__
#define __DATAPROV_WEBDAV2_HH__


#include <string>

#include "dataprov.hh"


class scdc_dataprov_webdav2_session_handler;


class scdc_dataprov_webdav2_access: public scdc_dataprov
{
  public:

    /**
     * Session handler to manage the communication with the WebDAV server
     */
    scdc_dataprov_webdav2_session_handler* session_handler;


    scdc_dataprov_webdav2_access();
    virtual ~scdc_dataprov_webdav2_access();

    /**
     * Open data store
     * @param conf
     * @param args
     * @return true on success, false otherwise
     */
    virtual bool open(const char *conf, scdc_args *args, scdc_result &result);

    /**
     * close data store
     */
    virtual bool close(scdc_result &result);

    /**
     * open data set
     * @param path
     * @param path_size
     * @param output
     * @return scdc_dataset
     */
    virtual scdc_dataset *dataset_open(std::string &path, scdc_result &result);

    /**
     * close data set
     * @param dataset
     * @param output
     */
    virtual bool dataset_close(scdc_dataset *dataset, scdc_result &result);
};


#endif /* __DATAPROV_WEBDAV2_HH__ */
