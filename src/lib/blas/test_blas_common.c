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
#include <stdlib.h>
#include <math.h>

#include "common.h"
#include "z_pack.h"
#include "netlib_common.h"
#include "test_blas_common.h"

#define RANDOM  0


void dvector_init_nan(const int n, double *x, const int incx)
{
  int i, j;

  for (i = 0; i < n; ++i)
  {
    for (j = 0; j < incx; ++j)
      x[i * incx + j] = NAN;
  }
}


void dmatrix_cmo_init_nan(const int nrows, const int ncols, double *a, const int lda)
{
  int i, j;

  for (i = 0; i < lda; ++i)
  {
    for (j = 0; j < ncols; ++j)
    {
      a[j * lda + i] = NAN;
    }
  }
}


void dvector_init(const int n, double *x, const int incx)
{
  int i;

  dvector_init_nan(n, x, incx);

  for (i = 0; i < n; ++i)
  {
    x[i * incx + 0] =
#if RANDOM
      random() / (double) RAND_MAX;
#else
      1.0;
#endif
  }
}


void dvector_print(const int n, double *x, const int incx)
{
  int i, j;

  for (i = 0; i < n; ++i)
  {
    printf("  %8f", x[i * incx]);

    for (j = 1; j < incx; ++j)
      printf("  %8c", 'X');

    printf("\n");
  }
}


void dmatrix_cmo_init(const int nrows, const int ncols, double *a, const int lda)
{
  int i, j;

  dmatrix_cmo_init_nan(nrows, ncols, a, lda);

  for (i = 0; i < lda; ++i)
  {
    for (j = 0; j < ncols; ++j)
    {
      if (i < nrows)
      {
        a[j * lda + i] =
#if RANDOM
          random() / (double) RAND_MAX;
#else
          1.0;
#endif
      }
    }
  }
}


void dmatrix_cmo_print(const int nrows, const int ncols, double *a, const int lda)
{
  int i, j;

  for (i = 0; i < nrows; ++i)
  {
    for (j = 0; j < ncols; ++j)
      printf("  %8f", a[j * lda + i]);

    printf("\n");
  }

  for (; i < lda; ++i)
  {
    for (j = 0; j < ncols; ++j)
      printf("  %8c", 'X');

    printf("\n");
  }
}


void dmatvec_lgs_init(const char UPLO, const char TRANS, const char DIAG, const int n, double *a, const int lda, double *x, const int incx)
{
  int i, j;

  dmatrix_cmo_init_nan(n, n, a, lda);

  for (i = 0; i < n; ++i)
  {
    for (j = 0; j < n; ++j)
    {
      if ((NETLIB_UPLO_IS_UPPER(UPLO) && j >= i) || (NETLIB_UPLO_IS_LOWER(UPLO) && j <= i) || (!NETLIB_UPLO_IS_UPPER(UPLO) && !NETLIB_UPLO_IS_LOWER(UPLO)))
        a[j * lda + i] =
#if RANDOM
          random() / (double) RAND_MAX;
#else
          1.0;
#endif
      else a[j * lda + i] = 0.0;
    }

    if (NETLIB_DIAG_IS_UNIT(DIAG)) a[i * lda + i] = 1.0;
  }

  dvector_init_nan(n, x, incx);

#if !RANDOM
  int i_f = 0, i_a = n;
  if (NETLIB_UPLO_IS_UPPER(UPLO))
  {
    if (NETLIB_TRANS_IS_TRANS(TRANS)) { i_f = 1; i_a = 1; }
    else  { i_f = -1; i_a = n; }

  } else if (NETLIB_UPLO_IS_LOWER(UPLO))
  {
    if (NETLIB_TRANS_IS_NONTRANS(TRANS)) { i_f = 1; i_a = 1; }
    else  { i_f = -1; i_a = n; }
  }
#endif

  for (i = 0; i < n; ++i)
  {
    x[i * incx + 0] = 
#if RANDOM
      random() / (double) RAND_MAX;
#else
      i * i_f + i_a;
#endif
  }
}


