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

#include "multi.h"
#include "multi_hook.h"

#define MULTI_LOG_PREFIX  "multi_hook: "


char output_buf_m[4 * sizeof(scdc_buf_t)];
char output_buf_0[] = "4444";
char output_buf_1[] = "333";
char output_buf_2[] = "22";
char output_buf_3[] = "1";


scdcint_t multidemo_dataset_cmd(void *dataprov, void *dataset, const char *cmd, const char *params, scdc_dataset_input_t *input, scdc_dataset_output_t *output, scdc_result_t *result)
{
  MULTI_TRACE("dataset_cmd:");
  MULTI_TRACE("  dataprov: '%p'", dataprov);
  MULTI_TRACE("  dataset: '%p'", dataset);
  MULTI_TRACE("  cmd: '%s'", cmd);
  MULTI_TRACE("  params: '%s'", params);

  MULTI_TRACE("before:");
  MULTI_TRACE_N("  "); scdc_dataset_input_print(input); MULTI_TRACE_NL("");
  MULTI_TRACE("  input buf: '%.*s'", (int) SCDC_DATASET_INOUT_BUF_SIZE(input), (char *) SCDC_DATASET_INOUT_BUF_PTR(input));
  MULTI_TRACE_N("  "); scdc_dataset_output_print(output); MULTI_TRACE_NL("");

  SCDC_DATASET_INOUT_MBUF_INIT(output, output_buf_m, sizeof(output_buf_m));
  SCDC_DATASET_INOUT_MBUF_SET_C(output, 4);
  SCDC_DATASET_INOUT_MBUF_M_SET(output, 0, output_buf_0, sizeof(output_buf_0), strlen(output_buf_0));
  SCDC_DATASET_INOUT_MBUF_M_SET(output, 1, output_buf_1, sizeof(output_buf_1), strlen(output_buf_1));
  SCDC_DATASET_INOUT_MBUF_M_SET(output, 2, output_buf_2, sizeof(output_buf_2), strlen(output_buf_2));
  SCDC_DATASET_INOUT_MBUF_M_SET(output, 3, output_buf_3, sizeof(output_buf_3), strlen(output_buf_3));

  MULTI_TRACE("after:");
  MULTI_TRACE_N("  "); scdc_dataset_output_print(output); MULTI_TRACE_NL("");

  return SCDC_SUCCESS;
}


const scdc_dataprov_hook_t multidemo_hook = {
  0, /* open */
  0, /* close */
  0, /* config */
  0, /* dataset_open */
  0, /* dataset_close */
  0, /* dataset_open_read_state */
  0, /* dataset_close_write_state */
  multidemo_dataset_cmd,
};

#undef PREFIX
