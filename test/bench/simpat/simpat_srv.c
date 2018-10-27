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
#include <string.h>

#include "scdc.h"
#include "simpat.h"

#define SIMPAT_LOG_PREFIX  "srv: "


int main(int argc, char *argv[])
{
  /* scdc.log_init("log_FILE", sys.stdout, sys.stderr)*/

  scdc_init(SCDC_INIT_DEFAULT);

  simpat_init();

  scdc_nodeport_t np = SCDC_NODEPORT_NULL;

  if (argc <= 1)
  {
    SIMPAT_TRACE("setup: none");

  } else if (strcmp(argv[1], "uds") == 0)
  {
    SIMPAT_TRACE("setup: UDS");

    const char *socketname = (argc <= 2)?"simpat":argv[2];

    np = scdc_nodeport_open("uds:socketname:max_connections", socketname, 2);

  } else if (strcmp(argv[1], "tcp") == 0)
  {
    SIMPAT_TRACE("setup: TCP");

    np = scdc_nodeport_open("tcp:max_connections", 2);

  } else if (strcmp(argv[1], "mpi") == 0)
  {
    SIMPAT_TRACE("setup: MPI");

    np = scdc_nodeport_open("mpi");

  } else SIMPAT_TRACE("unknown mode: %s", argv[1]);

  SIMPAT_TRACE("start");
  scdc_nodeport_start(np, SCDC_NODEPORT_START_ASYNC_UNTIL_CANCEL);

  printf("Press <ENTER> to quit!\n");
  getchar();

  SIMPAT_TRACE("stop");
  scdc_nodeport_stop(np);

  SIMPAT_TRACE("release");
  scdc_nodeport_close(np);

  simpat_release();

  scdc_release();

  /*scdc.log_release()*/

  return 0;
}
