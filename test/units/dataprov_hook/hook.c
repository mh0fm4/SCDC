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


#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "z_pack.h"

#include "scdc.h"

#include "hook.h"


typedef void *hook_dataprov_t;
typedef void *hook_dataset_t;


void *hook_open(const char *conf, va_list ap)
{
  void *param = NULL;

  param = va_arg(ap, void *);

  printf("hook_open: conf: '%s', args: '%p'\n", conf, param);

  return (hook_dataprov_t) 0xDEADBEEF;
}


scdcint_t hook_close(void *dataprov)
{
  printf("hook_close: dataprov: '%p'\n", dataprov);
  
  return SCDC_SUCCESS;
}


scdcint_t hook_config(void *dataprov, const char *cmd, const char *param, const char *input, scdcint_t input_size, scdc_result_t *result)
{
  if (strncmp(cmd, "ls", 2) == 0)
  {
    printf("hook_config: listing parameters\n");

    SCDC_RESULT_SET_STR(result, "param0,param1,param2");

  } else if (strncmp(cmd, "put", 3) == 0)
  {
    printf("hook_config: setting '%s' to '%.*s'\n", param, (int) input_size, input);

  } else if (strncmp(cmd, "get", 3) == 0)
  {
    printf("hook_config: getting '%s'\n", param);

    snprintf(SCDC_RESULT_STR(result), SCDC_RESULT_SIZE(result), "%s_VAL", param);

  } else
  {
    return SCDC_FAILURE;
  }

  printf("%s: return %" scdcint_fmt ", result: '%s'\n", __func__, SCDC_SUCCESS, SCDC_RESULT_STR(result));

  return SCDC_SUCCESS;
}


void *hook_dataset_open(void *dataprov, const char *path, scdc_result_t *result)
{
  hook_dataset_t *dataset_hook;
  
  union { void *dataprov; hook_dataset_t dataset_hook; } z;


  printf("hook_dataset_open: dataprov: '%p', path: '%s'\n", dataprov, path);

  dataset_hook = (hook_dataset_t *) malloc(sizeof(hook_dataset_t));

  z.dataprov = dataprov;

  *dataset_hook = ((char *) z.dataset_hook) + 1;

  SCDC_RESULT_SET_STR(result, "result of hook_dataset_open");

  printf("hook_dataset_open: return '%p' (%p), result: '%s'\n", dataset_hook, *dataset_hook, SCDC_RESULT_STR(result));

  return dataset_hook;
}


scdcint_t hook_dataset_close(void *dataprov, void *dataset, scdc_result_t *result)
{
  hook_dataset_t *dataset_hook = (hook_dataset_t *) dataset;


  printf("hook_dataset_close: dataprov: '%p', dataset: '%p' (%p)\n", dataprov, dataset_hook, *dataset_hook);

  free(dataset_hook);

  SCDC_RESULT_SET_STR(result, "result of hook_dataset_close");

  printf("hook_dataset_close: return '%" scdcint_fmt "', result: '%s'\n", SCDC_SUCCESS, SCDC_RESULT_STR(result));
  
  return SCDC_SUCCESS;
}


void *hook_dataset_open_read_state(void *dataprov, const void *state, scdcint_t state_size, scdc_result_t *result)
{
  hook_dataset_t *dataset_hook;


  printf("hook_dataset_open_read_state: dataprov: '%p', state: '%.*s'\n", dataprov, (int) state_size, (const char *) state);

  dataset_hook = (hook_dataset_t *) malloc(sizeof(hook_dataset_t));

  z_snscanf(state, state_size, "%p", dataset_hook);

  SCDC_RESULT_SET_STR(result, "result of hook_dataset_open_read_state");

  printf("hook_dataset_open_read_state: return: '%p' (%p), result: '%s'\n", dataset_hook, *dataset_hook, SCDC_RESULT_STR(result));

  return dataset_hook;
}


scdcint_t hook_dataset_close_write_state(void *dataprov, void *dataset, void *state, scdcint_t state_size, scdc_result_t *result)
{
  scdcint_t n;
  hook_dataset_t *dataset_hook = (hook_dataset_t *) dataset;


  printf("hook_dataset_close_write_state: dataset: '%p' (%p)\n", dataset_hook, *dataset_hook);

  n = snprintf(state, state_size, "%p", *dataset_hook);

  free(dataset_hook);

  SCDC_RESULT_SET_STR(result, "result of hook_dataset_close_write_state");

  printf("hook_dataset_close_write_state: return: '%" scdcint_fmt "', result: '%s', state: '%.*s'\n", n, SCDC_RESULT_STR(result), (int) state_size, (const char *) state);

  return n;
}


void print_dataset_input(const char *prefix, scdc_dataset_input_t *input)
{
  printf("%sinput: ", prefix);
  if (input) scdc_dataset_input_print(input);
  else printf("none\n");
}


