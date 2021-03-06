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


#ifndef __REDIRECT_TEST_TIMING_H__
#define __REDIRECT_TEST_TIMING_H__


#if REDIRECT_TEST_TIMING

# if REDIRECT_TEST_REMOTE

#  if REDIRECT_TEST_TIMING_REMOTE
#   define REDIRECT_TEST_TIMING        REDIRECT_TEST_TIMING_REMOTE
#   define REDIRECT_TEST_TIMING_PRINT  REDIRECT_TEST_TIMING_PRINT_REMOTE
#  endif /* REDIRECT_TEST_TIMING_REMOTE */

# else /* REDIRECT_TEST_REMOTE */

#  define REDIRECT_TEST_TIMING        REDIRECT_TEST_TIMING
#  define REDIRECT_TEST_TIMING_PRINT  REDIRECT_TEST_TIMING_PRINT

# endif /* REDIRECT_TEST_REMOTE */

#if !REDIRECT_TEST_TIMING_REMOTE
# undef REDIRECT_TEST_TIMING_REDIRECT_REMOTE
#endif

#else /* REDIRECT_TEST_TIMING */

# undef REDIRECT_TEST_TIMING_REMOTE
# undef REDIRECT_TEST_TIMING_REDIRECT_REMOTE
# undef REDIRECT_TEST_TIMING_PRINT
# undef REDIRECT_TEST_TIMING_PRINT_REMOTE

#endif /* REDIRECT_TEST_TIMING */


#if REDIRECT_TEST_TIMING

# define REDIRECT_TEST_TIMING_INIT(_a_, _i_, _x_)  Z_MOP(for ((_i_) = 0; (_i_) < (_x_); (_i_)++) (_a_)[_i_] = -1;)
# define REDIRECT_TEST_TIMING_START(_t_)           TIMING_START(_t_)
# define REDIRECT_TEST_TIMING_STOP(_t_)            TIMING_STOP(_t_)
# ifndef REDIRECT_TEST_TIMING_PREFIX
#  define REDIRECT_TEST_TIMING_PREFIX              "TIMING: "
# endif
# if REDIRECT_TEST_TIMING_PRINT
#  define REDIRECT_TEST_TIMING_PRINT_F(_f_...)     Z_MOP(printf(REDIRECT_TEST_TIMING_PREFIX _f_); printf("\n"); )
# else
#  define REDIRECT_TEST_TIMING_PRINT_F(_f_...)     Z_NOP()
# endif

#else /* REDIRECT_TEST_TIMING */

# define REDIRECT_TEST_TIMING_INIT(_a_, _i_, _x_)  Z_NOP()
# define REDIRECT_TEST_TIMING_START(_t_)           Z_NOP()
# define REDIRECT_TEST_TIMING_STOP(_t_)            Z_NOP()
# define REDIRECT_TEST_TIMING_PRINT_F(_f_...)      Z_NOP()

#endif /* REDIRECT_TEST_TIMING */


#endif /* __REDIRECT_TEST_TIMING_H__ */
