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


/* required for the later include of dlfct.h */
#define _GNU_SOURCE  1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LIBBLAS_SCDC_REMOTE  1

#include "z_pack.h"
#include "blas_scdc_config.h"
#include "common.h"
#include "blas.h"
#include "blas_call.h"
#include "blas_scdc.h"
#include "blas_timing.h"


#define TRACE_PREFIX  "blas_scdc: "


#if LIBBLAS_SCDC_PREFIX

#else /* LIBBLAS_SCDC_PREFIX */

# define BLAS_ORIG_ENABLED  1

#endif /* LIBBLAS_SCDC_PREFIX */

#include "blas_orig.h"


#if LIBBLAS_SCDC_TIMING_REMOTE
# undef BLAS_TIMING_PREFIX
# define BLAS_TIMING_PREFIX  TRACE_PREFIX  "TIMING: "
# define LIBBLAS_SCDC_TIMING_REMOTE_X  5
int libblas_scdc_timing_remote_i;
double libblas_scdc_timing_remote[LIBBLAS_SCDC_TIMING_REMOTE_X];
#endif
#define BLAS_TIMING_INIT_()  BLAS_TIMING_INIT(libblas_scdc_timing_remote, libblas_scdc_timing_remote_i, LIBBLAS_SCDC_TIMING_REMOTE_X)

#if LIBBLAS_SCDC_TIMING_REDIRECT_REMOTE
# define BLAS_TIMING_REMOTE_PUT(_bc_, _t_)  BLAS_CALL(put_output_conf_double)(_bc_, "TIMING", _t_);
#else
# define BLAS_TIMING_REMOTE_PUT(_bc_, _t_)  Z_NOP()
#endif


/* BLAS bench */
#if BLAS_BENCH

static void do_dvin(blas_call_t *bc)
{
  TRACE_F("%s: bc: %p", __func__, bc);

  BLAS_TIMING_INIT_();

  int N;
  BLAS_CALL(get_input_conf_int)(bc, "N", &N);

  double *DX = 0;
  int INCX = 0;
  BLAS_CALL(get_input_param_vector_double)(bc, "DX", &DX, &N, &INCX);

  TRACE_F("%s: N: %d, DX: %p, INCX: %d", __func__, N, DX, INCX);

/*  BLAS_TIMING_REMOTE_PUT(bc, libblas_scdc_timing_remote[0]);*/

  BLAS_TIMING_PRINT_F("%s: %f", __func__, libblas_scdc_timing_remote[0]);

  TRACE_F("%s: return", __func__);
}


static void do_dvout(blas_call_t *bc)
{
  TRACE_F("%s: bc: %p", __func__, bc);

  BLAS_TIMING_INIT_();

  int N;
  BLAS_CALL(get_input_conf_int)(bc, "N", &N);

  double *DX = 0;
  int INCX = 0;
  BLAS_CALL(get_output_param_vector_double)(bc, "DX", &DX, &N, &INCX);

#if HAVE_SCDC_DEBUG
  if (N > 0) memset(DX, 0, ((N - 1) * INCX + 1) * sizeof(double));
#endif

  TRACE_F("%s: N: %d, DX: %p, INCX: %d", __func__, N, DX, INCX);

  BLAS_CALL(put_output_param_vector_double)(bc, "DX", DX, N, INCX);

/*  BLAS_TIMING_REMOTE_PUT(bc, libblas_scdc_timing_remote[0]);*/

  BLAS_TIMING_PRINT_F("%s: %f", __func__, libblas_scdc_timing_remote[0]);

  TRACE_F("%s: return", __func__);
}


static void do_dvinout(blas_call_t *bc)
{
  TRACE_F("%s: bc: %p", __func__, bc);

  BLAS_TIMING_INIT_();

  int N;
  BLAS_CALL(get_input_conf_int)(bc, "N", &N);

  double *DX = 0;
  int INCX = 0;
  BLAS_CALL(get_inout_param_vector_double)(bc, "DX", &DX, &N, &INCX);

#if HAVE_SCDC_DEBUG
  if (N > 0) memset(DX, 0, ((N - 1) * INCX + 1) * sizeof(double));
#endif

  TRACE_F("%s: N: %d, DX: %p, INCX: %d", __func__, N, DX, INCX);

  BLAS_CALL(put_output_param_vector_double)(bc, "DX", DX, N, INCX);

/*  BLAS_TIMING_REMOTE_PUT(bc, libblas_scdc_timing_remote[0]);*/

  BLAS_TIMING_PRINT_F("%s: %f", __func__, libblas_scdc_timing_remote[0]);

  TRACE_F("%s: return", __func__);
}


