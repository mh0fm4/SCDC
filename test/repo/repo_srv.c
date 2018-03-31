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
#include <signal.h>
#include <inttypes.h>
#include <stdint.h>

#include "scdc.h"

#include "repoH.h"


scdc_nodeport_t np_direct = SCDC_NODEPORT_NULL;
scdc_nodeport_t np_tcp = SCDC_NODEPORT_NULL;
scdc_nodeport_t np_uds = SCDC_NODEPORT_NULL;
scdc_nodeport_t np_mpi = SCDC_NODEPORT_NULL;
scdc_nodeport_t np_timer = SCDC_NODEPORT_NULL;
scdc_nodeport_t np_stream = SCDC_NODEPORT_NULL;


#define NODEPORT_DIRECT  0
#define NODEPORT_TCP     SCDC_NODEPORT_START_ASYNC_UNTIL_CANCEL
#define NODEPORT_UDS     SCDC_NODEPORT_START_ASYNC_UNTIL_CANCEL
#define NODEPORT_MPI     SCDC_NODEPORT_START_ASYNC_UNTIL_CANCEL
#define NODEPORT_TIMER   SCDC_NODEPORT_START_ASYNC_UNTIL_CANCEL
#define NODEPORT_STREAM  SCDC_NODEPORT_START_LOOP_UNTIL_CANCEL


void sighandler(int sig)
{
  printf("sighandler: sig: %d\n", sig);

  scdcint_t interrupt = (sig != 0);

#if NODEPORT_DIRECT
  scdc_nodeport_cancel(np_direct, interrupt);
#endif
#if NODEPORT_TCP
  scdc_nodeport_cancel(np_tcp, interrupt);
#endif
#if NODEPORT_UDS
  scdc_nodeport_cancel(np_uds, interrupt);
#endif
#if NODEPORT_MPI
  scdc_nodeport_cancel(np_uds, interrupt);
#endif
#if NODEPORT_TIMER
  scdc_nodeport_cancel(np_timer, interrupt);
#endif
#if NODEPORT_STREAM
  scdc_nodeport_cancel(np_stream, interrupt);
#endif
}


scdcint_t cmd_handler(void *data, const char *cmd, const char *params, scdcint_t params_size)
{
  printf("cmd_handler: cmd: '%s', params: '%s', data: %p\n", cmd, params, data);

  if (strcmp(cmd, "quit") == 0)
  {
    printf("quiting server\n");
    sighandler(0);

  } else
  {
    printf("unkown server command: '%s'\n", cmd);
  }

  return SCDC_SUCCESS;
}


scdc_nodeport_cmd_handler_f *dummy_cmd_handler = cmd_handler;


scdcint_t timer_handler(void *data)
{
  printf("TIMER: TICK #%" scdcint_fmt "\n", *((scdcint_t *) data));

  *((scdcint_t *) data) += 1;

  return SCDC_SUCCESS;
}


