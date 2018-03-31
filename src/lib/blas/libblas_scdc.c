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

#include "common.h"
#include "z_pack.h"
#include "blas.h"
#include "blas_call.h"
#include "blas_scdc.h"
#include "blas_timing.h"


#define TRACE_PREFIX  "libblas_scdc: "


#if LIBBLAS_SCDC_PREFIX

# undef __BLAS_H__
# undef MANGLE_BLAS
# define MANGLE_BLAS(_f_)  Z_CONCAT(LIBBLAS_SCDC_PREFIX_NAME, _f_)
# include "blas.h"

# define LIB_F(_f_)      Z_CONCAT(LIBBLAS_SCDC_PREFIX_NAME, _f_)
# define LIB_F_STR(_f_)  Z_STRINGIFY(Z_CONCAT(LIBBLAS_SCDC_PREFIX_NAME, _f_))

#else /* LIBBLAS_SCDC_PREFIX */

# define LIB_F(_f_)      _f_
# define LIB_F_STR(_f_)  Z_STRINGIFY(_f_)

# define BLAS_ORIG_ENABLED  1

#endif /* LIBBLAS_SCDC_PREFIX */

#include "blas_orig.h"


#if LIBBLAS_SCDC_ORIGINALS

#define ORIG_F(_f_)  Z_CONCAT(original_, _f_)

#endif /* LIBBLAS_SCDC_ORIGINALS */


const char *libblas_scdc_local_base = 0;
scdc_dataprov_t libblas_scdc_local_dataprov = SCDC_DATAPROV_NULL;

const char *libblas_scdc_uri = 0;

int libblas_scdc_initialized = -1;

#if LIBBLAS_SCDC_TIMING
# undef BLAS_TIMING_PREFIX
# define BLAS_TIMING_PREFIX  TRACE_PREFIX  "TIMING: "
# define LIBBLAS_SCDC_TIMING_X  5
int libblas_scdc_timing_i;
double libblas_scdc_timing[LIBBLAS_SCDC_TIMING_X];
#endif
#define BLAS_TIMING_INIT_()  BLAS_TIMING_INIT(libblas_scdc_timing, libblas_scdc_timing_i, LIBBLAS_SCDC_TIMING_X)

#if LIBBLAS_SCDC_TIMING_REDIRECT_REMOTE
# define BLAS_TIMING_REMOTE_GET(_bc_, _t_)  BLAS_CALL(get_output_conf_double)(_bc_, "TIMING", _t_);
#else
# define BLAS_TIMING_REMOTE_GET(_bc_, _t_)  Z_NOP()
#endif


#if LIBBLAS_SCDC_ENABLED

void libblas_scdc_release();

void libblas_scdc_init()
{
  TRACE_F("%s: libblas_scdc_initialized: %d", __func__, libblas_scdc_initialized);

  if (libblas_scdc_initialized < 0)
  {
    /* on first init */
    TRACE_F("%s: first init", __func__);

    BLAS_ORIG_INIT();

    scdc_init(SCDC_INIT_DEFAULT);

#if LIBBLAS_SCDC_LOCAL
    libblas_scdc_local_base = getenv("LIBBLAS_SCDC_LOCAL_BASE");
    if (!libblas_scdc_local_base) libblas_scdc_local_base = LIBBLAS_SCDC_LOCAL_BASE;

    libblas_scdc_local_dataprov = scdc_dataprov_open(libblas_scdc_local_base, "hook", &blas_scdc_hook);
#endif

    libblas_scdc_uri = getenv("LIBBLAS_SCDC_URI");

    if (!libblas_scdc_uri)
    {
#if defined(LIBBLAS_SCDC_URI_DEFAULT)
      libblas_scdc_uri = LIBBLAS_SCDC_URI_DEFAULT;
#elif LIBBLAS_SCDC_LOCAL
      libblas_scdc_uri = LIBBLAS_SCDC_LOCAL_URI LIBBLAS_SCDC_LOCAL_BASE;
#endif
    }

    /* register release handler */
    atexit(libblas_scdc_release);

    ++libblas_scdc_initialized;
  }

  if (libblas_scdc_initialized == 0)
  {
    /* on every init */
    TRACE_F("%s: every init", __func__);
  }

  ++libblas_scdc_initialized;

  TRACE_F("%s: return: libblas_scdc_initialized: %d", __func__, libblas_scdc_initialized);
}


void libblas_scdc_release()
{
  --libblas_scdc_initialized;

  if (libblas_scdc_initialized == 0)
  {
    /* on every release */
    TRACE_F("%s: every release", __func__);
  }

  if (libblas_scdc_initialized < 0)
  {
    /* on last release */
    TRACE_F("%s: last release", __func__);

#if LIBBLAS_SCDC_LOCAL
    scdc_dataprov_close(libblas_scdc_local_dataprov);
    libblas_scdc_local_dataprov = SCDC_DATAPROV_NULL;
#endif

    scdc_release();

    BLAS_ORIG_RELEASE();
  }
}

#endif /* LIBBLAS_SCDC_ENABLED */


/* BLAS bench */
#if BLAS_BENCH

void LIB_F(dvin_)(const int *N, double *DX, const int *INCX)
{
  TRACE_F("%s: N: %d, DX: %p, INCX: %d", __func__, *N, DX, *INCX);

#if LIBBLAS_SCDC_PROGRESS
  printf("%s: libblas_scdc_uri: %s\n", __func__, libblas_scdc_uri);
#endif

#if LIBBLAS_SCDC_TRACE_DATA
  TRACE_CMD(
    TRACE_F("%s: DX", __func__);
    BLAS_CALL(print_param_vector_double)(*N, DX, *INCX);
  );
#endif

  BLAS_TIMING_INIT_();

  BLAS_TIMING_START(libblas_scdc_timing[0]);

#if LIBBLAS_SCDC_ENABLED

  libblas_scdc_init();

  blas_call_t bc;

  BLAS_CALL(create_scdc)(&bc, "dvin", libblas_scdc_uri);

  BLAS_CALL(put_input_conf_int)(&bc, "N", *N);

  BLAS_CALL(put_input_param_vector_double)(&bc, "DX", *N, DX, *INCX);

  if (!BLAS_CALL(execute)(&bc))
  {
    TRACE_F("%s: failed", __func__);
    goto do_quit;
  }

/*  BLAS_TIMING_REMOTE_GET(&bc, &libblas_scdc_timing[1]);*/

do_quit:
  BLAS_CALL(destroy_scdc)(&bc);

  libblas_scdc_release();

#endif /* LIBBLAS_SCDC_ENABLED */

  BLAS_TIMING_STOP(libblas_scdc_timing[0]);

  BLAS_TIMING_PRINT_F("%s: %f  %f", __func__, libblas_scdc_timing[0], libblas_scdc_timing[1]);

  TRACE_F("%s: return", __func__);
}