static void do_dgein(blas_call_t *bc)
{
  TRACE_F("%s: bc: %p", __func__, bc);

  BLAS_TIMING_INIT_();

  int M, N;
  BLAS_CALL(get_input_conf_int)(bc, "M", &M);
  BLAS_CALL(get_input_conf_int)(bc, "N", &N);

  double *A = 0;
  int LDA = 0, rcma;
  BLAS_CALL(get_input_param_matrix_double)(bc, "A", &A, &M, &N, &LDA, &rcma);

  ASSERT(rcma == (RCM_TYPE_DENSE|RCM_ORDER_COL_MAJOR));

  TRACE_F("%s: M: %d, N: %d, A: %p, LDA: %d", __func__, M, N, A, LDA);

/*  BLAS_TIMING_REMOTE_PUT(bc, libblas_scdc_timing_remote[0]);*/

  BLAS_TIMING_PRINT_F("%s: %f", __func__, libblas_scdc_timing_remote[0]);

  TRACE_F("%s: return", __func__);
}


static void do_dgeout(blas_call_t *bc)
{
  TRACE_F("%s: bc: %p", __func__, bc);

  BLAS_TIMING_INIT_();

  int M, N;
  BLAS_CALL(get_input_conf_int)(bc, "M", &M);
  BLAS_CALL(get_input_conf_int)(bc, "N", &N);

  double *A = 0;
  int LDA = 0, rcma;
  BLAS_CALL(get_output_param_matrix_double)(bc, "A", &A, &M, &N, &LDA, &rcma);

  ASSERT(rcma == (RCM_TYPE_DENSE|RCM_ORDER_COL_MAJOR));

#if HAVE_SCDC_DEBUG
  if (N > 0) memset(A, 0, ((N - 1) * LDA + M) * sizeof(double));
#endif

  TRACE_F("%s: M: %d, N: %d, A: %p, LDA: %d", __func__, M, N, A, LDA);

  BLAS_CALL(put_output_param_matrix_double)(bc, "A", A, M, N, LDA, RCM_TYPE_DENSE|RCM_ORDER_COL_MAJOR);

/*  BLAS_TIMING_REMOTE_PUT(bc, libblas_scdc_timing_remote[0]);*/

  BLAS_TIMING_PRINT_F("%s: %f", __func__, libblas_scdc_timing_remote[0]);

  TRACE_F("%s: return", __func__);
}


static void do_dgeinout(blas_call_t *bc)
{
  TRACE_F("%s: bc: %p", __func__, bc);

  BLAS_TIMING_INIT_();

  int M, N;
  BLAS_CALL(get_input_conf_int)(bc, "M", &M);
  BLAS_CALL(get_input_conf_int)(bc, "N", &N);

  double *A = 0;
  int LDA = 0, rcma;
  BLAS_CALL(get_inout_param_matrix_double)(bc, "A", &A, &M, &N, &LDA, &rcma);

  ASSERT(rcma == (RCM_TYPE_DENSE|RCM_ORDER_COL_MAJOR));

#if HAVE_SCDC_DEBUG
  if (N > 0) memset(A, 0, ((N - 1) * LDA + M) * sizeof(double));
#endif

  TRACE_F("%s: M: %d, N: %d, A: %p, LDA: %d", __func__, M, N, A, LDA);

  BLAS_CALL(put_output_param_matrix_double)(bc, "A", A, M, N, LDA, RCM_TYPE_DENSE|RCM_ORDER_COL_MAJOR);

/*  BLAS_TIMING_REMOTE_PUT(bc, libblas_scdc_timing_remote[0]);*/

  BLAS_TIMING_PRINT_F("%s: %f", __func__, libblas_scdc_timing_remote[0]);

  TRACE_F("%s: return", __func__);
}

#endif /* BLAS_BENCH */


/* BLAS level 1 */
#if BLAS_LEVEL1

#if BLAS_IDAMAX

