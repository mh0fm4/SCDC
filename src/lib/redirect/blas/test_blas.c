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


#if HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "common.h"
#include "z_pack.h"
#include "blas.h"
#include "test_blas_common.h"


#if LIBBLAS_SCDC
# include "blas_scdc_config.h"
# if LIBBLAS_SCDC_PREFIX
#  undef __BLAS_H__
#  undef MANGLE_BLAS
#  define MANGLE_BLAS(_f_)  Z_CONCAT(libblas_scdc_, _f_)
#  include "blas.h"
# endif
#endif


#define TEST_BENCH   0
#define TEST_IDAMAX  0
#define TEST_DCOPY   0
#define TEST_DSCAL   0
#define TEST_DAXPY   0
#define TEST_DGER    0
#define TEST_DGEMV   0
#define TEST_DTRSV   0
#define TEST_DTRSM   0

#define TEST_JACOBI  1


#define PRINT_DATA  1

#define GLOBAL_F 1


void test_bench()
{
  const int M = 4 * GLOBAL_F;
  const int N = 3 * GLOBAL_F;


  const int INCX = 2;

  double *DX = malloc(N * INCX * sizeof(double));

  dvector_init(N, DX, INCX);

#if LIBBLAS_SCDC

  MANGLE_BLAS(dvin_)(&N, DX, &INCX);
  MANGLE_BLAS(dvout_)(&N, DX, &INCX);
  MANGLE_BLAS(dvinout_)(&N, DX, &INCX);

#endif

  free(DX);


  const int LDA = M + 1;

  double *A = malloc(LDA * N * sizeof(double));

  dmatrix_cmo_init(M, N, A, LDA);

#if LIBBLAS_SCDC

  MANGLE_BLAS(dgein_)(&M, &N, A, &LDA);
  MANGLE_BLAS(dgeout_)(&M, &N, A, &LDA);
  MANGLE_BLAS(dgeinout_)(&M, &N, A, &LDA);

#endif

  free(A);
}


void test_idamax()
{
  printf("%s:\n", __func__);

  const int N = 3 * GLOBAL_F;

  const int INCX = 2;

  double *DX = malloc(N * INCX * sizeof(double));

  dvector_init(N, DX, INCX);

#if PRINT_DATA
  printf("DX: %d\n", N);
  dvector_print(N, DX, INCX);
#endif

  int r = MANGLE_BLAS(idamax_)(&N, DX, &INCX);

#if PRINT_DATA
  printf("idamax: %d\n", r);
#endif

  free(DX);
}


void test_dcopy()
{
  printf("%s:\n", __func__);

  const int N = 3 * GLOBAL_F;

  const int INCX = 2;
  const int INCY = 3;

  double *DX = malloc(N * INCX * sizeof(double));
  double *DY = malloc(N * INCY * sizeof(double));

  dvector_init(N, DX, INCX);

#if PRINT_DATA
  printf("DX: %d\n", N);
  dvector_print(N, DX, INCX);
#endif

  MANGLE_BLAS(dcopy_)(&N, DX, &INCX, DY, &INCY);

#if PRINT_DATA
  printf("DY: %d\n", N);
  dvector_print(N, DY, INCY);
#endif

  free(DX);
  free(DY);
}


void test_dscal()
{
  printf("%s:\n", __func__);

  const int N = 3 * GLOBAL_F;

  const int INCX = 2;

  double *DX = malloc(N * INCX * sizeof(double));

  dvector_init(N, DX, INCX);

  const double DA = 0.1;

#if PRINT_DATA
  printf("DX: %d\n", N);
  dvector_print(N, DX, INCX);
#endif

  MANGLE_BLAS(dscal_)(&N, &DA, DX, &INCX);

#if PRINT_DATA
  printf("DX: %d\n", N);
  dvector_print(N, DX, INCX);
#endif

  free(DX);
}


