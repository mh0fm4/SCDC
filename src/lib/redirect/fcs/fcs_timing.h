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


#ifndef __FCS_TIMING_H__
#define __FCS_TIMING_H__


#if LIBFCS_SCDC_TIMING

# if LIBFCS_SCDC_REMOTE

#  if LIBFCS_SCDC_TIMING_REMOTE
#   define FCS_TIMING        LIBFCS_SCDC_TIMING_REMOTE
#   define FCS_TIMING_PRINT  LIBFCS_SCDC_TIMING_PRINT_REMOTE
#  endif /* LIBFCS_SCDC_TIMING_REMOTE */

# else /* LIBFCS_SCDC_REMOTE */

#  define FCS_TIMING        LIBFCS_SCDC_TIMING
#  define FCS_TIMING_PRINT  LIBFCS_SCDC_TIMING_PRINT

# endif /* LIBFCS_SCDC_REMOTE */

#if !LIBFCS_SCDC_TIMING_REMOTE
# undef LIBFCS_SCDC_TIMING_REDIRECT_REMOTE
#endif

#else /* LIBFCS_SCDC_TIMING */

# undef LIBFCS_SCDC_TIMING_REMOTE
# undef LIBFCS_SCDC_TIMING_REDIRECT_REMOTE
# undef LIBFCS_SCDC_TIMING_PRINT
# undef LIBFCS_SCDC_TIMING_PRINT_REMOTE

#endif /* LIBFCS_SCDC_TIMING */


#if FCS_TIMING

# define FCS_TIMING_INIT(_a_, _i_, _x_)  Z_MOP(for ((_i_) = 0; (_i_) < (_x_); (_i_)++) (_a_)[_i_] = -1;)
# define FCS_TIMING_START(_t_)           TIMING_START(_t_)
# define FCS_TIMING_STOP(_t_)            TIMING_STOP(_t_)
# ifndef FCS_TIMING_PREFIX
#  define FCS_TIMING_PREFIX              "TIMING: "
# endif
# if FCS_TIMING_PRINT
#  define FCS_TIMING_PRINT_F(_f_...)     Z_MOP(printf(FCS_TIMING_PREFIX _f_); printf("\n"); )
# else
#  define FCS_TIMING_PRINT_F(_f_...)     Z_NOP()
# endif

#else /* FCS_TIMING */

# define FCS_TIMING_INIT(_a_, _i_, _x_)  Z_NOP()
# define FCS_TIMING_START(_t_)           Z_NOP()
# define FCS_TIMING_STOP(_t_)            Z_NOP()
# define FCS_TIMING_PRINT_F(_f_...)      Z_NOP()

#endif /* FCS_TIMING */


#endif /* __FCS_TIMING_H__ */
