/*
 *  Copyright (C) 2014, 2015, 2016, 2017 Michael Hofmann
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
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "scdc.h"

#include "hook.h"


#define PREFIX  "hook_cli: "


scdcint_t input_next(scdc_dataset_input_t *input)
{
  scdcint_t i = (intptr_t) input->data;


  printf(PREFIX "preparing INPUT #%" scdcint_fmt "\n", i);

  ((char *) SCDC_DATASET_INOUT_BUF_PTR(input))[0] = 0;

  SCDC_DATASET_INOUT_BUF_CURRENT(input) = 0;

  if (i >= 1) input->next = NULL;

  input->data = (void *) (intptr_t) (i + 1);

  return SCDC_SUCCESS;
}


scdcint_t output_next(scdc_dataset_output_t *output)
{
  scdcint_t i = (intptr_t) output->data;


  if (strcmp(output->format, "text") == 0 || strcmp(output->format, "fslist") == 0)
    printf(PREFIX "processing OUTPUT #%" scdcint_fmt ": '%.*s'\n", i, (int) SCDC_DATASET_INOUT_BUF_CURRENT(output), (char *) SCDC_DATASET_INOUT_BUF_PTR(output));
  else
    printf(PREFIX "processing OUTPUT #%" scdcint_fmt": %" scdcint_fmt " bytes\n", i, SCDC_DATASET_INOUT_BUF_CURRENT(output));

  output->data = (void *) (intptr_t) (i + 1);

  return SCDC_SUCCESS;
}


void do_dataset_cmd(scdc_dataset_t dataset, const char *cmd)
{
  scdc_dataset_input_t input, output;
  int i;

#define DEFAULT_INPUT_BUF_SIZE   256
#define DEFAULT_OUTPUT_BUF_SIZE  256

  char input_buf[DEFAULT_INPUT_BUF_SIZE], output_buf[DEFAULT_OUTPUT_BUF_SIZE];


  /* prepare command input */
  scdc_dataset_input_unset(&input);

  strcpy(input.format, "data");

  SCDC_DATASET_INOUT_BUF_PTR(&input) = input_buf;
  SCDC_DATASET_INOUT_BUF_SIZE(&input) = DEFAULT_INPUT_BUF_SIZE;
  SCDC_DATASET_INOUT_BUF_CURRENT(&input) = 0;
  input.total_size = 0;
  input.total_size_given = SCDC_DATASET_INOUT_TOTAL_SIZE_GIVEN_AT_LEAST;

  input.next = input_next;
  input.data = (void *) 0;

  input.next(&input);

  /* prepare command output */
  scdc_dataset_output_unset(&output);

  SCDC_DATASET_INOUT_BUF_PTR(&output) = output_buf;
  SCDC_DATASET_INOUT_BUF_SIZE(&output) = DEFAULT_OUTPUT_BUF_SIZE;

  output.next = 0;
  output.data = (void *) 0;

  scdcint_t ret = scdc_dataset_cmd(dataset, cmd, &input, &output);

  /* process output */
  i = 0;
  do {
    void *data = output.data;
    output.data = (void *) (intptr_t) i;

    output_next(&output);

    i = (intptr_t) output.data;
    output.data = data;

    if (!output.next) break;

#if 0
    printf(PREFIX "Press <Enter>");
    getchar();
#endif

    output.next(&output);

  } while (1);

  printf(PREFIX "command '%s' %s!\n", cmd, ((ret == SCDC_SUCCESS)?"success":"failed"));
}


int main(int argc, char *argv[])
{
  scdc_dataprov_t dp_hook = SCDC_DATAPROV_NULL;

  scdc_dataset_t dataset = SCDC_DATASET_NULL;

#define OPTARG_MAX  256
  char scheme[OPTARG_MAX] = { '\0' }, authority[OPTARG_MAX] = { '\0' }, open[OPTARG_MAX] = { '\0' }, cmdline[OPTARG_MAX] = { '\0' };


  printf(PREFIX "start hook demo client\n");

#if 0
  strcpy(scheme, "scdc");
  strcpy(authority, "");
#elif 0
  strcpy(scheme, "scdc+udp");
  strcpy(authority, "");
#elif 1
  strcpy(scheme, "scdc+tcp");
  strcpy(authority, "localhost");
#endif
  strcpy(open, "hookdemo/");
  strcpy(cmdline, "ls");

  scdc_init(SCDC_INIT_DEFAULT);

  dp_hook = scdc_dataprov_open("hookdemo", "hook:id", &hook, 0x2501);

  if (strlen(scheme) > 0 || strlen(authority) > 0 || strlen(open) > 0)
  {
    char uri[1024];

    sprintf(uri, "%s://%s/%s", scheme, authority, open);

    printf(PREFIX "uri: '%s'\n", uri);

    dataset = scdc_dataset_open(uri);
  }

  printf(PREFIX "dataset: %p\n", dataset);

  do_dataset_cmd(dataset, cmdline);

  scdc_dataset_close(dataset);

  scdc_dataprov_close(dp_hook);

  scdc_release();

  printf(PREFIX "quit hook demo client\n");

  return 0;
}
