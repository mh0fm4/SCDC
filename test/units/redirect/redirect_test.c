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
#include <stdlib.h>
#include <string.h>

#define REDIRECT_TEST_REMOTE  1

#include "redirect_test_config.h"
#include "common.h"
#include "redirect_call.h"
#include "redirect_common.h"
#include "redirect_test.h"
#include "redirect_test_timing.h"


#define TRACE_PREFIX  "redirect_test: "


#if REDIRECT_TEST_TIMING_REMOTE
# undef REDIRECT_TEST_TIMING_PREFIX
# define REDIRECT_TEST_TIMING_PREFIX  TRACE_PREFIX  "TIMING: "
# define REDIRECT_TEST_TIMING_REMOTE_X  5
int redirect_test_timing_remote_i;
double redirect_test_timing_remote[REDIRECT_TEST_TIMING_REMOTE_X];
#endif
#define REDIRECT_TEST_TIMING_INIT_()  REDIRECT_TEST_TIMING_INIT(redirect_test_timing_remote, redirect_test_timing_remote_i, LIBREDIRECT_TEST_TIMING_REMOTE_X)

#if REDIRECT_TEST_TIMING_REDIRECT_REMOTE
# define REDIRECT_TEST_TIMING_REMOTE_PUT(_lc_, _t_)  REDIRECT_TEST_CALL(put_output_conf_double)(_lc_, "TIMING", _t_);
#else
# define REDIRECT_TEST_TIMING_REMOTE_PUT(_lc_, _t_)  Z_NOP()
#endif


static void do_result(redirect_call_t *rc)
{
  TRACE_F("rc: %p", rc);

  REDIRECT_TEST_TIMING_INIT_();

  const char *result = "This is a test result with special character '|' inside!";
  redirect_call_set_result_str(rc, result);

  TRACE_F("result: '%s'", result);

  TRACE_F("return");
}


static void do_conf(redirect_call_t *rc)
{
  TRACE_F("rc: %p", rc);

  REDIRECT_TEST_TIMING_INIT_();

  char input_char = 0;
  redirect_call_get_input_conf_char(rc, "char", &input_char);

  int input_int = 0;
  redirect_call_get_input_conf_int(rc, "int", &input_int);

  void *input_void_p = NULL;
  redirect_call_get_input_conf_void_p(rc, "void_p", &input_void_p);

  char input_str[256];
  rdint_t input_str_n = 0;
  redirect_call_get_input_conf_str(rc, "str", input_str, &input_str_n);

  int input_ints[] = { 0, 0 };
  redirect_call_get_input_conf_ints(rc, "int", input_ints, 2);

  long output_long = 0;
  redirect_call_get_output_conf_long(rc, "int", &output_long);

  TRACE_F("input: char: %c, int: %d, void_p: %p, str: '%.*s' (%" rdint_fmt "), ints: [%d, %d]", input_char, input_int, input_void_p, (int) input_str_n, input_str, input_str_n, input_ints[0], input_ints[1]);
  TRACE_F("output: long: %ld", output_long);

  char output_char = '8';
  redirect_call_put_output_conf_char(rc, "char", output_char);

  int output_int = 8;
  redirect_call_put_output_conf_int(rc, "int", output_int);

  void *output_void_p = (void *) 0x88888888;
  redirect_call_put_output_conf_void_p(rc, "void_p", output_void_p);

  const char *output_str = "8 8 8 8 8 8 8 8";
  rdint_t output_str_n = strlen(output_str);
  redirect_call_put_output_conf_str(rc, "str", output_str, output_str_n);

  int output_ints[] = { 8, 88 };
  redirect_call_put_output_conf_ints(rc, "int", output_ints, 2);

  TRACE_F("output: char: %c, int: %d, void_p: %p, str: '%.*s' (%" rdint_fmt "), ints: [%d, %d]", output_char, output_int, output_void_p, (int) output_str_n, output_str, output_str_n, output_ints[0], output_ints[1]);

  redirect_call_set_result_str(rc, "OK");
  TRACE_F("result: '%s'", redirect_call_get_result_str(rc, NULL, 0));

  TRACE_F("return");
}