static void do_idamax(blas_call_t *bc)
{
  TRACE_F("%s: bc: %p", __func__, bc);

  BLAS_TIMING_INIT_();

  int N;
  BLAS_CALL(get_input_conf_int)(bc, "N", &N);

  double *DX = 0;
  int INCX = 0;
  BLAS_CALL(get_input_param_vector_double)(bc, "DX", &DX, &N, &INCX);

  TRACE_F("%s: N: %d, DX: %p, INCX: %d", __func__, N, DX, INCX);

  BLAS_ORIG_F_INIT(idamax_);
  BLAS_TIMING_START(libblas_scdc_timing_remote[0]);
  int r = BLAS_ORIG_F(MANGLE_BLAS(idamax_))(&N, DX, &INCX);
  BLAS_TIMING_STOP(libblas_scdc_timing_remote[0]);

  TRACE_F("%s: N: %d, DX: %p, INCX: %d, r: %d", __func__, N, DX, INCX, r);

  BLAS_CALL(put_output_conf_int)(bc, "r", r);

  BLAS_TIMING_REMOTE_PUT(bc, libblas_scdc_timing_remote[0]);

  BLAS_TIMING_PRINT_F("%s: %f", __func__, libblas_scdc_timing_remote[0]);

  TRACE_F("%s: return", __func__);
}

#endif /* BLAS_IDAMAX */


#if BLAS_DCOPY

static void do_dcopy(blas_call_t *bc)
{
  TRACE_F("%s: bc: %p", __func__, bc);

  BLAS_TIMING_INIT_();

  int N;
  BLAS_CALL(get_input_conf_int)(bc, "N", &N);

  double *DX = 0, *DY = 0;
  int INCX = 0, INCY = 0;
  BLAS_CALL(get_input_param_vector_double)(bc, "DX", &DX, &N, &INCX);
  BLAS_CALL(get_output_param_vector_double)(bc, "DY", &DY, &N, &INCY);

  TRACE_F("%s: N: %d, DX: %p, INCX: %d, DY: %p, INCY: %d", __func__, N, DX, INCX, DY, INCY);

  BLAS_ORIG_F_INIT(dcopy_);
  BLAS_TIMING_START(libblas_scdc_timing_remote[0]);
  BLAS_ORIG_F(MANGLE_BLAS(dcopy_))(&N, DX, &INCX, DY, &INCY);
  BLAS_TIMING_STOP(libblas_scdc_timing_remote[0]);

  TRACE_F("%s: N: %d, DX: %p, INCX: %d, DY: %p, INCY: %d", __func__, N, DX, INCX, DY, INCY);

  BLAS_CALL(put_output_param_vector_double)(bc, "DY", DY, N, INCY);

  BLAS_TIMING_REMOTE_PUT(bc, libblas_scdc_timing_remote[0]);

  BLAS_TIMING_PRINT_F("%s: %f", __func__, libblas_scdc_timing_remote[0]);

  TRACE_F("%s: return", __func__);
}

#endif /* BLAS_DCOPY */


#if BLAS_DSCAL

static void do_dscal(blas_call_t *bc)
{
  TRACE_F("%s: bc: %p", __func__, bc);

  BLAS_TIMING_INIT_();

  int N;
  BLAS_CALL(get_input_conf_int)(bc, "N", &N);

  double DA;
  BLAS_CALL(get_input_conf_double)(bc, "DA", &DA);

  double *DX = 0;
  int INCX = 0;
  BLAS_CALL(get_inout_param_vector_double)(bc, "DX", &DX, &N, &INCX);

  TRACE_F("%s: N: %d, DA: %e, DX: %p, INCX: %d", __func__, N, DA, DX, INCX);

  BLAS_ORIG_F_INIT(dscal_);
  BLAS_TIMING_START(libblas_scdc_timing_remote[0]);
  BLAS_ORIG_F(MANGLE_BLAS(dscal_))(&N, &DA, DX, &INCX);
  BLAS_TIMING_STOP(libblas_scdc_timing_remote[0]);

  TRACE_F("%s: N: %d, DA: %e, DX: %p, INCX: %d", __func__, N, DA, DX, INCX);

  BLAS_CALL(put_output_param_vector_double)(bc, "DX", DX, N, INCX);

  BLAS_TIMING_REMOTE_PUT(bc, libblas_scdc_timing_remote[0]);

  BLAS_TIMING_PRINT_F("%s: %f", __func__, libblas_scdc_timing_remote[0]);

  TRACE_F("%s: return", __func__);
}

