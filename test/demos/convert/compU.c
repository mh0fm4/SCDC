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

#define TRACE_PREFIX  "compU: "

#include "common.h"
#include "compU.h"


void compU_run()
{
  const char *file_in = "porsche.stl";
  const char *file_out = "porsche.msh";
  const char *compS_uri = "scdc:/compS/convert";
  const char *compC_uri = "scdc:/compC";

  char s[256];

  scdc_dataset_input_t _inp, *inp = &_inp;
  scdc_dataset_output_t _outp, *outp = &_outp;

  /* step U1: put to compS */
  sprintf(s, "%s put %s", compS_uri, file_in);
  TRACE_F("cmd: '%s'", s);
  inp = scdc_dataset_input_create(inp, "file", file_in);
  scdc_dataset_cmd(SCDC_DATASET_NULL, s, inp, NULL);
  scdc_dataset_input_destroy(inp);

  /* step U2: convert on compC */
  sprintf(s, "%s convert %s/%s %s/%s", compC_uri, compS_uri, file_in, compS_uri, file_out);
  TRACE_F("cmd: '%s'", s);
  scdc_dataset_cmd(SCDC_DATASET_NULL, s, NULL, NULL);

  /* step U3: get from compS */
  sprintf(s, "%s get %s", compS_uri, file_out);
  TRACE_F("cmd: '%s'", s);
  outp = scdc_dataset_output_create(outp, "file", file_out);
  scdc_dataset_cmd(SCDC_DATASET_NULL, s, NULL, outp);
  scdc_dataset_output_destroy(outp);
}
