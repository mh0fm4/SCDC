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


#include "scdc.h"

#define TRACE_PREFIX  "compS: "

#include "common.h"
#include "compS.h"


static scdc_dataprov_t compS_dp = SCDC_DATAPROV_NULL;

void compS_init()
{
  compS_dp = scdc_dataprov_open("compS", "store:fs", "store/");

  /* create 'convert' folder */
  scdc_dataset_cmd(SCDC_DATASET_NULL, "scdc:/compS/ADMIN put convert", NULL, NULL);
}


void compS_release()
{
#if 1
  /* delete 'convert' folder */
  scdc_dataset_cmd(SCDC_DATASET_NULL, "scdc:/compS/ADMIN rm convert", NULL, NULL);
#endif

  scdc_dataprov_close(compS_dp);
}
