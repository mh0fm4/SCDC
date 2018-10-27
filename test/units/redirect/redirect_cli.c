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

#include "scdc.h"

#include "redirect_test_config.h"
#include "common.h"
#include "redirect_call.h"
#include "redirect_common.h"
#include "redirect_test.h"


#define TEST_HASH          0
#define TEST_RESULT        0
#define TEST_CONF          0
#define TEST_CACHE         0
#define TEST_PARAM_ARRAY   0
#define TEST_PARAM_VECTOR  1
#define TEST_PARAM_MATRIX  0



#define TRACE_PREFIX  "redirect_cli: "


void test_hash()
{
  rdint_t in_n = 250 * 1000 * 1000;
  double *in_v = malloc(in_n * sizeof(*in_v));
  vector_init(in_v, in_n, 1, 1.0, NAN);

  rdint_t size = in_n * sizeof(*in_v);

  double t = z_time_wtime();
  redirect_call_hash_param_array_double(in_v, in_n);
  t = z_time_wtime() - t;

  printf("test_hash: size: %" rdint_fmt ", time: %f, rate: %f\n", size, t, (double) size / t * 1e-6);

  free(in_v);
}


void test_result(const char *uri)
{
  redirect_call_t rc;

  redirect_call_create_scdc(&rc, "result", uri);

  redirect_call_execute(&rc);

  TRACE_F("result: '%s'", redirect_call_get_result_str(&rc, NULL, 0));

  redirect_call_destroy_scdc(&rc);
}


void test_conf(const char *uri)
{
  redirect_call_t rc;

  redirect_call_create_scdc(&rc, "conf", uri);

  char input_char = '7';
  redirect_call_put_input_conf_char(&rc, "char", input_char);

  int input_int = 7;
  redirect_call_put_input_conf_int(&rc, "int", input_int);

  void *input_void_p = (void *) 0x77777777;
  redirect_call_put_input_conf_void_p(&rc, "void_p", input_void_p);

  const char *input_str = "7 7 7 7 7 7 7 7";
  rdint_t input_str_n = strlen(input_str);
  redirect_call_put_input_conf_str(&rc, "str", input_str, input_str_n);

  int input_ints[2] = { 7, 77 };
  redirect_call_put_input_conf_ints(&rc, "ints", input_ints, 2);

  long output_long = 7;
  redirect_call_put_output_conf_long(&rc, "long", output_long);

  TRACE_F("input: char: %c, int: %d, void_p: %p, str: '%.*s' (%" rdint_fmt "), ints: [%d, %d]", input_char, input_int, input_void_p, (int) input_str_n, input_str, input_str_n, input_ints[0], input_ints[1]);
  TRACE_F("output: long: %ld", output_long);

  redirect_call_execute(&rc);

  char output_char = 0;
  redirect_call_get_output_conf_char(&rc, "char", &output_char);

  int output_int = 0;
  redirect_call_get_output_conf_int(&rc, "int", &output_int);

  void *output_void_p = NULL;
  redirect_call_get_output_conf_void_p(&rc, "void_p", &output_void_p);

  char output_str[256];
  rdint_t output_str_n = 0;
  redirect_call_get_output_conf_str(&rc, "str", output_str, &output_str_n);

  int output_ints[2] = { 0, 0 };
  redirect_call_get_output_conf_ints(&rc, "ints", output_ints, 2);

  TRACE_F("output: char: %c, int: %d, void_p: %p, str: '%.*s' (%" rdint_fmt "), ints: [%d, %d]", output_char, output_int, output_void_p, (int) output_str_n, output_str, output_str_n, output_ints[0], output_ints[1]);

  redirect_call_destroy_scdc(&rc);
}


#if REDIRECT_CALL_PARAMS_CACHE