void LIB_F(dvout_)(const int *N, double *DX, const int *INCX)
{
  TRACE_F("%s: N: %d, DX: %p, INCX: %d", __func__, *N, DX, *INCX);

#if LIBBLAS_SCDC_PROGRESS
  printf("%s: libblas_scdc_uri: %s\n", __func__, libblas_scdc_uri);
#endif

  BLAS_TIMING_INIT_();

  BLAS_TIMING_START(libblas_scdc_timing[0]);

#if LIBBLAS_SCDC_ENABLED

  libblas_scdc_init();

  blas_call_t bc;

  BLAS_CALL(create_scdc)(&bc, "dvout", libblas_scdc_uri);

  BLAS_CALL(put_input_conf_int)(&bc, "N", *N);

  BLAS_CALL(put_output_param_vector_double)(&bc, "DX", *N, DX, *INCX);

  if (!BLAS_CALL(execute)(&bc))
  {
    TRACE_F("%s: failed", __func__);
    goto do_quit;
  }

  double *DX_ = DX;
  int INCX_ = *INCX;
  BLAS_CALL(get_output_param_vector_double)(&bc, "DX", *N, &DX_, &INCX_);

  ASSERT(DX_ == DX && INCX_ == *INCX);

/*  BLAS_TIMING_REMOTE_GET(&bc, &libblas_scdc_timing[1]);*/

do_quit:
  BLAS_CALL(destroy_scdc)(&bc);

  libblas_scdc_release();

#endif /* LIBBLAS_SCDC_ENABLED */

  BLAS_TIMING_STOP(libblas_scdc_timing[0]);

  BLAS_TIMING_PRINT_F("%s: %f  %f", __func__, libblas_scdc_timing[0], libblas_scdc_timing[1]);

#if LIBBLAS_SCDC_TRACE_DATA
  TRACE_CMD(
    TRACE_F("%s: DX", __func__);
    BLAS_CALL(print_param_vector_double)(*N, DX, *INCX);
  );
#endif

  TRACE_F("%s: return", __func__);
}


void LIB_F(dvinout_)(const int *N, double *DX, const int *INCX)
{
  TRACE_F("%s: N: %d, DX: %p, INCX: %d", __func__, *N, DX, *INCX);

#if LIBBLAS_SCDC_PROGRESS
  printf("%s: libblas_scdc_uri: %s\n", __func__, libblas_scdc_uri);
#endif

#if LIBBLAS_SCDC_TRACE_DATA
  TRACE_CMD(
    TRACE_F("%s: DX", __func__);
    BLAS_CALL(print_param_vector_double)(*N, DX, *INCX);
  );
#endif

  BLAS_TIMING_INIT_();

  BLAS_TIMING_START(libblas_scdc_timing[0]);

#if LIBBLAS_SCDC_ENABLED

  libblas_scdc_init();

  blas_call_t bc;

  BLAS_CALL(create_scdc)(&bc, "dvinout", libblas_scdc_uri);

  BLAS_CALL(put_input_conf_int)(&bc, "N", *N);

  BLAS_CALL(put_inout_param_vector_double)(&bc, "DX", *N, DX, *INCX);

  if (!BLAS_CALL(execute)(&bc))
  {
    TRACE_F("%s: failed", __func__);
    goto do_quit;
  }

  double *DX_ = DX;
  int INCX_ = *INCX;
  BLAS_CALL(get_output_param_vector_double)(&bc, "DX", *N, &DX_, &INCX_);

  ASSERT(DX_ == DX && INCX_ == *INCX);

/*  BLAS_TIMING_REMOTE_GET(&bc, &libblas_scdc_timing[1]);*/

do_quit:
  BLAS_CALL(destroy_scdc)(&bc);

  libblas_scdc_release();

#endif /* LIBBLAS_SCDC_ENABLED */

  BLAS_TIMING_STOP(libblas_scdc_timing[0]);

  BLAS_TIMING_PRINT_F("%s: %f  %f", __func__, libblas_scdc_timing[0], libblas_scdc_timing[1]);

#if LIBBLAS_SCDC_TRACE_DATA
  TRACE_CMD(
    TRACE_F("%s: DX", __func__);
    BLAS_CALL(print_param_vector_double)(*N, DX, *INCX);
  );
#endif

  TRACE_F("%s: return", __func__);
}


void LIB_F(dgein_)(const int *M, const int *N, double *A, const int *LDA)
{
  TRACE_F("%s: M: %d, N: %d, A: %p, LDA: %d", __func__, *M, *N, A, *LDA);

#if LIBBLAS_SCDC_PROGRESS
  printf("%s: libblas_scdc_uri: %s\n", __func__, libblas_scdc_uri);
#endif

#if LIBBLAS_SCDC_TRACE_DATA
  TRACE_CMD(
    TRACE_F("%s: A", __func__);
    BLAS_CALL(print_param_matrix_double)(*M, *N, A, *LDA, RCM_TYPE_DENSE|RCM_ORDER_COL_MAJOR);
  );
#endif

  BLAS_TIMING_INIT_();

  BLAS_TIMING_START(libblas_scdc_timing[0]);

#if LIBBLAS_SCDC_ENABLED

  libblas_scdc_init();

  blas_call_t bc;

  BLAS_CALL(create_scdc)(&bc, "dgein", libblas_scdc_uri);

  BLAS_CALL(put_input_conf_int)(&bc, "M", *M);
  BLAS_CALL(put_input_conf_int)(&bc, "N", *N);

  BLAS_CALL(put_input_param_matrix_double)(&bc, "A", *M, *N, A, *LDA, RCM_TYPE_DENSE|RCM_ORDER_COL_MAJOR);

  if(!BLAS_CALL(execute)(&bc))
  {
    TRACE_F("%s: failed", __func__);
    goto do_quit;
  }

/*  BLAS_TIMING_REMOTE_GET(&bc, &libblas_scdc_timing[1]);*/

do_quit:
  BLAS_CALL(destroy_scdc)(&bc);

  libblas_scdc_release();

#endif /* LIBBLAS_SCDC_ENABLED */

  BLAS_TIMING_STOP(libblas_scdc_timing[0]);

  BLAS_TIMING_PRINT_F("%s: %f  %f", __func__, libblas_scdc_timing[0], libblas_scdc_timing[1]);

  TRACE_F("%s: return", __func__);
}


void LIB_F(dgeout_)(const int *M, const int *N, double *A, const int *LDA)
{
  TRACE_F("%s: M: %d, N: %d, A: %p, LDA: %d", __func__, *M, *N, A, *LDA);

#if LIBBLAS_SCDC_PROGRESS
  printf("%s: libblas_scdc_uri: %s\n", __func__, libblas_scdc_uri);
#endif

  BLAS_TIMING_INIT_();

  BLAS_TIMING_START(libblas_scdc_timing[0]);

#if LIBBLAS_SCDC_ENABLED

  libblas_scdc_init();

  blas_call_t bc;

  BLAS_CALL(create_scdc)(&bc, "dgeout", libblas_scdc_uri);

  BLAS_CALL(put_input_conf_int)(&bc, "M", *M);
  BLAS_CALL(put_input_conf_int)(&bc, "N", *N);

  BLAS_CALL(put_output_param_matrix_double)(&bc, "A", *M, *N, A, *LDA, RCM_TYPE_DENSE|RCM_ORDER_COL_MAJOR);

  if(!BLAS_CALL(execute)(&bc))
  {
    TRACE_F("%s: failed", __func__);
    goto do_quit;
  }

  double *A_ = A;
  int LDA_ = *LDA;
  int rcma_ = RCM_TYPE_DENSE|RCM_ORDER_COL_MAJOR;
  BLAS_CALL(get_output_param_matrix_double)(&bc, "A", *M, *N, &A_, &LDA_, &rcma_);

  ASSERT(A_ == A && LDA_ == *LDA && rcma_ == (RCM_TYPE_DENSE|RCM_ORDER_COL_MAJOR));

/*  BLAS_TIMING_REMOTE_GET(&bc, &libblas_scdc_timing[1]);*/

do_quit:
  BLAS_CALL(destroy_scdc)(&bc);

  libblas_scdc_release();

#endif /* LIBBLAS_SCDC_ENABLED */

  BLAS_TIMING_STOP(libblas_scdc_timing[0]);

  BLAS_TIMING_PRINT_F("%s: %f  %f", __func__, libblas_scdc_timing[0], libblas_scdc_timing[1]);

#if LIBBLAS_SCDC_TRACE_DATA
  TRACE_CMD(
    TRACE_F("%s: A", __func__);
    BLAS_CALL(print_param_matrix_double)(*M, *N, A, *LDA, RCM_TYPE_DENSE|RCM_ORDER_COL_MAJOR);
  );
#endif

  TRACE_F("%s: return", __func__);
}


