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
  TRACE_F("bc: %p", bc);

  BLAS_TIMING_INIT_();

  int N;
  BLAS_CALL(get_input_conf_int)(bc, "N", &N);

  double *DX = 0;
  rdint_t NX_ = N, INCX_ = 0;
  BLAS_CALL(get_input_param_vector_double)(bc, "DX", &DX, &NX_, &INCX_);

  ASSERT(NX_ == N);
  int INCX = INCX_;

  TRACE_F("N: %d, DX: %p, INCX: %d", N, DX, INCX);

/*  BLAS_TIMING_REMOTE_PUT(bc, libblas_scdc_timing_remote[0]);*/

  BLAS_TIMING_PRINT_F("%s: %f", __func__, libblas_scdc_timing_remote[0]);

  TRACE_F("return");
}


static void do_dvout(blas_call_t *bc)
{
  TRACE_F("bc: %p", bc);

  BLAS_TIMING_INIT_();

  int N;
  BLAS_CALL(get_input_conf_int)(bc, "N", &N);

  double *DX = 0;
  rdint_t NX_ = N, INCX_ = 0;
  BLAS_CALL(get_output_param_vector_double)(bc, "DX", &DX, &NX_, &INCX_);

  ASSERT(NX_ == N);
  int INCX = INCX_;

#if HAVE_SCDC_DEBUG
  if (N > 0) memset(DX, 0, ((N - 1) * INCX + 1) * sizeof(double));
#endif

  TRACE_F("N: %d, DX: %p, INCX: %d", N, DX, INCX);

  BLAS_CALL(put_output_param_vector_double)(bc, "DX", DX, N, INCX);

/*  BLAS_TIMING_REMOTE_PUT(bc, libblas_scdc_timing_remote[0]);*/

  BLAS_TIMING_PRINT_F("%s: %f", __func__, libblas_scdc_timing_remote[0]);

  TRACE_F("return");
}


static void do_dvinout(blas_call_t *bc)
{
  TRACE_F("bc: %p", bc);

  BLAS_TIMING_INIT_();

  int N;
  BLAS_CALL(get_input_conf_int)(bc, "N", &N);

  double *DX = 0;
  rdint_t NX_ = N, INCX_ = 0;
  BLAS_CALL(get_inout_param_vector_double)(bc, "DX", &DX, &NX_, &INCX_);

  ASSERT(NX_ == N);
  int INCX = INCX_;

#if HAVE_SCDC_DEBUG
  if (N > 0) memset(DX, 0, ((N - 1) * INCX + 1) * sizeof(double));
#endif

  TRACE_F("N: %d, DX: %p, INCX: %d", N, DX, INCX);

  BLAS_CALL(put_output_param_vector_double)(bc, "DX", DX, N, INCX);

/*  BLAS_TIMING_REMOTE_PUT(bc, libblas_scdc_timing_remote[0]);*/

  BLAS_TIMING_PRINT_F("%s: %f", __func__, libblas_scdc_timing_remote[0]);

  TRACE_F("return");
}


static void do_dgein(blas_call_t *bc)
{
  TRACE_F("bc: %p", bc);

  BLAS_TIMING_INIT_();

  int M, N;
  BLAS_CALL(get_input_conf_int)(bc, "M", &M);
  BLAS_CALL(get_input_conf_int)(bc, "N", &N);

  double *A = 0;
  rdint_t NRA_ = M, NCA_ = N, LDA_ = 0, rcma;
  BLAS_CALL(get_input_param_matrix_double)(bc, "A", &A, &NRA_, &NCA_, &LDA_, &rcma);

  ASSERT(NRA_ == M && NCA_ == N && rcma == (RCM_TYPE_DENSE|RCM_ORDER_COL_MAJOR));
  int LDA = LDA_;

  TRACE_F("M: %d, N: %d, A: %p, LDA: %d", M, N, A, LDA);

/*  BLAS_TIMING_REMOTE_PUT(bc, libblas_scdc_timing_remote[0]);*/

  BLAS_TIMING_PRINT_F("%s: %f", __func__, libblas_scdc_timing_remote[0]);

  TRACE_F("return");
}