#endif /* BLAS_DSCAL */


#if BLAS_DAXPY

static void do_daxpy(blas_call_t *bc)
{
  TRACE_F("%s: bc: %p", __func__, bc);

  BLAS_TIMING_INIT_();

  int N;
  BLAS_CALL(get_input_conf_int)(bc, "N", &N);

  double DA;
  BLAS_CALL(get_input_conf_double)(bc, "DA", &DA);

  double *DX = 0, *DY = 0;
  int INCX = 0, INCY = 0;
  BLAS_CALL(get_input_param_vector_double)(bc, "DX", &DX, &N, &INCX);
  BLAS_CALL(get_inout_param_vector_double)(bc, "DY", &DY, &N, &INCY);

  TRACE_F("%s: N: %d, DA: %e, DX: %p, INCX: %d, DY: %p, INCY: %d", __func__, N, DA, DX, INCX, DY, INCY);

  BLAS_ORIG_F_INIT(daxpy_);
  BLAS_TIMING_START(libblas_scdc_timing_remote[0]);
  BLAS_ORIG_F(MANGLE_BLAS(daxpy_))(&N, &DA, DX, &INCX, DY, &INCY);
  BLAS_TIMING_STOP(libblas_scdc_timing_remote[0]);

  TRACE_F("%s: N: %d, DA: %e, DX: %p, INCX: %d, DY: %p, INCY: %d", __func__, N, DA, DX, INCX, DY, INCY);

  BLAS_CALL(put_output_param_vector_double)(bc, "DY", DY, N, INCY);

  BLAS_TIMING_REMOTE_PUT(bc, libblas_scdc_timing_remote[0]);

  BLAS_TIMING_PRINT_F("%s: %f", __func__, libblas_scdc_timing_remote[0]);

  TRACE_F("%s: return", __func__);
}

#endif /* BLAS_DAXPY */

#endif /* BLAS_LEVEL1 */


/* BLAS level 2 */
#if BLAS_LEVEL2

#if BLAS_DGER

static void do_dger(blas_call_t *bc)
{
  TRACE_F("%s: bc: %p", __func__, bc);

  BLAS_TIMING_INIT_();

  int M, N;
  BLAS_CALL(get_input_conf_int)(bc, "M", &M);
  BLAS_CALL(get_input_conf_int)(bc, "N", &N);

  double ALPHA;
  BLAS_CALL(get_input_conf_double)(bc, "ALPHA", &ALPHA);

  double *X = 0, *Y = 0;
  int INCX = 0, INCY = 0;
  BLAS_CALL(get_input_param_vector_double)(bc, "X", &X, &M, &INCX);
  BLAS_CALL(get_input_param_vector_double)(bc, "Y", &Y, &N, &INCY);

  double *A = 0;
  int LDA = 0, rcma;
  BLAS_CALL(get_inout_param_matrix_double)(bc, "A", &A, &M, &N, &LDA, &rcma);

  ASSERT(rcma == (RCM_TYPE_DENSE|RCM_ORDER_COL_MAJOR));

  TRACE_F("%s: M: %d, N: %d, ALPHA: %e, X: %p, INCX: %d, Y: %p, INCY: %d, A: %p, LDA: %d", __func__, M, N, ALPHA, X, INCX, Y, INCY, A, LDA);

  BLAS_ORIG_F_INIT(dger_);
  BLAS_TIMING_START(libblas_scdc_timing_remote[0]);
  BLAS_ORIG_F(MANGLE_BLAS(dger_))(&M, &N, &ALPHA, X, &INCX, Y, &INCY, A, &LDA);
  BLAS_TIMING_STOP(libblas_scdc_timing_remote[0]);

  TRACE_F("%s: M: %d, N: %d, ALPHA: %e, X: %p, INCX: %d, Y: %p, INCY: %d, A: %p, LDA: %d", __func__, M, N, ALPHA, X, INCX, Y, INCY, A, LDA);

  BLAS_CALL(put_output_param_matrix_double)(bc, "A", A, M, N, LDA, RCM_TYPE_DENSE|RCM_ORDER_COL_MAJOR);

  BLAS_TIMING_REMOTE_PUT(bc, libblas_scdc_timing_remote[0]);

  BLAS_TIMING_PRINT_F("%s: %f", __func__, libblas_scdc_timing_remote[0]);

  TRACE_F("%s: return", __func__);
}