void dmatvec_lgs_print(const char UPLO, const char TRANS, const char DIAG, const int n, double *a, const int lda, double *x, const int incx)
{
  int i, j;

  for (i = 0; i < n; ++i)
  {
    for (j = 0; j < n; ++j)
      printf("  %8f", a[j * lda + i]);

    printf("  |");

    printf("  %8f", x[i * incx]);

    for (j = 1; j < incx; ++j)
      printf("  %8c", 'X');

    printf("\n");
  }

  for (; i < lda; ++i)
  {
    for (j = 0; j < n; ++j)
      printf("  %8c", 'X');

    printf("\n");
  }
}


void dmatmat_lgs_init(const char SIDE, const char UPLO, const char TRANS, const char DIAG, const int m, const int n, double *a, const int lda, double *b, const int ldb)
{
  int i, j;

  const int k = NETLIB_SIDE_IS_LEFT(SIDE)?m:n;

  dmatrix_cmo_init_nan(k, k, a, lda);

  for (i = 0; i < k; ++i)
  {
    for (j = 0; j < k; ++j)
    {
      if ((NETLIB_UPLO_IS_UPPER(UPLO) && j >= i) || (NETLIB_UPLO_IS_LOWER(UPLO) && j <= i) || (!NETLIB_UPLO_IS_UPPER(UPLO) && !NETLIB_UPLO_IS_LOWER(UPLO)))
        a[j * lda + i] =
#if RANDOM
          random() / (double) RAND_MAX;
#else
          1.0;
#endif
      else a[j * lda + i] = 0.0;
    }

    if (NETLIB_DIAG_IS_UNIT(DIAG)) a[i * lda + i] = 1.0;
  }

  dmatrix_cmo_init_nan(m, n, b, ldb);

#if !RANDOM
  int i_f = 0, i_a = m;
  if (NETLIB_UPLO_IS_UPPER(UPLO))
  {
    if (NETLIB_TRANS_IS_TRANS(TRANS)) { i_f = 1; i_a = 1; }
    else  { i_f = -1; i_a = m; }

  } else if (NETLIB_UPLO_IS_LOWER(UPLO))
  {
    if (NETLIB_TRANS_IS_NONTRANS(TRANS)) { i_f = 1; i_a = 1; }
    else  { i_f = -1; i_a = m; }
  }
#endif

  for (i = 0; i < m; ++i)
  {
    for (j = 0; j < n; ++j)
    {
      b[j * ldb + i] =
#if RANDOM
        random() / (double) RAND_MAX;
#else
        (i * i_f + i_a) * pow(10, -j);
#endif
    }
  }
}


void dmatmat_lgs_print(const char SIDE, const char UPLO, const char TRANS, const char DIAG, const int m, const int n, double *a, const int lda, double *b, const int ldb)
{
  int i, j;

  for (i = 0; i < m; ++i)
  {
    if (NETLIB_SIDE_IS_LEFT(SIDE))
    {
      for (j = 0; j < m; ++j)
        printf("  %8f", a[j * lda + i]);

      printf("  |");
    }

    for (j = 0; j < n; ++j)
      printf("  %8f", b[j * ldb + i]);

    if (NETLIB_SIDE_IS_RIGHT(SIDE))
    {
      printf("  |");

      for (j = 0; j < n; ++j)
        printf("  %8f", a[j * lda + i]);
    }

    printf("\n");
  }

  for (; i < z_max(lda, ldb); ++i)
  {
    if (NETLIB_SIDE_IS_LEFT(SIDE))
    {
      for (j = 0; j < m; ++j)
        printf("  %8c", (i < lda)?'X':' ');

      printf("   ");
    }

    for (j = 0; j < n; ++j)
      printf("  %8c", (i < ldb)?'X':' ');

    if (NETLIB_SIDE_IS_RIGHT(SIDE))
    {
      printf("   ");

      for (j = 0; j < n; ++j)
        printf("  %8c", (i < lda)?'X':' ');
    }

    printf("\n");
  }
}
