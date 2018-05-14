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


#ifndef __REDIRECT_CALL_H__
#define __REDIRECT_CALL_H__


#include "scdc.h"

#include "redirect_config.h"
#include "redirect_handle.h"


#ifdef __cplusplus
extern "C" {
#endif


/* matrix */
#define RCM_TYPE                   0x000F
#define RCM_TYPE_DENSE             0x0000
#define RCM_TYPE_TRIANGULAR_UPPER  0x0001
#define RCM_TYPE_TRIANGULAR_LOWER  0x0002
#define RCM_GET_TYPE(_x_)          ((_x_) & RCM_TYPE)

#define RCM_ORDER                  0x00F0
#define RCM_ORDER_COL_MAJOR        0x0000
#define RCM_ORDER_ROW_MAJOR        0x0010
#define RCM_GET_ORDER(_x_)         ((_x_) & RCM_ORDER)

#define RCM_DIAG_NOT               0x0000
#define RCM_DIAG                   0x0100
#define RCM_GET_DIAG(_x_)          ((_x_) & RCM_DIAG)

#define RCM_IS_SET(_x_, _y_)       (((_x_) & (_y_)) == (_y_))


typedef struct _redirect_param_dense_t
{
  void *buf;
  rdint_t size;

} redirect_param_dense_t, redirect_params_dense_t;

typedef struct _redirect_param_blocks_t
{
  void *buf;
  rdint_t count, size, stride;

} redirect_param_blocks_t, redirect_params_blocks_t;

#define REDIRECT_PARAM_TYPE_DENSE   0
#define REDIRECT_PARAM_TYPE_BLOCKS  1

typedef struct _redirect_param_t
{
  int type;
  union
  {
    redirect_param_dense_t dense;
    redirect_param_blocks_t blocks;
  } p;
  
} redirect_param_t;


#define REDIRECT_CALL_OP_MAX_SIZE            64
#define REDIRECT_CALL_URI_MAX_SIZE          256
#define REDIRECT_CALL_INPUT_CONF_MAX_SIZE   512
#define REDIRECT_CALL_OUTPUT_CONF_MAX_SIZE  512
#define REDIRECT_CALL_RESULT_MAX_SIZE       256
#define REDIRECT_CALL_PARAMS_MAX             16
#define REDIRECT_CALL_FREE_BUFS_MAX          16


typedef struct _redirect_call_t
{
  int client;
  char op[REDIRECT_CALL_OP_MAX_SIZE], uri[REDIRECT_CALL_URI_MAX_SIZE];

  scdc_dataset_t dataset;

  int input_nconf, output_nconf;
  char input_conf[REDIRECT_CALL_INPUT_CONF_MAX_SIZE], output_conf[REDIRECT_CALL_OUTPUT_CONF_MAX_SIZE];

  char result[REDIRECT_CALL_RESULT_MAX_SIZE];

#if REDIRECT_CALL_PARAMS_NEW
  int nparams, params_state[2];
  redirect_param_t params[REDIRECT_CALL_PARAMS_MAX];
#else
  int dense_nparams;
  redirect_params_dense_t dense_params[8];
  int input_state, output_state;
#endif

  int input_offset, output_offset;
  scdc_dataset_input_t *input;
  scdc_dataset_output_t *output;

  int free_nbufs;
  void *free_bufs[REDIRECT_CALL_FREE_BUFS_MAX];

} redirect_call_t;


/* client side */
int redirect_call_create_scdc(redirect_call_t *rc, const char *op, const char *uri);
void redirect_call_destroy_scdc(redirect_call_t *rc);

int redirect_call_execute(redirect_call_t *rc);

#define DECLARE_CLI_CONF_VAL(_t_, _tn_) \
  void redirect_call_put_input_conf_ ## _tn_(redirect_call_t *rc, const char *id, _t_ x); \
  void redirect_call_get_output_conf_ ## _tn_(redirect_call_t *rc, const char *id, _t_ *x); \
  void redirect_call_put_input_conf_ ## _tn_ ## s(redirect_call_t *rc, const char *id, const _t_ *x, rdint_t n); \
  void redirect_call_get_output_conf_ ## _tn_ ## s(redirect_call_t *rc, const char *id, _t_ *x, rdint_t n);

void redirect_call_put_input_conf_rdint(redirect_call_t *rc, const char *id, rdint_t x); \
void redirect_call_get_output_conf_rdint(redirect_call_t *rc, const char *id, rdint_t *x);
void redirect_call_put_input_conf_rdints(redirect_call_t *rc, const char *id, const rdint_t *x, rdint_t n); \
void redirect_call_get_output_conf_rdints(redirect_call_t *rc, const char *id, rdint_t *x, rdint_t n);

void redirect_call_put_input_conf_str(redirect_call_t *rc, const char *id, const char *x, rdint_t n);
void redirect_call_get_output_conf_str(redirect_call_t *rc, const char *id, char *x, rdint_t *n);

/* server side */
void redirect_call_init_scdc(redirect_call_t *rc, const char *params, scdc_dataset_input_t *input, scdc_dataset_output_t *output);
void redirect_call_release_scdc(redirect_call_t *rc);

#define DECLARE_SRV_CONF_VAL(_t_, _tn_) \
  void redirect_call_get_input_conf_ ## _tn_(redirect_call_t *rc, const char *id, _t_ *x); \
  void redirect_call_put_output_conf_ ## _tn_(redirect_call_t *rc, const char *id, _t_ x); \
  void redirect_call_get_input_conf_ ## _tn_ ## s(redirect_call_t *rc, const char *id, _t_ *x, rdint_t n); \
  void redirect_call_put_output_conf_ ## _tn_ ## s(redirect_call_t *rc, const char *id, const _t_ *x, rdint_t n);

void redirect_call_get_input_conf_rdint(redirect_call_t *rc, const char *id, rdint_t *x); \
void redirect_call_put_output_conf_rdint(redirect_call_t *rc, const char *id, rdint_t x);
void redirect_call_get_input_conf_rdints(redirect_call_t *rc, const char *id, rdint_t *x, rdint_t n); \
void redirect_call_put_output_conf_rdints(redirect_call_t *rc, const char *id, const rdint_t *x, rdint_t n);

void redirect_call_get_input_conf_str(redirect_call_t *rc, const char *id, char *x, rdint_t *n); \
void redirect_call_put_output_conf_str(redirect_call_t *rc, const char *id, const char *x, rdint_t n);

/* client and server side */
void redirect_call_set_handle(redirect_call_t *rc, redirect_handle_t *rh);
void redirect_call_get_handle(redirect_call_t *rc, redirect_handle_t *rh);

void redirect_call_set_result(redirect_call_t *rc, const char *result);
const char * redirect_call_get_result(redirect_call_t *rc, char *result, rdint_t n);

void redirect_call_put_input_param_bytes(redirect_call_t *rc, const char *id, const void *b, rdint_t n);
void redirect_call_get_input_param_bytes(redirect_call_t *rc, const char *id, void **b, rdint_t *n);
void redirect_call_put_output_param_bytes(redirect_call_t *rc, const char *id, const void *b, rdint_t n);
void redirect_call_get_output_param_bytes(redirect_call_t *rc, const char *id, void **b, rdint_t *n);
void redirect_call_put_inout_param_bytes(redirect_call_t *rc, const char *id, const void *b, rdint_t n);
void redirect_call_get_inout_param_bytes(redirect_call_t *rc, const char *id, void **b, rdint_t *n);

#define DECLARE_FLOAT_ARRAY(_t_, _tn_) \
  void redirect_call_print_param_array_ ##  _tn_(const _t_ *a, rdint_t n); \
  void redirect_call_put_input_param_array_ ##  _tn_(redirect_call_t *rc, const char *id, const _t_ *a, rdint_t n); \
  void redirect_call_get_input_param_array_ ##  _tn_(redirect_call_t *rc, const char *id, _t_ **a, rdint_t *n); \
  void redirect_call_put_output_param_array_ ##  _tn_(redirect_call_t *rc, const char *id, const _t_ *a, rdint_t n); \
  void redirect_call_get_output_param_array_ ##  _tn_(redirect_call_t *rc, const char *id, _t_ **a, rdint_t *n); \
  void redirect_call_put_inout_param_array_ ##  _tn_(redirect_call_t *rc, const char *id, const _t_ *a, rdint_t n); \
  void redirect_call_get_inout_param_array_ ##  _tn_(redirect_call_t *rc, const char *id, _t_ **a, rdint_t *n); \

#define DECLARE_FLOAT_VECTOR(_t_, _tn_) \
  void redirect_call_print_param_vector_ ##  _tn_(const _t_ *v, rdint_t n, rdint_t inc); \
  void redirect_call_put_input_param_vector_ ##  _tn_(redirect_call_t *rc, const char *id, const _t_ *v, rdint_t n, rdint_t inc); \
  void redirect_call_get_input_param_vector_ ##  _tn_(redirect_call_t *rc, const char *id, _t_ **v, rdint_t *n, rdint_t *inc); \
  void redirect_call_put_output_param_vector_ ##  _tn_(redirect_call_t *rc, const char *id, const _t_ *v, rdint_t n, rdint_t inc); \
  void redirect_call_get_output_param_vector_ ##  _tn_(redirect_call_t *rc, const char *id, _t_ **v, rdint_t *n, rdint_t *inc); \
  void redirect_call_put_inout_param_vector_ ##  _tn_(redirect_call_t *rc, const char *id, const _t_ *v, rdint_t n, rdint_t inc); \
  void redirect_call_get_inout_param_vector_ ##  _tn_(redirect_call_t *rc, const char *id, _t_ **v, rdint_t *n, rdint_t *inc); \

#define DECLARE_FLOAT_MATRIX(_t_, _tn_) \
  void redirect_call_print_param_matrix_ ## _tn_(const _t_ *m, rdint_t n0, rdint_t n1, rdint_t ld, rdint_t rcm); \
  void redirect_call_put_input_param_matrix_ ## _tn_(redirect_call_t *rc, const char *id, const _t_ *m, rdint_t n0, rdint_t n1, rdint_t ld, rdint_t rcm); \
  void redirect_call_get_input_param_matrix_ ## _tn_(redirect_call_t *rc, const char *id, _t_ **m, rdint_t *n0, rdint_t *n1, rdint_t *ld, rdint_t *rcm); \
  void redirect_call_put_output_param_matrix_ ## _tn_(redirect_call_t *rc, const char *id, const _t_ *m, rdint_t n0, rdint_t n1, rdint_t ld, rdint_t rcm); \
  void redirect_call_get_output_param_matrix_ ## _tn_(redirect_call_t *rc, const char *id, _t_ **m, rdint_t *n0, rdint_t *n1, rdint_t *ld, rdint_t *rcm); \
  void redirect_call_put_inout_param_matrix_ ## _tn_(redirect_call_t *rc, const char *id, const _t_ *m, rdint_t n0, rdint_t n1, rdint_t ld, rdint_t rcm); \
  void redirect_call_get_inout_param_matrix_ ## _tn_(redirect_call_t *rc, const char *id, _t_ **m, rdint_t *n0, rdint_t *n1, rdint_t *ld, rdint_t *rcm);

#define DECLARE_INT_ARRAY(_t_, _tn_) \
  void redirect_call_print_param_array_ ## _tn_(const _t_ *a, rdint_t n); \
  void redirect_call_put_input_param_array_ ## _tn_(redirect_call_t *rc, const char *id, const _t_ *a, rdint_t n); \
  void redirect_call_get_input_param_array_ ## _tn_(redirect_call_t *rc, const char *id, _t_ **a, rdint_t *n); \
  void redirect_call_put_output_param_array_ ## _tn_(redirect_call_t *rc, const char *id, const _t_ *a, rdint_t n); \
  void redirect_call_get_output_param_array_ ## _tn_(redirect_call_t *rc, const char *id, _t_ **a, rdint_t *n); \
  void redirect_call_put_inout_param_array_ ## _tn_(redirect_call_t *rc, const char *id, const _t_ *a, rdint_t n); \
  void redirect_call_get_inout_param_vector_ ## _tn_(redirect_call_t *rc, const char *id, _t_ **a, rdint_t *n);

#define DECLARE_FLOAT(_t_, _tn_) \
  DECLARE_CLI_CONF_VAL(_t_, _tn_); \
  DECLARE_SRV_CONF_VAL(_t_, _tn_); \
  DECLARE_FLOAT_ARRAY(_t_, _tn_); \
  DECLARE_FLOAT_VECTOR(_t_, _tn_); \
  DECLARE_FLOAT_MATRIX(_t_, _tn_);

DECLARE_FLOAT(float, float);
DECLARE_FLOAT(double, double);

#define DECLARE_INT(_t_, _tn_) \
  DECLARE_CLI_CONF_VAL(_t_, _tn_); \
  DECLARE_SRV_CONF_VAL(_t_, _tn_); \
  DECLARE_INT_ARRAY(_t_, _tn_);

DECLARE_INT(char, char);
DECLARE_INT(int, int);

typedef void *void_p;
DECLARE_CLI_CONF_VAL(void_p, void_p);
DECLARE_SRV_CONF_VAL(void_p, void_p);
typedef char *char_p;
DECLARE_CLI_CONF_VAL(char_p, char_p);
DECLARE_SRV_CONF_VAL(char_p, char_p);


#undef DECLARE_CLI_CONF_VAL
#undef DECLARE_SRV_CONF_VAL
#undef DECLARE_FLOAT_ARRAY
#undef DECLARE_FLOAT_VECTOR
#undef DECLARE_FLOAT_MATRIX
#undef DECLARE_INT_ARRAY
#undef DECLARE_FLOAT
#undef DECLARE_INT


#ifdef __cplusplus
}
#endif


#endif /* __REDIRECT_CALL_H__ */