#endif /* BLAS_DGER */


#if BLAS_DGEMV

static void do_dgemv(blas_call_t *bc)
{
  TRACE_F("%s: bc: %p", __func__, bc);

  BLAS_TIMING_INIT_();

  char TRANS;
  BLAS_CALL(get_input_conf_char)(bc, "TRANS", &TRANS);

  int M, N;
  BLAS_CALL(get_input_conf_int)(bc, "M", &M);
  BLAS_CALL(get_input_conf_int)(bc, "N", &N);

  double ALPHA;
  BLAS_CALL(get_input_conf_double)(bc, "ALPHA", &ALPHA);

  double *A = 0;
  int LDA = 0, rcma;
  BLAS_CALL(get_input_param_matrix_double)(bc, "A", &A, &M, &N, &LDA, &rcma);

  ASSERT(rcma == (RCM_TYPE_DENSE|RCM_ORDER_COL_MAJOR));

  double *X = 0;
  rdint_t NX = NETLIB_TRANS_GET_NCOLS(TRANS, M, N);
  int INCX = 0;
  BLAS_CALL(get_input_param_vector_double)(bc, "X", &X, &NX, &INCX);

  double BETA;
  BLAS_CALL(get_input_conf_double)(bc, "BETA", &BETA);

  double *Y = 0;
  rdint_t NY = NETLIB_TRANS_GET_NROWS(TRANS, M, N);
  int INCY = 0;
  BLAS_CALL(get_inout_param_vector_double)(bc, "Y", &Y, &NY, &INCY);

  TRACE_F("%s: TRANS: %c, M: %d, N: %d, ALPHA: %e, A: %p, LDA: %d, X: %p, INCX: %d, BETA: %e, Y: %p, INCY: %d", __func__, TRANS, M, N, ALPHA, A, LDA, X, INCX, BETA, Y, INCY);

  BLAS_ORIG_F_INIT(dgemv_);
  BLAS_TIMING_START(libblas_scdc_timing_remote[0]);
  BLAS_ORIG_F(MANGLE_BLAS(dgemv_))(&TRANS, &M, &N, &ALPHA, A, &LDA, X, &INCX, &BETA, Y, &INCY);
  BLAS_TIMING_STOP(libblas_scdc_timing_remote[0]);

  TRACE_F("%s: TRANS: %c, M: %d, N: %d, ALPHA: %e, A: %p, LDA: %d, X: %p, INCX: %d, BETA: %e, Y: %p, INCY: %d", __func__, TRANS, M, N, ALPHA, A, LDA, X, INCX, BETA, Y, INCY);

  BLAS_CALL(put_output_param_vector_double)(bc, "Y", Y, NETLIB_TRANS_GET_NROWS(TRANS, M, N), INCY);

  BLAS_TIMING_REMOTE_PUT(bc, libblas_scdc_timing_remote[0]);

  BLAS_TIMING_PRINT_F("%s: %f", __func__, libblas_scdc_timing_remote[0]);

  TRACE_F("%s: return", __func__);
}

#endif /* BLAS_DGEMV */


#if BLAS_DTRSV