static void do_dgeout(blas_call_t *bc)
{
  TRACE_F("bc: %p", bc);

  BLAS_TIMING_INIT_();

  int M, N;
  BLAS_CALL(get_input_conf_int)(bc, "M", &M);
  BLAS_CALL(get_input_conf_int)(bc, "N", &N);

  double *A = 0;
  rdint_t NRA_ = M, NCA_ = N, LDA_ = 0, rcma;
  BLAS_CALL(get_output_param_matrix_double)(bc, "A", &A, &NRA_, &NCA_, &LDA_, &rcma);

  ASSERT(NRA_ == M && NCA_ == N && rcma == (RCM_TYPE_DENSE|RCM_ORDER_COL_MAJOR));
  int LDA = LDA_;

#if HAVE_SCDC_DEBUG
  if (N > 0) memset(A, 0, ((N - 1) * LDA + M) * sizeof(double));
#endif

  TRACE_F("M: %d, N: %d, A: %p, LDA: %d", M, N, A, LDA);

  BLAS_CALL(put_output_param_matrix_double)(bc, "A", A, M, N, LDA, RCM_TYPE_DENSE|RCM_ORDER_COL_MAJOR);

/*  BLAS_TIMING_REMOTE_PUT(bc, libblas_scdc_timing_remote[0]);*/

  BLAS_TIMING_PRINT_F("%s: %f", __func__, libblas_scdc_timing_remote[0]);

  TRACE_F("return");
}


static void do_dgeinout(blas_call_t *bc)
{
  TRACE_F("bc: %p", bc);

  BLAS_TIMING_INIT_();

  int M, N;
  BLAS_CALL(get_input_conf_int)(bc, "M", &M);
  BLAS_CALL(get_input_conf_int)(bc, "N", &N);

  double *A = 0;
  rdint_t NRA_ = M, NCA_ = N, LDA_ = 0, rcma;
  BLAS_CALL(get_inout_param_matrix_double)(bc, "A", &A, &NRA_, &NCA_, &LDA_, &rcma);

  ASSERT(NRA_ == M && NCA_ == N && rcma == (RCM_TYPE_DENSE|RCM_ORDER_COL_MAJOR));
  int LDA = LDA_;

#if HAVE_SCDC_DEBUG
  if (N > 0) memset(A, 0, ((N - 1) * LDA + M) * sizeof(double));
#endif

  TRACE_F("M: %d, N: %d, A: %p, LDA: %d", M, N, A, LDA);

  BLAS_CALL(put_output_param_matrix_double)(bc, "A", A, M, N, LDA, RCM_TYPE_DENSE|RCM_ORDER_COL_MAJOR);

/*  BLAS_TIMING_REMOTE_PUT(bc, libblas_scdc_timing_remote[0]);*/

  BLAS_TIMING_PRINT_F("%s: %f", __func__, libblas_scdc_timing_remote[0]);

  TRACE_F("return");
}

#endif /* BLAS_BENCH */


/* BLAS level 1 */
#if BLAS_LEVEL1

#if BLAS_IDAMAX