void test_daxpy()
{
  printf("%s:\n", __func__);

  const int N = 3 * GLOBAL_F;

  const int INCX = 2;
  const int INCY = 3;

  double *DX = malloc(N * INCX * sizeof(double));
  double *DY = malloc(N * INCY * sizeof(double));

  dvector_init(N, DX, INCX);
  dvector_init(N, DY, INCY);

  const double DA = 1.0;

#if PRINT_DATA
  printf("DX: %d\n", N);
  dvector_print(N, DX, INCX);
  printf("DY: %d\n", N);
  dvector_print(N, DY, INCY);
#endif

  MANGLE_BLAS(daxpy_)(&N, &DA, DX, &INCX, DY, &INCY);

#if PRINT_DATA
  printf("DY: %d\n", N);
  dvector_print(N, DY, INCY);
#endif

  free(DX);
  free(DY);
}


void test_dger()
{
  printf("%s:\n", __func__);

  const int M = 4 * GLOBAL_F;
  const int N = 3 * GLOBAL_F;

  const int INCX = 2;
  const int INCY = 3;
  const int LDA = M + 1;

  double *X = malloc(M * INCX * sizeof(double));
  double *Y = malloc(N * INCY * sizeof(double));
  double *A = malloc(LDA * N * sizeof(double));

  dvector_init(M, X, INCX);
  dvector_init(N, Y, INCY);
  dmatrix_cmo_init(M, N, A, LDA);

  const double ALPHA = 1.0;

#if PRINT_DATA
  printf("X: %d\n", M);
  dvector_print(M, X, INCX);
  printf("Y: %d\n", N);
  dvector_print(N, Y, INCY);
  printf("A: %d x %d\n", M, N);
  dmatrix_cmo_print(M, N, A, LDA);
#endif

  MANGLE_BLAS(dger_)(&M, &N, &ALPHA, X, &INCX, Y, &INCY, A, &LDA);

#if PRINT_DATA
  printf("A: %d x %d\n", M, N);
  dmatrix_cmo_print(M, N, A, LDA);
#endif

  free(X);
  free(Y);
  free(A);
}


void test_dgemv()
{
  printf("%s:\n", __func__);

  char TRANS = 'N'; /* N, T */

  const int M = 4 * GLOBAL_F;
  const int N = 3 * GLOBAL_F;

  const int LDA = M + 1;
  const int INCX = 2;
  const int INCY = 3;

  double *A = malloc(LDA * N * sizeof(double));
  double *X = malloc(N * INCX * sizeof(double));
  double *Y = malloc(M * INCY * sizeof(double));

  dmatrix_cmo_init(M, N, A, LDA);
  dvector_init(N, X, INCX);
  dvector_init(M, Y, INCY);

  double ALPHA = 1.0;
  double BETA = 1.0;

#if PRINT_DATA
  printf("A: %d x %d\n", M, N);
  dmatrix_cmo_print(M, N, A, LDA);
  printf("X: %d\n", N);
  dvector_print(N, X, INCX);
  printf("Y: %d\n", M);
  dvector_print(M, Y, INCY);
#endif

  MANGLE_BLAS(dgemv_)(&TRANS, &M, &N, &ALPHA, A, &LDA, X, &INCX, &BETA, Y, &INCY);

#if PRINT_DATA
  printf("Y: %d\n", M);
  dvector_print(M, Y, INCY);
#endif

  free(A);
  free(X);
  free(Y);
}


void test_dtrsv()
{
  printf("%s:\n", __func__);

  char UPLO = 'L'; /* U, L */
  char TRANS = 'N'; /* N, T */
  char DIAG = 'N'; /* U, N */

  const int N = 4 * GLOBAL_F;

  const int LDA = N + 1;
  const int INCX = 2;

  double *A = malloc(LDA * N * sizeof(double));
  double *X = malloc(N * INCX * sizeof(double));

  dmatvec_lgs_init(UPLO, TRANS, DIAG, N, A, LDA, X, INCX);

#if PRINT_DATA
  printf("A: %d x %d%s, X: %d\n", N, N, (NETLIB_TRANS_IS_TRANS(TRANS)?" ^T":""), N);
  dmatvec_lgs_print(UPLO, TRANS, DIAG, N, A, LDA, X, INCX);
#endif

  MANGLE_BLAS(dtrsv_)(&UPLO, &TRANS, &DIAG, &N, A, &LDA, X, &INCX);

#if PRINT_DATA
  printf("X: %d\n", N);
  dvector_print(N, X, INCX);
#endif

  free(A);
  free(X);
}