static void do_cache(redirect_call_t *rc, int step)
{
  TRACE_F("rc: %p", rc);

  REDIRECT_TEST_TIMING_INIT_();

  rdint_t in_n0 = 0;
  double *in_v0 = NULL;
  redirect_call_get_input_conf_rdint(rc, "in_n0", &in_n0);
  redirect_call_get_input_param_array_double(rc, "in_v0", &in_v0, &in_n0);
  printf("in_v0: %p, in_n0: %" rdint_fmt "\n", in_v0, in_n0);
  redirect_call_print_param_array_double(in_v0, in_n0);

  if (step == '0')
  {
    rdint_t out_n = 0;
    double *out_v = NULL;
    redirect_call_get_input_conf_rdint(rc, "out_n", &out_n);
    redirect_call_get_output_param_array_double(rc, "out_v", &out_v, &out_n);
    printf("out_v: %p, out_n: %" rdint_fmt "\n", out_v, out_n);
    vector_init(out_v, out_n, 1, -1.0, NAN);
    redirect_call_print_param_array_double(out_v, out_n);
    redirect_call_put_output_param_array_double(rc, "out_v", out_v, out_n);
  }

  if (step == '1')
  {
    rdint_t in_n1 = 0;
    double *in_v1 = NULL;
    redirect_call_get_input_conf_rdint(rc, "in_n1", &in_n1);
    redirect_call_get_input_param_array_double(rc, "in_v1", &in_v1, &in_n1);
    printf("in_v1: %p, in_n1: %" rdint_fmt "\n", in_v1, in_n1);
    redirect_call_print_param_array_double(in_v1, in_n1);

    rdint_t in_n2 = 0;
    double *in_v2 = NULL;
    redirect_call_get_input_conf_rdint(rc, "in_n2", &in_n2);
    redirect_call_get_input_param_array_double(rc, "in_v2", &in_v2, &in_n2);
    printf("in_v2: %p, in_n2: %" rdint_fmt "\n", in_v2, in_n2);
    redirect_call_print_param_array_double(in_v2, in_n2);
  }

  redirect_call_set_result_str(rc, "OK");
  TRACE_F("result: '%s'", redirect_call_get_result_str(rc, NULL, 0));

  TRACE_F("return");
}


static void do_param_array(redirect_call_t *rc)
{
  TRACE_F("rc: %p", rc);

  REDIRECT_TEST_TIMING_INIT_();

  rdint_t in_n = 0;
  double *in_v = NULL;
  redirect_call_get_input_conf_rdint(rc, "in_n", &in_n);
  redirect_call_get_input_param_array_double(rc, "in_v", &in_v, &in_n);
  printf("in_v: %p, in_n: %" rdint_fmt "\n", in_v, in_n);
  redirect_call_print_param_array_double(in_v, in_n);

  rdint_t out_n = 0;
  double *out_v = NULL;
  redirect_call_get_input_conf_rdint(rc, "out_n", &out_n);
  redirect_call_get_output_param_array_double(rc, "out_v", &out_v, &out_n);
  printf("out_v: %p, out_n: %" rdint_fmt "\n", out_v, out_n);
  vector_init(out_v, out_n, 1, -1.0, NAN);
  redirect_call_print_param_array_double(out_v, out_n);
  redirect_call_put_output_param_array_double(rc, "out_v", out_v, out_n);

  redirect_call_set_result_str(rc, "OK");
  TRACE_F("result: '%s'", redirect_call_get_result_str(rc, NULL, 0));

  TRACE_F("return");
}