static void do_dtrsv(blas_call_t *bc)
{
  TRACE_F("%s: bc: %p", __func__, bc);

  BLAS_TIMING_INIT_();

  char UPLO, TRANS, DIAG;
  BLAS_CALL(get_input_conf_char)(bc, "UPLO", &UPLO);
  BLAS_CALL(get_input_conf_char)(bc, "TRANS", &TRANS);
  BLAS_CALL(get_input_conf_char)(bc, "DIAG", &DIAG);

  int N;
  BLAS_CALL(get_input_conf_int)(bc, "N", &N);

  double *A = 0;
  int LDA = 0, rcma;
  BLAS_CALL(get_input_param_matrix_double)(bc, "A", &A, &N, &N, &LDA, &rcma);

  ASSERT(RCM_GET_ORDER(rcma) == RCM_ORDER_COL_MAJOR);

  double *X = 0;
  int INCX = 0;
  BLAS_CALL(get_inout_param_vector_double)(bc, "X", &X, &N, &INCX);

  TRACE_F("%s: UPLO: %c, TRANS: %c, DIAG: %c, N: %d, A: %p, LDA: %d, X: %p, INCX: %d", __func__, UPLO, TRANS, DIAG, N, A, LDA, X, INCX);

  BLAS_ORIG_F_INIT(dtrsv_);
  BLAS_TIMING_START(libblas_scdc_timing_remote[0]);
  BLAS_ORIG_F(MANGLE_BLAS(dtrsv_))(&UPLO, &TRANS, &DIAG, &N, A, &LDA, X, &INCX);
  BLAS_TIMING_STOP(libblas_scdc_timing_remote[0]);

  TRACE_F("%s: UPLO: %c, TRANS: %c, DIAG: %c, N: %d, A: %p, LDA: %d, X: %p, INCX: %d", __func__, UPLO, TRANS, DIAG, N, A, LDA, X, INCX);

  BLAS_CALL(put_output_param_vector_double)(bc, "X", X, N, INCX);

  BLAS_TIMING_REMOTE_PUT(bc, libblas_scdc_timing_remote[0]);

  BLAS_TIMING_PRINT_F("%s: %f", __func__, libblas_scdc_timing_remote[0]);

  TRACE_F("%s: return", __func__);
}

#endif /* BLAS_DTRSV */

#endif /* BLAS_LEVEL2 */


/* BLAS level 3 */
#if BLAS_LEVEL3

#if BLAS_DGEMM

static void do_dgemm(blas_call_t *bc)
{
  TRACE_F("%s: bc: %p", __func__, bc);

  BLAS_TIMING_INIT_();

  char TRANSA, TRANSB;
  BLAS_CALL(get_input_conf_char)(bc, "TRANSA", &TRANSA);
  BLAS_CALL(get_input_conf_char)(bc, "TRANSB", &TRANSB);

  int M, N, K;
  BLAS_CALL(get_input_conf_int)(bc, "M", &M);
  BLAS_CALL(get_input_conf_int)(bc, "N", &N);
  BLAS_CALL(get_input_conf_int)(bc, "K", &K);

  double ALPHA;
  BLAS_CALL(get_input_conf_double)(bc, "ALPHA", &ALPHA);

  double *A = 0, *B = 0;
  rdint_t NRA = NETLIB_TRANS_GET_NROWS(TRANSA, M, K);
  rdint_t NCA = NETLIB_TRANS_GET_NCOLS(TRANSA, M, K);
  rdint_t NRB = NETLIB_TRANS_GET_NROWS(TRANSB, K, N);
  rdint_t NCB = NETLIB_TRANS_GET_NCOLS(TRANSB, K, N);
  int LDA = 0, rcma, LDB = 0, rcmb;
  BLAS_CALL(get_input_param_matrix_double)(bc, "A", &A, &NRA, &NCA, &LDA, &rcma);
  BLAS_CALL(get_input_param_matrix_double)(bc, "B", &B, &NRB, &NCB, &LDB, &rcmb);

  ASSERT(rcma == (RCM_TYPE_DENSE|RCM_ORDER_COL_MAJOR));
  ASSERT(rcmb == (RCM_TYPE_DENSE|RCM_ORDER_COL_MAJOR));

  double BETA;
  BLAS_CALL(get_input_conf_double)(bc, "BETA", &BETA);

  double *C = 0;
  int LDC = 0, rcmc;
  BLAS_CALL(get_inout_param_matrix_double)(bc, "C", &C, &M, &N, &LDC, &rcmc);

  ASSERT(rcmc == (RCM_TYPE_DENSE|RCM_ORDER_COL_MAJOR));

  TRACE_F("%s: TRANSA: %c, TRANSB: %c, M: %d, N: %d, K: %d, ALPHA: %e, A: %p, LDA: %d, B: %p, LDB: %d, BETA: %e, C: %p, LDC: %d", __func__, TRANSA, TRANSB, M, N, K, ALPHA, A, LDA, B, LDB, BETA, C, LDC);

  BLAS_ORIG_F_INIT(dgemm_);
  BLAS_TIMING_START(libblas_scdc_timing_remote[0]);
  BLAS_ORIG_F(MANGLE_BLAS(dgemm_))(&TRANSA, &TRANSB, &M, &N, &K, &ALPHA, A, &LDA, B, &LDB, &BETA, C, &LDC);
  BLAS_TIMING_STOP(libblas_scdc_timing_remote[0]);

  TRACE_F("%s: TRANSA: %c, TRANSB: %c, M: %d, N: %d, K: %d, ALPHA: %e, A: %p, LDA: %d, B: %p, LDB: %d, BETA: %e, C: %p, LDC: %d", __func__, TRANSA, TRANSB, M, N, K, ALPHA, A, LDA, B, LDB, BETA, C, LDC);

  BLAS_CALL(put_output_param_matrix_double)(bc, "C", C, M, N, LDC, rcmc);

  BLAS_TIMING_REMOTE_PUT(bc, libblas_scdc_timing_remote[0]);

  BLAS_TIMING_PRINT_F("%s: %f", __func__, libblas_scdc_timing_remote[0]);

  TRACE_F("%s: return", __func__);
}