void LIB_F(dgeinout_)(const int *M, const int *N, double *A, const int *LDA)
{
  TRACE_F("%s: M: %d, N: %d, A: %p, LDA: %d", __func__, *M, *N, A, *LDA);

#if LIBBLAS_SCDC_PROGRESS
  printf("%s: libblas_scdc_uri: %s\n", __func__, libblas_scdc_uri);
#endif

#if LIBBLAS_SCDC_TRACE_DATA
  TRACE_CMD(
    TRACE_F("%s: A", __func__);
    BLAS_CALL(print_param_matrix_double)(*M, *N, A, *LDA, RCM_TYPE_DENSE|RCM_ORDER_COL_MAJOR);
  );
#endif

  BLAS_TIMING_INIT_();

  BLAS_TIMING_START(libblas_scdc_timing[0]);

#if LIBBLAS_SCDC_ENABLED

  libblas_scdc_init();

  blas_call_t bc;

  BLAS_CALL(create_scdc)(&bc, "dgeinout", libblas_scdc_uri);

  BLAS_CALL(put_input_conf_int)(&bc, "M", *M);
  BLAS_CALL(put_input_conf_int)(&bc, "N", *N);

  BLAS_CALL(put_inout_param_matrix_double)(&bc, "A", *M, *N, A, *LDA, RCM_TYPE_DENSE|RCM_ORDER_COL_MAJOR);

  if(!BLAS_CALL(execute)(&bc))
  {
    TRACE_F("%s: failed", __func__);
    goto do_quit;
  }

  double *A_ = A;
  int LDA_ = *LDA;
  int rcma_ = RCM_TYPE_DENSE|RCM_ORDER_COL_MAJOR;
  BLAS_CALL(get_output_param_matrix_double)(&bc, "A", *M, *N, &A_, &LDA_, &rcma_);

  ASSERT(A_ == A && LDA_ == *LDA && rcma_ == (RCM_TYPE_DENSE|RCM_ORDER_COL_MAJOR));

/*  BLAS_TIMING_REMOTE_GET(&bc, &libblas_scdc_timing[1]);*/

do_quit:
  BLAS_CALL(destroy_scdc)(&bc);

  libblas_scdc_release();

#endif /* LIBBLAS_SCDC_ENABLED */

  BLAS_TIMING_STOP(libblas_scdc_timing[0]);

  BLAS_TIMING_PRINT_F("%s: %f  %f", __func__, libblas_scdc_timing[0], libblas_scdc_timing[1]);

#if LIBBLAS_SCDC_TRACE_DATA
  TRACE_CMD(
    TRACE_F("%s: A", __func__);
    BLAS_CALL(print_param_matrix_double)(*M, *N, A, *LDA, RCM_TYPE_DENSE|RCM_ORDER_COL_MAJOR);
  );
#endif

  TRACE_F("%s: return", __func__);
}

#endif /* BLAS_BENCH */


/* BLAS level 1 */
#if BLAS_LEVEL1

#if BLAS_IDAMAX

int LIB_F(idamax_)(const int *N, double *DX, const int *INCX)
{
  TRACE_F("%s: N: %d, DX: %p, INCX: %d", __func__, *N, DX, *INCX);

#if LIBBLAS_SCDC_PROGRESS
  printf("%s: libblas_scdc_uri: %s\n", __func__, libblas_scdc_uri);
#endif

#if LIBBLAS_SCDC_TRACE_DATA
  TRACE_CMD(
    TRACE_F("%s: DX", __func__);
    BLAS_CALL(print_param_vector_double)(*N, DX, *INCX);
  );
#endif

  BLAS_TIMING_INIT_();

  BLAS_TIMING_START(libblas_scdc_timing[0]);

#if LIBBLAS_SCDC_ENABLED

  libblas_scdc_init();

  blas_call_t bc;

  BLAS_CALL(create_scdc)(&bc, "idamax", libblas_scdc_uri);

  BLAS_CALL(put_input_conf_int)(&bc, "N", *N);

  BLAS_CALL(put_input_param_vector_double)(&bc, "DX", *N, DX, *INCX);

  if (!BLAS_CALL(execute)(&bc))
  {
    TRACE_F("%s: failed", __func__);
    goto do_quit;
  }

  int r;
  BLAS_CALL(get_output_conf_int)(&bc, "r", &r);

  BLAS_TIMING_REMOTE_GET(&bc, &libblas_scdc_timing[1]);

do_quit:
  BLAS_CALL(destroy_scdc)(&bc);

  libblas_scdc_release();

#else /* LIBBLAS_SCDC_ENABLED */

  BLAS_ORIG_F_INIT(idamax_);
  int r = BLAS_ORIG_F(MANGLE_BLAS(idamax_))(N, DX, INCX);

#endif /* LIBBLAS_SCDC_ENABLED */

  BLAS_TIMING_STOP(libblas_scdc_timing[0]);

  BLAS_TIMING_PRINT_F("%s: %f  %f", __func__, libblas_scdc_timing[0], libblas_scdc_timing[1]);

  TRACE_F("%s: return: %d", __func__, r);

  return r;
}

int LIB_F(idamax)(const int *N, double *DX, const int *INCX)
{
  return LIB_F(idamax_)(N, DX, INCX);
}

#if LIBBLAS_SCDC_ORIGINALS

int LIB_F(ORIG_F(idamax_))(const int *N, double *DX, const int *INCX)
{
  BLAS_ORIG_F_INIT(idamax_);
  return BLAS_ORIG_F(MANGLE_BLAS(idamax_))(N, DX, INCX);
}

#endif /* LIBBLAS_SCDC_ORIGS */

#endif /* BLAS_IDAMAX */


#if BLAS_DCOPY