static void do_param_vector(redirect_call_t *rc)
{
  TRACE_F("rc: %p", rc);

  REDIRECT_TEST_TIMING_INIT_();

  /* input */
#if REDIRECT_TEST_PARAM_VECTOR_INPUT
  rdint_t in_n0 = 0, in_inc0 = 0;
  double *in_v0 = NULL;
  redirect_call_get_input_conf_rdint(rc, "in_n0", &in_n0);
  redirect_call_get_input_param_vector_double(rc, "in_v0", &in_v0, &in_n0, &in_inc0);
  printf("in_v0: %p, in_n0: %" rdint_fmt ", in_inc0: %" rdint_fmt "\n", in_v0, in_n0, in_inc0);
  redirect_call_print_param_vector_double(in_v0, in_n0, in_inc0);

  rdint_t in_n1 = 0, in_inc1 = 0;
  double *in_v1 = NULL;
  redirect_call_get_input_conf_rdint(rc, "in_n1", &in_n1);
  redirect_call_get_input_param_vector_double(rc, "in_v1", &in_v1, &in_n1, &in_inc1);
  printf("in_v1: %p, in_n1: %" rdint_fmt ", in_inc1: %" rdint_fmt "\n", in_v1, in_n1, in_inc1);
  redirect_call_print_param_vector_double(in_v1, in_n1, in_inc1);
#endif /* REDIRECT_TEST_PARAM_VECTOR_INPUT */

  /* output */
#if REDIRECT_TEST_PARAM_VECTOR_OUTPUT
  rdint_t out_n0 = 0, out_inc0 = 1;
  double *out_v0 = NULL;
  redirect_call_get_input_conf_rdint(rc, "out_n0", &out_n0);
  redirect_call_get_output_param_vector_double(rc, "out_v0", &out_v0, &out_n0, &out_inc0);
  printf("out_v0: %p, out_n0: %" rdint_fmt ", out_inc0: %" rdint_fmt "\n", out_v0, out_n0, out_inc0);
  vector_init(out_v0, out_n0, out_inc0, -1.0, NAN);
  redirect_call_print_param_vector_double(out_v0, out_n0, out_inc0);
  redirect_call_put_output_param_vector_double(rc, "out_v0", out_v0, out_n0, out_inc0);

  rdint_t out_n1 = 0, out_inc1 = 2;
  double *out_v1 = NULL;
  redirect_call_get_input_conf_rdint(rc, "out_n1", &out_n1);
  redirect_call_get_output_param_vector_double(rc, "out_v1", &out_v1, &out_n1, &out_inc1);
  printf("out_v1: %p, out_n1: %" rdint_fmt ", out_inc1: %" rdint_fmt "\n", out_v1, out_n1, out_inc1);
  vector_init(out_v1, out_n1, out_inc1, -2.0, NAN);
  redirect_call_print_param_vector_double(out_v1, out_n1, out_inc1);
  redirect_call_put_output_param_vector_double(rc, "out_v1", out_v1, out_n1, out_inc1);
#endif /* REDIRECT_TEST_PARAM_VECTOR_OUTPUT */

  /* inout */
#if REDIRECT_TEST_PARAM_VECTOR_INOUT
  rdint_t inout_n0 = 0, inout_inc0 = 0;
  double *inout_v0 = NULL;
  redirect_call_get_input_conf_rdint(rc, "inout_n0", &inout_n0);
  redirect_call_get_inout_param_vector_double(rc, "inout_v0", &inout_v0, &inout_n0, &inout_inc0);
  printf("inout_v0: %p, inout_n0: %" rdint_fmt ", inout_inc0: %" rdint_fmt "\n", inout_v0, inout_n0, inout_inc0);
  redirect_call_print_param_vector_double(inout_v0, inout_n0, inout_inc0);
  vector_init(inout_v0, inout_n0, inout_inc0, -3.0, NAN);
  redirect_call_print_param_vector_double(inout_v0, inout_n0, inout_inc0);
  redirect_call_put_output_param_vector_double(rc, "inout_v0", inout_v0, inout_n0, inout_inc0);

  rdint_t inout_n1 = 0, inout_inc1 = 0;
  double *inout_v1 = NULL;
  redirect_call_get_input_conf_rdint(rc, "inout_n1", &inout_n1);
  redirect_call_get_inout_param_vector_double(rc, "inout_v1", &inout_v1, &inout_n1, &inout_inc1);
  printf("inout_v1: %p, inout_n1: %" rdint_fmt ", inout_inc1: %" rdint_fmt "\n", inout_v1, inout_n1, inout_inc1);
  redirect_call_print_param_vector_double(inout_v1, inout_n1, inout_inc1);
  vector_init(inout_v1, inout_n1, inout_inc1, -4.0, NAN);
  redirect_call_print_param_vector_double(inout_v1, inout_n1, inout_inc1);
  redirect_call_put_output_param_vector_double(rc, "out_v1", inout_v1, inout_n1, inout_inc1);
#endif /* REDIRECT_TEST_PARAM_VECTOR_INOUT */

  redirect_call_set_result_str(rc, "OK");
  TRACE_F("result: '%s'", redirect_call_get_result_str(rc, NULL, 0));

  TRACE_F("return");
}


