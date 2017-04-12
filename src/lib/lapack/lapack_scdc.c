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


/* required for the later include of dlfct.h */
#define _GNU_SOURCE  1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lapack.h"
#include "common.h"
#include "lapack_call.h"
#include "lapack_scdc.h"


#define TRACE_PREFIX  "lapack_scdc: "


#if LIBBLAS_SCDC_PREFIX

#else /* LIBBLAS_SCDC_PREFIX */

# define LAPACK_ORIG_ENABLED  1

#endif /* LIBBLAS_SCDC_PREFIX */

#include "lapack_orig.h"


#if LAPACK_SGESV

static void do_sgesv(lapack_call_t *lc)
{
  TRACE_F("%s: %p", __func__, lc);

  int N, NRHS;
  LAPACK_CALL(get_input_conf_int)(lc, "N", &N);
  LAPACK_CALL(get_input_conf_int)(lc, "NRHS", &NRHS);

  float *A = 0, *B = 0;
  int LDA, oa, LDB, ob;
  LAPACK_CALL(get_input_param_matrix_float)(lc, "A", N, N, &A, &LDA, &oa);
  LAPACK_CALL(get_input_param_matrix_float)(lc, "B", N, NRHS, &B, &LDB, &ob);

  ASSERT(oa == (RCM_TYPE_DENSE|RCM_ORDER_COL_MAJOR));
  ASSERT(ob == (RCM_TYPE_DENSE|RCM_ORDER_COL_MAJOR));

  int *IPIV = 0;
  LAPACK_CALL(get_output_param_vector_int)(lc, "IPIV", N, &IPIV); /* output buffer 'IPIV' anfordern */

  int INFO = 0;

  TRACE_F("%s: N: %d, NRHS: %d, A: %p, LDA: %d, IPIV: %p, B: %p, LDB: %d, INFO: %d", __func__, N, NRHS, A, LDA, IPIV, B, LDB, INFO);

  LAPACK_ORIG_F_INIT(sgesv_);
  LAPACK_ORIG_F(MANGLE_LAPACK(sgesv_))(&N, &NRHS, A, &LDA, IPIV, B, &LDB, &INFO);

  TRACE_F("%s: N: %d, NRHS: %d, A: %p, LDA: %d, IPIV: %p, B: %p, LDB: %d, INFO: %d", __func__, N, NRHS, A, LDA, IPIV, B, LDB, INFO);

  LAPACK_CALL(put_output_param_matrix_float)(lc, "A", N, N, A, LDA, RCM_TYPE_DENSE|RCM_ORDER_COL_MAJOR);
  LAPACK_CALL(put_output_param_matrix_float)(lc, "B", N, NRHS, B, LDB, RCM_TYPE_DENSE|RCM_ORDER_COL_MAJOR);

  LAPACK_CALL(put_output_param_vector_int)(lc, "IPIV", N, IPIV);

  LAPACK_CALL(put_output_conf_int)(lc, "INFO", INFO);

  TRACE_F("%s: return", __func__);
}

#endif /* LAPACK_SGESV */


#if LAPACK_STRSV

static void do_strsv(lapack_call_t *lc)
{
  TRACE_F("%s: %p", __func__, lc);

  char UPLO, TRANS, DIAG;
  LAPACK_CALL(get_input_conf_char)(lc, "UPLO", &UPLO);
  LAPACK_CALL(get_input_conf_char)(lc, "TRANS", &TRANS);
  LAPACK_CALL(get_input_conf_char)(lc, "DIAG", &DIAG);

  int N;
  LAPACK_CALL(get_input_conf_int)(lc, "N", &N);
  
  float *A = 0;
  int LDA;
  LAPACK_CALL(get_input_param_matrix_float)(lc, "A", N, N, &A, &LDA, RCM_TYPE_DENSE|RCM_ORDER_COL_MAJOR);

  float *X = 0;
  int INCX;
  LAPACK_CALL(get_input_param_vector_float)(lc, "X", N, &X, &INCX);

  TRACE_F("%s: UPLO: %c, TRANS: %c, DIAG: %c, N: %d, A: %p, LDA: %d, X: %p, INCX: %d", __func__, UPLO, TRANS, DIAG, N, A, LDA, X, INCX);

  LAPACK_ORIG_F_INIT(strsv_);
  LAPACK_ORIG_F(MANGLE_LAPACK(strsv_))(&UPLO, &TRANS, &DIAG, &N, A, &LDA, X, &INCX);

  TRACE_F("%s: UPLO: %c, TRANS: %c, DIAG: %c, N: %d, A: %p, LDA: %d, X: %p, INCX: %d", __func__, UPLO, TRANS, DIAG, N, A, LDA, X, INCX);

  LAPACK_CALL(put_output_param_vector_float)(lc, "X", N, X, INCX);

  TRACE_F("%s: return", __func__);
}

#endif /* LAPACK_STRSV */


void *lapack_scdc_open(const char *conf, va_list ap)
{
  TRACE_F("%s: conf: '%s'", __func__, conf);

  LAPACK_ORIG_INIT();

  lapack_call_t *lc = malloc(sizeof(lapack_call_t));

  TRACE_F("%s: return: %p", __func__, lc);

  return lc;
}


scdcint_t lapack_scdc_close(void *dataprov)
{
  TRACE_F("%s: dataprov: %p", __func__, dataprov);

  lapack_call_t *lc = dataprov;

  free(lc);

  LAPACK_ORIG_RELEASE();

  TRACE_F("%s: return", __func__);

  return SCDC_SUCCESS;
}


/*void *lapack_scdc_dataset_open(void *dataprov, const char *path)
{
  TRACE_F("%s:lapack_scdc_dataset_open:\n");
  TRACE_F("%s:  dataprov: '%p'\n", dataprov);
  TRACE_F("%s:  path: '%s'\n", path);

  lapack_call_t *lc = malloc(sizeof(lapack_call_t));

  TRACE_F("%s:  lc: '%p'\n", path);

  return lc;
}


scdcint_t lapack_scdc_dataset_close(void *dataprov, void *dataset)
{
  TRACE_F("%s:lapack_scdc_dataset_close:\n");
  TRACE_F("%s:  dataprov: '%p'\n", dataprov);
  TRACE_F("%s:  dataset: '%p'\n", dataset);

  lapack_call_t *lc = dataset;

  free(lc);

  return SCDC_SUCCESS;
}*/


scdcint_t lapack_scdc_dataset_cmd(void *dataprov, void *dataset, const char *cmd, const char *params, scdc_dataset_input_t *input, scdc_dataset_output_t *output)
{
  TRACE_F("%s: dataprov: %p, dataset: %p, cmd: '%s', params: '%s', input: %p, output: %p", __func__, dataprov, dataset, cmd, params, input, output);

  printf("input: "); scdc_dataset_input_print(input); printf("\n");
  printf("output: "); scdc_dataset_output_print(output); printf("\n");

  lapack_call_t *lc = dataprov;

  LAPACK_CALL(init_scdc)(lc, params, input, output);

#if LAPACK_SGESV
  if (strcmp(cmd, "sgesv") == 0) do_sgesv(lc); else
#endif
#if LAPACK_STRSV
  if (strcmp(cmd, "strsv") == 0) do_strsv(lc); else
#endif
  {
    TRACE_F("%s: command '%s' not supported!", __func__, cmd);
  }

  LAPACK_CALL(release_scdc)(lc);

  TRACE_F("%s: return", __func__);

  return SCDC_SUCCESS;
}