void LIB_F(dcopy_)(const int *N, double *DX, const int *INCX, double *DY, const int *INCY)
{
  TRACE_F("%s: N: %d, DX: %p, INCX: %d, DY: %p, INCY: %d", __func__, *N, DX, *INCX, DY, *INCY);

#if LIBBLAS_SCDC_PROGRESS
  printf("%s: libblas_scdc_uri: %s\n", __func__, libblas_scdc_uri);
#endif

#if LIBBLAS_SCDC_TRACE_DATA
  TRACE_CMD(
    TRACE_F("%s: DX", __func__);
    BLAS_CALL(print_param_vector_double)(*N, DX, *INCX);
  );
#endif

  BLAS_TIMING_INIT_();

  BLAS_TIMING_START(libblas_scdc_timing[0]);

#if LIBBLAS_SCDC_ENABLED

  libblas_scdc_init();

  blas_call_t bc;

  BLAS_CALL(create_scdc)(&bc, "dcopy", libblas_scdc_uri);

  BLAS_CALL(put_input_conf_int)(&bc, "N", *N);

  BLAS_CALL(put_input_param_vector_double)(&bc, "DX", *N, DX, *INCX);
  BLAS_CALL(put_output_param_vector_double)(&bc, "DY", *N, DY, *INCY);

  if (!BLAS_CALL(execute)(&bc))
  {
    TRACE_F("%s: failed", __func__);
    goto do_quit;
  }

  double *DY_ = DY;
  int INCY_ = *INCY;
  BLAS_CALL(get_output_param_vector_double)(&bc, "DY", *N, &DY_, &INCY_);

  ASSERT(DY_ == DY && INCY_ == *INCY);

  BLAS_TIMING_REMOTE_GET(&bc, &libblas_scdc_timing[1]);

do_quit:
  BLAS_CALL(destroy_scdc)(&bc);

  libblas_scdc_release();

#else /* LIBBLAS_SCDC_ENABLED */

  BLAS_ORIG_F_INIT(dcopy_);
  BLAS_ORIG_F(MANGLE_BLAS(dcopy_))(N, DX, INCX, DY, INCY);

#endif /* LIBBLAS_SCDC_ENABLED */

  BLAS_TIMING_STOP(libblas_scdc_timing[0]);

  BLAS_TIMING_PRINT_F("%s: %f  %f", __func__, libblas_scdc_timing[0], libblas_scdc_timing[1]);

#if LIBBLAS_SCDC_TRACE_DATA
  TRACE_CMD(
    TRACE_F("%s: DY", __func__);
    BLAS_CALL(print_param_vector_double)(*N, DY, *INCY);
  );
#endif

  TRACE_F("%s: return", __func__);
}

void LIB_F(dcopy)(const int *N, double *DX, const int *INCX, double *DY, const int *INCY)
{
  LIB_F(dcopy_)(N, DX, INCX, DY, INCY);
}

#if LIBBLAS_SCDC_ORIGINALS

void LIB_F(ORIG_F(dcopy))(const int *N, double *DX, const int *INCX, double *DY, const int *INCY)
{
  BLAS_ORIG_F_INIT(dcopy_);
  BLAS_ORIG_F(MANGLE_BLAS(dcopy_))(N, DX, INCX, DY, INCY);
}

#endif /* LIBBLAS_SCDC_ORIGS */

#endif /* BLAS_DCOPY */


#if BLAS_DSCAL

void LIB_F(dscal_)(const int *N, const double *DA, double *DX, const int *INCX)
{
  TRACE_F("%s: N: %d, DA: %e, DX: %p, INCX: %d", __func__, *N, *DA, DX, *INCX);

#if LIBBLAS_SCDC_PROGRESS
  printf("%s: libblas_scdc_uri: %s\n", __func__, libblas_scdc_uri);
#endif

#if LIBBLAS_SCDC_TRACE_DATA
  TRACE_CMD(
    TRACE_F("%s: DX", __func__);
    BLAS_CALL(print_param_vector_double)(*N, DX, *INCX);
  );
#endif

  BLAS_TIMING_INIT_();

  BLAS_TIMING_START(libblas_scdc_timing[0]);

#if LIBBLAS_SCDC_ENABLED

  libblas_scdc_init();

  blas_call_t bc;

  BLAS_CALL(create_scdc)(&bc, "dscal", libblas_scdc_uri);

  BLAS_CALL(put_input_conf_int)(&bc, "N", *N);

  BLAS_CALL(put_input_conf_double)(&bc, "DA", *DA);

  BLAS_CALL(put_inout_param_vector_double)(&bc, "DX", *N, DX, *INCX);

  if (!BLAS_CALL(execute)(&bc))
  {
    TRACE_F("%s: failed", __func__);
    goto do_quit;
  }

  double *DX_ = DX;
  int INCX_ = *INCX;
  BLAS_CALL(get_output_param_vector_double)(&bc, "DX", *N, &DX_, &INCX_);

  ASSERT(DX_ == DX && INCX_ == *INCX);

  BLAS_TIMING_REMOTE_GET(&bc, &libblas_scdc_timing[1]);

do_quit:
  BLAS_CALL(destroy_scdc)(&bc);

  libblas_scdc_release();

#else /* LIBBLAS_SCDC_ENABLED */

  BLAS_ORIG_F_INIT(dscal_);
  BLAS_ORIG_F(MANGLE_BLAS(dscal_))(N, DA, DX, INCX);

#endif /* LIBBLAS_SCDC_ENABLED */

  BLAS_TIMING_STOP(libblas_scdc_timing[0]);

  BLAS_TIMING_PRINT_F("%s: %f  %f", __func__, libblas_scdc_timing[0], libblas_scdc_timing[1]);

#if LIBBLAS_SCDC_TRACE_DATA
  TRACE_CMD(
    TRACE_F("%s: DY", __func__);
    BLAS_CALL(print_param_vector_double)(*N, DX, *INCX);
  );
#endif

  TRACE_F("%s: return", __func__);
}

void LIB_F(dscal)(const int *N, const double *DA, double *DX, const int *INCX)
{
  LIB_F(dscal_)(N, DA, DX, INCX);
}

#if LIBBLAS_SCDC_ORIGINALS

void LIB_F(ORIG_F(dscal_))(const int *N, const double *DA, double *DX, const int *INCX)
{
  BLAS_ORIG_F_INIT(dscal_);
  BLAS_ORIG_F(MANGLE_BLAS(dscal_))(N, DA, DX, INCX);
}

#endif /* LIBBLAS_SCDC_ORIGS */

#endif /* BLAS_DSCAL */


#if BLAS_DAXPY

