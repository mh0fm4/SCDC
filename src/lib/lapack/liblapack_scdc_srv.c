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

#include "scdc.h"

#include "lapack_scdc.h"


#define PREFIX  "liblapack_scdc_srv: "

#define LIBLAPACK_SCDC_PATH  "lapack"


int main(int argc, char *argv[])
{
  scdc_init(SCDC_INIT_DEFAULT);

  printf(PREFIX "dataprov close\n");
  scdc_dataprov_t dp = scdc_dataprov_open("lapack", "hook", &lapack_scdc_hook);

  printf(PREFIX "nodeport open\n");
  scdc_nodeport_t np = scdc_nodeport_open("uds:socketname:max_connections", "liblapack_scdc", 2);
/*  scdc_nodeport_t np = scdc_nodeport_open("tcp:max_connections", 2);*/

  printf(PREFIX "nodeport start\n");
  scdc_nodeport_start(np, SCDC_NODEPORT_START_ASYNC_UNTIL_CANCEL);

  printf(PREFIX "Press <ENTER> to quit!\n");
  getchar();

  printf(PREFIX "nodeport stop\n");
  scdc_nodeport_stop(np);

  printf(PREFIX "nodeport close\n");
  scdc_nodeport_close(np);

  printf(PREFIX "dataprov close\n");
  scdc_dataprov_close(dp);

  scdc_release();

  return 0;
}