#endif /* BLAS_DGEMM */


#if BLAS_DTRSM

static void do_dtrsm(blas_call_t *bc)
{
  TRACE_F("%s: bc: %p", __func__, bc);

  BLAS_TIMING_INIT_();

  char SIDE, UPLO, TRANSA, DIAG;
  BLAS_CALL(get_input_conf_char)(bc, "SIDE", &SIDE);
  BLAS_CALL(get_input_conf_char)(bc, "UPLO", &UPLO);
  BLAS_CALL(get_input_conf_char)(bc, "TRANSA", &TRANSA);
  BLAS_CALL(get_input_conf_char)(bc, "DIAG", &DIAG);

  int M, N;
  BLAS_CALL(get_input_conf_int)(bc, "M", &M);
  BLAS_CALL(get_input_conf_int)(bc, "N", &N);

  double ALPHA;
  BLAS_CALL(get_input_conf_double)(bc, "ALPHA", &ALPHA);

  double *A = 0, *B = 0;
  rdint_t K = NETLIB_SIDE_IS_LEFT(SIDE)?M:N;
  int LDA = 0, rcma, LDB = 0, rcmb;
  BLAS_CALL(get_input_param_matrix_double)(bc, "A", &A, &K, &K, &LDA, &rcma);
  BLAS_CALL(get_inout_param_matrix_double)(bc, "B", &B, &M, &N, &LDB, &rcmb);

  ASSERT(RCM_GET_ORDER(rcma) == RCM_ORDER_COL_MAJOR);
  ASSERT(rcmb == (RCM_TYPE_DENSE|RCM_ORDER_COL_MAJOR));

  TRACE_F("%s: SIDE: %c, UPLO: %c, TRANSA: %c, DIAG: %c, M: %d, N: %d, ALPHA: %e, A: %p, LDA: %d, B: %p, LDB: %d", __func__, SIDE, UPLO, TRANSA, DIAG, M, N, ALPHA, A, LDA, B, LDB);

  BLAS_ORIG_F_INIT(dtrsm_);
  BLAS_TIMING_START(libblas_scdc_timing_remote[0]);
  BLAS_ORIG_F(MANGLE_BLAS(dtrsm_))(&SIDE, &UPLO, &TRANSA, &DIAG, &M, &N, &ALPHA, A, &LDA, B, &LDB);
  BLAS_TIMING_STOP(libblas_scdc_timing_remote[0]);

  TRACE_F("%s: SIDE: %c, UPLO: %c, TRANSA: %c, DIAG: %c, M: %d, N: %d, ALPHA: %e, A: %p, LDA: %d, B: %p, LDB: %d", __func__, SIDE, UPLO, TRANSA, DIAG, M, N, ALPHA, A, LDA, B, LDB);

  BLAS_CALL(put_output_param_matrix_double)(bc, "B", B, M, N, LDB, rcmb);

  BLAS_TIMING_REMOTE_PUT(bc, libblas_scdc_timing_remote[0]);

  BLAS_TIMING_PRINT_F("%s: %f", __func__, libblas_scdc_timing_remote[0]);

  TRACE_F("%s: return", __func__);
}

#endif /* BLAS_DTRSM */

#endif /* BLAS_LEVEL3 */


void *blas_scdc_open(const char *conf, va_list ap)
{
  TRACE_F("%s: conf: '%s'", __func__, conf);

  BLAS_ORIG_INIT();

  blas_call_t *bc = malloc(sizeof(blas_call_t));

  TRACE_F("%s: return: %p", __func__, bc);

  return bc;
}


