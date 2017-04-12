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

#include "lapack.h"
#include "z_pack.h"
#include "common.h"
#include "lapack_call.h"
#include "lapack_scdc.h"


#define TRACE_PREFIX  "liblapack_scdc: "


#define LIBLAPACK_SCDC_ENABLED          0

/*#define LIBLAPACK_SCDC_PREFIX           0*/
#define LIBLAPACK_SCDC_PREFIX_NAME      liblapack_scdc_

#if LIBLAPACK_SCDC_PREFIX

# undef __BLAS_H__
# undef MANGLE_BLAS
# define MANGLE_BLAS(_f_)  Z_CONCAT(LIBLAPACK_SCDC_PREFIX_NAME, _f_)
# include "lapack.h"

# define LIB_F(_f_)      Z_CONCAT(LIBLAPACK_SCDC_PREFIX_NAME, _f_)
# define LIB_F_STR(_f_)  Z_STRINGIFY(Z_CONCAT(LIBLAPACK_SCDC_PREFIX_NAME, _f_))

#else /* LIBLAPACK_SCDC_PREFIX */

# define LIB_F(_f_)      _f_
# define LIB_F_STR(_f_)  Z_STRINGIFY(_f_)

# define LAPACK_ORIG_ENABLED  1

#endif /* LIBLAPACK_SCDC_PREFIX */

#include "lapack_orig.h"


#define LIBLAPACK_SCDC_LOCAL       1
#define LIBLAPACK_SCDC_LOCAL_BASE  "lapack"
#define LIBLAPACK_SCDC_LOCAL_URI   "scdc:/"

const char *liblapack_scdc_local_base = 0;
scdc_dataprov_t liblapack_scdc_local_dataprov = SCDC_DATAPROV_NULL;

const char *liblapack_scdc_uri = 0;

int liblapack_scdc_initialized = -1;

#if 1
# define LIBLAPACK_SCDC_URI  "scdc:/lapack"
#else
# define LIBLAPACK_SCDC_URI  "scdc+uds://liblapack_scdc/lapack"
#endif


static void liblapack_scdc_init();
static void liblapack_scdc_release();


static void liblapack_scdc_init()
{
  TRACE_F("%s: liblapack_scdc_initialized: %d", __func__, liblapack_scdc_initialized);

  if (liblapack_scdc_initialized < 0)
  {
    /* on first init */
    TRACE_F("%s: first init", __func__);

    LAPACK_ORIG_INIT();

    scdc_init(SCDC_INIT_DEFAULT);

#if LIBLAPACK_SCDC_LOCAL
    liblapack_scdc_local_base = getenv("LILAPACK_SCDC_LOCAL_BASE");
    if (!liblapack_scdc_local_base) liblapack_scdc_local_base = LIBLAPACK_SCDC_LOCAL_BASE;

    liblapack_scdc_local_dataprov = scdc_dataprov_open(liblapack_scdc_local_base, "hook", &lapack_scdc_hook);
#endif

    liblapack_scdc_uri = getenv("LILAPACK_SCDC_URI");

    if (!liblapack_scdc_uri)
    {
#if defined(LIBLAPACK_SCDC_URI)
      liblapack_scdc_uri = LIBLAPACK_SCDC_URI;
#elif LIBLAPACK_SCDC_LOCAL
      liblapack_scdc_uri = LIBLAPACK_SCDC_LOCAL_URI LIBLAPACK_SCDC_LOCAL_BASE;
#endif
    }

    /* register release handler */
    atexit(liblapack_scdc_release);

    ++liblapack_scdc_initialized;
  }

  if (liblapack_scdc_initialized == 0)
  {
    /* on every init */
    TRACE_F("%s: every init", __func__);
  }

  ++liblapack_scdc_initialized;

  TRACE_F("%s: return: liblapack_scdc_initialized: %d", __func__, liblapack_scdc_initialized);
}


static void liblapack_scdc_release()
{
  --liblapack_scdc_initialized;

  if (liblapack_scdc_initialized == 0)
  {
    /* on every release */
    TRACE_F("%s: every release", __func__);
  }

  if (liblapack_scdc_initialized < 0)
  {
    /* on last release */
    TRACE_F("%s: last release", __func__);

#if LIBLAPACK_SCDC_LOCAL
    scdc_dataprov_close(liblapack_scdc_local_dataprov);
    liblapack_scdc_local_dataprov = SCDC_DATAPROV_NULL;
#endif

    scdc_release();

    LAPACK_ORIG_RELEASE();
  }
}


#if LAPACK_SGESV

