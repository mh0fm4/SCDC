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

#include "redirect_test_config.h"
#include "common.h"
#include "redirect_test.h"


#define TRACE_PREFIX  "redirect_srv: "


int main(int argc, char *argv[])
{
  scdc_init(SCDC_INIT_DEFAULT);

  TRACE_F("dataprov open");
  scdc_dataprov_t dp = scdc_dataprov_open(REDIRECT_TEST_SCDC_PATH, "hook", &redirect_test_hook);

  TRACE_F("nodeport open");
  scdc_nodeport_t np = scdc_nodeport_open("uds:socketname:max_connections", "redirect", 2);
/*  scdc_nodeport_t np = scdc_nodeport_open("tcp:max_connections", 2);*/

  TRACE_F("nodeport start");
  scdc_nodeport_start(np, SCDC_NODEPORT_START_ASYNC_UNTIL_CANCEL);

  printf(TRACE_PREFIX "Press <ENTER> to quit!\n");
  getchar();

  TRACE_F("nodeport stop");
  scdc_nodeport_stop(np);

  TRACE_F("nodeport close");
  scdc_nodeport_close(np);

  TRACE_F("dataprov close");
  scdc_dataprov_close(dp);

  scdc_release();

  return 0;
}
