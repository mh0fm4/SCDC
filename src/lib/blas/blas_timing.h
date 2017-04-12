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


#ifndef __BLAS_TIMING_H__
#define __BLAS_TIMING_H__


#if LIBBLAS_SCDC_TIMING

# if LIBBLAS_SCDC_REMOTE

#  if LIBBLAS_SCDC_TIMING_REMOTE
#   define BLAS_TIMING        LIBBLAS_SCDC_TIMING_REMOTE
#   define BLAS_TIMING_PRINT  LIBBLAS_SCDC_TIMING_PRINT_REMOTE
#  endif /* LIBBLAS_SCDC_TIMING_REMOTE */

# else /* LIBBLAS_SCDC_REMOTE */

#  define BLAS_TIMING        LIBBLAS_SCDC_TIMING
#  define BLAS_TIMING_PRINT  LIBBLAS_SCDC_TIMING_PRINT

# endif /* LIBBLAS_SCDC_REMOTE */

#if !LIBBLAS_SCDC_TIMING_REMOTE
# undef LIBBLAS_SCDC_TIMING_REDIRECT_REMOTE
#endif

#else /* LIBBLAS_SCDC_TIMING */

# undef LIBBLAS_SCDC_TIMING_REMOTE
# undef LIBBLAS_SCDC_TIMING_REDIRECT_REMOTE
# undef LIBBLAS_SCDC_TIMING_PRINT
# undef LIBBLAS_SCDC_TIMING_PRINT_REMOTE

#endif /* LIBBLAS_SCDC_TIMING */


#if BLAS_TIMING

# define BLAS_TIMING_INIT(_a_, _i_, _x_)  Z_MOP(for ((_i_) = 0; (_i_) < (_x_); (_i_)++) (_a_)[_i_] = -1;)
# define BLAS_TIMING_START(_t_)           TIMING_START(_t_)
# define BLAS_TIMING_STOP(_t_)            TIMING_STOP(_t_)
# ifndef BLAS_TIMING_PREFIX
#  define BLAS_TIMING_PREFIX              "TIMING: "
# endif
# if BLAS_TIMING_PRINT
#  define BLAS_TIMING_PRINT_F(_f_...)     Z_MOP(printf(BLAS_TIMING_PREFIX _f_); printf("\n"); )
# else
#  define BLAS_TIMING_PRINT_F(_f_...)     Z_NOP()
# endif

#else /* BLAS_TIMING */

# define BLAS_TIMING_INIT(_a_, _i_, _x_)  Z_NOP()
# define BLAS_TIMING_START(_t_)           Z_NOP()
# define BLAS_TIMING_STOP(_t_)            Z_NOP()
# define BLAS_TIMING_PRINT_F(_f_...)      Z_NOP()

#endif /* BLAS_TIMING */


#endif /* __BLAS_TIMING_H__ */
