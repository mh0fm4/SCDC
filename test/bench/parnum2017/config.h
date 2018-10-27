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


#ifndef __CONFIG_H__
#define __CONFIG_H__


#define BLAS_NREP                2

#define BLAS_BENCH_VECTOR_N      10 * 1000 * 1000
#define BLAS_BENCH_VECTOR_INC    1, 2, 4
#define BLAS_BENCH_VECTOR_ALLOC  BLAS_BENCH_VECTOR_N * 4

#define BLAS_BENCH_MATRIX_M      4000
#define BLAS_BENCH_MATRIX_LDA    4000, 4001, 8000
#define BLAS_BENCH_MATRIX_ALLOC  BLAS_BENCH_MATRIX_M * 8000

#define BLAS_LEVEL1_N            100, 100 * 1000, 100 * 1000 * 1000
#define BLAS_LEVEL1_ALLOC        100 * 1000 * 1000

#define BLAS_LEVEL2_N            100, 1000, 4000, 8000
#define BLAS_LEVEL2_ALLOC        8000

#define BLAS_LEVEL3_N            100, 1000, 2000, 4000
#define BLAS_LEVEL3_ALLOC        4000


#define LAPACK_NREP       2

#define LAPACK_BENCH_N    16, 32, 64, 128, 256, 512, 1024, 2048, 4096
#define LAPACK_BENCH_MAX  4096


#endif /* __CONFIG_H__ */