static void do_idamax(blas_call_t *bc)
{
  TRACE_F("bc: %p", bc);

  BLAS_TIMING_INIT_();

  int N;
  BLAS_CALL(get_input_conf_int)(bc, "N", &N);

  double *DX = 0;
  rdint_t NX_ = N, INCX_ = 0;
  BLAS_CALL(get_input_param_vector_double)(bc, "DX", &DX, &NX_, &INCX_);

  ASSERT(NX_ == N);
  int INCX = INCX_;

  TRACE_F("N: %d, DX: %p, INCX: %d", N, DX, INCX);

  BLAS_ORIG_F_INIT(idamax_);
  BLAS_TIMING_START(libblas_scdc_timing_remote[0]);
  int r = BLAS_ORIG_F(MANGLE_BLAS(idamax_))(&N, DX, &INCX);
  BLAS_TIMING_STOP(libblas_scdc_timing_remote[0]);

  TRACE_F("N: %d, DX: %p, INCX: %d, r: %d", N, DX, INCX, r);

  BLAS_CALL(put_output_conf_int)(bc, "r", r);

  BLAS_TIMING_REMOTE_PUT(bc, libblas_scdc_timing_remote[0]);

  BLAS_TIMING_PRINT_F("%s: %f", __func__, libblas_scdc_timing_remote[0]);

  TRACE_F("return");
}

#endif /* BLAS_IDAMAX */


#if BLAS_DCOPY

static void do_dcopy(blas_call_t *bc)
{
  TRACE_F("bc: %p", bc);

  BLAS_TIMING_INIT_();

  int N;
  BLAS_CALL(get_input_conf_int)(bc, "N", &N);

  double *DX = 0, *DY = 0;
  rdint_t NX_ = N, NY_ = N, INCX_ = 0, INCY_ = 0;
  BLAS_CALL(get_input_param_vector_double)(bc, "DX", &DX, &NX_, &INCX_);
  BLAS_CALL(get_output_param_vector_double)(bc, "DY", &DY, &NY_, &INCY_);

  ASSERT(NX_ == N && NY_ == N);
  int INCX = INCX_, INCY = INCY_;

  TRACE_F("N: %d, DX: %p, INCX: %d, DY: %p, INCY: %d", N, DX, INCX, DY, INCY);

  BLAS_ORIG_F_INIT(dcopy_);
  BLAS_TIMING_START(libblas_scdc_timing_remote[0]);
  BLAS_ORIG_F(MANGLE_BLAS(dcopy_))(&N, DX, &INCX, DY, &INCY);
  BLAS_TIMING_STOP(libblas_scdc_timing_remote[0]);

  TRACE_F("N: %d, DX: %p, INCX: %d, DY: %p, INCY: %d", N, DX, INCX, DY, INCY);

  BLAS_CALL(put_output_param_vector_double)(bc, "DY", DY, N, INCY);

  BLAS_TIMING_REMOTE_PUT(bc, libblas_scdc_timing_remote[0]);

  BLAS_TIMING_PRINT_F("%s: %f", __func__, libblas_scdc_timing_remote[0]);

  TRACE_F("return");
}

#endif /* BLAS_DCOPY */


#if BLAS_DSCAL

static void do_dscal(blas_call_t *bc)
{
  TRACE_F("bc: %p", bc);

  BLAS_TIMING_INIT_();

  int N;
  BLAS_CALL(get_input_conf_int)(bc, "N", &N);

  double DA;
  BLAS_CALL(get_input_conf_double)(bc, "DA", &DA);

  double *DX = 0;
  rdint_t NX_ = N, INCX_ = 0;
  BLAS_CALL(get_inout_param_vector_double)(bc, "DX", &DX, &NX_, &INCX_);

  ASSERT(NX_ == N);
  int INCX = INCX_;

  TRACE_F("N: %d, DA: %e, DX: %p, INCX: %d", N, DA, DX, INCX);

  BLAS_ORIG_F_INIT(dscal_);
  BLAS_TIMING_START(libblas_scdc_timing_remote[0]);
  BLAS_ORIG_F(MANGLE_BLAS(dscal_))(&N, &DA, DX, &INCX);
  BLAS_TIMING_STOP(libblas_scdc_timing_remote[0]);

  TRACE_F("N: %d, DA: %e, DX: %p, INCX: %d", N, DA, DX, INCX);

  BLAS_CALL(put_output_param_vector_double)(bc, "DX", DX, N, INCX);

  BLAS_TIMING_REMOTE_PUT(bc, libblas_scdc_timing_remote[0]);

  BLAS_TIMING_PRINT_F("%s: %f", __func__, libblas_scdc_timing_remote[0]);

  TRACE_F("return");
}

