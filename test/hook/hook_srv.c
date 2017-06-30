/*
 *  Copyright (C) 2014, 2015, 2016, 2017 Michael Hofmann
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
#include <stdint.h>
#include <signal.h>

#include "scdc.h"

#include "hook.h"


#define PREFIX  "hook_srv: "


scdc_nodeport_t np_tcp = SCDC_NODEPORT_NULL;
scdc_nodeport_t np_uds = SCDC_NODEPORT_NULL;
scdc_nodeport_t np_stream = SCDC_NODEPORT_NULL;


#define NODEPORT_TCP     SCDC_NODEPORT_START_ASYNC_UNTIL_CANCEL
#define NODEPORT_UDS     SCDC_NODEPORT_START_ASYNC_UNTIL_CANCEL
/*#define NODEPORT_STREAM  SCDC_NODEPORT_START_LOOP_UNTIL_CANCEL*/


void sighandler(int sig)
{
  printf(PREFIX "sighandler: sig: %d\n", sig);

  scdcint_t interrupt = (sig != 0);

#if NODEPORT_TCP
  scdc_nodeport_cancel(np_tcp, interrupt);
#endif
#if NODEPORT_STREAM
  scdc_nodeport_cancel(np_stream, interrupt);
#endif
}


scdcint_t cmd_handler(void *data, const char *cmd, const char *params, scdcint_t params_size)
{
  printf(PREFIX "cmd_handler: cmd: '%s', data: %p\n", cmd, data);

  if (strcmp(cmd, "quit") == 0)
  {
    printf(PREFIX "quiting server\n");
    sighandler(0);

  } else
  {
    printf(PREFIX "unkown server command: '%s'\n", cmd);
  }

  return SCDC_SUCCESS;
}


scdc_nodeport_cmd_handler_f *dummy_cmd_handler = cmd_handler;


int main(int argc, char *argv[])
{
  scdc_dataprov_t dp_hook = SCDC_DATAPROV_NULL;


  printf(PREFIX "start hook demo server\n");

  signal(SIGABRT, &sighandler);
  signal(SIGTERM, &sighandler);
  signal(SIGINT, &sighandler);

  scdc_init(SCDC_INIT_DEFAULT);

  dp_hook = scdc_dataprov_open("hookdemo", "hook:id", &hook, 0x2501);

#if NODEPORT_TCP
  np_tcp = scdc_nodeport_open("tcp:max_connections", 2);
#endif
#if NODEPORT_UDS
  np_uds = scdc_nodeport_open("uds:max_connections", 2);
#endif
#if NODEPORT_STREAM
  np_stream = scdc_nodeport_open("stream:cmd_handler", cmd_handler, NULL);
#endif

#if NODEPORT_TCP
  scdc_nodeport_start(np_tcp, NODEPORT_TCP);
#endif
#if NODEPORT_UDS
  scdc_nodeport_start(np_uds, NODEPORT_UDS);
#endif
#if NODEPORT_STREAM
  scdc_nodeport_start(np_stream, NODEPORT_STREAM);
#endif

  printf(PREFIX "Press <ENTER> to quit!\n");
  getchar();

#if NODEPORT_STREAM
  scdc_nodeport_stop(np_stream);
#endif
#if NODEPORT_UDS
  scdc_nodeport_stop(np_uds);
#endif
#if NODEPORT_TCP
  scdc_nodeport_stop(np_tcp);
#endif

#if NODEPORT_STREAM
  scdc_nodeport_close(np_stream);
#endif
#if NODEPORT_UDS
  scdc_nodeport_close(np_uds);
#endif
#if NODEPORT_TCP
  scdc_nodeport_close(np_tcp);
#endif

  scdc_dataprov_close(dp_hook);

  scdc_release();

  printf(PREFIX "quit hook demo server\n");

  return 0;
}