void print_dataset_output(const char *prefix, scdc_dataset_output_t *output)
{
  printf("%soutput: ", prefix);
  if (output) scdc_dataset_output_print(output);
  else printf("none\n");
}


#define DEFAULT_GET_SIZE  13

scdcint_t hook_dataset_get_next(scdc_dataset_output_t *output, scdc_result_t *result)
{
  ptrdiff_t x;


  printf("hook_dataset_get_next: output: '%p'\n", output);
  print_dataset_output("hook_dataset_get_next: ", output);

  x = (ptrdiff_t) output->data;

  SCDC_DATASET_INOUT_BUF_CURRENT(output) = z_min(SCDC_DATASET_INOUT_BUF_SIZE(output), DEFAULT_GET_SIZE);

#if 0
  memset(SCDC_DATASET_INOUT_BUF_PTR(output), 'x', SCDC_DATASET_INOUT_BUF_CURRENT(output) - 1);
  ((char *) SCDC_DATASET_INOUT_BUF_PTR(output))[SCDC_DATASET_INOUT_BUF_CURRENT(output) - 1] = '\0';
#else
  memset(SCDC_DATASET_INOUT_BUF_PTR(output), 'x', SCDC_DATASET_INOUT_BUF_CURRENT(output));
#endif

  --x;

  output->next = (x > 0)?hook_dataset_get_next:NULL;
  output->data = (void *) x;

  return SCDC_SUCCESS;
}


static void consume_input(const char *prefix, scdc_dataset_input_t *input)
{
  while (input)
  {
    printf("%sinput content: '%.*s'\n", prefix, (int) SCDC_DATASET_INOUT_BUF_CURRENT(input), (char *) SCDC_DATASET_INOUT_BUF_PTR(input));
    if (!input->next) break;
    printf("%snext input\n", prefix);
    input->next(input, 0);
    print_dataset_input(prefix, input);
  }
}


static void clear_output(scdc_dataset_output_t *output)
{
  if (!output) return;

  SCDC_DATASET_INOUT_BUF_CURRENT(output) = 0;
  output->total_size = 0;
  output->total_size_given = SCDC_DATASET_INOUT_TOTAL_SIZE_GIVEN_NONE;
  output->data = 0;
  output->next = 0;
}


scdcint_t hook_dataset_cmd(void *dataprov, void *dataset, const char *cmd, const char *params, scdc_dataset_input_t *input, scdc_dataset_output_t *output, scdc_result_t *result)
{
  ptrdiff_t x;

  printf("hook_dataset_cmd: dataprov: %p, dataset: %p, cmd: '%s', params: '%s', input: %p, output: %p, result: %p\n", dataprov, dataset, cmd, params, input, output, result);
  print_dataset_input("hook_dataset_cmd: ", input);
  print_dataset_output("hook_dataset_cmd: ", output);

  clear_output(output);

  if (strncmp(cmd, "get", 3) == 0)
  {
    if (strncmp(params, "in2out", 6) == 0)
    {
      if (input)
      {
        scdc_dataset_input_redirect(input, "to:output", output);
      }

    } else if (strncmp(params, "in4out", 6) == 0)
    {
      if (input)
      {
        scdc_dataset_output_redirect(output, "from:input", input);
      }

    } else if (strncmp(params, "in5out", 6) == 0)
    {
      /* stored */
      if (input)
      {
        scdc_dataset_input_redirect(input, "to:file", "hook_get.tmp");

        scdc_dataset_output_redirect(output, "from:file", "hook_get.tmp");
      }

    } else
    {
      consume_input("hook_dataset_cmd: ", input);

      if (strlen(params) > 0 && sscanf(params, "x%td", &x) == 1);
      else x = 2;

      strcpy(output->format, "text");

      output->total_size = x * DEFAULT_GET_SIZE;
      output->total_size_given = SCDC_DATASET_INOUT_TOTAL_SIZE_GIVEN_EXACT;

      output->data = (void *) x;

      hook_dataset_get_next(output, 0);
    }

  } else
  {
    consume_input("hook_dataset_cmd: ", input);

    strcpy(output->format, "text");

    SCDC_DATASET_INOUT_BUF_CURRENT(output) = sprintf(SCDC_DATASET_INOUT_BUF_PTR(output), "dataset: '%p', cmd: '%s', params: '%s'", dataset, cmd, params);
    output->total_size = SCDC_DATASET_INOUT_BUF_CURRENT(output);
    output->total_size_given = SCDC_DATASET_INOUT_TOTAL_SIZE_GIVEN_EXACT;
  }

  print_dataset_output("hook_dataset_cmd: ", output);

  SCDC_RESULT_SET_STR(result, "result of hook_dataset_cmd");
  printf("hook_dataset_cmd: return: %" scdcint_fmt ", result: '%s'", SCDC_SUCCESS, SCDC_RESULT_STR(result));

  return SCDC_SUCCESS;
}
