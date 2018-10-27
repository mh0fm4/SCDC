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

#define TRACE_PREFIX  "proc: "

#include "common.h"

#if USE_COMPU
#include "compU.h"
#endif
#if USE_COMPS
#include "compS.h"
#endif
#if USE_COMPC
#include "compC.h"
#endif


int main(int argc, char **argv)
{
  TRACE_F("init");

  scdc_init(SCDC_INIT_DEFAULT);

#if USE_COMPS
  compS_init();
#endif

#if USE_COMPC
  compC_init();
#endif

#if USE_COMPU
  compU_run();
#elif defined(USE_COMPS) || defined(USE_COMPC)
  TRACE_F("press ENTER to exit");
  getchar();
#endif

#if USE_COMPS
  compS_release();
#endif

#if USE_COMPC
  compC_release();
#endif

  TRACE_F("release");

  scdc_release();

  return 0;
}