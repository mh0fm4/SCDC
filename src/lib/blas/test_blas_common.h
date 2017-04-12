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


#ifndef __TEST_BLAS_COMMON_H__
#define __TEST_BLAS_COMMON_H__


void dvector_init(const int n, double *x, const int incx);
void dvector_print(const int n, double *x, const int incx);
void dmatrix_cmo_init(const int nrows, const int ncols, double *a, const int lda);
void dmatrix_cmo_print(const int nrows, const int ncols, double *a, const int lda);
void dmatvec_lgs_init(const char UPLO, const char TRANS, const char DIAG, const int n, double *a, const int lda, double *x, const int incx);
void dmatvec_lgs_print(const char UPLO, const char TRANS, const char DIAG, const int n, double *a, const int lda, double *x, const int incx);
void dmatmat_lgs_init(const char SIDE, const char UPLO, const char TRANS, const char DIAG, const int m, const int n, double *a, const int lda, double *b, const int ldb);
void dmatmat_lgs_print(const char SIDE, const char UPLO, const char TRANS, const char DIAG, const int m, const int n, double *a, const int lda, double *b, const int ldb);


#endif /* __TEST_BLAS_COMMON_H__ */
