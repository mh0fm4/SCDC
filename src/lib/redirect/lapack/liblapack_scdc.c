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

#include "z_pack.h"

#include "lapack_scdc_config.h"
#include "common.h"
#include "lapack.h"
#include "lapack_call.h"
#include "lapack_scdc.h"
#include "lapack_timing.h"


#define TRACE_PREFIX  "liblapack_scdc: "


#if LIBLAPACK_SCDC_PREFIX

# undef __LAPACK_H__
# undef MANGLE_LAPACK
# define MANGLE_LAPACK(_f_)  Z_CONCAT(LIBLAPACK_SCDC_PREFIX_NAME, _f_)
# include "lapack.h"

# define LIB_F(_f_)      Z_CONCAT(LIBLAPACK_SCDC_PREFIX_NAME, _f_)
# define LIB_F_STR(_f_)  Z_STRINGIFY(Z_CONCAT(LIBLAPACK_SCDC_PREFIX_NAME, _f_))

#else /* LIBLAPACK_SCDC_PREFIX */

# define LIB_F(_f_)      _f_
# define LIB_F_STR(_f_)  Z_STRINGIFY(_f_)

# define LAPACK_ORIG_ENABLED  1

#endif /* LIBLAPACK_SCDC_PREFIX */

#include "lapack_orig.h"


const char *liblapack_scdc_local_base = 0;
scdc_dataprov_t liblapack_scdc_local_dataprov = SCDC_DATAPROV_NULL;

const char *liblapack_scdc_uri = 0;

int liblapack_scdc_initialized = -1;

#if LIBLAPACK_SCDC_TIMING
# undef LAPACK_TIMING_PREFIX
# define LAPACK_TIMING_PREFIX  TRACE_PREFIX  "TIMING: "
# define LIBLAPACK_SCDC_TIMING_X  5
int liblapack_scdc_timing_i;
double liblapack_scdc_timing[LIBLAPACK_SCDC_TIMING_X];
#endif
#define LAPACK_TIMING_INIT_()  LAPACK_TIMING_INIT(liblapack_scdc_timing, liblapack_scdc_timing_i, LIBLAPACK_SCDC_TIMING_X)

#if LIBLAPACK_SCDC_TIMING_REDIRECT_REMOTE
# define LAPACK_TIMING_REMOTE_GET(_bc_, _t_)  LAPACK_CALL(get_output_conf_double)(_bc_, "TIMING", _t_);
#else
# define LAPACK_TIMING_REMOTE_GET(_bc_, _t_)  Z_NOP()
#endif


#if LIBLAPACK_SCDC_ENABLED

void liblapack_scdc_release();

void liblapack_scdc_init()
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
#if defined(LIBLAPACK_SCDC_URI_DEFAULT)
      liblapack_scdc_uri = LIBLAPACK_SCDC_URI_DEFAULT;
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


void liblapack_scdc_release()
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

#endif /* LIBLAPACK_SCDC_ENABLED */


#if LAPACK_SGESV

