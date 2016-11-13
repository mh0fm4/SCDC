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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lapack_call.h"
#include "lapack_scdc.h"


#define PREFIX  "lapack_scdc: "


int sgesv_(int *n, int *nrhs, float *a, int *lda, int *ipiv, float *b, int *ldb, int *info);
int strsv_(char *uplo, char *trans, char *diag, int *n, float *A, int *lda, float *X, int *incx);


static void do_sgesv(lapack_call_t *lpc)
{
  printf(PREFIX "do_sgesv:\n");
  printf(PREFIX "  lpc: '%p'\n", lpc);

  int N, NRHS;
  lapack_call_get_input_conf_int(lpc, "N", &N);
  lapack_call_get_input_conf_int(lpc, "NRHS", &NRHS);

  float *A = 0, *B = 0;
  int LDA, LDB;
  lapack_call_get_input_param_matrix_float(lpc, "A", N, N, &A, &LDA);
  lapack_call_get_input_param_matrix_float(lpc, "B", N, NRHS, &B, &LDB);

  int *IPIV = 0;
  lapack_call_get_output_param_vector_int(lpc, "IPIV", N, &IPIV); /* output buffer 'IPIV' anfordern */

  int INFO = 0;

  printf(PREFIX "sgesv_: %d, %d, %p, %d, %p, %p, %d, %d\n", N, NRHS, A, LDA, IPIV, B, LDB, INFO);

  sgesv_(&N, &NRHS, A, &LDA, IPIV, B, &LDB, &INFO);

  printf(PREFIX "sgesv_: %d, %d, %p, %d, %p, %p, %d, %d\n", N, NRHS, A, LDA, IPIV, B, LDB, INFO);

  lapack_call_put_output_param_matrix_float(lpc, "A", N, N, A, LDA);
  lapack_call_put_output_param_matrix_float(lpc, "B", N, NRHS, B, LDB);

  lapack_call_put_output_param_vector_int(lpc, "IPIV", N, IPIV);

  lapack_call_put_output_conf_int(lpc, "INFO", INFO);
}


static void do_strsv(lapack_call_t *lpc)
{
  printf(PREFIX "do_strsv:\n");
  printf(PREFIX "  lpc: '%p'\n", lpc);

  char uplo, trans, diag;
  lapack_call_get_input_conf_char(lpc, "UPLO", &uplo);
  lapack_call_get_input_conf_char(lpc, "TRANS", &trans);
  lapack_call_get_input_conf_char(lpc, "DIAG", &diag);

  int n, lda, incx;
  lapack_call_get_input_conf_int(lpc, "N", &n);
  lapack_call_get_input_conf_int(lpc, "LDA", &lda);
  lapack_call_get_input_conf_int(lpc, "INCX", &incx);

  float *A = 0, *X = 0;
  int ldx;
  lapack_call_get_input_param_matrix_float(lpc, "A", n, n, &A, &lda);
  lapack_call_get_input_param_matrix_float(lpc, "X", n, 1, &X, &ldx);

  printf(PREFIX "strsv_: %c, %c, %c, %d, %p, %d, %p, %d\n", uplo, trans, diag, n, A, lda, X, incx);

  strsv_(&uplo, &trans, &diag, &n, A, &lda, X, &incx);

  printf(PREFIX "strsv_: %c, %c, %c, %d, %p, %d, %p, %d\n", uplo, trans, diag, n, A, lda, X, incx);

  lapack_call_put_output_param_matrix_float(lpc, "X", n, 1, X, n);
}


void *lapack_scdc_open(const char *conf, va_list ap)
{
  printf(PREFIX "lapack_scdc_open:\n");
  printf(PREFIX "  conf: '%s'\n", conf);

  lapack_call_t *lpc = malloc(sizeof(lapack_call_t));

  printf(PREFIX "  lpc: '%p'\n", lpc);

  return lpc;
}


scdcint_t lapack_scdc_close(void *dataprov)
{
  printf(PREFIX "lapack_scdc_close:\n");
  printf(PREFIX "  dataprov: '%p'\n", dataprov);

  lapack_call_t *lpc = dataprov;

  free(lpc);

  return SCDC_SUCCESS;
}


/*void *lapack_scdc_dataset_open(void *dataprov, const char *path)
{
  printf(PREFIX "lapack_scdc_dataset_open:\n");
  printf(PREFIX "  dataprov: '%p'\n", dataprov);
  printf(PREFIX "  path: '%s'\n", path);

  lapack_call_t *lpc = malloc(sizeof(lapack_call_t));

  printf(PREFIX "  lpc: '%p'\n", path);

  return lpc;
}


scdcint_t lapack_scdc_dataset_close(void *dataprov, void *dataset)
{
  printf(PREFIX "lapack_scdc_dataset_close:\n");
  printf(PREFIX "  dataprov: '%p'\n", dataprov);
  printf(PREFIX "  dataset: '%p'\n", dataset);

  lapack_call_t *lpc = dataset;

  free(lpc);

  return SCDC_SUCCESS;
}*/


scdcint_t lapack_scdc_dataset_cmd(void *dataprov, void *dataset, const char *cmd, const char *params, scdc_dataset_input_t *input, scdc_dataset_output_t *output)
{
  printf(PREFIX "lapack_scdc_dataset_cmd:\n");
  printf(PREFIX "  dataprov: '%p'\n", dataprov);
  printf(PREFIX "  dataset: '%p'\n", dataset);
  printf(PREFIX "  cmd: '%s'\n", cmd);
  printf(PREFIX "  params: '%s'\n", params);

  printf(PREFIX "  "); scdc_dataset_input_print(input); printf("\n");

  printf(PREFIX "  "); scdc_dataset_output_print(output); printf("\n");

  printf(PREFIX "\n");

  lapack_call_t *lpc = dataprov;

  lapack_call_init_scdc(lpc, params, input, output);

  if (strcmp(cmd, "sgesv") == 0) do_sgesv(lpc);
  else if (strcmp(cmd, "strsv") == 0) do_strsv(lpc);
  else printf(PREFIX "lapack_scdc_dataset_cmd: command '%s' not supported!\n", cmd);

  lapack_call_release_scdc(lpc);

  return SCDC_SUCCESS;
}
