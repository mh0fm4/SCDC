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


#include <pthread.h>
#include <cerrno>

#include "config.hh"
#include "common.hh"
#include "log.hh"
#include "args.hh"
#include "nodeport_timer.hh"


using namespace std;


#define SCDC_LOG_PREFIX  "nodeport-timer: "


scdc_nodeport_timer::scdc_nodeport_timer()
  :scdc_nodeport("timer", SCDC_NODEPORT_START_LOOP_UNTIL_CANCEL|SCDC_NODEPORT_START_ASYNC_UNTIL_CANCEL),
  timeout(0), max_count(-1)
{
  timer_handler_args.handler = 0;
  timer_handler_args.data = 0;
}


scdc_nodeport_timer::~scdc_nodeport_timer()
{
}


bool scdc_nodeport_timer::open_config_conf(const std::string &conf, scdc_args *args, bool &done)
{
  done = true;
  bool ret = true;

  if (conf == "max_count")
  {
    if (args->get<scdcint_t>(SCDC_ARGS_TYPE_SCDCINT, &max_count) == SCDC_ARG_REF_NULL)
    {
      SCDC_ERROR("open_config_conf: getting max. count");
      ret = false;

    } else
    {
      SCDC_TRACE("open_config_conf: max_count: '" << max_count << "'");
    }

  } else
  {
    done = false;
    ret = scdc_nodeport::open_config_conf(conf, args, done);
  }

  return ret;
}


bool scdc_nodeport_timer::open(const char *conf, scdc_args *args)
{
  SCDC_TRACE("open: conf: '" << conf << "'");

  bool ret = true;

  args = open_args_init(args);

  if (args->get<double>(SCDC_ARGS_TYPE_DOUBLE, &timeout) == SCDC_ARG_REF_NULL)
  {
    SCDC_ERROR("open: getting timeout");
    ret = false;
    goto do_quit;
  }

  if (args->get<scdc_nodeport_timer_handler_args_t>(SCDC_ARGS_TYPE_NODEPORT_TIMER_HANDLER, &timer_handler_args, true) == SCDC_ARG_REF_NULL)
  {
    SCDC_ERROR("open: getting timer handler");
    ret = false;
    goto do_quit;
  }

  if (!scdc_nodeport::open(conf, args))
  {
    SCDC_FAIL("open: opening base");
    ret = false;
  }

  open_args_clear();

do_quit:
  if (!ret) open_args_release();

  return ret;
}


void scdc_nodeport_timer::close()
{
  SCDC_TRACE("close:");

  scdc_nodeport::close();

  open_args_release();
}


static void *timer_server_run(void *arg)
{
  scdc_nodeport_timer *nodeport_timer = static_cast<scdc_nodeport_timer *>(arg);

  nodeport_timer->loop();

  return NULL;
}


bool scdc_nodeport_timer::start(scdcint_t mode)
{
  SCDC_TRACE("start: ");

  if (!scdc_nodeport::start(mode)) return false;

  last_time = z_time_wtime();
  current_count = 0;

  loop_run = true;

  loop_async = false;

  pthread_mutex_init(&loop_lock, NULL);
  pthread_cond_init(&loop_cond, NULL);

  if (mode & SCDC_NODEPORT_START_LOOP_UNTIL_CANCEL) loop();
  else
  {
    loop_async = true;
    pthread_create(&loop_thread, NULL, timer_server_run, this);
  }

  return true;
}


bool scdc_nodeport_timer::stop()
{
  SCDC_TRACE("stop: ");

  cancel(false);

  if (loop_async) pthread_join(loop_thread, NULL);

  pthread_cond_destroy(&loop_cond);
  pthread_mutex_destroy(&loop_lock);

  scdc_nodeport::stop();

  return true;
}


bool scdc_nodeport_timer::cancel(bool interrupt)
{
  SCDC_TRACE("cancel: ");

  pthread_mutex_lock(&loop_lock);
  loop_run = false;
  pthread_mutex_unlock(&loop_lock);
  pthread_cond_signal(&loop_cond);

  return true;
}


/*bool scdc_nodeport_timer::resume()
{
  return false;
}*/


void scdc_nodeport_timer::loop()
{
  SCDC_TRACE("loop: ");

  pthread_mutex_lock(&loop_lock);
  while (loop_run)
  {
    struct timespec abstime;
    abstimeout(&abstime, timeout, last_time);

    if (pthread_cond_timedwait(&loop_cond, &loop_lock, &abstime) == ETIMEDOUT)
    {
      last_time = z_time_wtime();

      /* unlock, so that timer_handler can do cancel */
      pthread_mutex_unlock(&loop_lock);

      timer_handler_args.handler(timer_handler_args.data);

      pthread_mutex_lock(&loop_lock);

      ++current_count;

      if (max_count >= 0 && current_count >= max_count) loop_run = false;
    }
  }
  pthread_mutex_unlock(&loop_lock);
}


#undef SCDC_LOG_PREFIX
