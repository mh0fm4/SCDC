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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "z_pack.h"

#include "blas.h"
#if LIBBLAS_SCDC
# include "libblas_scdc.h"
#endif

#include "config.h"
#include "parnum.h"
#include "test_blas_common.h"


#define REP_BENCH(_f_, _n_, _i_)  Z_MOP(for (j = 0; j < (_n_); ++j) _f_;)

#if LIBBLAS_SCDC
# define LIBBLAS_SCDC_CMD(_cmd_)  Z_MOP(_cmd_)
#else
# define LIBBLAS_SCDC_CMD(_cmd_)  Z_NOP()
#endif


#if LIBBLAS_SCDC

static void blas_bench_vector()
{
  const int N = BLAS_BENCH_VECTOR_N;

  const int INCX[] = { BLAS_BENCH_VECTOR_INC };
  const int nincx = sizeof(INCX) / sizeof(int);

  double *DX = malloc(BLAS_BENCH_VECTOR_ALLOC * sizeof(double));

  int i, j;
  double t[3 * 2];

  printf("#N  INC  dvin  X  dvout  X  dvinout\n");

  for (i = 0; i < nincx; ++i)
  {
    dvector_init_rand(N, DX, INCX[i]);

    MANGLE_BLAS(dvin_)(&N, DX, &INCX[i]);
    t[0] = z_time_wtime();
    REP_BENCH(MANGLE_BLAS(dvin_)(&N, DX, &INCX[i]), BLAS_NREP, j);
    t[0] = (z_time_wtime() - t[0]) / BLAS_NREP;
    LIBBLAS_SCDC_CMD(t[1] = libblas_scdc_timing[1];);

    MANGLE_BLAS(dvout_)(&N, DX, &INCX[i]);
    t[2] = z_time_wtime();
    REP_BENCH(MANGLE_BLAS(dvout_)(&N, DX, &INCX[i]), BLAS_NREP, j);
    t[2] = (z_time_wtime() - t[2]) / BLAS_NREP;
    LIBBLAS_SCDC_CMD(t[3] = libblas_scdc_timing[1];);

    MANGLE_BLAS(dvinout_)(&N, DX, &INCX[i]);
    t[4] = z_time_wtime();
    REP_BENCH(MANGLE_BLAS(dvinout_)(&N, DX, &INCX[i]), BLAS_NREP, j);
    t[4] = (z_time_wtime() - t[4]) / BLAS_NREP;
    LIBBLAS_SCDC_CMD(t[5] = libblas_scdc_timing[1];);

    printf("%d  %d  %f  %f  %f  %f  %f  %f\n", N, INCX[i], t[0], t[1], t[2], t[3], t[4], t[5]);
  }

  free(DX);
}


static void blas_bench_matrix()
{
  const int M = BLAS_BENCH_MATRIX_M;
  const int N = BLAS_BENCH_MATRIX_M;

  const int LDA[] = { BLAS_BENCH_MATRIX_LDA };
  const int nlda = sizeof(LDA) / sizeof(int);

  double *A = malloc(BLAS_BENCH_MATRIX_ALLOC * sizeof(double));

  int i, j;
  double t[3 * 2];

  printf("#M  LDA  dgein  X  dgeout  X  dgeinout\n");

  for (i = 0; i < nlda; ++i)
  {
    dmatrix_cmo_init_rand(M, N, A, LDA[i]);

    MANGLE_BLAS(dgein_)(&M, &N, A, &LDA[i]);
    t[0] = z_time_wtime();
    REP_BENCH(MANGLE_BLAS(dgein_)(&M, &N, A, &LDA[i]), BLAS_NREP, j);
    t[0] = (z_time_wtime() - t[0]) / BLAS_NREP;
    LIBBLAS_SCDC_CMD(t[1] = libblas_scdc_timing[1];);

    MANGLE_BLAS(dgeout_)(&M, &N, A, &LDA[i]);
    t[2] = z_time_wtime();
    REP_BENCH(MANGLE_BLAS(dgeout_)(&M, &N, A, &LDA[i]), BLAS_NREP, j);
    t[2] = (z_time_wtime() - t[2]) / BLAS_NREP;
    LIBBLAS_SCDC_CMD(t[3] = libblas_scdc_timing[1];);

    MANGLE_BLAS(dgeinout_)(&M, &N, A, &LDA[i]);
    t[4] = z_time_wtime();
    REP_BENCH(MANGLE_BLAS(dgeinout_)(&M, &N, A, &LDA[i]), BLAS_NREP, j);
    t[4] = (z_time_wtime() - t[4]) / BLAS_NREP;
    LIBBLAS_SCDC_CMD(t[5] = libblas_scdc_timing[1];);

    printf("%d  %2d  %f  %f  %f  %f  %f  %f\n", M, LDA[i], t[0], t[1], t[2], t[3], t[4], t[5]);
  }

  free(A);
}