void test_dgemm()
{
  printf("%s:\n", __func__);

  char TRANSA = 'N'; /* N, T */
  char TRANSB = 'N'; /* N, T */

  const int M = 4 * GLOBAL_F;
  const int N = 3 * GLOBAL_F;
  const int K = 2 * GLOBAL_F;

  const int LDA = NETLIB_TRANS_GET_NROWS(TRANSA, M, K) + 1;
  const int LDB = NETLIB_TRANS_GET_NROWS(TRANSB, K, N) + 1;
  const int LDC = M + 1;

  double *A = malloc(LDA * NETLIB_TRANS_GET_NCOLS(TRANSA, M, K) * sizeof(double));
  double *B = malloc(LDB * NETLIB_TRANS_GET_NCOLS(TRANSA, K, N) * sizeof(double));
  double *C = malloc(LDC * N * sizeof(double));

  dmatrix_cmo_init(NETLIB_TRANS_GET_NROWS(TRANSA, M, K), NETLIB_TRANS_GET_NCOLS(TRANSA, M, K), A, LDA);
  dmatrix_cmo_init(NETLIB_TRANS_GET_NROWS(TRANSB, K, N), NETLIB_TRANS_GET_NCOLS(TRANSB, K, N), B, LDB);
  dmatrix_cmo_init(M, N, C, LDC);

  double ALPHA = 1.0;
  double BETA = 1.0;

#if PRINT_DATA
  printf("A: %d x %d\n", NETLIB_TRANS_GET_NROWS(TRANSA, M, K), NETLIB_TRANS_GET_NCOLS(TRANSA, M, K));
  dmatrix_cmo_print(NETLIB_TRANS_GET_NROWS(TRANSA, M, K), NETLIB_TRANS_GET_NCOLS(TRANSA, M, K), A, LDA);
  printf("B: %d x %d\n", NETLIB_TRANS_GET_NROWS(TRANSB, K, N), NETLIB_TRANS_GET_NCOLS(TRANSB, K, N));
  dmatrix_cmo_print(NETLIB_TRANS_GET_NROWS(TRANSB, K, N), NETLIB_TRANS_GET_NCOLS(TRANSB, K, N), B, LDB);
  printf("C: %d x %d\n", M, N);
  dmatrix_cmo_print(M, N, C, LDC);
#endif

  MANGLE_BLAS(dgemm_)(&TRANSA, &TRANSB, &M, &N, &K, &ALPHA, A, &LDA, B, &LDB, &BETA, C, &LDC);

#if PRINT_DATA
  printf("C: %d x %d\n", M, N);
  dmatrix_cmo_print(M, N, C, LDC);
#endif

  free(A);
  free(B);
  free(C);
}


void test_dtrsm()
{
  printf("%s:\n", __func__);

  char SIDE = 'L'; /* L, R */
  char UPLO = 'L'; /* U, L */
  char TRANSA = 'N'; /* N, T */
  char DIAG = 'N'; /* U, N */

  const int M = 4 * GLOBAL_F;
  const int N = 3 * GLOBAL_F;
  const int k = NETLIB_SIDE_IS_LEFT(SIDE)?M:N;

  const int LDA = k + 1;
  const int LDB = M + 2;

  double *A = malloc(LDA * k * sizeof(double));
  double *B = malloc(LDB * N * sizeof(double));

  dmatmat_lgs_init(SIDE, UPLO, TRANSA, DIAG, M, N, A, LDA, B, LDB);

  double ALPHA = 1.0;

#if PRINT_DATA
  printf("A: %d x %d%s %c, X: %d x %d\n", k, k, (NETLIB_TRANS_IS_TRANS(TRANSA)?" ^T":""), SIDE, M, N);
  dmatmat_lgs_print(SIDE, UPLO, TRANSA, DIAG, M, N, A, LDA, B, LDB);
#endif

  MANGLE_BLAS(dtrsm_)(&SIDE, &UPLO, &TRANSA, &DIAG, &M, &N, &ALPHA, A, &LDA, B, &LDB);

#if PRINT_DATA
  printf("B: %d x %d\n", M, N);
  dmatrix_cmo_print(M, N, B, LDB);
#endif

  free(A);
  free(B);
}