static void do_param_matrix(redirect_call_t *rc)
{
  TRACE_F("rc: %p", rc);

  REDIRECT_TEST_TIMING_INIT_();

  redirect_call_set_result_str(rc, "OK");
  TRACE_F("result: '%s'", redirect_call_get_result_str(rc, NULL, 0));

  TRACE_F("return");
}


void *redirect_test_open(const char *conf, va_list ap)
{
  TRACE_F("conf: '%s'", conf);

  redirect_call_t *rc = malloc(sizeof(redirect_call_t)
#if REDIRECT_CALL_PARAMS_CACHE
    + sizeof(redirect_cache_t)
#endif
    );

#if REDIRECT_CALL_PARAMS_CACHE
  redirect_cache_t *rch = (redirect_cache_t *) (rc + 1);

  redirect_cache_init(rch);
#endif

  TRACE_F("return: %p", rc);

  return rc;
}


scdcint_t redirect_test_close(void *dataprov)
{
  TRACE_F("dataprov: %p", dataprov);

  redirect_call_t *rc = dataprov;

#if REDIRECT_CALL_PARAMS_CACHE
  redirect_cache_t *rch = (redirect_cache_t *) (rc + 1);

  redirect_cache_release(rch);
#endif

  free(rc);

  TRACE_F("return");

  return SCDC_SUCCESS;
}


/*void *redirect_test_dataset_open(void *dataprov, const char *path)
{
  TRACE_F("%s:redirect_test_dataset_open:\n");
  TRACE_F("%s:  dataprov: '%p'\n", dataprov);
  TRACE_F("%s:  path: '%s'\n", path);

  lapack_call_t *lc = malloc(sizeof(lapack_call_t));

  TRACE_F("%s:  lc: '%p'\n", path);

  return lc;
}


scdcint_t redirect_test_dataset_close(void *dataprov, void *dataset)
{
  TRACE_F("%s:redirect_test_dataset_close:\n");
  TRACE_F("%s:  dataprov: '%p'\n", dataprov);
  TRACE_F("%s:  dataset: '%p'\n", dataset);

  lapack_call_t *lc = dataset;

  free(lc);

  return SCDC_SUCCESS;
}*/


scdcint_t redirect_test_dataset_cmd(void *dataprov, void *dataset, const char *cmd, const char *params, scdc_dataset_input_t *input, scdc_dataset_output_t *output, scdc_result_t *result)
{
  TRACE_F("dataprov: %p, dataset: %p, cmd: '%s', params: '%s', input: %p, output: %p, result: %p", dataprov, dataset, cmd, params, input, output, result);

  printf("input: "); scdc_dataset_input_print(input); printf("\n");
  printf("output: "); scdc_dataset_output_print(output); printf("\n");

  redirect_call_t *rc = dataprov;

  redirect_call_init_scdc(rc);

#if REDIRECT_CALL_PARAMS_CACHE
  redirect_cache_t *rch = (redirect_cache_t *) (rc + 1);

  redirect_call_set_cache_ptr(rc, rch);
#endif

  if (redirect_call_cmd(rc, cmd, params, input, output, result))
  {
    if (strcmp(cmd, "result") == 0) do_result(rc); else
    if (strcmp(cmd, "conf") == 0) do_conf(rc); else
    if (strncmp(cmd, "cache", 5) == 0) do_cache(rc, cmd[5]); else
    if (strcmp(cmd, "param_array") == 0) do_param_array(rc); else
    if (strcmp(cmd, "param_vector") == 0) do_param_vector(rc); else
    if (strcmp(cmd, "param_matrix") == 0) do_param_matrix(rc); else
    {
      TRACE_F("command '%s' not supported!", cmd);
    }
  }

  redirect_call_release_scdc(rc);

  TRACE_F("return");

  return SCDC_SUCCESS;
}