void LIB_F(daxpy_)(const int *N, const double *DA, double *DX, const int *INCX, double *DY, const int *INCY)
{
  TRACE_F("%s: N: %d, DA: %e, DX: %p, INCX: %d, DY: %p, INCY: %d", __func__, *N, *DA, DX, *INCX, DY, *INCY);

#if LIBBLAS_SCDC_PROGRESS
  printf("%s: libblas_scdc_uri: %s\n", __func__, libblas_scdc_uri);
#endif

#if LIBBLAS_SCDC_TRACE_DATA
  TRACE_CMD(
    TRACE_F("%s: DX", __func__);
    BLAS_CALL(print_param_vector_double)(*N, DX, *INCX);
    TRACE_F("%s: DY", __func__);
    BLAS_CALL(print_param_vector_double)(*N, DY, *INCY);
  );
#endif

  BLAS_TIMING_INIT_();

  BLAS_TIMING_START(libblas_scdc_timing[0]);

#if LIBBLAS_SCDC_ENABLED

  libblas_scdc_init();

  blas_call_t bc;

  BLAS_CALL(create_scdc)(&bc, "daxpy", libblas_scdc_uri);

  BLAS_CALL(put_input_conf_int)(&bc, "N", *N);

  BLAS_CALL(put_input_conf_double)(&bc, "DA", *DA);

  BLAS_CALL(put_input_param_vector_double)(&bc, "DX", *N, DX, *INCX);
  BLAS_CALL(put_inout_param_vector_double)(&bc, "DY", *N, DY, *INCY);

  if (!BLAS_CALL(execute)(&bc))
  {
    TRACE_F("%s: failed", __func__);
    goto do_quit;
  }

  double *DY_ = DY;
  int INCY_ = *INCY;
  BLAS_CALL(get_output_param_vector_double)(&bc, "DY", *N, &DY_, &INCY_);

  ASSERT(DY_ == DY && INCY_ == *INCY);

  BLAS_TIMING_REMOTE_GET(&bc, &libblas_scdc_timing[1]);

do_quit:
  BLAS_CALL(destroy_scdc)(&bc);

  libblas_scdc_release();

#else /* LIBBLAS_SCDC_ENABLED */

  BLAS_ORIG_F_INIT(daxpy_);
  BLAS_ORIG_F(MANGLE_BLAS(daxpy_))(N, DA, DX, INCX, DY, INCY);

#endif /* LIBBLAS_SCDC_ENABLED */

  BLAS_TIMING_STOP(libblas_scdc_timing[0]);

  BLAS_TIMING_PRINT_F("%s: %f  %f", __func__, libblas_scdc_timing[0], libblas_scdc_timing[1]);

#if LIBBLAS_SCDC_TRACE_DATA
  TRACE_CMD(
    TRACE_F("%s: DY", __func__);
    BLAS_CALL(print_param_vector_double)(*N, DY, *INCY);
  );
#endif

  TRACE_F("%s: return", __func__);
}

void LIB_F(daxpy)(const int *N, const double *DA, double *DX, const int *INCX, double *DY, const int *INCY)
{
  LIB_F(daxpy_)(N, DA, DX, INCX, DY, INCY);
}

#if LIBBLAS_SCDC_ORIGINALS

void LIB_F(ORIG_F(daxpy_))(const int *N, const double *DA, double *DX, const int *INCX, double *DY, const int *INCY)
{
  BLAS_ORIG_F_INIT(daxpy_);
  BLAS_ORIG_F(MANGLE_BLAS(daxpy_))(N, DA, DX, INCX, DY, INCY);
}

#endif /* LIBBLAS_SCDC_ORIGS */

#endif /* BLAS_DAXPY */

#endif /* BLAS_LEVEL1 */


/* BLAS level 2 */
#if BLAS_LEVEL2

#if BLAS_DGER

void LIB_F(dger_)(const int *M, const int *N, const double *ALPHA, double *X, const int *INCX, double *Y, const int *INCY, double *A, const int *LDA)
{
  TRACE_F("%s: M: %d, N: %d, ALPHA: %e, X: %p, INCX: %d, Y: %p, INCY: %d, A: %p, LDA: %d", __func__, *M, *N, *ALPHA, X, *INCX, Y, *INCY, A, *LDA);

#if LIBBLAS_SCDC_PROGRESS
  printf("%s: libblas_scdc_uri: %s\n", __func__, libblas_scdc_uri);
#endif

#if LIBBLAS_SCDC_TRACE_DATA
  TRACE_CMD(
    TRACE_F("%s: X", __func__);
    BLAS_CALL(print_param_vector_double)(*M, X, *INCX);
    TRACE_F("%s: Y", __func__);
    BLAS_CALL(print_param_vector_double)(*N, Y, *INCY);
    TRACE_F("%s: A", __func__);
    BLAS_CALL(print_param_matrix_double)(*M, *N, A, *LDA, RCM_TYPE_DENSE|RCM_ORDER_COL_MAJOR);
  );
#endif

  BLAS_TIMING_INIT_();

  BLAS_TIMING_START(libblas_scdc_timing[0]);

#if LIBBLAS_SCDC_ENABLED

  libblas_scdc_init();

  blas_call_t bc;

  BLAS_CALL(create_scdc)(&bc, "dger", libblas_scdc_uri);

  BLAS_CALL(put_input_conf_int)(&bc, "M", *M);
  BLAS_CALL(put_input_conf_int)(&bc, "N", *N);

  BLAS_CALL(put_input_conf_double)(&bc, "ALPHA", *ALPHA);

  BLAS_CALL(put_input_param_vector_double)(&bc, "X", *M, X, *INCX);
  BLAS_CALL(put_input_param_vector_double)(&bc, "Y", *N, Y, *INCY);

  BLAS_CALL(put_inout_param_matrix_double)(&bc, "A", *M, *N, A, *LDA, RCM_TYPE_DENSE|RCM_ORDER_COL_MAJOR);

  if(!BLAS_CALL(execute)(&bc))
  {
    TRACE_F("%s: failed", __func__);
    goto do_quit;
  }

  double *A_ = A;
  int LDA_ = *LDA;
  int rcma_ = RCM_TYPE_DENSE|RCM_ORDER_COL_MAJOR;
  BLAS_CALL(get_output_param_matrix_double)(&bc, "A", *M, *N, &A_, &LDA_, &rcma_);

  ASSERT(A_ == A && LDA_ == *LDA && rcma_ == (RCM_TYPE_DENSE|RCM_ORDER_COL_MAJOR));

  BLAS_TIMING_REMOTE_GET(&bc, &libblas_scdc_timing[1]);

do_quit:
  BLAS_CALL(destroy_scdc)(&bc);

  libblas_scdc_release();

#else /* LIBBLAS_SCDC_ENABLED */

  BLAS_ORIG_F_INIT(dger_);
  BLAS_ORIG_F(MANGLE_BLAS(dger_))(M, N, ALPHA, X, INCX, Y, INCY, A, LDA);

#endif /* LIBBLAS_SCDC_ENABLED */

  BLAS_TIMING_STOP(libblas_scdc_timing[0]);

  BLAS_TIMING_PRINT_F("%s: %f  %f", __func__, libblas_scdc_timing[0], libblas_scdc_timing[1]);

#if LIBBLAS_SCDC_TRACE_DATA
  TRACE_CMD(
    TRACE_F("%s: Y", __func__);
    BLAS_CALL(print_param_matrix_double)(*M, *N, A, *LDA, RCM_TYPE_DENSE|RCM_ORDER_COL_MAJOR);
  );
#endif

  TRACE_F("%s: return", __func__);
}

void LIB_F(dger)(const int *M, const int *N, const double *ALPHA, double *X, const int *INCX, double *Y, const int *INCY, double *A, const int *LDA)
{
  LIB_F(dger_)(M, N, ALPHA, X, INCX, Y, INCY, A, LDA);
}

#if LIBBLAS_SCDC_ORIGINALS

void LIB_F(ORIG_F(dger_))(const int *M, const int *N, const double *ALPHA, double *X, const int *INCX, double *Y, const int *INCY, double *A, const int *LDA)
{
  BLAS_ORIG_F_INIT(dger_);
  BLAS_ORIG_F(MANGLE_BLAS(dger_))(M, N, ALPHA, X, INCX, Y, INCY, A, LDA);
}

#endif /* LIBBLAS_SCDC_ORIGS */

#endif /* BLAS_DGER */


#if BLAS_DGEMV

