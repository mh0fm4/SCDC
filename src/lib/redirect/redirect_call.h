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


#ifndef __REDIRECT_CALL_H__
#define __REDIRECT_CALL_H__


#include "scdc.h"

#include "redirect_config.h"


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


#if REDIRECT_CALL_PARAMS_DENSE

typedef struct _redirect_params_dense_t
{
  void *buf;
  rdint_t size;

} redirect_params_dense_t;

#else

typedef struct _redirect_params_blocks_t
{
  void *buf;
  rdint_t count, size, stride;

} redirect_params_blocks_t;

#endif

typedef struct _redirect_call_t
{
  int client;
  char op[32], uri[256];

  int input_nconf, output_nconf;
  char input_conf[256], output_conf[256];

#if REDIRECT_CALL_PARAMS_DENSE
  int dense_nparams;
  redirect_params_dense_t dense_params[8];
#else
  int blocks_nparams;
  redirect_params_blocks_t blocks_params[8];
#endif

  int input_offset, output_offset;
  scdc_dataset_input_t *input;
  scdc_dataset_output_t *output;

#if REDIRECT_CALL_PARAMS_DENSE
  int input_state, output_state;
#else
  int input_state[2], output_state[2];
#endif

  int free_nbufs;
  void *free_bufs[8];

} redirect_call_t;


/* client side */
void redirect_call_create_scdc(redirect_call_t *rc, const char *op, const char *uri);
void redirect_call_destroy_scdc(redirect_call_t *rc);

int redirect_call_execute(redirect_call_t *rc);

#define DECLARE_CLI_CONF(_t_, _tn_) \
  void redirect_call_put_input_conf_ ## _tn_(redirect_call_t *rc, const char *id, _t_ x); \
  void redirect_call_get_output_conf_ ## _tn_(redirect_call_t *rc, const char *id, _t_ *x);

/* server side */
void redirect_call_init_scdc(redirect_call_t *rc, const char *params, scdc_dataset_input_t *input, scdc_dataset_output_t *output);
void redirect_call_release_scdc(redirect_call_t *rc);

#define DECLARE_SRV_CONF(_t_, _tn_) \
  void redirect_call_get_input_conf_ ## _tn_(redirect_call_t *rc, const char *id, _t_ *x); \
  void redirect_call_put_output_conf_ ## _tn_(redirect_call_t *rc, const char *id, _t_ x);

/* client and server side */
void redirect_call_put_input_param_bytes(redirect_call_t *rc, const char *id, rdint_t n, char *b);
void redirect_call_get_input_param_bytes(redirect_call_t *rc, const char *id, rdint_t n, char **b);
void redirect_call_put_output_param_bytes(redirect_call_t *rc, const char *id, rdint_t n, char *b);
void redirect_call_get_output_param_bytes(redirect_call_t *rc, const char *id, rdint_t n, char **b);
void redirect_call_put_inout_param_bytes(redirect_call_t *rc, const char *id, rdint_t n, char *b);
void redirect_call_get_inout_param_bytes(redirect_call_t *rc, const char *id, rdint_t n, char **b);

#define DECLARE_FLOAT_VECTOR(_t_, _tn_) \
  void redirect_call_print_param_vector_ ##  _tn_(rdint_t n, _t_ *v, rdint_t inc); \
  void redirect_call_put_input_param_vector_ ##  _tn_(redirect_call_t *rc, const char *id, rdint_t n, _t_ *v, rdint_t inc); \
  void redirect_call_get_input_param_vector_ ##  _tn_(redirect_call_t *rc, const char *id, rdint_t n, _t_ **v, rdint_t *inc); \
  void redirect_call_put_output_param_vector_ ##  _tn_(redirect_call_t *rc, const char *id, rdint_t n, _t_ *v, rdint_t inc); \
  void redirect_call_get_output_param_vector_ ##  _tn_(redirect_call_t *rc, const char *id, rdint_t n, _t_ **v, rdint_t *inc); \
  void redirect_call_put_inout_param_vector_ ##  _tn_(redirect_call_t *rc, const char *id, rdint_t n, _t_ *v, rdint_t inc); \
  void redirect_call_get_inout_param_vector_ ##  _tn_(redirect_call_t *rc, const char *id, rdint_t n, _t_ **v, rdint_t *inc); \