#endif


void blas_bench_level1()
{
  const int N[] = { BLAS_LEVEL1_N };
  const int ns = sizeof(N) / sizeof(int);

  const int INCX = 1;
  const int INCY = 1;

  double *DX = malloc(BLAS_LEVEL1_ALLOC * INCX * sizeof(double));
  double *DY = malloc(BLAS_LEVEL1_ALLOC * INCY * sizeof(double));

  const double DA = 0.1;

  int i, j;
  double t[4 * 2];

  printf("#N  idamax  X  dcopy  X  dscal  X  daxpy  X\n");

  for (i = 0; i < ns; ++i)
  {
    dvector_init_rand(N[i], DX, INCX);
    dvector_init_rand(N[i], DY, INCY);

    MANGLE_BLAS(idamax_)(&N[i], DX, &INCX);
    t[0] = z_time_wtime();
    REP_BENCH(MANGLE_BLAS(idamax_)(&N[i], DX, &INCX), BLAS_NREP, j);
    t[0] = (z_time_wtime() - t[0]) / BLAS_NREP;
    LIBBLAS_SCDC_CMD(t[1] = libblas_scdc_timing[1];);

    MANGLE_BLAS(dcopy_)(&N[i], DX, &INCX, DY, &INCY);
    t[2] = z_time_wtime();
    REP_BENCH(MANGLE_BLAS(dcopy_)(&N[i], DX, &INCX, DY, &INCY), BLAS_NREP, j);
    t[2] = (z_time_wtime() - t[2]) / BLAS_NREP;
    LIBBLAS_SCDC_CMD(t[3] = libblas_scdc_timing[1];);

    MANGLE_BLAS(dscal_)(&N[i], &DA, DX, &INCX);
    t[4] = z_time_wtime();
    REP_BENCH(MANGLE_BLAS(dscal_)(&N[i], &DA, DX, &INCX), BLAS_NREP, j);
    t[4] = (z_time_wtime() - t[4]) / BLAS_NREP;
    LIBBLAS_SCDC_CMD(t[5] = libblas_scdc_timing[1];);

    MANGLE_BLAS(daxpy_)(&N[i], &DA, DX, &INCX, DY, &INCY);
    t[6] = z_time_wtime();
    REP_BENCH(MANGLE_BLAS(daxpy_)(&N[i], &DA, DX, &INCX, DY, &INCY), BLAS_NREP, j);
    t[6] = (z_time_wtime() - t[6]) / BLAS_NREP;
    LIBBLAS_SCDC_CMD(t[7] = libblas_scdc_timing[1];);

    printf("%9d  %f  %f  %f  %f  %f  %f  %f  %f\n", N[i], t[0], t[1], t[2], t[3], t[4], t[5], t[6], t[7]);
  }

  free(DX);
  free(DY);
}


void blas_bench_level2()
{
  char UPLO = 'L';
  char TRANS = 'N';
  char DIAG = 'N';

  const int N[] = { BLAS_LEVEL2_N };
  const int ns = sizeof(N) / sizeof(int);

  const int INCX = 1;
  const int INCY = 1;

  double *X = malloc(BLAS_LEVEL2_ALLOC * INCX * sizeof(double));
  double *Y = malloc(BLAS_LEVEL2_ALLOC * INCY * sizeof(double));
  double *A = malloc(BLAS_LEVEL2_ALLOC * BLAS_LEVEL2_ALLOC * sizeof(double));

  double ALPHA = 1.0;
  double BETA = 1.0;

  int i, j;
  double t[3 * 2];

  printf("#N  dger  X  dgemv  X  dscal  X  dtrsv  X\n");

  for (i = 0; i < ns; ++i)
  {
    int LDA = N[i];

    dvector_init_rand(N[i], X, INCX);
    dvector_init_rand(N[i], Y, INCY);
    dmatrix_cmo_init_rand(N[i], N[i], A, LDA);

    MANGLE_BLAS(dger_)(&N[i], &N[i], &ALPHA, X, &INCX, Y, &INCY, A, &LDA);
    t[0] = z_time_wtime();
    REP_BENCH(MANGLE_BLAS(dger_)(&N[i], &N[i], &ALPHA, X, &INCX, Y, &INCY, A, &LDA), BLAS_NREP, j);
    t[0] = (z_time_wtime() - t[0]) / BLAS_NREP;
    LIBBLAS_SCDC_CMD(t[1] = libblas_scdc_timing[1];);

    MANGLE_BLAS(dgemv_)(&TRANS, &N[i], &N[i], &ALPHA, A, &LDA, X, &INCX, &BETA, Y, &INCY);
    t[2] = z_time_wtime();
    REP_BENCH(MANGLE_BLAS(dgemv_)(&TRANS, &N[i], &N[i], &ALPHA, A, &LDA, X, &INCX, &BETA, Y, &INCY), BLAS_NREP, j);
    t[2] = (z_time_wtime() - t[2]) / BLAS_NREP;
    LIBBLAS_SCDC_CMD(t[3] = libblas_scdc_timing[1];);

    dmatvec_lgs_init(UPLO, TRANS, DIAG, N[i], A, LDA, X, INCX);

    MANGLE_BLAS(dtrsv_)(&UPLO, &TRANS, &DIAG, &N[i], A, &LDA, X, &INCX);
    t[4] = z_time_wtime();
    REP_BENCH(MANGLE_BLAS(dtrsv_)(&UPLO, &TRANS, &DIAG, &N[i], A, &LDA, X, &INCX), BLAS_NREP, j);
    t[4] = (z_time_wtime() - t[4]) / BLAS_NREP;
    LIBBLAS_SCDC_CMD(t[5] = libblas_scdc_timing[1];);

    printf("%4d  %f  %f  %f  %f  %f  %f\n", N[i], t[0], t[1], t[2], t[3], t[4], t[5]);
  }

  free(X);
  free(Y);
  free(A);
}


