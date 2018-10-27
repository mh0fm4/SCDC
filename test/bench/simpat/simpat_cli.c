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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include "scdc.h"
#include "simpat.h"

#define SIMPAT_LOG_PREFIX  "cli: "


#define BUF_SIZE  1024 * 1024


double z_time_wtime()
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return (double) tv.tv_sec + (tv.tv_usec / 1000000.0);
}


int main(int argc, char *argv[])
{
  /* scdc.log_init("log_FILE", sys.stdout, sys.stderr)*/

  scdc_init(SCDC_INIT_DEFAULT);

  simpat_init();

  char uri_base[256], uri[512], cmd[512];

  uri_base[0] = '\0';

  if (argc <= 1)
  {
    SIMPAT_TRACE("test: direct");
    sprintf(uri_base, "scdc:");

  } else if (strcmp(argv[1], "uds") == 0)
  {
    SIMPAT_TRACE("test: UDS");

    const char *socketname = (argc <= 2)?"simpat":argv[2];

    sprintf(uri_base, "scdc+uds://%s", socketname);

  } else if (strcmp(argv[1], "tcp") == 0)
  {
    SIMPAT_TRACE("test: TCP");

    const char *hostname = (argc <= 2)?"localhost":argv[2];

    sprintf(uri_base, "scdc+tcp://%s", hostname);

  } else if (strcmp(argv[1], "mpi") == 0)
  {
    SIMPAT_TRACE("test: MPI");

    const char *rank = (argc <= 2)?"0":argv[2];

    sprintf(uri_base, "scdc+mpi://%s", rank);

  } else SIMPAT_TRACE("unknown mode: %s", argv[1]);


  int bw_put = 0;

  scdcint_t bw_size = 1000000000;

  int lat_ds = 1;
  int lat_ah = 1;
  scdcint_t lat_size = 1;
  scdcint_t lat_num = 100;

  if (strlen(uri_base) > 0)
  {
    const char *mode = "file/zero";

    /* bandwidth: put */
    if (bw_put)
    {
      sprintf(uri, "%s/%s/%s", uri_base, SIMPAT_BASE, mode);

      scdc_dataset_t ds = scdc_dataset_open(uri);

      scdc_dataset_input_t input_;
      scdc_dataset_input_t *input = scdc_dataset_input_create(&input_, "produce:size", bw_size, BUF_SIZE);

      sprintf(cmd, "put %" scdcint_fmt, bw_size);

      double t = z_time_wtime();
      int ret = scdc_dataset_cmd(ds, cmd, input, NULL);
      t = z_time_wtime() - t;

      scdc_dataset_input_destroy(input);

      scdc_dataset_close(ds);

      if (ret) SIMPAT_TRACE("bandwith put: %f", bw_size / t * 1e-6);
      else SIMPAT_TRACE("bandwith put: FAILED");
    }

/*  # bandwidth: get
  if bw_get:
    ds = scdc.dataset_open(uri + "/" + simpat.simpat_base + "/" + mode)

    output = output_init(bw_size)

    t = time.time()
    ret = scdc.dataset_cmd(ds, "get " + str(bw_size), None, output)
    t = time.time() - t

    output_release(output)

    scdc.dataset_close(ds)

    if ret:
      SIMPAT_TRACE("bandwith get: " + str(bw_size / t * 1e-6))
    else:
      SIMPAT_TRACE("bandwith get: FAILED")
*/

    /* latency: dataset put */
    if (lat_ds)
    {
      sprintf(uri, "%s/%s/%s", uri_base, SIMPAT_BASE, mode);

      scdc_dataset_t ds = scdc_dataset_open(uri);

      scdc_dataset_input_t input_;
      scdc_dataset_input_t *input = scdc_dataset_input_create(&input_, "produce:size", lat_size, BUF_SIZE);

      sprintf(cmd, "put %" scdcint_fmt, lat_size);

      double t = z_time_wtime();
      int i, ret = 1;
      for (i = 0; i < lat_num; ++i) ret = ret && scdc_dataset_cmd(ds, cmd, input, NULL);
      t = z_time_wtime() - t;

      scdc_dataset_input_destroy(input);

      scdc_dataset_close(ds);

      if (ret) SIMPAT_TRACE("latency dataset put: %f", t / lat_num * 1e3);
      else SIMPAT_TRACE("latency dataset put: FAILED");
    }

    /* latency: ad-hoc put */
    if (lat_ah)
    {
      scdc_dataset_cmd(SCDC_DATASET_NULL, "scdc:////CONFIG put nodeconn_cache_connections 0", NULL, NULL);

      scdc_dataset_input_t input_;
      scdc_dataset_input_t *input = scdc_dataset_input_create(&input_, "produce:size", lat_size, BUF_SIZE);

      sprintf(cmd, "%s/%s/%s put %" scdcint_fmt, uri_base, SIMPAT_BASE, mode, lat_size);

      double t = z_time_wtime();
      int i, ret = 1;
      for (i = 0; i < lat_num; ++i) ret = ret && scdc_dataset_cmd(SCDC_DATASET_NULL, cmd, input, NULL);
      t = z_time_wtime() - t;

      scdc_dataset_input_destroy(input);

      if (ret) SIMPAT_TRACE("latency ad-hoc put: %f", t / lat_num * 1e3);
      else SIMPAT_TRACE("latency ad-hoc put: FAILED");
    }
  }

  simpat_release();

  scdc_release();

  /*scdc.log_release()*/

  return 0;
}
