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


#ifndef __DATAPROV_POOL_HH__
#define __DATAPROV_POOL_HH__


#include <map>

#include "config.hh"
#include "args.hh"
#include "data.hh"
#include "dataset.hh"
#include "dataprov.hh"


class scdc_dataprov_pool: public std::map<std::string, scdc_dataprov *>
{
  public:
    scdc_dataprov_pool() { };

    scdc_dataprov *open(const char *base_path, const char *conf, scdc_args *args);
    void close(scdc_dataprov *dataprov);

    void close_all();

    bool exists(scdc_dataprov *dataprov);

    scdc_dataset *dataset_open(const char *path, scdcint_t path_size, scdc_dataset_output_t *output);
    void dataset_close(scdc_dataset *dataset, scdc_dataset_output_t *output);

    scdc_dataset *dataset_open_read_state(scdc_data *incoming, scdc_dataset_output_t *output);
    void dataset_close_write_state(scdc_dataset *dataset, scdc_data *outgoing, scdc_dataset_output_t *output);
};


#endif /* __DATAPROV_POOL_HH__ */