void blas_bench_level3()
{
  char SIDE = 'L'; /* L, R */
  char UPLO = 'L'; /* U, L */
  char TRANSA = 'N'; /* N, T */
  char TRANSB = 'N'; /* N, T */
  char DIAG = 'N'; /* U, N */

  const int N[] = { BLAS_LEVEL3_N };
  const int ns = sizeof(N) / sizeof(int);

  double *A = malloc(BLAS_LEVEL3_ALLOC * BLAS_LEVEL3_ALLOC * sizeof(double));
  double *B = malloc(BLAS_LEVEL3_ALLOC * BLAS_LEVEL3_ALLOC * sizeof(double));
  double *C = malloc(BLAS_LEVEL3_ALLOC * BLAS_LEVEL3_ALLOC * sizeof(double));

  double ALPHA = 1.0;
  double BETA = 1.0;

  int i, j;
  double t[2 * 2];

  printf("#N  dgemm  X  dtrsm  X\n");

  for (i = 0; i < ns; ++i)
  {
    int LDA = N[i];
    int LDB = N[i];
    int LDC = N[i];

    dmatrix_cmo_init(N[i], N[i], A, LDA);
    dmatrix_cmo_init(N[i], N[i], B, LDB);
    dmatrix_cmo_init(N[i], N[i], C, LDC);

    MANGLE_BLAS(dgemm_)(&TRANSA, &TRANSB, &N[i], &N[i], &N[i], &ALPHA, A, &LDA, B, &LDB, &BETA, C, &LDC);
    t[0] = z_time_wtime();
    REP_BENCH(MANGLE_BLAS(dgemm_)(&TRANSA, &TRANSB, &N[i], &N[i], &N[i], &ALPHA, A, &LDA, B, &LDB, &BETA, C, &LDC), BLAS_NREP, j);
    t[0] = (z_time_wtime() - t[0]) / BLAS_NREP;
    LIBBLAS_SCDC_CMD(t[1] = libblas_scdc_timing[1];);

    dmatmat_lgs_init(SIDE, UPLO, TRANSA, DIAG, N[i], N[i], A, LDA, B, LDB);

    MANGLE_BLAS(dtrsm_)(&SIDE, &UPLO, &TRANSA, &DIAG, &N[i], &N[i], &ALPHA, A, &LDA, B, &LDB);
    t[2] = z_time_wtime();
    REP_BENCH(MANGLE_BLAS(dtrsm_)(&SIDE, &UPLO, &TRANSA, &DIAG, &N[i], &N[i], &ALPHA, A, &LDA, B, &LDB), BLAS_NREP, j);
    t[2] = (z_time_wtime() - t[2]) / BLAS_NREP;
    LIBBLAS_SCDC_CMD(t[3] = libblas_scdc_timing[1];);

    printf("%4d  %f  %f  %f  %f\n", N[i], t[0], t[1], t[2], t[3]);
  }

  free(A);
  free(B);
  free(C);
}


void parnum_blas(int argc, char *argv[])
{
  --argc; ++argv;

  if (argc < 1) return;

#if LIBBLAS_SCDC
  libblas_scdc_init();

#if 1
  libblas_scdc_uri = "scdc+uds://libblas_scdc/blas";
#endif
#endif

#if LIBBLAS_SCDC
  if (strcmp(argv[0], "bvec") == 0)
  {
    blas_bench_vector();

  } else if (strcmp(argv[0], "bmat") == 0)
  {
    blas_bench_matrix();

  } else
#endif
  if (strcmp(argv[0], "level1") == 0)
  {
    blas_bench_level1();

  } else if (strcmp(argv[0], "level2") == 0)
  {
    blas_bench_level2();

  } else if (strcmp(argv[0], "level3") == 0)
  {
    blas_bench_level3();
  }

#if LIBBLAS_SCDC
  libblas_scdc_release();
#endif
}