void test_cache(const char *uri)
{
  redirect_call_t rc;

  rdint_t in_n0 = 10;
  double *in_v0 = malloc(in_n0 * sizeof(*in_v0));
  vector_init(in_v0, in_n0, 1, 1.0, NAN);

  redirect_cache_t rch;

  redirect_cache_create(&rch);

  redirect_call_create_scdc(&rc, "cache0", uri);

  redirect_call_set_cache_ptr(&rc, &rch);

  printf("in_v0: %p, in_n0: %" rdint_fmt "\n", in_v0, in_n0);
  redirect_call_print_param_array_double(in_v0, in_n0);
  redirect_call_put_input_conf_rdint(&rc, "in_n0", in_n0);
  redirect_call_put_input_param_array_double(&rc, "in_v0", in_v0, in_n0);

  rdint_t out_n = 10;
  double *out_v = malloc(out_n * sizeof(*out_v));
  printf("out_v: %p, out_n: %" rdint_fmt "\n", out_v, out_n);
  redirect_call_put_input_conf_rdint(&rc, "out_n", out_n);
  redirect_call_put_output_param_array_double(&rc, "out_v", out_v, out_n);

  redirect_call_execute(&rc);

  redirect_call_get_output_param_array_double(&rc, "out_v", &out_v, &out_n);
  printf("out_v: %p, out_n: %" rdint_fmt "\n", out_v, out_n);
  redirect_call_print_param_array_double(out_v, out_n);

  TRACE_F("result: '%s'", redirect_call_get_result_str(&rc, NULL, 0));

  redirect_call_destroy_scdc(&rc);

  redirect_cache_reset_params(&rch);

  redirect_call_create_scdc(&rc, "cache1", uri);

  redirect_call_set_cache_ptr(&rc, &rch);

  printf("in_v0: %p, in_n0: %" rdint_fmt "\n", in_v0, in_n0);
  redirect_call_print_param_array_double(in_v0, in_n0);
  redirect_call_put_input_conf_rdint(&rc, "in_n0", in_n0);
  redirect_call_put_input_param_array_double(&rc, "in_v0", in_v0, in_n0);

  rdint_t in_n1 = 10;
  double *in_v1 = malloc(in_n1 * sizeof(*in_v1));
  vector_init(in_v1, in_n1, 1, 2.0, NAN);

  printf("in_v1: %p, in_n1: %" rdint_fmt "\n", in_v1, in_n1);
  redirect_call_print_param_array_double(in_v1, in_n1);
  redirect_call_put_input_conf_rdint(&rc, "in_n1", in_n1);
  redirect_call_put_input_param_array_double(&rc, "in_v1", in_v1, in_n1);

  rdint_t in_n2 = out_n;
  double *in_v2 = out_v;
  printf("in_v2: %p, in_n2: %" rdint_fmt "\n", in_v2, in_n2);
  redirect_call_print_param_array_double(in_v2, in_n2);
  redirect_call_put_input_conf_rdint(&rc, "in_n2", in_n2);
  redirect_call_put_input_param_array_double(&rc, "in_v2", in_v2, in_n2);

  redirect_call_execute(&rc);

  TRACE_F("result: '%s'", redirect_call_get_result_str(&rc, NULL, 0));

  redirect_call_destroy_scdc(&rc);

  redirect_cache_destroy(&rch);

  free(in_v0);
  free(in_v1);
  free(out_v);
}

#endif /* REDIRECT_CALL_PARAMS_CACHE */


void test_param_array(const char *uri)
{
  redirect_call_t rc;

  redirect_call_create_scdc(&rc, "param_array", uri);

  rdint_t in_n = 10;
  double *in_v = malloc(in_n * sizeof(*in_v));
  vector_init(in_v, in_n, 1, 1.0, NAN);
  printf("in_v: %p, in_n: %" rdint_fmt "\n", in_v, in_n);
  redirect_call_print_param_array_double(in_v, in_n);
  redirect_call_put_input_conf_rdint(&rc, "in_n", in_n);
  redirect_call_put_input_param_array_double(&rc, "in_v", in_v, in_n);

  rdint_t out_n = 10;
  double *out_v = malloc(out_n * sizeof(*out_v));
  printf("out_v: %p, out_n: %" rdint_fmt "\n", out_v, out_n);
  redirect_call_put_input_conf_rdint(&rc, "out_n", out_n);
  redirect_call_put_output_param_array_double(&rc, "out_v", out_v, out_n);

  redirect_call_execute(&rc);

  redirect_call_get_output_param_array_double(&rc, "out_v", &out_v, &out_n);
  printf("out_v: %p, out_n: %" rdint_fmt "\n", out_v, out_n);
  redirect_call_print_param_array_double(out_v, out_n);

  free(in_v);
  free(out_v);

  TRACE_F("result: '%s'", redirect_call_get_result_str(&rc, NULL, 0));

  redirect_call_destroy_scdc(&rc);
}