void LIB_F(sgesv)(const int *N, const int *NRHS, float *A, const int *LDA, int *IPIV, float *B, const int *LDB, int *INFO)
{
  TRACE_F("%s: N: %d, NRHS: %d, A: %p, LDA: %d, IPIV: %p, B: %p, LDB: %d, INFO: %d", __func__, *N, *NRHS, A, *LDA, IPIV, B, *LDB, *INFO);

  liblapack_scdc_init();

  lapack_call_t lc;

  LAPACK_CALL(create_scdc)(&lc, "sgesv", liblapack_scdc_uri);

  LAPACK_CALL(put_input_conf_int)(&lc, "N", *N);
  LAPACK_CALL(put_input_conf_int)(&lc, "NRHS", *NRHS);

  LAPACK_CALL(put_input_param_matrix_float)(&lc, "A", *N, *N, A, *LDA, RCM_TYPE_DENSE|RCM_ORDER_COL_MAJOR);
  LAPACK_CALL(put_input_param_matrix_float)(&lc, "B", *N, *NRHS, B, *LDB, RCM_TYPE_DENSE|RCM_ORDER_COL_MAJOR);

  LAPACK_CALL(put_output_param_vector_int)(&lc, "IPIV", *N, IPIV);

  LAPACK_CALL(execute)(&lc);

  int LDA_ = *LDA, LDB_ = *LDB;
  int rcma_ = RCM_TYPE_DENSE|RCM_ORDER_COL_MAJOR;
  int rcmb_ = RCM_TYPE_DENSE|RCM_ORDER_COL_MAJOR;
  LAPACK_CALL(get_output_param_matrix_float)(&lc, "A", *N, *N, &A, &LDA_, &rcma_);
  LAPACK_CALL(get_output_param_matrix_float)(&lc, "B", *N, *NRHS, &B, &LDB_, &rcmb_);

  ASSERT(LDA_ == *LDA);
  ASSERT(rcma_ == (RCM_TYPE_DENSE|RCM_ORDER_COL_MAJOR));
  ASSERT(LDB_ == *LDB);
  ASSERT(rcmb_ == (RCM_TYPE_DENSE|RCM_ORDER_COL_MAJOR));

  LAPACK_CALL(get_output_param_vector_int)(&lc, "IPIV", *N, &IPIV);

  LAPACK_CALL(get_output_conf_int)(&lc, "INFO", INFO);

  LAPACK_CALL(destroy_scdc)(&lc);

  liblapack_scdc_release();

  TRACE_F("%s: return", __func__);
}

void LIB_F(sgesv_)(const int *N, const int *NRHS, float *A, const int *LDA, int *IPIV, float *B, const int *LDB, int *INFO)
{
  LIB_F(sgesv)(N, NRHS, A, LDA, IPIV, B, LDB, INFO);
}

#endif /* LAPACK_SGESV */


#if LAPACK_STRSV

void LIB_F(strsv)(const char *UPLO, const char *TRANS, const char *DIAG, const int *N, float *A, const int *LDA, float *X, const int *INCX)
{
  TRACE_F("%s: UPLO: %c, TRANS: %c, DIAG: %c, N: %d, A: %p, LDA: %d, X: %p, INCX: %d", __func__, *UPLO, *TRANS, *DIAG, *N, A, *LDA, X, *INCX);

  liblapack_scdc_init();

  lapack_call_t lc;

  LAPACK_CALL(create_scdc)(&lc, "strsv", liblapack_scdc_uri);

  LAPACK_CALL(put_input_conf_char)(&lc, "UPLO", *UPLO);
  LAPACK_CALL(put_input_conf_char)(&lc, "TRANS", *TRANS);
  LAPACK_CALL(put_input_conf_char)(&lc, "DIAG", *DIAG);

  LAPACK_CALL(put_input_conf_int)(&lc, "N", *N);
  LAPACK_CALL(put_input_param_matrix_float)(&lc, "A", *N, *N, A, *LDA, (LAPACK_IS_UPPER(*UPLO)?RCM_TYPE_TRIANGULAR_UPPER:RCM_TYPE_TRIANGULAR_LOWER)|RCM_ORDER_COL_MAJOR);

  LAPACK_CALL(put_input_param_vector_float)(&lc, "X", *N, X, *INCX);

  LAPACK_CALL(execute)(&lc);

  int INCX_ = *INCX;
  LAPACK_CALL(get_output_param_vector_float)(&lc, "X", *N, &X, &INCX_);

  ASSERT(INCX_ == *INCX);

  LAPACK_CALL(destroy_scdc)(&lc);

  liblapack_scdc_release();

  TRACE_F("%s: return", __func__);
}

void LIB_F(strsv_)(const char *UPLO, const char *TRANS, const char *DIAG, const int *N, float *A, const int *LDA, float *X, const int *INCX)
{
  LIB_F(strsv)(UPLO, TRANS, DIAG, N, A, LDA, X, INCX);
}

#endif /* LAPACK_STRSV */