void test_jacobi()
{
  printf("%s:\n", __func__);

  char UPLO = 'X'; /* U, L */
  char TRANS = 'N'; /* N, T */
  char DIAG = 'N'; /* U, N */

  // const int N = 4 * 1000;
  const int N = 10;
  const int LD = N;
  const int INC = 1;

  double *A = malloc(N * N * sizeof(double));
  double *B = malloc(N * sizeof(double));

  double *LR = malloc(N * N * sizeof(double));
  double *D = malloc(N * sizeof(double));
  double *b = malloc(N * sizeof(double));
  double *x0 = malloc(2 * N * sizeof(double));
  double *x1 = x0 + N;

  double *x_old, *x_new, *x_tmp;

  dmatvec_lgs_init_one(UPLO, TRANS, DIAG, N, A, LD, B, INC);

  int i, j;
  for (i = 0; i < N; ++i)
  {
    D[i] = 0;
    for (j = 0; j < N; ++j)
    {
      if (i == j) continue;

      A[i * LD + i] += A[j * LD + i];
      B[i] += A[j * LD + i];
    }
    D[i] = A[i * LD + i];
  }

  // dmatvec_lgs_print(UPLO, TRANS, DIAG, N, A, LD, B, INC);

  for (i = 0; i < N; ++i)
  {
    for (j = 0; j < N; ++j)
    {
      LR[j * LD + i] = A[j * LD + i] / D[i];
    }
    LR[i * LD + i] = 0;
    b[i] = B[i] / D[i];
  }

  dvector_init_rand(N, x0, INC);

  x_old = x0;
  x_new = x1;

  const int rounds = 10;

  int r;
  for (r = 0; r < rounds; ++r)
  {
    const double ALPHA = -1.0;
    const double BETA = 1.0;

    // x' = D^-1 * (B - LR * x) = (D^1 * B) - (D^1 * LR) * x

    for (i = 0; i < N; ++i) x_new[i] = b[i];

    double t = z_time_wtime();
    MANGLE_BLAS(dgemv_)(&TRANS, &N, &N, &ALPHA, LR, &LD, x_old, &INC, &BETA, x_new, &INC);
    t = z_time_wtime() - t;

    double e = 0.0;
    for (i = 0; i < N; ++i)
    {
      double v = B[i];
      for (j = 0; j < N; ++j)
      {
        v -= A[j * LD + i] * x_new[i];
      }
      e += fabs(v);
    }

    x_tmp = x_old;
    x_old = x_new;
    x_new = x_tmp;

    printf("round %d: error: %e, time: %f\n", r, e, t);
    // dvector_print(N, x_old, INC);
  }

  free(A);
  free(B);
  free(LR);
  free(D);
  free(b);
  free(x0);
}


int main(int argc, char *argv[])
{
#if RANDOM
  srandom(RANDOM);
#endif

#if TEST_BENCH
  test_bench();
#endif

#if TEST_IDAMAX
  test_idamax();
#endif

#if TEST_DCOPY
  test_dcopy();
#endif

#if TEST_DSCAL
  test_dscal();
#endif

#if TEST_DAXPY
  test_daxpy();
#endif

#if TEST_DGER
  test_dger();
#endif

#if TEST_DGEMV
  test_dgemv();
#endif

#if TEST_DTRSV
  test_dtrsv();
#endif

#if TEST_DTRSM
  test_dtrsm();
#endif

#if TEST_JACOBI
  test_jacobi();
#endif

  return 0;
}
