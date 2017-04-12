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


#ifndef __COMMON_H__
#define __COMMON_H__


#if HAVE_CONFIG_H
# include "config.h"
#endif

#define Z_IGNORE_CONFIG_H
#include "z_pack.h"


#ifndef TRACE_FILE
# define TRACE_FILE  stdout
#endif

#ifndef TRACE_FPRINTF
# define TRACE_FPRINTF(...)  fprintf(__VA_ARGS__)
#endif

#if HAVE_TRACE
# define TRACE_CMD(_c_)  Z_MOP(_c_)
# define TRACE(_s_)      Z_MOP(TRACE_FPRINTF(TRACE_FILE, TRACE_PREFIX "%s\n", (_s_));)
# define TRACE_N(_s_)    Z_MOP(TRACE_FPRINTF(TRACE_FILE, TRACE_PREFIX "%s", (_s_));)
# define TRACE_F(...)    Z_MOP(TRACE_FPRINTF(TRACE_FILE, TRACE_PREFIX __VA_ARGS__); TRACE_FPRINTF(TRACE_FILE, "\n");)
# define TRACE_F_N(...)  Z_MOP(TRACE_FPRINTF(TRACE_FILE, TRACE_PREFIX __VA_ARGS__);)
# define TRACE_R(_s_)    Z_MOP(TRACE_FPRINTF(TRACE_FILE, (_s_));)
#else
# define TRACE_CMD(_c_)  Z_NOP()
# define TRACE(_s_)      Z_NOP()
# define TRACE_N(_s_)    Z_NOP()
# define TRACE_F(...)    Z_NOP()
# define TRACE_F_N(...)  Z_NOP()
# define TRACE_R(_s_)    Z_NOP()
#endif

#if HAVE_ASSERT
# define ASSERT(_b_)      Z_MOP(if (!(_b_)) fprintf(stderr, "ASSERT: '" #_b_ "' failed!\n");)
#else
# define ASSERT(_b_)      Z_NOP()
#endif

#if HAVE_TIMING
# define TIMING_DECL(_decl_)                     _decl_
# define TIMING_CMD(_cmd_)                       Z_MOP(_cmd_)
# define TIMING_START(_t_)                       ((_t_) = z_time_wtime())
# define TIMING_STOP(_t_)                        ((_t_) = z_time_wtime() - (_t_))
# define TIMING_STOP_ADD(_t_, _r_)               ((_r_) += z_time_wtime() - (_t_))
# ifndef TIMING_PRINT
#  define TIMING_PRINT(_i_, _s_, _n_, _v_, _r_)  Z_NOP()
# endif
# ifndef TIMING_PRINT_PREFIX
#  define TIMING_PRINT_PREFIX                    "TIMING: "
# endif
#else
# define TIMING_DECL(_decl_)
# define TIMING_CMD(_cmd_)                      Z_NOP()
# define TIMING_START(_t_)                      Z_NOP()
# define TIMING_STOP(_t_)                       Z_NOP()
# define TIMING_STOP_ADD(_t_, _r_)              Z_NOP()
# undef TIMING_PRINT
# define TIMING_PRINT(_i_, _s_, _n_, _v_, _r_)  Z_NOP()
#endif


#endif /* __COMMON_H__ */