void test_param_vector(const char *uri)
{
  redirect_call_t rc;

  redirect_call_create_scdc(&rc, "param_vector", uri);

  /* input */
#if REDIRECT_TEST_PARAM_VECTOR_INPUT
  rdint_t in_n0 = 10, in_inc0 = 1;
  double *in_v0 = malloc(in_n0 * in_inc0 * sizeof(*in_v0));
  vector_init(in_v0, in_n0, in_inc0, 1.0, NAN);
  printf("in_v0: %p, in_n0: %" rdint_fmt ", in_inc0: %" rdint_fmt "\n", in_v0, in_n0, in_inc0);
  redirect_call_print_param_vector_double(in_v0, in_n0, in_inc0);
  redirect_call_put_input_conf_rdint(&rc, "in_n0", in_n0);
  redirect_call_put_input_param_vector_double(&rc, "in_v0", in_v0, in_n0, in_inc0);

  rdint_t in_n1 = 10, in_inc1 = 2;
  double *in_v1 = malloc(in_n1 * in_inc1 * sizeof(*in_v1));
  vector_init(in_v1, in_n1, in_inc1, 2.0, NAN);
  printf("in_v1: %p, in_n1: %" rdint_fmt ", in_inc1: %" rdint_fmt "\n", in_v1, in_n1, in_inc1);
  redirect_call_print_param_vector_double(in_v1, in_n1, in_inc1);
  redirect_call_put_input_conf_rdint(&rc, "in_n1", in_n1);
  redirect_call_put_input_param_vector_double(&rc, "in_v1", in_v1, in_n1, in_inc1);
#endif /* REDIRECT_TEST_PARAM_VECTOR_INPUT */

  /* output */
#if REDIRECT_TEST_PARAM_VECTOR_OUTPUT
  rdint_t out_n0 = 10, out_inc0 = 1;
  double *out_v0 = malloc(out_n0 * out_inc0 * sizeof(*out_v0));
  printf("out_v0: %p, out_n0: %" rdint_fmt ", out_inc0: %" rdint_fmt "\n", out_v0, out_n0, out_inc0);
  redirect_call_put_input_conf_rdint(&rc, "out_n0", out_n0);
  redirect_call_put_output_param_vector_double(&rc, "out_v0", out_v0, out_n0, out_inc0);

  rdint_t out_n1 = 10, out_inc1 = 2;
  double *out_v1 = malloc(out_n1 * out_inc1 * sizeof(*out_v1));
  printf("out_v1: %p, out_n1: %" rdint_fmt ", out_inc1: %" rdint_fmt "\n", out_v1, out_n1, out_inc1);
  redirect_call_put_input_conf_rdint(&rc, "out_n1", out_n1);
  redirect_call_put_output_param_vector_double(&rc, "out_v1", out_v1, out_n1, out_inc1);
#endif

  /* inout */
#if REDIRECT_TEST_PARAM_VECTOR_INOUT
  rdint_t inout_n0 = 10, inout_inc0 = 1;
  double *inout_v0 = malloc(inout_n0 * inout_inc0 * sizeof(*inout_v0));
  vector_init(inout_v0, inout_n0, inout_inc0, 3.0, NAN);
  printf("inout_v0: %p, inout_n0: %" rdint_fmt ", inout_inc0: %" rdint_fmt "\n", inout_v0, inout_n0, inout_inc0);
  redirect_call_print_param_vector_double(inout_v0, inout_n0, inout_inc0);
  redirect_call_put_input_conf_rdint(&rc, "inout_n0", inout_n0);
  redirect_call_put_inout_param_vector_double(&rc, "inout_v0", inout_v0, inout_n0, inout_inc0);

  rdint_t inout_n1 = 10, inout_inc1 = 2;
  double *inout_v1 = malloc(inout_n1 * inout_inc1 * sizeof(*inout_v1));
  vector_init(inout_v1, inout_n1, inout_inc1, 4.0, NAN);
  printf("inout_v1: %p, inout_n1: %" rdint_fmt ", inout_inc1: %" rdint_fmt "\n", inout_v1, inout_n1, inout_inc1);
  redirect_call_print_param_vector_double(inout_v1, inout_n1, inout_inc1);
  redirect_call_put_input_conf_rdint(&rc, "inout_n1", inout_n1);
  redirect_call_put_inout_param_vector_double(&rc, "inout_v1", inout_v1, inout_n1, inout_inc1);
#endif /* REDIRECT_TEST_PARAM_VECTOR_INOUT */

  redirect_call_execute(&rc);

  /* output */
#if REDIRECT_TEST_PARAM_VECTOR_OUTPUT
  redirect_call_get_output_param_vector_double(&rc, "out_v0", &out_v0, &out_n0, &out_inc0);
  printf("out_v0: %p, out_n0: %" rdint_fmt ", out_inc0: %" rdint_fmt "\n", out_v0, out_n0, out_inc0);
  redirect_call_print_param_vector_double(out_v0, out_n0, out_inc0);

  redirect_call_get_output_param_vector_double(&rc, "out_v1", &out_v1, &out_n1, &out_inc1);
  printf("out_v1: %p, out_n1: %" rdint_fmt ", out_inc1: %" rdint_fmt "\n", out_v1, out_n1, out_inc1);
  redirect_call_print_param_vector_double(out_v1, out_n1, out_inc1);
#endif /* REDIRECT_TEST_PARAM_VECTOR_OUTPUT */

  /* inout */
#if REDIRECT_TEST_PARAM_VECTOR_INOUT
  redirect_call_get_output_param_vector_double(&rc, "inout_v0", &inout_v0, &inout_n0, &inout_inc0);
  printf("inout_v0: %p, inout_n0: %" rdint_fmt ", inout_inc0: %" rdint_fmt "\n", inout_v0, inout_n0, inout_inc0);
  redirect_call_print_param_vector_double(inout_v0, inout_n0, inout_inc0);

  redirect_call_get_output_param_vector_double(&rc, "inout_v1", &inout_v1, &inout_n1, &inout_inc1);
  printf("inout_v1: %p, inout_n1: %" rdint_fmt ", inout_inc1: %" rdint_fmt "\n", inout_v1, inout_n1, inout_inc1);
  redirect_call_print_param_vector_double(inout_v1, inout_n1, inout_inc1);
#endif /* REDIRECT_TEST_PARAM_VECTOR_INOUT */

#if REDIRECT_TEST_PARAM_VECTOR_INPUT
  free(in_v0);
  free(in_v1);
#endif /* REDIRECT_TEST_PARAM_VECTOR_INPUT */
#if REDIRECT_TEST_PARAM_VECTOR_OUTPUT
  free(out_v0);
  free(out_v1);
#endif /* REDIRECT_TEST_PARAM_VECTOR_OUTPUT */
#if REDIRECT_TEST_PARAM_VECTOR_INOUT
  free(inout_v0);
  free(inout_v1);
#endif /* REDIRECT_TEST_PARAM_VECTOR_INOUT */

  TRACE_F("result: '%s'", redirect_call_get_result_str(&rc, NULL, 0));

  redirect_call_destroy_scdc(&rc);
}


