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


#ifndef __LAPACK_TIMING_H__
#define __LAPACK_TIMING_H__


#if LIBLAPACK_SCDC_TIMING

# if LIBLAPACK_SCDC_REMOTE

#  if LIBLAPACK_SCDC_TIMING_REMOTE
#   define LAPACK_TIMING        LIBLAPACK_SCDC_TIMING_REMOTE
#   define LAPACK_TIMING_PRINT  LIBLAPACK_SCDC_TIMING_PRINT_REMOTE
#  endif /* LIBLAPACK_SCDC_TIMING_REMOTE */

# else /* LIBLAPACK_SCDC_REMOTE */

#  define LAPACK_TIMING        LIBLAPACK_SCDC_TIMING
#  define LAPACK_TIMING_PRINT  LIBLAPACK_SCDC_TIMING_PRINT

# endif /* LIBLAPACK_SCDC_REMOTE */

#if !LIBLAPACK_SCDC_TIMING_REMOTE
# undef LIBLAPACK_SCDC_TIMING_REDIRECT_REMOTE
#endif

#else /* LIBLAPACK_SCDC_TIMING */

# undef LIBLAPACK_SCDC_TIMING_REMOTE
# undef LIBLAPACK_SCDC_TIMING_REDIRECT_REMOTE
# undef LIBLAPACK_SCDC_TIMING_PRINT
# undef LIBLAPACK_SCDC_TIMING_PRINT_REMOTE

#endif /* LIBLAPACK_SCDC_TIMING */


#if LAPACK_TIMING

# define LAPACK_TIMING_INIT(_a_, _i_, _x_)  Z_MOP(for ((_i_) = 0; (_i_) < (_x_); (_i_)++) (_a_)[_i_] = -1;)
# define LAPACK_TIMING_START(_t_)           TIMING_START(_t_)
# define LAPACK_TIMING_STOP(_t_)            TIMING_STOP(_t_)
# ifndef LAPACK_TIMING_PREFIX
#  define LAPACK_TIMING_PREFIX              "TIMING: "
# endif
# if LAPACK_TIMING_PRINT
#  define LAPACK_TIMING_PRINT_F(_f_...)     Z_MOP(printf(LAPACK_TIMING_PREFIX _f_); printf("\n"); )
# else
#  define LAPACK_TIMING_PRINT_F(_f_...)     Z_NOP()
# endif

#else /* LAPACK_TIMING */

# define LAPACK_TIMING_INIT(_a_, _i_, _x_)  Z_NOP()
# define LAPACK_TIMING_START(_t_)           Z_NOP()
# define LAPACK_TIMING_STOP(_t_)            Z_NOP()
# define LAPACK_TIMING_PRINT_F(_f_...)      Z_NOP()

#endif /* LAPACK_TIMING */


#endif /* __LAPACK_TIMING_H__ */
