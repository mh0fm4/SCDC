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
#include <math.h>

#include "common.h"
#include "z_pack.h"
#include "netlib_common.h"
#include "test_blas_common.h"

#define RANDOM  0


template<typename T>
void xvector_init_nan(const int n, T *x, const int incx)
{
  int i, j;

  for (i = 0; i < n; ++i)
  {
    for (j = 0; j < incx; ++j)
      x[i * incx + j] = NAN;
  }
}


template<typename T>
void xvector_init_one(const int n, T *x, const int incx)
{
  int i;

  xvector_init_nan<T>(n, x, incx);

  for (i = 0; i < n; ++i)
  {
    x[i * incx + 0] = 1.0;
  }
}


template<typename T>
void xvector_init_rand(const int n, T *x, const int incx)
{
  int i;

  xvector_init_nan<T>(n, x, incx);

  for (i = 0; i < n; ++i)
  {
    x[i * incx + 0] = (T) random() / (T) RAND_MAX;
  }
}


template<typename T>
void xvector_init(const int n, T *x, const int incx)
{
#if RANDOM
  xvector_init_rand<T>(n, x, incx);
#else
  xvector_init_one<T>(n, x, incx);
#endif
}


template<typename T>
void xvector_print(const int n, T *x, const int incx)
{
  int i, j;

  for (i = 0; i < n; ++i)
  {
    printf("  %8f", static_cast<T>(x[i * incx]));

    for (j = 1; j < incx; ++j)
      printf("  %8c", 'X');

    printf("\n");
  }
}


#define DEFINE_FLOAT_VECTOR(_t_, _tn_) \
  void _tn_ ## vector_init_one(const int n, _t_ *x, const int incx) { \
    xvector_init_one<_t_>(n, x, incx); \
  } \
  void _tn_ ## vector_init_rand(const int n, _t_ *x, const int incx) { \
    xvector_init_rand<_t_>(n, x, incx); \
  } \
  void _tn_ ## vector_init(const int n, _t_ *x, const int incx) { \
    xvector_init<_t_>(n, x, incx); \
  } \
  void _tn_ ## vector_print(const int n, _t_ *x, const int incx) { \
    xvector_print<_t_>(n, x, incx); \
  }


template<typename T>
void xmatrix_cmo_init_nan(const int nrows, const int ncols, T *a, const int lda)
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


template<typename T>
void xmatrix_cmo_init_one(const int nrows, const int ncols, T *a, const int lda)
{
  int i, j;

  xmatrix_cmo_init_nan<T>(nrows, ncols, a, lda);

  for (i = 0; i < lda; ++i)
  {
    for (j = 0; j < ncols; ++j)
    {
      if (i < nrows)
      {
        a[j * lda + i] = 1.0;
      }
    }
  }
}


template<typename T>
void xmatrix_cmo_init_rand(const int nrows, const int ncols, T *a, const int lda)
{
  int i, j;

  xmatrix_cmo_init_nan<T>(nrows, ncols, a, lda);

  for (i = 0; i < lda; ++i)
  {
    for (j = 0; j < ncols; ++j)
    {
      if (i < nrows)
      {
        a[j * lda + i] = (T) random() / (T) RAND_MAX;
      }
    }
  }
}


template<typename T>
void xmatrix_cmo_init(const int nrows, const int ncols, T *a, const int lda)
{
#if RANDOM
  xmatrix_cmo_init_rand<T>(nrows, ncols, a, lda);
#else
  xmatrix_cmo_init_one<T>(nrows, ncols, a, lda);
#endif
}


template<typename T>
void xmatrix_cmo_print(const int nrows, const int ncols, T *a, const int lda)
{
  int i, j;

  for (i = 0; i < nrows; ++i)
  {
    for (j = 0; j < ncols; ++j)
      printf("  %8f", static_cast<T>(a[j * lda + i]));

    printf("\n");
  }

  for (; i < lda; ++i)
  {
    for (j = 0; j < ncols; ++j)
      printf("  %8c", 'X');

    printf("\n");
  }
}

#define DEFINE_FLOAT_MATRIX(_t_, _tn_) \
  void _tn_ ## matrix_cmo_init_one(const int nrows, const int ncols, _t_ *a, const int lda) { \
    xmatrix_cmo_init_one<_t_>(nrows, ncols, a, lda); \
  } \
  void _tn_ ## matrix_cmo_init_rand(const int nrows, const int ncols, _t_ *a, const int lda) { \
    xmatrix_cmo_init_rand<_t_>(nrows, ncols, a, lda); \
  } \
  void _tn_ ## matrix_cmo_init(const int nrows, const int ncols, _t_ *a, const int lda) { \
    xmatrix_cmo_init<_t_>(nrows, ncols, a, lda); \
  } \
  void _tn_ ## matrix_cmo_print(const int nrows, const int ncols, _t_ *a, const int lda) { \
    xmatrix_cmo_print<_t_>(nrows, ncols, a, lda); \
  }


