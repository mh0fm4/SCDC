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


#ifndef __COMMON_H__
#define __COMMON_H__


#include "z_pack.h"


#define HAVE_TRACE       1
#define HAVE_ASSERT      1

#ifndef TRACE_FILE
# define TRACE_FILE  stderr
#endif

#ifndef TRACE_FPRINTF
int original_fprintf(FILE *stream, const char *format, ...);
# define TRACE_FPRINTF(...)  original_fprintf(__VA_ARGS__)
#endif

#if HAVE_TRACE
# define TRACE_CMD(_c_)  Z_MOP(_c_)
# define TRACE(_s_)      Z_MOP(TRACE_FPRINTF(TRACE_FILE, TRACE_PREFIX "%s\n", (_s_));)
# define TRACE_F(...)    Z_MOP(TRACE_FPRINTF(TRACE_FILE, TRACE_PREFIX __VA_ARGS__); TRACE_FPRINTF(TRACE_FILE, "\n");)
#else
# define TRACE_CMD(_c_)  Z_NOP()
# define TRACE(_s_)      Z_NOP()
# define TRACE_F(...)    Z_NOP()
#endif

#if HAVE_ASSERT
# define ASSERT(_b_)      Z_MOP(if (!(_b_)) fprintf(stderr, "ASSERT: '" #_b_ "' failed!\n");)
#else
# define ASSERT(_b_)      Z_NOP()
#endif

typedef long long dsint_t;
#define dsint_fmt   "lld"
#define DSINT(_x_)  ((dsint_t) _x_)

typedef unsigned long long duint_t;
#define duint_fmt   "llu"
#define DUINT(_x_)  ((duint_t) _x_)

typedef void *dptr_t;
#define dptr_fmt   "p"
#define DPTR(_x_)  ((dptr_t) _x_)


#endif /* __COMMON_H__ */
