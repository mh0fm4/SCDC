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


#ifndef __LAPACK_H__
#define __LAPACK_H__


#include "lapack_config.h"


#define LAPACK_TRANSPOSED_NROWS(_t_, _m_, _n_)  (((_t_) == 'N' || (_t_) == 'n')?(_m_):(_n_))
#define LAPACK_TRANSPOSED_NCOLS(_t_, _m_, _n_)  (((_t_) == 'N' || (_t_) == 'n')?(_n_):(_m_))

#define LAPACK_IS_UPPER(_uplo_)  ((_uplo_) == 'U' || (_uplo_) == 'u')
#define LAPACK_IS_LOWER(_uplo_)  ((_uplo_) == 'L' || (_uplo_) == 'l')

#define LAPACK_IS_DIAG(_diag_)      ((_diag_) == 'U' || (_diag_) == 'u')
#define LAPACK_IS_NOT_DIAG(_diag_)  ((_diag_) == 'N' || (_diag_) == 'n')


#ifndef MANGLE_LAPACK
# define MANGLE_LAPACK(_f_)  _f_
#endif


#define DECLARE_PROTOTYPE(_r_, _f_, _p_...) \
  typedef _r_ MANGLE_LAPACK(_f_ ## _f)(_p_); \
  MANGLE_LAPACK(_f_ ## _f) MANGLE_LAPACK(_f_), MANGLE_LAPACK(_f_ ## _)


DECLARE_PROTOTYPE(void, sgesv, const int *N, const int *NRHS, float *A, const int *LDA, int *IPIV, float *B, const int *LDB, int *INFO);

DECLARE_PROTOTYPE(void, strsv, const char *UPLO, const char *TRANS, const char *DIAG, const int *N, float *A, const int *LDA, float *X, const int *INCX);


#endif /* __LAPACK_H__ */