#endif /* BLAS_DSCAL */


#if BLAS_DAXPY

static void do_daxpy(blas_call_t *bc)
{
  TRACE_F("bc: %p", bc);

  BLAS_TIMING_INIT_();

  int N;
  BLAS_CALL(get_input_conf_int)(bc, "N", &N);

  double DA;
  BLAS_CALL(get_input_conf_double)(bc, "DA", &DA);

  double *DX = 0, *DY = 0;
  rdint_t NX_ = N, NY_ = N, INCX_ = 0, INCY_ = 0;
  BLAS_CALL(get_input_param_vector_double)(bc, "DX", &DX, &NX_, &INCX_);
  BLAS_CALL(get_inout_param_vector_double)(bc, "DY", &DY, &NY_, &INCY_);

  ASSERT(NX_ == N && NY_ == N);
  int INCX = INCX_, INCY = INCY_;

  TRACE_F("N: %d, DA: %e, DX: %p, INCX: %d, DY: %p, INCY: %d", N, DA, DX, INCX, DY, INCY);

  BLAS_ORIG_F_INIT(daxpy_);
  BLAS_TIMING_START(libblas_scdc_timing_remote[0]);
  BLAS_ORIG_F(MANGLE_BLAS(daxpy_))(&N, &DA, DX, &INCX, DY, &INCY);
  BLAS_TIMING_STOP(libblas_scdc_timing_remote[0]);

  TRACE_F("N: %d, DA: %e, DX: %p, INCX: %d, DY: %p, INCY: %d", N, DA, DX, INCX, DY, INCY);

  BLAS_CALL(put_output_param_vector_double)(bc, "DY", DY, N, INCY);

  BLAS_TIMING_REMOTE_PUT(bc, libblas_scdc_timing_remote[0]);

  BLAS_TIMING_PRINT_F("%s: %f", __func__, libblas_scdc_timing_remote[0]);

  TRACE_F("return");
}

#endif /* BLAS_DAXPY */

#endif /* BLAS_LEVEL1 */


/* BLAS level 2 */
#if BLAS_LEVEL2

#if BLAS_DGER

static void do_dger(blas_call_t *bc)
{
  TRACE_F("bc: %p", bc);

  BLAS_TIMING_INIT_();

  int M, N;
  BLAS_CALL(get_input_conf_int)(bc, "M", &M);
  BLAS_CALL(get_input_conf_int)(bc, "N", &N);

  double ALPHA;
  BLAS_CALL(get_input_conf_double)(bc, "ALPHA", &ALPHA);

  double *X = 0, *Y = 0;
  rdint_t NX_ = M, NY_ = N, INCX_ = 0, INCY_ = 0;
  BLAS_CALL(get_input_param_vector_double)(bc, "X", &X, &NX_, &INCX_);
  BLAS_CALL(get_input_param_vector_double)(bc, "Y", &Y, &NY_, &INCY_);

  ASSERT(NX_ == M && NY_ == N);
  int INCX = INCX_, INCY = INCY_;

  double *A = 0;
  rdint_t NRA_ = M, NCA_ = N, LDA_ = 0, rcma;
  BLAS_CALL(get_inout_param_matrix_double)(bc, "A", &A, &NRA_, &NCA_, &LDA_, &rcma);

  ASSERT(NRA_ == M && NCA_ == N && rcma == (RCM_TYPE_DENSE|RCM_ORDER_COL_MAJOR));
  int LDA = LDA_;

  TRACE_F("M: %d, N: %d, ALPHA: %e, X: %p, INCX: %d, Y: %p, INCY: %d, A: %p, LDA: %d", M, N, ALPHA, X, INCX, Y, INCY, A, LDA);

  BLAS_ORIG_F_INIT(dger_);
  BLAS_TIMING_START(libblas_scdc_timing_remote[0]);
  BLAS_ORIG_F(MANGLE_BLAS(dger_))(&M, &N, &ALPHA, X, &INCX, Y, &INCY, A, &LDA);
  BLAS_TIMING_STOP(libblas_scdc_timing_remote[0]);

  TRACE_F("M: %d, N: %d, ALPHA: %e, X: %p, INCX: %d, Y: %p, INCY: %d, A: %p, LDA: %d", M, N, ALPHA, X, INCX, Y, INCY, A, LDA);

  BLAS_CALL(put_output_param_matrix_double)(bc, "A", A, M, N, LDA, RCM_TYPE_DENSE|RCM_ORDER_COL_MAJOR);

  BLAS_TIMING_REMOTE_PUT(bc, libblas_scdc_timing_remote[0]);

  BLAS_TIMING_PRINT_F("%s: %f", __func__, libblas_scdc_timing_remote[0]);

  TRACE_F("return");
}

