/*
 *  Copyright (C) 2014, 2015, 2016, 2017 Michael Hofmann
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


#ifndef __DATAPROV_WEBDAV_HH__
#define __DATAPROV_WEBDAV_HH__


#include <string>

#include "dataprov.hh"


class scdc_dataprov_webdav_session_handler;


class scdc_dataprov_webdav_store: public scdc_dataprov
{
  public:

    /**
     * Session handler to manage the communication with the WebDAV server
     */
    scdc_dataprov_webdav_session_handler* session_handler;


    scdc_dataprov_webdav_store();
    virtual ~scdc_dataprov_webdav_store();

    /**
     * Open data store
     * @param conf
     * @param args
     * @return true on success, false otherwise
     */
    virtual bool open(const char *conf, scdc_args *args);

    /**
     * close data store
     */
    virtual void close();

    /**
     * open data set
     * @param path
     * @param path_size
     * @param output
     * @return scdc_dataset
     */
    virtual scdc_dataset *dataset_open(const char *path, scdcint_t path_size, scdc_dataset_output_t *output);

    /**
     * close data set
     * @param dataset
     * @param output
     */
    virtual void dataset_close(scdc_dataset *dataset, scdc_dataset_output_t *output);

    /**
    * Sets the parameter of the session handler.
    * @param config string formated like this:
    *
    * for http "protocol:host:base"
    *
    * for https "protocol:host:base:username:password"
    * @return false if an error occurred, true otherwise
    */
    bool set_session_handler_config(const std::string &conf);
};


#endif /* __DATAPROV_WEBDAV_HH__ */
