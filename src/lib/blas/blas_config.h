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


#ifndef __BLAS_CONFIG_H__
#define __BLAS_CONFIG_H__


/* BLAS bench */
#define BLAS_BENCH   1

/* BLAS level 1 */
#define BLAS_LEVEL1  1
#define BLAS_IDAMAX  1
#define BLAS_DCOPY   1
#define BLAS_DSCAL   1
#define BLAS_DAXPY   1

/* BLAS level 2 */
#define BLAS_LEVEL2  1
#define BLAS_DGER    1
#define BLAS_DGEMV   1
#define BLAS_DTRSV   1

/* BLAS level 3 */
#define BLAS_LEVEL3  1
#define BLAS_DGEMM   1
#define BLAS_DTRSM   1


#if 0
#undef BLAS_LEVEL1
#undef BLAS_LEVEL2
#undef BLAS_LEVEL3
#undef BLAS_DGEMM
#undef BLAS_DTRSM

#define BLAS_LEVEL1  0
#define BLAS_LEVEL2  0
#define BLAS_LEVEL3  1
#define BLAS_DGEMM   1
#define BLAS_DTRSM   0
#endif

#endif /* __BLAS_CONFIG_H__ */
