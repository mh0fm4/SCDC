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

#include "lapack_call.h"
#include "lapack_scdc.h"


#define PREFIX  "liblapack_scdc: "


int liblapack_scdc_initialized = 0;
scdc_dataprov_t liblapack_scdc_hook = SCDC_DATAPROV_NULL;

#define LIBLAPACK_SCDC_PATH  "lapack"
/*#define LIBLAPACK_SCDC_URI  "scdc+uds://liblapack_scdc/"*/
#define LIBLAPACK_SCDC_URI  "scdc:/"


void liblapack_scdc_init()
{
  if (liblapack_scdc_initialized == 0)
  {
    scdc_init(SCDC_INIT_DEFAULT);

    liblapack_scdc_hook = scdc_dataprov_open(LIBLAPACK_SCDC_PATH, "hook", &lapack_scdc_hook);
  }

  ++liblapack_scdc_initialized;
}


void liblapack_scdc_release()
{
  --liblapack_scdc_initialized;

  if (liblapack_scdc_initialized == 0)
  {
    scdc_dataprov_close(liblapack_scdc_hook);

    liblapack_scdc_hook = SCDC_DATAPROV_NULL;

    scdc_release();
  }
}


void scdc_sgesv(int N, int NRHS, float *A, int LDA, int *IPIV, float *B, int LDB, int INFO)
{
  printf(PREFIX "scdc_sgesv: %d, %d, %p, %d, %p, %p, %d, %d\n", N, NRHS, A, LDA, IPIV, B, LDB, INFO);

  liblapack_scdc_init();

  lapack_call_t lpc;

  lapack_call_create_scdc(&lpc, "sgesv", LIBLAPACK_SCDC_URI LIBLAPACK_SCDC_PATH);

  lapack_call_put_input_conf_int(&lpc, "N", N);
  lapack_call_put_input_conf_int(&lpc, "NRHS", NRHS);

  lapack_call_put_input_param_matrix_float(&lpc, "A", N, N, A, LDA);
  lapack_call_put_input_param_matrix_float(&lpc, "B", N, NRHS, B, LDB);

  lapack_call_put_output_param_vector_int(&lpc, "IPIV", N, IPIV); /* output buffer 'IPIV' bekannt machen */

  lapack_call_do(&lpc);

  lapack_call_get_output_param_matrix_float(&lpc, "A", N, N, &A, &LDA);
  lapack_call_get_output_param_matrix_float(&lpc, "B", N, NRHS, &B, &LDB);

  lapack_call_get_output_param_vector_int(&lpc, "IPIV", N, &IPIV);

  lapack_call_get_output_conf_int(&lpc, "INFO", &INFO);

  lapack_call_destroy_scdc(&lpc);

  liblapack_scdc_release();

  printf(PREFIX "scdc_sgesv return: %d, %d, %p, %d, %p, %p, %d, %d\n", N, NRHS, A, LDA, IPIV, B, LDB, INFO);
}


void scdc_sgesv_(int *N, int *NRHS, float *A, int *LDA, int *IPIV, float *B, int *LDB, int *INFO)
{
  scdc_sgesv(*N, *NRHS, A, *LDA, IPIV, B, *LDB, *INFO);
}


void scdc_strsv(char uplo, char trans, char diag, int n, float *A, int lda, float *X, int incx)
{
  printf(PREFIX "scdc_strsv: %c, %c, %c, %d, %p, %d, %p, %d\n", uplo, trans, diag, n, A, lda, X, incx);

  liblapack_scdc_init();

  lapack_call_t lpc;

  lapack_call_create_scdc(&lpc, "strsv", LIBLAPACK_SCDC_URI LIBLAPACK_SCDC_PATH);

  lapack_call_put_input_conf_char(&lpc, "UPLO", uplo);
  lapack_call_put_input_conf_char(&lpc, "TRANS", trans);
  lapack_call_put_input_conf_char(&lpc, "DIAG", diag);

  lapack_call_put_input_conf_int(&lpc, "N", n);
  lapack_call_put_input_conf_int(&lpc, "LDA", lda);
  lapack_call_put_input_conf_int(&lpc, "INCX", incx);

  lapack_call_put_input_param_matrix_float(&lpc, "A", n, n, A, lda);
  lapack_call_put_input_param_matrix_float(&lpc, "X", n, 1, X, n);

  lapack_call_do(&lpc);

  int ldx;
  lapack_call_get_output_param_matrix_float(&lpc, "X", n, 1, &X, &ldx);

  lapack_call_destroy_scdc(&lpc);

  liblapack_scdc_release();

  printf(PREFIX "scdc_strsv return: %c, %c, %c, %d, %p, %d, %p, %d\n", uplo, trans, diag, n, A, lda, X, incx);
}

void scdc_strsv_(char *uplo, char *trans, char *diag, int *n, float *A, int *lda, float *X, int *incx)
{
  scdc_strsv(*uplo, *trans, *diag, *n, A, *lda, X, *incx);
}
