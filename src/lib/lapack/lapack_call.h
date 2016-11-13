/*
 *  Copyright (C) 2014, 2015, 2016 Michael Hofmann
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


#ifndef __LAPACK_CALL_H__
#define __LAPACK_CALL_H__


#include <scdc.h>


typedef struct _lapack_params_dense_t
{
  void *buf;
  int size;

} lapack_params_dense_t;


typedef struct _lapack_call_t
{
  int client;
  char op[32], uri[256];

  int input_nconf, output_nconf;
  char input_conf[256], output_conf[256];

  int dense_nparams;
  lapack_params_dense_t dense_params[8];

  int input_offset, output_offset;
  scdc_dataset_input_t *input;
  scdc_dataset_output_t *output;

  int input_state, output_state;

  int free_nbufs;
  void *free_bufs[8];

} lapack_call_t;


/* client side */
void lapack_call_create_scdc(lapack_call_t *lpc, const char *op, const char *uri);
void lapack_call_destroy_scdc(lapack_call_t *lpc);

void lapack_call_do(lapack_call_t *lpc);

void lapack_call_put_input_conf_int(lapack_call_t *lpc, const char *id, int i);
void lapack_call_get_output_conf_int(lapack_call_t *lpc, const char *id, int *i);

void lapack_call_put_input_conf_char(lapack_call_t *lpc, const char *id, char c);
void lapack_call_get_output_conf_char(lapack_call_t *lpc, const char *id, char *c);

/* server side */
void lapack_call_init_scdc(lapack_call_t *lpc, const char *params, scdc_dataset_input_t *input, scdc_dataset_output_t *output);
void lapack_call_release_scdc(lapack_call_t *lpc);

void lapack_call_get_input_conf_int(lapack_call_t *lpc, const char *id, int *i);
void lapack_call_put_output_conf_int(lapack_call_t *lpc, const char *id, int i);

void lapack_call_get_input_conf_char(lapack_call_t *lpc, const char *id, char *c);
void lapack_call_put_output_conf_char(lapack_call_t *lpc, const char *id, char c);

/* client and server side */
void lapack_call_put_input_param_bytes(lapack_call_t *lpc, const char *id, int n, char *b);
void lapack_call_get_input_param_bytes(lapack_call_t *lpc, const char *id, int n, char **b);
void lapack_call_put_output_param_bytes(lapack_call_t *lpc, const char *id, int n, char *b);
void lapack_call_get_output_param_bytes(lapack_call_t *lpc, const char *id, int n, char **b);

void lapack_call_put_input_param_matrix_float(lapack_call_t *lpc, const char *id, int n0, int n1, float *m, int ld);
void lapack_call_get_input_param_matrix_float(lapack_call_t *lpc, const char *id, int n0, int n1, float **m, int *ld);
void lapack_call_put_output_param_matrix_float(lapack_call_t *lpc, const char *id, int n0, int n1, float *m, int ld);
void lapack_call_get_output_param_matrix_float(lapack_call_t *lpc, const char *id, int n0, int n1, float **m, int *ld);

void lapack_call_put_input_param_vector_float(lapack_call_t *lpc, const char *id, int n, float *v, int ld);
void lapack_call_get_input_param_vector_float(lapack_call_t *lpc, const char *id, int n, float **v, int *ld);
void lapack_call_put_output_param_vector_float(lapack_call_t *lpc, const char *id, int n, float *v, int ld);
void lapack_call_get_output_param_vector_float(lapack_call_t *lpc, const char *id, int n, float **v, int *ld);

void lapack_call_put_input_param_vector_int(lapack_call_t *lpc, const char *id, int n, int *v);
void lapack_call_get_input_param_vector_int(lapack_call_t *lpc, const char *id, int n, int **v);
void lapack_call_put_output_param_vector_int(lapack_call_t *lpc, const char *id, int n, int *v);
void lapack_call_get_output_param_vector_int(lapack_call_t *lpc, const char *id, int n, int **v);


#endif /* __LAPACK_CALL_H__ */
