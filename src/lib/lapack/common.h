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


#define HAVE_TRACE       1
#define HAVE_TRACE_DATA  1
#define HAVE_ASSERT      1

#if HAVE_TRACE
# define TRACE(_c_)       Z_MOP(_c_)
#else
# define TRACE(_c_)       Z_NOP()
#endif

#if HAVE_TRACE_DATA
# define TRACE_DATA(_c_)  TRACE(_c_)
#else
# define TRACE_DATA(_c_)  Z_NOP()
#endif

#if HAVE_ASSERT
# define ASSERT(_b_)      Z_MOP(if (!(_b_)) fprintf(stderr, "ASSERT: '" #_b_ "' failed!\n");)
#else
# define ASERT(_b_)       Z_NOP()
#endif


#endif /* __COMMON_H__ */