#endif /* BLAS_DGER */


#if BLAS_DGEMV

static void do_dgemv(blas_call_t *bc)
{
  TRACE_F("bc: %p", bc);

  BLAS_TIMING_INIT_();

  char TRANS;
  BLAS_CALL(get_input_conf_char)(bc, "TRANS", &TRANS);

  int M, N;
  BLAS_CALL(get_input_conf_int)(bc, "M", &M);
  BLAS_CALL(get_input_conf_int)(bc, "N", &N);

  double ALPHA;
  BLAS_CALL(get_input_conf_double)(bc, "ALPHA", &ALPHA);

  double *A = 0;
  rdint_t NRA_ = M, NCA_ = N, LDA_ = 0, rcma;
  BLAS_CALL(get_input_param_matrix_double)(bc, "A", &A, &NRA_, &NCA_, &LDA_, &rcma);

  ASSERT(NRA_ == M && NCA_ == N && rcma == (RCM_TYPE_DENSE|RCM_ORDER_COL_MAJOR));
  int LDA = LDA_;

  double *X = 0;
  rdint_t NX_ = NETLIB_TRANS_GET_NCOLS(TRANS, M, N);
  rdint_t INCX_ = 0;
  BLAS_CALL(get_input_param_vector_double)(bc, "X", &X, &NX_, &INCX_);

  ASSERT(NX_ == NETLIB_TRANS_GET_NCOLS(TRANS, M, N));
  int INCX = INCX_;

  double BETA;
  BLAS_CALL(get_input_conf_double)(bc, "BETA", &BETA);

  double *Y = 0;
  rdint_t NY_ = NETLIB_TRANS_GET_NROWS(TRANS, M, N);
  rdint_t INCY_ = 0;
  BLAS_CALL(get_inout_param_vector_double)(bc, "Y", &Y, &NY_, &INCY_);

  ASSERT(NY_ == NETLIB_TRANS_GET_NROWS(TRANS, M, N));
  int INCY = INCY_;

  TRACE_F("TRANS: %c, M: %d, N: %d, ALPHA: %e, A: %p, LDA: %d, X: %p, INCX: %d, BETA: %e, Y: %p, INCY: %d", TRANS, M, N, ALPHA, A, LDA, X, INCX, BETA, Y, INCY);

  BLAS_ORIG_F_INIT(dgemv_);
  BLAS_TIMING_START(libblas_scdc_timing_remote[0]);
  BLAS_ORIG_F(MANGLE_BLAS(dgemv_))(&TRANS, &M, &N, &ALPHA, A, &LDA, X, &INCX, &BETA, Y, &INCY);
  BLAS_TIMING_STOP(libblas_scdc_timing_remote[0]);

  TRACE_F("TRANS: %c, M: %d, N: %d, ALPHA: %e, A: %p, LDA: %d, X: %p, INCX: %d, BETA: %e, Y: %p, INCY: %d", TRANS, M, N, ALPHA, A, LDA, X, INCX, BETA, Y, INCY);

  BLAS_CALL(put_output_param_vector_double)(bc, "Y", Y, NETLIB_TRANS_GET_NROWS(TRANS, M, N), INCY);

  BLAS_TIMING_REMOTE_PUT(bc, libblas_scdc_timing_remote[0]);

  BLAS_TIMING_PRINT_F("%s: %f", __func__, libblas_scdc_timing_remote[0]);

  TRACE_F("return");
}