void LIB_F(dgemv_)(const char *TRANS, const int *M, const int *N, const double *ALPHA, double *A, const int *LDA, double *X, const int *INCX, const double *BETA, double *Y, const int *INCY)
{
  TRACE_F("%s: TRANS: %c, M: %d, N: %d, ALPHA: %e, A: %p, LDA: %d, X: %p, INCX: %d, BETA: %e, Y: %p, INCY: %d", __func__, *TRANS, *M, *N, *ALPHA, A, *LDA, X, *INCX, *BETA, Y, *INCY);

#if LIBBLAS_SCDC_PROGRESS
  printf("%s: libblas_scdc_uri: %s\n", __func__, libblas_scdc_uri);
#endif

#if LIBBLAS_SCDC_TRACE_DATA
  TRACE_CMD(
    TRACE_F("%s: A", __func__);
    BLAS_CALL(print_param_matrix_double)(*M, *N, A, *LDA, RCM_TYPE_DENSE|RCM_ORDER_COL_MAJOR);
    TRACE_F("%s: X", __func__);
    BLAS_CALL(print_param_vector_double)(NETLIB_TRANS_GET_NCOLS(*TRANS, *M, *N), X, *INCX);
    TRACE_F("%s: Y", __func__);
    BLAS_CALL(print_param_vector_double)(NETLIB_TRANS_GET_NROWS(*TRANS, *M, *N), Y, *INCY);
  );
#endif

  BLAS_TIMING_INIT_();

  BLAS_TIMING_START(libblas_scdc_timing[0]);

#if LIBBLAS_SCDC_ENABLED

  libblas_scdc_init();

  blas_call_t bc;

  BLAS_CALL(create_scdc)(&bc, "dgemv", libblas_scdc_uri);

  BLAS_CALL(put_input_conf_char)(&bc, "TRANS", *TRANS);

  BLAS_CALL(put_input_conf_int)(&bc, "M", *M);
  BLAS_CALL(put_input_conf_int)(&bc, "N", *N);

  BLAS_CALL(put_input_conf_double)(&bc, "ALPHA", *ALPHA);

  BLAS_CALL(put_input_param_matrix_double)(&bc, "A", *M, *N, A, *LDA, RCM_TYPE_DENSE|RCM_ORDER_COL_MAJOR);

  BLAS_CALL(put_input_param_vector_double)(&bc, "X", NETLIB_TRANS_GET_NCOLS(*TRANS, *M, *N), X, *INCX);

  BLAS_CALL(put_input_conf_double)(&bc, "BETA", *BETA);

  BLAS_CALL(put_inout_param_vector_double)(&bc, "Y", NETLIB_TRANS_GET_NROWS(*TRANS, *M, *N), Y, *INCY);

  if(!BLAS_CALL(execute)(&bc))
  {
    TRACE_F("%s: failed", __func__);
    goto do_quit;
  }

  double *Y_ = Y;
  int INCY_ = *INCY;
  BLAS_CALL(get_output_param_vector_double)(&bc, "Y", NETLIB_TRANS_GET_NROWS(*TRANS, *M, *N), &Y_, &INCY_);

  ASSERT(Y_ == Y && INCY_ == *INCY);

  BLAS_TIMING_REMOTE_GET(&bc, &libblas_scdc_timing[1]);

do_quit:
  BLAS_CALL(destroy_scdc)(&bc);

  libblas_scdc_release();

#else /* LIBBLAS_SCDC_ENABLED */

  BLAS_ORIG_F_INIT(dgemv_);
  BLAS_ORIG_F(MANGLE_BLAS(dgemv_))(TRANS, M, N, ALPHA, A, LDA, X, INCX, BETA, Y, INCY);

#endif /* LIBBLAS_SCDC_ENABLED */

  BLAS_TIMING_STOP(libblas_scdc_timing[0]);

  BLAS_TIMING_PRINT_F("%s: %f  %f", __func__, libblas_scdc_timing[0], libblas_scdc_timing[1]);

#if LIBBLAS_SCDC_TRACE_DATA
  TRACE_CMD(
    TRACE_F("%s: Y", __func__);
    BLAS_CALL(print_param_vector_double)(NETLIB_TRANS_GET_NROWS(*TRANS, *M, *N), Y, *INCY);
  );
#endif

  TRACE_F("%s: return", __func__);
}

void LIB_F(dgemv)(const char *TRANS, const int *M, const int *N, const double *ALPHA, double *A, const int *LDA, double *X, const int *INCX, const double *BETA, double *Y, const int *INCY)
{
  LIB_F(dgemv_)(TRANS, M, N, ALPHA, A, LDA, X, INCX, BETA, Y, INCY);
}

#if LIBBLAS_SCDC_ORIGINALS

void LIB_F(ORIG_F(dgemv_))(const char *TRANS, const int *M, const int *N, const double *ALPHA, double *A, const int *LDA, double *X, const int *INCX, const double *BETA, double *Y, const int *INCY)
{
  BLAS_ORIG_F_INIT(dgemv_);
  BLAS_ORIG_F(MANGLE_BLAS(dgemv_))(TRANS, M, N, ALPHA, A, LDA, X, INCX, BETA, Y, INCY);
}

#endif /* LIBBLAS_SCDC_ORIGS */

#endif /* BLAS_DGEMV */


#if BLAS_DTRSV

void LIB_F(dtrsv_)(const char *UPLO, const char *TRANS, const char *DIAG, const int *N, double *A, const int *LDA, double *X, const int *INCX)
{
  TRACE_F("%s: UPLO: %c, TRANS: %c, DIAG: %c, N: %d, A: %p, LDA: %d, X: %p, INCX: %d", __func__, *UPLO, *TRANS, *DIAG, *N, A, *LDA, X, *INCX);

#if LIBBLAS_SCDC_PROGRESS
  printf("%s: libblas_scdc_uri: %s\n", __func__, libblas_scdc_uri);
#endif

  int rcma = RCM_ORDER_COL_MAJOR;
  rcma |= NETLIB_UPLO_IS_UPPER(*UPLO)?RCM_TYPE_TRIANGULAR_UPPER:RCM_TYPE_TRIANGULAR_LOWER;
  rcma |= NETLIB_DIAG_IS_UNIT(*DIAG)?RCM_DIAG:RCM_DIAG_NOT;

#if LIBBLAS_SCDC_TRACE_DATA
  TRACE_CMD(
    TRACE_F("%s: A", __func__);
    BLAS_CALL(print_param_matrix_double)(*N, *N, A, *LDA, rcma);
    TRACE_F("%s: X", __func__);
    BLAS_CALL(print_param_vector_double)(*N, X, *INCX);
  );
#endif

  BLAS_TIMING_INIT_();

  BLAS_TIMING_START(libblas_scdc_timing[0]);

#if LIBBLAS_SCDC_ENABLED

  libblas_scdc_init();

  blas_call_t bc;

  BLAS_CALL(create_scdc)(&bc, "dtrsv", libblas_scdc_uri);

  BLAS_CALL(put_input_conf_char)(&bc, "UPLO", *UPLO);
  BLAS_CALL(put_input_conf_char)(&bc, "TRANS", *TRANS);
  BLAS_CALL(put_input_conf_char)(&bc, "DIAG", *DIAG);

  BLAS_CALL(put_input_conf_int)(&bc, "N", *N);

  BLAS_CALL(put_input_param_matrix_double)(&bc, "A", *N, *N, A, *LDA, rcma);

  BLAS_CALL(put_inout_param_vector_double)(&bc, "X", *N, X, *INCX);

  if(!BLAS_CALL(execute)(&bc))
  {
    TRACE_F("%s: failed", __func__);
    goto do_quit;
  }

  double *X_ = X;
  int INCX_ = *INCX;
  BLAS_CALL(get_output_param_vector_double)(&bc, "X", *N, &X_, &INCX_);

  ASSERT(X_ == X && INCX_ == *INCX);

  BLAS_TIMING_REMOTE_GET(&bc, &libblas_scdc_timing[1]);

do_quit:
  BLAS_CALL(destroy_scdc)(&bc);

  libblas_scdc_release();

#else /* LIBBLAS_SCDC_ENABLED */

  BLAS_ORIG_F_INIT(dtrsv_);
  BLAS_ORIG_F(MANGLE_BLAS(dtrsv_))(UPLO, TRANS, DIAG, N, A, LDA, X, INCX);

#endif /* LIBBLAS_SCDC_ENABLED */

  BLAS_TIMING_STOP(libblas_scdc_timing[0]);

  BLAS_TIMING_PRINT_F("%s: %f  %f", __func__, libblas_scdc_timing[0], libblas_scdc_timing[1]);

#if LIBBLAS_SCDC_TRACE_DATA
  TRACE_CMD(
    TRACE_F("%s: X", __func__);
    BLAS_CALL(print_param_vector_double)(*N, X, *INCX);
  );
#endif

  TRACE_F("%s: return", __func__);
}

