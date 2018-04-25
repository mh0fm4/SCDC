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


#ifndef __BLAS_H__
#define __BLAS_H__


#include "netlib_common.h"
#include "blas_config.h"


#ifndef MANGLE_BLAS
# define MANGLE_BLAS(_f_)  _f_
#endif


#define DECLARE_PROTOTYPE(_r_, _f_, _p_...) \
  typedef _r_ MANGLE_BLAS(_f_ ## _f)(_p_); \
  MANGLE_BLAS(_f_ ## _f) MANGLE_BLAS(_f_), MANGLE_BLAS(_f_ ## _)


/* BLAS bench */
DECLARE_PROTOTYPE(void, dvin, const int *N, double *DX, const int *INCX);
DECLARE_PROTOTYPE(void, dvout, const int *N, double *DX, const int *INCX);
DECLARE_PROTOTYPE(void, dvinout, const int *N, double *DX, const int *INCX);
DECLARE_PROTOTYPE(void, dgein, const int *M, const int *N, double *A, const int *LDA);
DECLARE_PROTOTYPE(void, dgeout, const int *M, const int *N, double *A, const int *LDA);
DECLARE_PROTOTYPE(void, dgeinout, const int *M, const int *N, double *A, const int *LDA);

/* BLAS level 1 */
DECLARE_PROTOTYPE(int, idamax, const int *N, double *DX, const int *INCX);
DECLARE_PROTOTYPE(void, dcopy, const int *N, double *DX, const int *INCX, double *DY, const int *INCY);
DECLARE_PROTOTYPE(void, dscal, const int *N, const double *DA, double *DX, const int *INCX );
DECLARE_PROTOTYPE(void, daxpy, const int *N, const double *DA, double *DX, const int *INCX, double *DY, const int *INCY);

/* BLAS level 2 */
DECLARE_PROTOTYPE(void, dger, const int *M, const int *N, const double *ALPHA, double *X, const int *INCX, double *Y, const int *INCY, double *A, const int *LDA);
DECLARE_PROTOTYPE(void, dgemv, const char *TRANS, const int *M, const int *N, const double *ALPHA, double *A, const int *LDA, double *X, const int *INCX, const double *BETA, double *Y, const int *INCY);
DECLARE_PROTOTYPE(void, dtrsv, const char *UPLO, const char *TRANS, const char *DIAG, const int *N, double *A, const int *LDA, double *X, const int *INCX);

/* BLAS level 3 */
DECLARE_PROTOTYPE(void, dgemm, const char *TRANSA, const char *TRANSB, const int *M, const int *N, const int *K, const double *ALPHA, double *A, const int *LDA, double *B, const int *LDB, const double *BETA, double *C, const int *LDC);
DECLARE_PROTOTYPE(void, dtrsm, const char *SIDE, const char *UPLO, const char *TRANSA, const char *DIAG, const int *M, const int *N, double *ALPHA, double *A, const int *LDA, double *B, const int *LDB);


#undef DECLARE_PROTOTYPE


#endif /* __BLAS_H__ */