void test_param_matrix(const char *uri)
{
  redirect_call_t rc;

  redirect_call_create_scdc(&rc, "param_matrix", uri);

  /* double ohne/mit stride */

  redirect_call_execute(&rc);

  TRACE_F("result: '%s'", redirect_call_get_result_str(&rc, NULL, 0));

  redirect_call_destroy_scdc(&rc);
}


int main(int argc, char *argv[])
{
  scdc_init(SCDC_INIT_DEFAULT);

  TRACE_F("dataprov open");
  scdc_dataprov_t dp = scdc_dataprov_open(REDIRECT_TEST_SCDC_PATH, "hook", &redirect_test_hook);

#if 1
  const char *uri = "scdc:/" REDIRECT_TEST_SCDC_PATH;
#else
  const char *uri = "scdc+uds://redirect/" REDIRECT_TEST_SCDC_PATH;
#endif

#if TEST_HASH
  test_hash();
#endif

#if TEST_RESULT
  test_result(uri);
#endif

#if TEST_CONF
  test_conf(uri);
#endif

#if TEST_CACHE && REDIRECT_CALL_PARAMS_CACHE
  test_cache(uri);
#endif

#if TEST_PARAM_ARRAY
  test_param_array(uri);
#endif

#if TEST_PARAM_VECTOR
  test_param_vector(uri);
#endif

#if TEST_PARAM_MATRIX
  test_param_matrix(uri);
#endif

  TRACE_F("dataprov close");
  scdc_dataprov_close(dp);

  scdc_release();

  return 0;
}
