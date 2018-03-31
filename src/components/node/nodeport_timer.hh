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


#ifndef __NODEPORT_TIMER_HH__
#define __NODEPORT_TIMER_HH__


#include <pthread.h>

#include "nodeport.hh"


class scdc_nodeport_timer: public scdc_nodeport
{
  public:
    scdc_nodeport_timer();
    virtual ~scdc_nodeport_timer();

    virtual bool open_config_conf(const std::string &conf, scdc_args *args, bool &done);

    virtual bool open(const char *conf, scdc_args *args);
    virtual void close();

    virtual bool start(scdcint_t mode);
    virtual bool stop();
    virtual bool cancel(bool interrupt);
/*    virtual bool resume();*/

    void loop();

  private:
    double last_time, timeout;
    scdc_nodeport_timer_handler_args_t timer_handler_args;
    scdcint_t current_count, max_count;
    bool loop_run, loop_async;
    pthread_t loop_thread;
    pthread_mutex_t loop_lock;
    pthread_cond_t loop_cond;
};


#endif /* __NODEPORT_TIMER_HH__ */