#endif /* BLAS_DGEMV */


#if BLAS_DTRSV

static void do_dtrsv(blas_call_t *bc)
{
  TRACE_F("bc: %p", bc);

  BLAS_TIMING_INIT_();

  char UPLO, TRANS, DIAG;
  BLAS_CALL(get_input_conf_char)(bc, "UPLO", &UPLO);
  BLAS_CALL(get_input_conf_char)(bc, "TRANS", &TRANS);
  BLAS_CALL(get_input_conf_char)(bc, "DIAG", &DIAG);

  int N;
  BLAS_CALL(get_input_conf_int)(bc, "N", &N);

  double *A = 0;
  rdint_t NRA_ = N, NCA_ = N;
  rdint_t LDA_ = 0, rcma;
  BLAS_CALL(get_input_param_matrix_double)(bc, "A", &A, &NRA_, &NCA_, &LDA_, &rcma);

  ASSERT(NRA_ == N && NCA_ == N && RCM_GET_ORDER(rcma) == RCM_ORDER_COL_MAJOR);
  int LDA = LDA_;

  double *X = 0;
  rdint_t N_ = N;
  rdint_t INCX_ = 0;
  BLAS_CALL(get_inout_param_vector_double)(bc, "X", &X, &N_, &INCX_);

  ASSERT(N_ == N);
  int INCX = INCX_;

  TRACE_F("UPLO: %c, TRANS: %c, DIAG: %c, N: %d, A: %p, LDA: %d, X: %p, INCX: %d", UPLO, TRANS, DIAG, N, A, LDA, X, INCX);

  BLAS_ORIG_F_INIT(dtrsv_);
  BLAS_TIMING_START(libblas_scdc_timing_remote[0]);
  BLAS_ORIG_F(MANGLE_BLAS(dtrsv_))(&UPLO, &TRANS, &DIAG, &N, A, &LDA, X, &INCX);
  BLAS_TIMING_STOP(libblas_scdc_timing_remote[0]);

  TRACE_F("UPLO: %c, TRANS: %c, DIAG: %c, N: %d, A: %p, LDA: %d, X: %p, INCX: %d", UPLO, TRANS, DIAG, N, A, LDA, X, INCX);

  BLAS_CALL(put_output_param_vector_double)(bc, "X", X, N, INCX);

  BLAS_TIMING_REMOTE_PUT(bc, libblas_scdc_timing_remote[0]);

  BLAS_TIMING_PRINT_F("%s: %f", __func__, libblas_scdc_timing_remote[0]);

  TRACE_F("return");
}

#endif /* BLAS_DTRSV */

#endif /* BLAS_LEVEL2 */


/* BLAS level 3 */
#if BLAS_LEVEL3

#if BLAS_DGEMM

