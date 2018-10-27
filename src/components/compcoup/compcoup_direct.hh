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


#ifndef __COMPCOUP_DIRECT_HH__
#define __COMPCOUP_DIRECT_HH__


#include "compcoup.hh"


class scdc_compcoup_direct: public scdc_compcoup
{
  public:
    scdc_compcoup_direct();
    virtual ~scdc_compcoup_direct();

    virtual scdc_dataset *dataset_open(const std::string &path, scdc_result &result);
    virtual bool dataset_close(scdc_dataset *dataset, scdc_result &result);
    virtual bool dataset_cmd(const std::string &cmd, scdc_dataset_input_t *input, scdc_dataset_output_t *output, scdc_result &result);

    virtual bool start(scdcint_t mode);
    virtual bool stop();
    virtual bool cancel(bool interrupt);
/*    virtual bool resume();*/

  private:
    void *data;
    bool running;
};


#endif /* __COMPCOUP_DIRECT_HH__ */