template<typename T>
void xmatvec_lgs_init_one(const char UPLO, const char TRANS, const char DIAG, const int n, T *a, const int lda, T *x, const int incx)
{
  int i, j;

  xmatrix_cmo_init_nan<T>(n, n, a, lda);

  for (i = 0; i < n; ++i)
  {
    for (j = 0; j < n; ++j)
    {
      if ((NETLIB_UPLO_IS_UPPER(UPLO) && j >= i) || (NETLIB_UPLO_IS_LOWER(UPLO) && j <= i) || (!NETLIB_UPLO_IS_UPPER(UPLO) && !NETLIB_UPLO_IS_LOWER(UPLO)))
        a[j * lda + i] = 1.0;
      else a[j * lda + i] = 0.0;
    }

    if (NETLIB_DIAG_IS_UNIT(DIAG)) a[i * lda + i] = 1.0;
  }

  xvector_init_nan<T>(n, x, incx);

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

  for (i = 0; i < n; ++i)
  {
    x[i * incx + 0] = i * i_f + i_a;
  }
}


template<typename T>
void xmatvec_lgs_init_rand(const char UPLO, const char TRANS, const char DIAG, const int n, T *a, const int lda, T *x, const int incx)
{
  int i, j;

  xmatrix_cmo_init_nan<T>(n, n, a, lda);

  for (i = 0; i < n; ++i)
  {
    for (j = 0; j < n; ++j)
    {
      if ((NETLIB_UPLO_IS_UPPER(UPLO) && j >= i) || (NETLIB_UPLO_IS_LOWER(UPLO) && j <= i) || (!NETLIB_UPLO_IS_UPPER(UPLO) && !NETLIB_UPLO_IS_LOWER(UPLO)))
        a[j * lda + i] = (T) random() / (T) RAND_MAX;
      else a[j * lda + i] = 0.0;
    }

    if (NETLIB_DIAG_IS_UNIT(DIAG)) a[i * lda + i] = 1.0;
  }

  xvector_init_nan<T>(n, x, incx);

  for (i = 0; i < n; ++i)
  {
    x[i * incx + 0] = (T) random() / (T) RAND_MAX;
  }
}


template<typename T>
void xmatvec_lgs_init(const char UPLO, const char TRANS, const char DIAG, const int n, T *a, const int lda, T *x, const int incx)
{
#if RANDOM
  xmatvec_lgs_init_rand<T>(UPLO, TRANS, DIAG, n, a, lda, x, incx);
#else
  xmatvec_lgs_init_one<T>(UPLO, TRANS, DIAG, n, a, lda, x, incx);
#endif
}


template<typename T>
void xmatvec_lgs_print(const char UPLO, const char TRANS, const char DIAG, const int n, T *a, const int lda, T *x, const int incx)
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

#define DEFINE_FLOAT_MATVEC(_t_, _tn_) \
  void _tn_ ## matvec_lgs_init_one(const char UPLO, const char TRANS, const char DIAG, const int n, _t_ *a, const int lda, _t_ *x, const int incx) { \
    xmatvec_lgs_init_one<_t_>(UPLO, TRANS, DIAG, n, a, lda, x, incx); \
  } \
  void _tn_ ## matvec_lgs_init_rand(const char UPLO, const char TRANS, const char DIAG, const int n, _t_ *a, const int lda, _t_ *x, const int incx) { \
    xmatvec_lgs_init_rand<_t_>(UPLO, TRANS, DIAG, n, a, lda, x, incx); \
  } \
  void _tn_ ## matvec_lgs_init(const char UPLO, const char TRANS, const char DIAG, const int n, _t_ *a, const int lda, _t_ *x, const int incx) { \
    xmatvec_lgs_init<_t_>(UPLO, TRANS, DIAG, n, a, lda, x, incx); \
  } \
  void _tn_ ## matvec_lgs_print(const char UPLO, const char TRANS, const char DIAG, const int n, _t_ *a, const int lda, _t_ *x, const int incx) { \
    xmatvec_lgs_print<_t_>(UPLO, TRANS, DIAG, n, a, lda, x, incx); \
  }