void LIB_F(dtrsv)(const char *UPLO, const char *TRANS, const char *DIAG, const int *N, double *A, const int *LDA, double *X, const int *INCX)
{
  LIB_F(dtrsv_)(UPLO, TRANS, DIAG, N, A, LDA, X, INCX);
}

#if LIBBLAS_SCDC_ORIGINALS

void LIB_F(ORIG_F(dtrsv_))(const char *UPLO, const char *TRANS, const char *DIAG, const int *N, double *A, const int *LDA, double *X, const int *INCX)
{
  BLAS_ORIG_F_INIT(dtrsv_);
  BLAS_ORIG_F(MANGLE_BLAS(dtrsv_))(UPLO, TRANS, DIAG, N, A, LDA, X, INCX);
}

#endif /* LIBBLAS_SCDC_ORIGS */

#endif /* BLAS_DTRSV */

#endif /* BLAS_LEVEL2 */


/* BLAS level 3 */
#if BLAS_LEVEL3

#if BLAS_DGEMM

void LIB_F(dgemm_)(const char *TRANSA, const char *TRANSB, const int *M, const int *N, const int *K, const double *ALPHA, double *A, const int *LDA, double *B, const int *LDB, const double *BETA, double *C, const int *LDC)
{
  TRACE_F("%s: TRANSA: %c, TRANSB: %c, M: %d, N: %d, K: %d, ALPHA: %e, A: %p, LDA: %d, B: %p, LDB: %d, BETA: %e, C: %p, LDC: %d", __func__, *TRANSA, *TRANSB, *M, *N, *K, *ALPHA, A, *LDA, B, *LDB, *BETA, C, *LDC);

#if LIBBLAS_SCDC_PROGRESS
  printf("%s: libblas_scdc_uri: %s\n", __func__, libblas_scdc_uri);
#endif

#if LIBBLAS_SCDC_TRACE_DATA
  TRACE_CMD(
    TRACE_F("%s: A", __func__);
    BLAS_CALL(print_param_matrix_double)(NETLIB_TRANS_GET_NROWS(*TRANSA, *M, *K), NETLIB_TRANS_GET_NCOLS(*TRANSA, *M, *K), A, *LDA, RCM_TYPE_DENSE|RCM_ORDER_COL_MAJOR);
    TRACE_F("%s: B", __func__);
    BLAS_CALL(print_param_matrix_double)(NETLIB_TRANS_GET_NROWS(*TRANSB, *K, *N), NETLIB_TRANS_GET_NCOLS(*TRANSB, *K, *N), B, *LDB, RCM_TYPE_DENSE|RCM_ORDER_COL_MAJOR);
    TRACE_F("%s: C", __func__);
    BLAS_CALL(print_param_matrix_double)(*M, *N, C, *LDC, RCM_TYPE_DENSE|RCM_ORDER_COL_MAJOR);
  );
#endif

  BLAS_TIMING_INIT_();

  BLAS_TIMING_START(libblas_scdc_timing[0]);

#if LIBBLAS_SCDC_ENABLED

  libblas_scdc_init();

  blas_call_t bc;

  BLAS_CALL(create_scdc)(&bc, "dgemm", libblas_scdc_uri);

  BLAS_CALL(put_input_conf_char)(&bc, "TRANSA", *TRANSA);
  BLAS_CALL(put_input_conf_char)(&bc, "TRANSB", *TRANSB);

  BLAS_CALL(put_input_conf_int)(&bc, "M", *M);
  BLAS_CALL(put_input_conf_int)(&bc, "N", *N);
  BLAS_CALL(put_input_conf_int)(&bc, "K", *K);

  BLAS_CALL(put_input_conf_double)(&bc, "ALPHA", *ALPHA);

  BLAS_CALL(put_input_param_matrix_double)(&bc, "A", NETLIB_TRANS_GET_NROWS(*TRANSA, *M, *K), NETLIB_TRANS_GET_NCOLS(*TRANSA, *M, *K), A, *LDA, RCM_TYPE_DENSE|RCM_ORDER_COL_MAJOR);
  BLAS_CALL(put_input_param_matrix_double)(&bc, "B", NETLIB_TRANS_GET_NROWS(*TRANSB, *K, *N), NETLIB_TRANS_GET_NCOLS(*TRANSB, *K, *N), B, *LDB, RCM_TYPE_DENSE|RCM_ORDER_COL_MAJOR);

  BLAS_CALL(put_input_conf_double)(&bc, "BETA", *BETA);

  BLAS_CALL(put_inout_param_matrix_double)(&bc, "C", *M, *N, C, *LDC, RCM_TYPE_DENSE|RCM_ORDER_COL_MAJOR);

  if (!BLAS_CALL(execute)(&bc))
  {
    TRACE_F("%s: failed", __func__);
    goto do_quit;
  }

  double *C_ = C;
  int LDC_ = *LDC;
  int rcmc_ = RCM_TYPE_DENSE|RCM_ORDER_COL_MAJOR;
  BLAS_CALL(get_output_param_matrix_double)(&bc, "A", *M, *N, &C_, &LDC_, &rcmc_);

  ASSERT(C_ == C && LDC_ == *LDC && rcmc_ == (RCM_TYPE_DENSE|RCM_ORDER_COL_MAJOR));

  BLAS_TIMING_REMOTE_GET(&bc, &libblas_scdc_timing[1]);

do_quit:
  BLAS_CALL(destroy_scdc)(&bc);

  libblas_scdc_release();

#else /* LIBBLAS_SCDC_ENABLED */

  BLAS_ORIG_F_INIT(dgemm_);
  BLAS_ORIG_F(MANGLE_BLAS(dgemm_))(TRANSA, TRANSB, M, N, K, ALPHA, A, LDA, B, LDB, BETA, C, LDC);

#endif /* LIBBLAS_SCDC_ENABLED */

  BLAS_TIMING_STOP(libblas_scdc_timing[0]);

  BLAS_TIMING_PRINT_F("%s: %f  %f", __func__, libblas_scdc_timing[0], libblas_scdc_timing[1]);

#if LIBBLAS_SCDC_TRACE_DATA
  TRACE_CMD(
    TRACE_F("%s: C", __func__);
    BLAS_CALL(print_param_matrix_double)(*M, *N, C, *LDC, RCM_TYPE_DENSE|RCM_ORDER_COL_MAJOR);
  );
#endif

  TRACE_F("%s: return", __func__);
}

