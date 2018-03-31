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


int main(int argc, char *argv[])
{
  --argc; ++argv;

  scdc_init(SCDC_INIT_DEFAULT);

  scdc_dataprov_t dp = scdc_dataprov_open("relay_service", "relay");

  /* configure relay from (local) 'scdc+tcp://.../relay_service/local_path' to (target)'scdc+tcp://target_hostname/target_path' */
  const char *cmd = "scdc:/relay_service/CONFIG put relay local_path scdc+tcp://target_hostname/target_path";

  scdc_dataset_cmd(SCDC_DATASET_NULL, cmd, NULL, NULL);

  scdc_nodeport_t np = scdc_nodeport_open("tcp");

  scdc_nodeport_start(np, SCDC_NODEPORT_START_ASYNC_UNTIL_CANCEL);

  printf("running: ENTER to exit");
  getchar();
  printf("quiting\n");

  scdc_nodeport_stop(np);

  scdc_nodeport_close(np);

  scdc_dataprov_close(dp);

  scdc_release();

  return 0;
}
