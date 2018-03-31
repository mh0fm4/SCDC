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


#define PREFIX  "hook_direct: "


scdcint_t hookdemo_dataset_cmd(void *dataprov, void *dataset, const char *cmd, const char *params, scdc_dataset_input_t *input, scdc_dataset_output_t *output)
{
  printf(PREFIX "dataset_cmd:\n");
  printf(PREFIX "  dataprov: '%p'\n", dataprov);
  printf(PREFIX "  dataset: '%p'\n", dataset);
  printf(PREFIX "  cmd: '%s'\n", cmd);
  printf(PREFIX "  params: '%s'\n", params);

  printf(PREFIX "  "); scdc_dataset_input_print(input); printf("\n");

  printf(PREFIX "  "); scdc_dataset_output_print(output); printf("\n");

  printf(PREFIX "\n");

  return SCDC_SUCCESS;
}


const static scdc_dataprov_hook_t hookdemo = {
  0, /* open */
  0, /* close */
  0, /* config */
  0, /* dataset_open */
  0, /* dataset_close */
  0, /* dataset_open_read_state */
  0, /* dataset_close_write_state */
  hookdemo_dataset_cmd,
};


int main(int argc, char *argv[])
{
  scdc_dataprov_t dp_hook = SCDC_DATAPROV_NULL;


  printf(PREFIX "start hook demo direct\n");
  printf(PREFIX "\n");

  scdc_init(SCDC_INIT_DEFAULT);

  dp_hook = scdc_dataprov_open("hookdemo", "hook", &hookdemo);

  scdc_dataset_input_t input;
  scdc_dataset_output_t output;

  scdc_dataset_input_unset(&input);
  scdc_dataset_output_unset(&output);

/*
  char format[SCDC_FORMAT_MAX_SIZE];
  scdcint_t buf_size;
  void *buf;
  scdcint_t total_size, current_size;
  char total_size_given;
  scdc_dataset_inout_next_f *next;
  void *data;
*/

  scdc_dataset_cmd(SCDC_DATASET_NULL, "scdc:/hookdemo CMD PARAM1 PARAM2 PARAM3", &input, &output);

  scdc_dataprov_close(dp_hook);

  scdc_release();

  printf(PREFIX "quit hook demo direct\n");
  printf(PREFIX "\n");

  return 0;
}
