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
#include <memory.h>

#include "scdc.h"

#include "multi.h"
#include "multi_hook.h"


#define MULTI_LOG_PREFIX  "multi_cli: "


int main(int argc, char *argv[])
{
  MULTI_TRACE("start multi demo client");

  scdc_init(SCDC_INIT_DEFAULT);

  scdc_dataprov_t dp_hook = dp_hook = scdc_dataprov_open(MULTI_BASE, "hook", &multidemo_hook);

  scdc_dataset_input_t input;
  scdc_dataset_output_t output;

  scdc_dataset_input_unset(&input);
  scdc_dataset_output_unset(&output);

#if SCDC_DATASET_INOUT_BUF_MULTIPLE
  char input_buf_m[2 * sizeof(scdc_buf_t)];
  char input_buf_0[] = "aaa";
  char input_buf_1[] = "zzzzz";

  SCDC_DATASET_INOUT_MBUF_INIT(&input, input_buf_m, sizeof(input_buf_m));
  SCDC_DATASET_INOUT_MBUF_SET_C(&input, 2);
  SCDC_DATASET_INOUT_MBUF_M_SET(&input, 0, input_buf_0, sizeof(input_buf_0), strlen(input_buf_0));
  SCDC_DATASET_INOUT_MBUF_M_SET(&input, 1, input_buf_1, sizeof(input_buf_1), strlen(input_buf_1));
#endif
/*
  char format[SCDC_FORMAT_MAX_SIZE];
  scdcint_t buf_size;
  void *buf;
  scdcint_t total_size, current_size;
  char total_size_given;
  scdc_dataset_inout_next_f *next;
  void *data;
*/

  MULTI_TRACE("before:");
  MULTI_TRACE_N("  "); scdc_dataset_input_print(&input); MULTI_TRACE_NL("");
  MULTI_TRACE_N("  "); scdc_dataset_output_print(&output); MULTI_TRACE_NL("");

  scdc_dataset_cmd(SCDC_DATASET_NULL,
#if 0
    "scdc:/"
#else
    "scdc+uds://multi/"
#endif
    MULTI_BASE " CMD PARAM1 PARAM2 PARAM3", &input, &output);

  MULTI_TRACE("after:");
  MULTI_TRACE_N("  "); scdc_dataset_output_print(&output); MULTI_TRACE_NL("");
  MULTI_TRACE("  output buf: '%.*s'", (int) SCDC_DATASET_INOUT_BUF_SIZE(&output), (char *) SCDC_DATASET_INOUT_BUF_PTR(&output));

  while (output.next) output.next(&output, 0);

  scdc_dataprov_close(dp_hook);

  scdc_release();

  MULTI_TRACE("quit multi demo client");

  return 0;
}