void LIB_F(sgesv_)(const int *N, const int *NRHS, float *A, const int *LDA, int *IPIV, float *B, const int *LDB, int *INFO)
{
  TRACE_F("%s: N: %d, NRHS: %d, A: %p, LDA: %d, IPIV: %p, B: %p, LDB: %d, INFO: %d", __func__, *N, *NRHS, A, *LDA, IPIV, B, *LDB, *INFO);

#if LIBLAPACK_SCDC_PROGRESS
  printf("%s: liblapack_scdc_uri: %s\n", __func__, liblapack_scdc_uri);
#endif

#if LIBLAPACK_SCDC_TRACE_DATA
  TRACE_CMD(
/*    TRACE_F("%s: DX", __func__);
    BLAS_CALL(print_param_vector_double)(*N, DX, *INCX);*/
  );
#endif

  LAPACK_TIMING_INIT_();

  LAPACK_TIMING_START(liblapack_scdc_timing[0]);

#if LIBLAPACK_SCDC_ENABLED

  liblapack_scdc_init();

  lapack_call_t lc;

  LAPACK_CALL(create_scdc)(&lc, "sgesv", liblapack_scdc_uri);

  LAPACK_CALL(put_input_conf_int)(&lc, "N", *N);
  LAPACK_CALL(put_input_conf_int)(&lc, "NRHS", *NRHS);

  LAPACK_CALL(put_inout_param_matrix_float)(&lc, "A", A, *N, *N, *LDA, RCM_TYPE_DENSE|RCM_ORDER_COL_MAJOR);
  LAPACK_CALL(put_inout_param_matrix_float)(&lc, "B", B, *N, *NRHS, *LDB, RCM_TYPE_DENSE|RCM_ORDER_COL_MAJOR);

  LAPACK_CALL(put_output_param_array_int)(&lc, "IPIV", IPIV, *N);

  if(!LAPACK_CALL(execute)(&lc))
  {
    TRACE_F("%s: failed", __func__);
    goto do_quit;
  }

  float *A_ = A, *B_ = B;
  rdint_t NRA_ = *N, NCA_ = *N, NRB_ = *N, NCB_ = *NRHS;
  rdint_t LDA_ = *LDA, LDB_ = *LDB;
  rdint_t rcma_ = RCM_TYPE_DENSE|RCM_ORDER_COL_MAJOR;
  rdint_t rcmb_ = RCM_TYPE_DENSE|RCM_ORDER_COL_MAJOR;
  LAPACK_CALL(get_output_param_matrix_float)(&lc, "A", &A_, &NRA_, &NCA_, &LDA_, &rcma_);
  LAPACK_CALL(get_output_param_matrix_float)(&lc, "B", &B_, &NRB_, &NCB_, &LDB_, &rcmb_);

  ASSERT(A_ == A && NRA_ == *N && NCA_ == *N && LDA_ == *LDA && rcma_ == (RCM_TYPE_DENSE|RCM_ORDER_COL_MAJOR));
  ASSERT(B_ == B && NCB_ == *N && NCB_ == *NRHS && LDB_ == *LDB && rcmb_ == (RCM_TYPE_DENSE|RCM_ORDER_COL_MAJOR));

  rdint_t NIPIV_ = *N;
  LAPACK_CALL(get_output_param_array_int)(&lc, "IPIV", &IPIV, &NIPIV_);

  ASSERT(NIPIV_ == *N);

  LAPACK_CALL(get_output_conf_int)(&lc, "INFO", INFO);

  LAPACK_TIMING_REMOTE_GET(&lc, &liblapack_scdc_timing[1]);

do_quit:
  LAPACK_CALL(destroy_scdc)(&lc);

  liblapack_scdc_release();

#else /* LIBLAPACK_SCDC_ENABLED */

  LAPACK_ORIG_F_INIT(sgesv_);
  LAPACK_ORIG_F(MANGLE_LAPACK(sgesv_))(N, NRHS, A, LDA, IPIV, B, LDB, INFO);

#endif /* LIBLAPACK_SCDC_ENABLED */

  LAPACK_TIMING_STOP(liblapack_scdc_timing[0]);

  LAPACK_TIMING_PRINT_F("%s: %f  %f", __func__, liblapack_scdc_timing[0], liblapack_scdc_timing[1]);

#if LIBLAPACK_SCDC_TRACE_DATA
  TRACE_CMD(
/*    TRACE_F("%s: Y", __func__);
    BLAS_CALL(print_param_matrix_double)(*M, *N, A, *LDA, RCM_TYPE_DENSE|RCM_ORDER_COL_MAJOR);*/
  );
#endif

  TRACE_F("%s: return", __func__);
}

void LIB_F(sgesv)(const int *N, const int *NRHS, float *A, const int *LDA, int *IPIV, float *B, const int *LDB, int *INFO)
{
  LIB_F(sgesv_)(N, NRHS, A, LDA, IPIV, B, LDB, INFO);
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
  LAPACK_CALL(put_input_param_matrix_float)(&lc, "A", A, *N, *N, *LDA, (NETLIB_UPLO_IS_UPPER(*UPLO)?RCM_TYPE_TRIANGULAR_UPPER:RCM_TYPE_TRIANGULAR_LOWER)|RCM_ORDER_COL_MAJOR);

  LAPACK_CALL(put_input_param_vector_float)(&lc, "X", X, *N, *INCX);

  LAPACK_CALL(execute)(&lc);

  rdint_t N_ = *N
  rdint_t INCX_ = *INCX;
  LAPACK_CALL(get_output_param_vector_float)(&lc, "X", &X, N_, &INCX_);

  ASSERT(N_ == *N && INCX_ == *INCX);

  LAPACK_CALL(destroy_scdc)(&lc);

  liblapack_scdc_release();

  TRACE_F("%s: return", __func__);
}

void LIB_F(strsv_)(const char *UPLO, const char *TRANS, const char *DIAG, const int *N, float *A, const int *LDA, float *X, const int *INCX)
{
  LIB_F(strsv)(UPLO, TRANS, DIAG, N, A, LDA, X, INCX);
}

#endif /* LAPACK_STRSV */