static void do_dgemm(blas_call_t *bc)
{
  TRACE_F("bc: %p", bc);

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
  rdint_t NRA_ = NETLIB_TRANS_GET_NROWS(TRANSA, M, K);
  rdint_t NCA_ = NETLIB_TRANS_GET_NCOLS(TRANSA, M, K);
  rdint_t NRB_ = NETLIB_TRANS_GET_NROWS(TRANSB, K, N);
  rdint_t NCB_ = NETLIB_TRANS_GET_NCOLS(TRANSB, K, N);
  rdint_t LDA_ = 0, rcma, LDB_ = 0, rcmb;
  BLAS_CALL(get_input_param_matrix_double)(bc, "A", &A, &NRA_, &NCA_, &LDA_, &rcma);
  BLAS_CALL(get_input_param_matrix_double)(bc, "B", &B, &NRB_, &NCB_, &LDB_, &rcmb);

  ASSERT(NRA_ == NETLIB_TRANS_GET_NROWS(TRANSA, M, K) && NCA_ == NETLIB_TRANS_GET_NCOLS(TRANSA, M, K) && rcma == (RCM_TYPE_DENSE|RCM_ORDER_COL_MAJOR));
  ASSERT(NRB_ == NETLIB_TRANS_GET_NROWS(TRANSB, K, N) && NCB_ == NETLIB_TRANS_GET_NCOLS(TRANSB, K, N) && rcmb == (RCM_TYPE_DENSE|RCM_ORDER_COL_MAJOR));
  int LDA = LDA_, LDB = LDB_;

  double BETA;
  BLAS_CALL(get_input_conf_double)(bc, "BETA", &BETA);

  double *C = 0;
  rdint_t NRC_ = M, NCC_ = N;
  rdint_t LDC_ = 0, rcmc;
  BLAS_CALL(get_inout_param_matrix_double)(bc, "C", &C, &NRC_, &NCC_, &LDC_, &rcmc);

  ASSERT(NRC_ == M && NCC_ == N && rcmc == (RCM_TYPE_DENSE|RCM_ORDER_COL_MAJOR));
  int LDC = LDC_;

  TRACE_F("TRANSA: %c, TRANSB: %c, M: %d, N: %d, K: %d, ALPHA: %e, A: %p, LDA: %d, B: %p, LDB: %d, BETA: %e, C: %p, LDC: %d", TRANSA, TRANSB, M, N, K, ALPHA, A, LDA, B, LDB, BETA, C, LDC);

  BLAS_ORIG_F_INIT(dgemm_);
  BLAS_TIMING_START(libblas_scdc_timing_remote[0]);
  BLAS_ORIG_F(MANGLE_BLAS(dgemm_))(&TRANSA, &TRANSB, &M, &N, &K, &ALPHA, A, &LDA, B, &LDB, &BETA, C, &LDC);
  BLAS_TIMING_STOP(libblas_scdc_timing_remote[0]);

  TRACE_F("TRANSA: %c, TRANSB: %c, M: %d, N: %d, K: %d, ALPHA: %e, A: %p, LDA: %d, B: %p, LDB: %d, BETA: %e, C: %p, LDC: %d", TRANSA, TRANSB, M, N, K, ALPHA, A, LDA, B, LDB, BETA, C, LDC);

  BLAS_CALL(put_output_param_matrix_double)(bc, "C", C, M, N, LDC, rcmc);

  BLAS_TIMING_REMOTE_PUT(bc, libblas_scdc_timing_remote[0]);

  BLAS_TIMING_PRINT_F("%s: %f", __func__, libblas_scdc_timing_remote[0]);

  TRACE_F("return");
}

#endif /* BLAS_DGEMM */


#if BLAS_DTRSM

