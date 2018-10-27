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


#include <stdlib.h>
#include <string.h>

#include "scdc.h"

#define TRACE_PREFIX  "compC: "

#include "common.h"
#include "compC.h"


#define GMSH  "/usr/bin/gmsh"
static char temp_dir[128] = { "/tmp/tmp.XXXXXX" };


static const char *make_temp_file(char *s, const char *filename)
{
  sprintf(s, "%s/%s", temp_dir, filename);

  return s;
}


static scdcint_t compC_hook_dataset_cmd(void *dataprov, void *dataset, const char *cmd, const char *params, scdc_dataset_input_t *input, scdc_dataset_output_t *output, scdc_result_t *result)
{
  TRACE_F("cmd: '%s', params: '%s'", cmd, params);

  char s[1024], uri_in[256], uri_out[256];

  char *c = strchr(params, ' ');
  strncpy(uri_in, params, c - params);
  strcpy(uri_out, c + 1);

  const char *file_in, *file_out;
  c = strrchr(uri_in, '/'); *c = '\0'; file_in = c + 1;
  c = strrchr(uri_out, '/'); *c = '\0'; file_out = c + 1;

  char temp_in[256], temp_out[256], temp_log[256];
  make_temp_file(temp_in, file_in);
  make_temp_file(temp_out, file_out);
  make_temp_file(temp_log, "gmsh.log");

  scdc_dataset_output_t _outp, *outp = &_outp;
  scdc_dataset_input_t _inp, *inp = &_inp;

  /* step C1: get from compS */
  sprintf(s, "%s get %s", uri_in, file_in);
  TRACE_F("cmd: '%s'", s);
  outp = scdc_dataset_output_create(outp, "file", temp_in);
  scdc_dataset_cmd(SCDC_DATASET_NULL, s, NULL, outp);
  scdc_dataset_output_destroy(outp);

  /* step C2: convert with gmsh */
  sprintf(s, "%s -0 -o %s %s >%s 2>&1", GMSH, temp_out, temp_in, temp_log);
  TRACE_F("system: '%s'", s);
  system(s);

  /* step C3: put to compS */
  sprintf(s, "%s put %s", uri_out, file_out);
  TRACE_F("cmd: '%s'", s);
  inp = scdc_dataset_input_create(inp, "file", temp_out);
  scdc_dataset_cmd(SCDC_DATASET_NULL, s, inp, NULL);
  scdc_dataset_input_destroy(inp);

  remove(temp_in);
  remove(temp_out);
  remove(temp_log);

  return SCDC_SUCCESS;
}


static scdc_dataprov_hook_t compC_hooks = {
  NULL, NULL,
  NULL,
  NULL, NULL,
  NULL, NULL,
  compC_hook_dataset_cmd,
};


static scdc_dataprov_t compC_dp = SCDC_DATAPROV_NULL;

void compC_init()
{
  /* create temporary directory */
  mkdtemp(temp_dir);

  compC_dp = scdc_dataprov_open("compC", "hook", &compC_hooks);
}


void compC_release()
{
  scdc_dataprov_close(compC_dp);

  /* delete temporary directory */
  remove(temp_dir);
}
