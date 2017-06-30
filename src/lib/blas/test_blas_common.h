/*
 *  Copyright (C) 2014, 2015, 2016, 2017 Michael Hofmann
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


#ifndef __TEST_BLAS_COMMON_H__
#define __TEST_BLAS_COMMON_H__


#ifdef __cplusplus
extern "C" {
#endif


#define DECLARE_FLOAT_VECTOR(_t_, _tn_) \
  void _tn_ ## vector_init_one(const int n, _t_ *x, const int incx); \
  void _tn_ ## vector_init_rand(const int n, _t_ *x, const int incx); \
  void _tn_ ## vector_init(const int n, _t_ *x, const int incx); \
  void _tn_ ## vector_print(const int n, _t_ *x, const int incx);

#define DECLARE_FLOAT_MATRIX(_t_, _tn_) \
  void _tn_ ## matrix_cmo_init_one(const int nrows, const int ncols, _t_ *a, const int lda); \
  void _tn_ ## matrix_cmo_init_rand(const int nrows, const int ncols, _t_ *a, const int lda); \
  void _tn_ ## matrix_cmo_init(const int nrows, const int ncols, _t_ *a, const int lda); \
  void _tn_ ## matrix_cmo_print(const int nrows, const int ncols, _t_ *a, const int lda);

#define DECLARE_FLOAT_MATVEC(_t_, _tn_) \
  void _tn_ ## matvec_lgs_init_one(const char UPLO, const char TRANS, const char DIAG, const int n, _t_ *a, const int lda, _t_ *x, const int incx); \
  void _tn_ ## matvec_lgs_init_rand(const char UPLO, const char TRANS, const char DIAG, const int n, _t_ *a, const int lda, _t_ *x, const int incx); \
  void _tn_ ## matvec_lgs_init(const char UPLO, const char TRANS, const char DIAG, const int n, _t_ *a, const int lda, _t_ *x, const int incx); \
  void _tn_ ## matvec_lgs_print(const char UPLO, const char TRANS, const char DIAG, const int n, _t_ *a, const int lda, _t_ *x, const int incx); \

#define DECLARE_FLOAT_MATMAT(_t_, _tn_) \
  void _tn_ ## matmat_lgs_init_one(const char SIDE, const char UPLO, const char TRANS, const char DIAG, const int m, const int n, _t_ *a, const int lda, _t_ *b, const int ldb); \
  void _tn_ ## matmat_lgs_init_rand(const char SIDE, const char UPLO, const char TRANS, const char DIAG, const int m, const int n, _t_ *a, const int lda, _t_ *b, const int ldb); \
  void _tn_ ## matmat_lgs_init(const char SIDE, const char UPLO, const char TRANS, const char DIAG, const int m, const int n, _t_ *a, const int lda, _t_ *b, const int ldb); \
  void _tn_ ## matmat_lgs_print(const char SIDE, const char UPLO, const char TRANS, const char DIAG, const int m, const int n, _t_ *a, const int lda, _t_ *b, const int ldb); \


#define DECLARE_FLOAT(_t_, _tn_, _fmt_) \
  DECLARE_FLOAT_VECTOR(_t_, _tn_); \
  DECLARE_FLOAT_MATRIX(_t_, _tn_); \
  DECLARE_FLOAT_MATVEC(_t_, _tn_); \
  DECLARE_FLOAT_MATMAT(_t_, _tn_); \


DECLARE_FLOAT(float, f, FMT_FLOAT);
DECLARE_FLOAT(double, d, FMT_DOUBLE);


#ifdef __cplusplus
}
#endif


#endif /* __TEST_BLAS_COMMON_H__ */