void LIB_F(dgemm)(const char *TRANSA, const char *TRANSB, const int *M, const int *N, const int *K, const double *ALPHA, double *A, const int *LDA, double *B, const int *LDB, const double *BETA, double *C, const int *LDC)
{
  LIB_F(dgemm_)(TRANSA, TRANSB, M, N, K, ALPHA, A, LDA, B, LDB, BETA, C, LDC);
}

#if LIBBLAS_SCDC_ORIGINALS

void LIB_F(ORIG_F(dgemm_))(const char *TRANSA, const char *TRANSB, const int *M, const int *N, const int *K, const double *ALPHA, double *A, const int *LDA, double *B, const int *LDB, const double *BETA, double *C, const int *LDC)
{
  BLAS_ORIG_F_INIT(dgemm_);
  BLAS_ORIG_F(MANGLE_BLAS(dgemm_))(TRANSA, TRANSB, M, N, K, ALPHA, A, LDA, B, LDB, BETA, C, LDC);
}

#endif /* LIBBLAS_SCDC_ORIGS */

#endif /* BLAS_DGEMM */


#if BLAS_DTRSM

void LIB_F(dtrsm_)(const char *SIDE, const char *UPLO, const char *TRANSA, const char *DIAG, const int *M, const int *N, double *ALPHA, double *A, const int *LDA, double *B, const int *LDB)
{
  TRACE_F("%s: SIDE: %c, UPLO: %c, TRANSA: %c, DIAG: %c, M: %d, N: %d, ALPHA: %e, A: %p, LDA: %d, B: %p, LDB: %d", __func__, *SIDE, *UPLO, *TRANSA, *DIAG, *M, *N, *ALPHA, A, *LDA, B, *LDB);

#if LIBBLAS_SCDC_PROGRESS
  printf("%s: libblas_scdc_uri: %s\n", __func__, libblas_scdc_uri);
#endif

  const int k = NETLIB_SIDE_IS_LEFT(*SIDE)?*M:*N;

  int rcma = RCM_ORDER_COL_MAJOR;
  rcma |= NETLIB_UPLO_IS_UPPER(*UPLO)?RCM_TYPE_TRIANGULAR_UPPER:RCM_TYPE_TRIANGULAR_LOWER;
  rcma |= NETLIB_DIAG_IS_UNIT(*DIAG)?RCM_DIAG:RCM_DIAG_NOT;

#if LIBBLAS_SCDC_TRACE_DATA
  TRACE_CMD(
    TRACE_F("%s: A", __func__);
    BLAS_CALL(print_param_matrix_double)(k, k, A, *LDA, rcma);
    TRACE_F("%s: B", __func__);
    BLAS_CALL(print_param_matrix_double)(*M, *N, B, *LDB, RCM_TYPE_DENSE|RCM_ORDER_COL_MAJOR);
  );
#endif

  BLAS_TIMING_INIT_();

  BLAS_TIMING_START(libblas_scdc_timing[0]);

#if LIBBLAS_SCDC_ENABLED

  libblas_scdc_init();

  blas_call_t bc;

  BLAS_CALL(create_scdc)(&bc, "dtrsm", libblas_scdc_uri);

  BLAS_CALL(put_input_conf_char)(&bc, "SIDE", *SIDE);
  BLAS_CALL(put_input_conf_char)(&bc, "UPLO", *UPLO);
  BLAS_CALL(put_input_conf_char)(&bc, "TRANSA", *TRANSA);
  BLAS_CALL(put_input_conf_char)(&bc, "DIAG", *DIAG);

  BLAS_CALL(put_input_conf_int)(&bc, "M", *M);
  BLAS_CALL(put_input_conf_int)(&bc, "N", *N);

  BLAS_CALL(put_input_conf_double)(&bc, "ALPHA", *ALPHA);

  BLAS_CALL(put_input_param_matrix_double)(&bc, "A", k, k, A, *LDA, rcma);
  BLAS_CALL(put_inout_param_matrix_double)(&bc, "B", *M, *N, B, *LDB, RCM_TYPE_DENSE|RCM_ORDER_COL_MAJOR);

  if (!BLAS_CALL(execute)(&bc))
  {
    TRACE_F("%s: failed", __func__);
    goto do_quit;
  }

  double *B_ = B;
  int LDB_ = *LDB;
  int rcmb_ = RCM_TYPE_DENSE|RCM_ORDER_COL_MAJOR;
  BLAS_CALL(get_output_param_matrix_double)(&bc, "B", *M, *N, &B_, &LDB_, &rcmb_);

  ASSERT(B_ == B && LDB_ == *LDB && rcmb_ == (RCM_TYPE_DENSE|RCM_ORDER_COL_MAJOR));

  BLAS_TIMING_REMOTE_GET(&bc, &libblas_scdc_timing[1]);

do_quit:
  BLAS_CALL(destroy_scdc)(&bc);

  libblas_scdc_release();

#else /* LIBBLAS_SCDC_ENABLED */

  BLAS_ORIG_F_INIT(dtrsm_);
  BLAS_ORIG_F(MANGLE_BLAS(dtrsm_))(SIDE, UPLO, TRANSA, DIAG, M, N, ALPHA, A, LDA, B, LDB);

#endif /* LIBBLAS_SCDC_ENABLED */

  BLAS_TIMING_STOP(libblas_scdc_timing[0]);

  BLAS_TIMING_PRINT_F("%s: %f  %f", __func__, libblas_scdc_timing[0], libblas_scdc_timing[1]);

#if LIBBLAS_SCDC_TRACE_DATA
  TRACE_CMD(
    TRACE_F("%s: C", __func__);
    BLAS_CALL(print_param_matrix_double)(*M, *N, B, *LDB, RCM_TYPE_DENSE|RCM_ORDER_COL_MAJOR);
  );
#endif

  TRACE_F("%s: return", __func__);
}

void LIB_F(dtrsm)(const char *SIDE, const char *UPLO, const char *TRANSA, const char *DIAG, const int *M, const int *N, double *ALPHA, double *A, const int *LDA, double *B, const int *LDB)
{
  LIB_F(dtrsm_)(SIDE, UPLO, TRANSA, DIAG, M, N, ALPHA, A, LDA, B, LDB);
}

#if LIBBLAS_SCDC_ORIGINALS

void LIB_F(ORIG_F(dtrsm_))(const char *SIDE, const char *UPLO, const char *TRANSA, const char *DIAG, const int *M, const int *N, double *ALPHA, double *A, const int *LDA, double *B, const int *LDB)
{
  BLAS_ORIG_F_INIT(dtrsm_);
  BLAS_ORIG_F(MANGLE_BLAS(dtrsm_))(SIDE, UPLO, TRANSA, DIAG, M, N, ALPHA, A, LDA, B, LDB);
}

#endif /* LIBBLAS_SCDC_ORIGS */

#endif /* BLAS_DTRSM */

#endif /* BLAS_LEVEL3 */