scdcint_t blas_scdc_close(void *dataprov)
{
  TRACE_F("%s: dataprov: %p", __func__, dataprov);

  blas_call_t *bc = dataprov;

  free(bc);

  BLAS_ORIG_RELEASE();

  TRACE_F("%s: return", __func__);

  return SCDC_SUCCESS;
}


/*void *blas_scdc_dataset_open(void *dataprov, const char *path)
{
  printf(PREFIX "blas_scdc_dataset_open:\n");
  printf(PREFIX "  dataprov: '%p'\n", dataprov);
  printf(PREFIX "  path: '%s'\n", path);

  blas_call_t *bc = malloc(sizeof(blas_call_t));

  printf(PREFIX "  bc: '%p'\n", path);

  return bc;
}


scdcint_t blas_scdc_dataset_close(void *dataprov, void *dataset)
{
  printf(PREFIX "blas_scdc_dataset_close:\n");
  printf(PREFIX "  dataprov: '%p'\n", dataprov);
  printf(PREFIX "  dataset: '%p'\n", dataset);

  blas_call_t *bc = dataset;

  free(bc);

  return SCDC_SUCCESS;
}*/


scdcint_t blas_scdc_dataset_cmd(void *dataprov, void *dataset, const char *cmd, const char *params, scdc_dataset_input_t *input, scdc_dataset_output_t *output)
{
  TRACE_F("%s: dataprov: %p, dataset: %p, cmd: '%s', params: '%s', input: %p, output: %p", __func__, dataprov, dataset, cmd, params, input, output);

#if LIBBLAS_SCDC_PROGRESS
  printf("%s: cmd: %s, params: %s\n", __func__, cmd, params);
#endif

  TRACE_F_N("%s: input: ", __func__); TRACE_CMD(scdc_dataset_input_print(input);); TRACE_R("\n");
  TRACE_F_N("%s: output: ", __func__); TRACE_CMD(scdc_dataset_output_print(output);); TRACE_R("\n");

  blas_call_t *bc = dataprov;

  BLAS_CALL(init_scdc)(bc, params, input, output);

#if BLAS_BENCH
  if (strcmp(cmd, "dvin") == 0) do_dvin(bc); else
  if (strcmp(cmd, "dvout") == 0) do_dvout(bc); else
  if (strcmp(cmd, "dvinout") == 0) do_dvinout(bc); else
  if (strcmp(cmd, "dgein") == 0) do_dgein(bc); else
  if (strcmp(cmd, "dgeout") == 0) do_dgeout(bc); else
  if (strcmp(cmd, "dgeinout") == 0) do_dgeinout(bc); else
#endif
#if BLAS_LEVEL1
# if BLAS_IDAMAX
  if (strcmp(cmd, "idamax") == 0) do_idamax(bc); else
# endif
# if BLAS_DCOPY
  if (strcmp(cmd, "dcopy") == 0) do_dcopy(bc); else
# endif
# if BLAS_DSCAL
  if (strcmp(cmd, "dscal") == 0) do_dscal(bc); else
# endif
# if BLAS_DAXPY
  if (strcmp(cmd, "daxpy") == 0) do_daxpy(bc); else
# endif
#endif /* BLAS_LEVEL1 */
#if BLAS_LEVEL2
# if BLAS_DGER
  if (strcmp(cmd, "dger") == 0) do_dger(bc); else
# endif
# if BLAS_DGEMV
  if (strcmp(cmd, "dgemv") == 0) do_dgemv(bc); else
# endif
# if BLAS_DTRSV
  if (strcmp(cmd, "dtrsv") == 0) do_dtrsv(bc); else
# endif
#endif /* BLAS_LEVEL2 */
#if BLAS_LEVEL3
# if BLAS_DGEMM
  if (strcmp(cmd, "dgemm") == 0) do_dgemm(bc); else
# endif
# if BLAS_DTRSM
  if (strcmp(cmd, "dtrsm") == 0) do_dtrsm(bc); else
# endif
#endif /* BLAS_LEVEL3 */
  {
    TRACE_F("%s: command '%s' not supported!", __func__, cmd);
  }

  BLAS_CALL(release_scdc)(bc);

  TRACE_F("%s: return", __func__);

  return SCDC_SUCCESS;
}