template<typename T>
void xmatmat_lgs_init_one(const char SIDE, const char UPLO, const char TRANS, const char DIAG, const int m, const int n, T *a, const int lda, T *b, const int ldb)
{
  int i, j;

  const int k = NETLIB_SIDE_IS_LEFT(SIDE)?m:n;

  xmatrix_cmo_init_nan<T>(k, k, a, lda);

  for (i = 0; i < k; ++i)
  {
    for (j = 0; j < k; ++j)
    {
      if ((NETLIB_UPLO_IS_UPPER(UPLO) && j >= i) || (NETLIB_UPLO_IS_LOWER(UPLO) && j <= i) || (!NETLIB_UPLO_IS_UPPER(UPLO) && !NETLIB_UPLO_IS_LOWER(UPLO)))
        a[j * lda + i] = 1.0;
      else a[j * lda + i] = 0.0;
    }

    if (NETLIB_DIAG_IS_UNIT(DIAG)) a[i * lda + i] = 1.0;
  }

  xmatrix_cmo_init_nan<T>(m, n, b, ldb);

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

  for (i = 0; i < m; ++i)
  {
    for (j = 0; j < n; ++j)
    {
      b[j * ldb + i] = (i * i_f + i_a) * pow(10, -j);
    }
  }
}


template<typename T>
void xmatmat_lgs_init_rand(const char SIDE, const char UPLO, const char TRANS, const char DIAG, const int m, const int n, T *a, const int lda, T *b, const int ldb)
{
  int i, j;

  const int k = NETLIB_SIDE_IS_LEFT(SIDE)?m:n;

  xmatrix_cmo_init_nan<T>(k, k, a, lda);

  for (i = 0; i < k; ++i)
  {
    for (j = 0; j < k; ++j)
    {
      if ((NETLIB_UPLO_IS_UPPER(UPLO) && j >= i) || (NETLIB_UPLO_IS_LOWER(UPLO) && j <= i) || (!NETLIB_UPLO_IS_UPPER(UPLO) && !NETLIB_UPLO_IS_LOWER(UPLO)))
        a[j * lda + i] = (T) random() / (T) RAND_MAX;
      else a[j * lda + i] = 0.0;
    }

    if (NETLIB_DIAG_IS_UNIT(DIAG)) a[i * lda + i] = 1.0;
  }

  xmatrix_cmo_init_nan<T>(m, n, b, ldb);

  for (i = 0; i < m; ++i)
  {
    for (j = 0; j < n; ++j)
    {
      b[j * ldb + i] = (T) random() / (T) RAND_MAX;
    }
  }
}


template<typename T>
void xmatmat_lgs_init(const char SIDE, const char UPLO, const char TRANS, const char DIAG, const int m, const int n, T *a, const int lda, T *b, const int ldb)
{
#if RANDOM
  xmatmat_lgs_init_rand<T>(SIDE, UPLO, TRANS, DIAG, m, n, a, lda, b, ldb);
#else
  xmatmat_lgs_init_one<T>(SIDE, UPLO, TRANS, DIAG, m, n, a, lda, b, ldb);
#endif
}


template<typename T>
void xmatmat_lgs_print(const char SIDE, const char UPLO, const char TRANS, const char DIAG, const int m, const int n, T *a, const int lda, T *b, const int ldb)
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

#define DEFINE_FLOAT_MATMAT(_t_, _tn_) \
  void _tn_ ## matmat_lgs_init_one(const char SIDE, const char UPLO, const char TRANS, const char DIAG, const int m, const int n, _t_ *a, const int lda, _t_ *b, const int ldb) { \
    xmatmat_lgs_init_one<_t_>(SIDE, UPLO, TRANS, DIAG, m, n, a, lda, b, ldb); \
  } \
  void _tn_ ## matmat_lgs_init_rand(const char SIDE, const char UPLO, const char TRANS, const char DIAG, const int m, const int n, _t_ *a, const int lda, _t_ *b, const int ldb) { \
    xmatmat_lgs_init_rand<_t_>(SIDE, UPLO, TRANS, DIAG, m, n, a, lda, b, ldb); \
  } \
  void _tn_ ## matmat_lgs_init(const char SIDE, const char UPLO, const char TRANS, const char DIAG, const int m, const int n, _t_ *a, const int lda, _t_ *b, const int ldb) { \
    xmatmat_lgs_init<_t_>(SIDE, UPLO, TRANS, DIAG, m, n, a, lda, b, ldb); \
  } \
  void _tn_ ## matmat_lgs_print(const char SIDE, const char UPLO, const char TRANS, const char DIAG, const int m, const int n, _t_ *a, const int lda, _t_ *b, const int ldb) { \
    xmatmat_lgs_print<_t_>(SIDE, UPLO, TRANS, DIAG, m, n, a, lda, b, ldb); \
  }


#define DEFINE_FLOAT(_t_, _tn_, _fmt_) \
  DEFINE_FLOAT_VECTOR(_t_, _tn_); \
  DEFINE_FLOAT_MATRIX(_t_, _tn_); \
  DEFINE_FLOAT_MATVEC(_t_, _tn_); \
  DEFINE_FLOAT_MATMAT(_t_, _tn_);

DEFINE_FLOAT(float, f, FMT_FLOAT);
DEFINE_FLOAT(double, d, FMT_DOUBLE);