int main(int argc, char *argv[])
{
#define DATAPROVS_MAX  16
  int ndataprovs = 0;
  scdc_dataprov_t dataprovs[DATAPROVS_MAX];
  char path[256], *e;
#if NODEPORT_TIMER
  scdcint_t t = 0;
#endif


  printf("start repository server\n");

  signal(SIGABRT, &sighandler);
  signal(SIGTERM, &sighandler);
  signal(SIGINT, &sighandler);

  scdc_init(SCDC_INIT_DEFAULT);

  dataprovs[ndataprovs++] = scdc_dataprov_open("repoA", "fs", (e = getenv("MERGE_SCDC_REPO_PATH"), sprintf(path, "%s%s", (e?e:""), "A"), path));
  dataprovs[ndataprovs++] = scdc_dataprov_open("repoB", "fs", (e = getenv("MERGE_SCDC_REPO_PATH"), sprintf(path, "%s%s", (e?e:""), "B"), path));
  dataprovs[ndataprovs++] = scdc_dataprov_open("repoC", "gen:");
/*  dataprovs[ndataprovs++] = scdc_dataprov_open("repoD", "mysql", getenv("MERGE_SCDC_MYSQL_CREDENTIALS"));*/
  dataprovs[ndataprovs++] = scdc_dataprov_open("repoH", "hook", &repoH, 0x2501);
  dataprovs[ndataprovs++] = scdc_dataprov_open("repoJ", "jobrun", "uname -a; sleep 1", (e = getenv("MERGE_SCDC_REPO_PATH"), sprintf(path, "%s%s", (e?e:""), "J"), path));
  dataprovs[ndataprovs++] = scdc_dataprov_open("repoR", "jobrun_relay:relay:relay", "relA scdc:///repoA", "relB scdc:///repoB");
  dataprovs[ndataprovs++] = scdc_dataprov_open("repoS", "fs:store", (e = getenv("MERGE_SCDC_REPO_PATH"), sprintf(path, "%s%s", (e?e:""), "S"), path));

#if NODEPORT_DIRECT
  np_direct = scdc_nodeport_open("direct", "directaccess");
#endif
#if NODEPORT_TCP
  np_tcp = scdc_nodeport_open("tcp:max_connections", 2);
#endif
#if NODEPORT_UDS
  np_uds = scdc_nodeport_open("uds");
#endif
#if NODEPORT_MPI
#if 1
  np_mpi = scdc_nodeport_open("mpi:port", NULL);
#else
  np_mpi = scdc_nodeport_open("mpi:publ", "TESTNAME");
#endif
#endif
#if NODEPORT_TIMER
  np_timer = scdc_nodeport_open("timer:max_count", (double) 1.0, timer_handler, &t, 50);
#endif
#if NODEPORT_STREAM
  np_stream = scdc_nodeport_open("stream:cmd_handler", cmd_handler, NULL);
#endif

#if NODEPORT_DIRECT
  scdc_nodeport_start(np_direct, NODEPORT_DIRECT);
#endif
#if NODEPORT_TCP
  scdc_nodeport_start(np_tcp, NODEPORT_TCP);
#endif
#if NODEPORT_UDS
  scdc_nodeport_start(np_uds, NODEPORT_UDS);
#endif
#if NODEPORT_MPI
  scdc_nodeport_start(np_mpi, NODEPORT_MPI);
#endif
#if NODEPORT_TIMER
  scdc_nodeport_start(np_timer, NODEPORT_TIMER);
#endif
#if NODEPORT_STREAM
  scdc_nodeport_start(np_stream, NODEPORT_STREAM);
#endif

#if NODEPORT_STREAM
  scdc_nodeport_stop(np_stream);
#endif
#if NODEPORT_TIMER
  scdc_nodeport_stop(np_timer);
#endif
#if NODEPORT_MPI
  scdc_nodeport_stop(np_mpi);
#endif
#if NODEPORT_UDS
  scdc_nodeport_stop(np_uds);
#endif
#if NODEPORT_TCP
  scdc_nodeport_stop(np_tcp);
#endif
#if NODEPORT_DIRECT
  scdc_nodeport_stop(np_direct);
#endif

#if NODEPORT_STREAM
  scdc_nodeport_close(np_stream);
#endif
#if NODEPORT_TIMER
  scdc_nodeport_close(np_timer);
#endif
#if NODEPORT_MPI
  scdc_nodeport_close(np_mpi);
#endif
#if NODEPORT_UDS
  scdc_nodeport_close(np_uds);
#endif
#if NODEPORT_TCP
  scdc_nodeport_close(np_tcp);
#endif
#if NODEPORT_DIRECT
  scdc_nodeport_close(np_direct);
#endif

  while (ndataprovs > 0) scdc_dataprov_close(dataprovs[--ndataprovs]);

  scdc_release();

  printf("quit repository server\n");

  return 0;
}
