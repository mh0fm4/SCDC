/*
 *  Copyright (C) 2014, 2015, 2016 Michael Hofmann
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

    virtual scdc_dataset *dataset_open(const char *path, scdcint_t path_size, scdc_dataset_output_t *output);
    virtual void dataset_close(scdc_dataset *dataset, scdc_dataset_output_t *output);
    virtual bool dataset_cmd(const char *cmd, scdcint_t cmd_size, scdc_dataset_input_t *input, scdc_dataset_output_t *output);

    virtual bool start(scdcint_t mode);
    virtual bool stop();
    virtual bool cancel(bool interrupt);
/*    virtual bool resume();*/

  private:
    void *data;
    bool running;
};


#endif /* __COMPCOUP_DIRECT_HH__ */