static void do_dtrsm(blas_call_t *bc)
{
  TRACE_F("bc: %p", bc);

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
  rdint_t NRA_ = K, NCA_ = K;
  rdint_t NRB_ = M, NCB_ = N;
  rdint_t LDA_ = 0, rcma, LDB_ = 0, rcmb;
  BLAS_CALL(get_input_param_matrix_double)(bc, "A", &A, &NRA_, &NCA_, &LDA_, &rcma);
  BLAS_CALL(get_inout_param_matrix_double)(bc, "B", &B, &NRB_, &NCB_, &LDB_, &rcmb);

  ASSERT(NRA_ = K && NCA_ == K && RCM_GET_ORDER(rcma) == RCM_ORDER_COL_MAJOR);
  ASSERT(NRB_ = M && NCB_ == N && rcmb == (RCM_TYPE_DENSE|RCM_ORDER_COL_MAJOR));
  int LDA = LDA_, LDB = LDB_;

  TRACE_F("SIDE: %c, UPLO: %c, TRANSA: %c, DIAG: %c, M: %d, N: %d, ALPHA: %e, A: %p, LDA: %d, B: %p, LDB: %d", SIDE, UPLO, TRANSA, DIAG, M, N, ALPHA, A, LDA, B, LDB);

  BLAS_ORIG_F_INIT(dtrsm_);
  BLAS_TIMING_START(libblas_scdc_timing_remote[0]);
  BLAS_ORIG_F(MANGLE_BLAS(dtrsm_))(&SIDE, &UPLO, &TRANSA, &DIAG, &M, &N, &ALPHA, A, &LDA, B, &LDB);
  BLAS_TIMING_STOP(libblas_scdc_timing_remote[0]);

  TRACE_F("SIDE: %c, UPLO: %c, TRANSA: %c, DIAG: %c, M: %d, N: %d, ALPHA: %e, A: %p, LDA: %d, B: %p, LDB: %d", SIDE, UPLO, TRANSA, DIAG, M, N, ALPHA, A, LDA, B, LDB);

  BLAS_CALL(put_output_param_matrix_double)(bc, "B", B, M, N, LDB, rcmb);

  BLAS_TIMING_REMOTE_PUT(bc, libblas_scdc_timing_remote[0]);

  BLAS_TIMING_PRINT_F("%s: %f", __func__, libblas_scdc_timing_remote[0]);

  TRACE_F("return");
}

#endif /* BLAS_DTRSM */

#endif /* BLAS_LEVEL3 */


void *blas_scdc_open(const char *conf, va_list ap)
{
  TRACE_F("conf: '%s'", conf);

  BLAS_ORIG_INIT();

  blas_call_t *bc = malloc(sizeof(blas_call_t)
#if LIBBLAS_SCDC_CACHE
    + sizeof(blas_cache_t)
#endif
    );

#if LIBBLAS_SCDC_CACHE
  blas_cache_t *bch = (blas_cache_t *) (bc + 1);

  BLAS_CACHE(init)(bch);
#endif

  TRACE_F("return: %p", bc);

  return bc;
}


scdcint_t blas_scdc_close(void *dataprov)
{
  TRACE_F("dataprov: %p", dataprov);

  blas_call_t *bc = dataprov;

#if LIBBLAS_SCDC_CACHE
  blas_cache_t *bch = (blas_cache_t *) (bc + 1);

  BLAS_CACHE(release)(bch);
#endif

  free(bc);

  BLAS_ORIG_RELEASE();

  TRACE_F("return");

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


scdcint_t blas_scdc_dataset_cmd(void *dataprov, void *dataset, const char *cmd, const char *params, scdc_dataset_input_t *input, scdc_dataset_output_t *output, scdc_result_t *result)
{
  TRACE_F("dataprov: %p, dataset: %p, cmd: '%s', params: '%s', input: %p, output: %p, result: %p", dataprov, dataset, cmd, params, input, output, result);

#if LIBBLAS_SCDC_PROGRESS
  printf("cmd: %s, params: %s\n", cmd, params);
#endif

  TRACE_F_N("input: "); TRACE_CMD(scdc_dataset_input_print(input);); TRACE_R("\n");
  TRACE_F_N("output: "); TRACE_CMD(scdc_dataset_output_print(output);); TRACE_R("\n");

  blas_call_t *bc = dataprov;

  BLAS_CALL(init_scdc)(bc);

#if LIBBLAS_SCDC_CACHE
  blas_cache_t *bch = (blas_cache_t *) (bc + 1);

  BLAS_CALL(set_cache_ptr)(bc, bch);
#endif

  if (BLAS_CALL(cmd)(bc, cmd, params, input, output, result))
  {
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
      TRACE_F("command '%s' not supported!", cmd);
    }
  }

  BLAS_CALL(release_scdc)(bc);

  TRACE_F("return");

  return SCDC_SUCCESS;
}
