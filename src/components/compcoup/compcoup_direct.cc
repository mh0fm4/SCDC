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


#include <pthread.h>

#include "config.hh"
#include "log.hh"
#include "dataset.hh"
#include "dataprov_pool.hh"
#include "compcoup_direct.hh"


using namespace std;


#define SCDC_LOG_PREFIX  "compcoup-direct: "


scdc_compcoup_direct::scdc_compcoup_direct()
  : scdc_compcoup(), data(0), running(false)
{
}


scdc_compcoup_direct::~scdc_compcoup_direct()
{
}


scdc_dataset *scdc_compcoup_direct::dataset_open(const char *path, scdcint_t path_size, scdc_dataset_output_t *output)
{
  SCDC_TRACE("dataset_open: '" << string(path, path_size) << "'");

  if (!running)
  {
    SCDC_DATASET_OUTPUT_PRINTF(output, "direct disabled");
    return 0;
  }

  if (!dataprovs)
  {
    SCDC_DATASET_OUTPUT_PRINTF(output, "no data provider available");
    return 0;
  }

  scdc_dataset *dataset = dataprovs->dataset_open(path, path_size, output);

  return dataset;
}


void scdc_compcoup_direct::dataset_close(scdc_dataset *dataset, scdc_dataset_output_t *output)
{
  SCDC_TRACE("dataset_close: '" << dataset << "'");

  if (!dataprovs)
  {
    SCDC_DATASET_OUTPUT_PRINTF(output, "no data provider available");
    return;
  }

  dataprovs->dataset_close(dataset, output);
}


bool scdc_compcoup_direct::dataset_cmd(const char *cmd, scdcint_t cmd_size, scdc_dataset_input_t *input, scdc_dataset_output_t *output)
{
  SCDC_TRACE("dataset_cmd: '" << string(cmd, cmd_size) << "'");

  if (!running)
  {
    SCDC_DATASET_OUTPUT_PRINTF(output, "direct disabled");
    return 0;
  }

  return scdc_compcoup::dataset_cmd(cmd, cmd_size, input, output);
}


bool scdc_compcoup_direct::start(scdcint_t mode)
{
  SCDC_TRACE("start: mode: '" << mode << "'");

  if (running) return false;

  if (mode == SCDC_NODEPORT_START_LOOP_UNTIL_CANCEL)
  {
    pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_lock(&lock);

    running = true;

    pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

    data = static_cast<void *>(&cond);
    pthread_cond_wait(&cond, &lock);
    data = 0;

    pthread_cond_destroy(&cond);

    running = false;

    pthread_mutex_unlock(&lock);
    pthread_mutex_destroy(&lock);

  } else if (mode == SCDC_NODEPORT_START_ASYNC_UNTIL_CANCEL)
  {
    running = true;

  } else
  {
    SCDC_ERROR("start: mode '" << mode << "' not supported");
    return false;
  }

  return true;
}


bool scdc_compcoup_direct::stop()
{
  SCDC_TRACE("stop");

  if (!running) return false;

  cancel(true);

  return true;
}


bool scdc_compcoup_direct::cancel(bool interrupt)
{
  SCDC_TRACE("cancel: interrupt: '" << interrupt << "'");

  if (!running) return false;

  if (data)
  {
    pthread_cond_t *cond = static_cast<pthread_cond_t *>(data);
    pthread_cond_signal(cond);
  }

  running = false;

  return true;
}


#undef SCDC_LOG_PREFIX