#define DECLARE_INT_VECTOR(_t_, _tn_) \
  void redirect_call_print_param_vector_ ## _tn_(rdint_t n, _t_ *v); \
  void redirect_call_put_input_param_vector_ ## _tn_(redirect_call_t *rc, const char *id, rdint_t n, _t_ *v); \
  void redirect_call_get_input_param_vector_ ## _tn_(redirect_call_t *rc, const char *id, rdint_t n, _t_ **v); \
  void redirect_call_put_output_param_vector_ ## _tn_(redirect_call_t *rc, const char *id, rdint_t n, _t_ *v); \
  void redirect_call_get_output_param_vector_ ## _tn_(redirect_call_t *rc, const char *id, rdint_t n, _t_ **v); \
  void redirect_call_put_inout_param_vector_ ## _tn_(redirect_call_t *rc, const char *id, rdint_t n, _t_ *v); \
  void redirect_call_get_inout_param_vector_ ## _tn_(redirect_call_t *rc, const char *id, rdint_t n, _t_ **v);

#define DECLARE_FLOAT_MATRIX(_t_, _tn_) \
  void redirect_call_print_param_matrix_ ## _tn_(rdint_t n0, rdint_t n1, _t_ *m, rdint_t ld, rdint_t rcm); \
  void redirect_call_put_input_param_matrix_ ## _tn_(redirect_call_t *rc, const char *id, rdint_t n0, rdint_t n1, _t_ *m, rdint_t ld, int rcm); \
  void redirect_call_get_input_param_matrix_ ## _tn_(redirect_call_t *rc, const char *id, rdint_t n0, rdint_t n1, _t_ **m, rdint_t *ld, int *rcm); \
  void redirect_call_put_output_param_matrix_ ## _tn_(redirect_call_t *rc, const char *id, rdint_t n0, rdint_t n1, _t_ *m, rdint_t ld, int rcm); \
  void redirect_call_get_output_param_matrix_ ## _tn_(redirect_call_t *rc, const char *id, rdint_t n0, rdint_t n1, _t_ **m, rdint_t *ld, int *rcm); \
  void redirect_call_put_inout_param_matrix_ ## _tn_(redirect_call_t *rc, const char *id, rdint_t n0, rdint_t n1, _t_ *m, rdint_t ld, int rcm); \
  void redirect_call_get_inout_param_matrix_ ## _tn_(redirect_call_t *rc, const char *id, rdint_t n0, rdint_t n1, _t_ **m, rdint_t *ld, int *rcm);

#define DECLARE_FLOAT(_t_, _tn_) \
  DECLARE_CLI_CONF(_t_, _tn_) \
  DECLARE_SRV_CONF(_t_, _tn_) \
  DECLARE_FLOAT_VECTOR(_t_, _tn_) \
  DECLARE_FLOAT_MATRIX(_t_, _tn_)

DECLARE_FLOAT(float, float)
DECLARE_FLOAT(double, double)

#define DECLARE_INT(_t_, _tn_) \
  DECLARE_CLI_CONF(_t_, _tn_) \
  DECLARE_SRV_CONF(_t_, _tn_) \
  DECLARE_INT_VECTOR(_t_, _tn_) \

DECLARE_INT(char, char)
DECLARE_INT(int, int)


#undef DECLARE_CLI_CONF
#undef DECLARE_SRV_CONF
#undef DECLARE_FLOAT_VECTOR
#undef DECLARE_INT_VECTOR
#undef DECLARE_FLOAT_MATRIX
#undef DECLARE_FLOAT
#undef DECLARE_INT


#ifdef __cplusplus
}
#endif


#endif /* __REDIRECT_CALL_H__ */
