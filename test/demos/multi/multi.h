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


#ifndef __MULTI_H__
#define __MULTI_H__


#define Z_MOP(_mop_)  do { _mop_ } while (0)


#define MULTI_TRACE_PREFIX  "MULTI_TRACE: "

#define MULTI_TRACE(_s_...)      Z_MOP(printf(MULTI_TRACE_PREFIX MULTI_LOG_PREFIX _s_); printf("\n");)
#define MULTI_TRACE_N(_s_...)    Z_MOP(printf(MULTI_TRACE_PREFIX MULTI_LOG_PREFIX _s_);)
#define MULTI_TRACE_C(_s_...)    Z_MOP(printf(_s_); printf("\n");)
#define MULTI_TRACE_C_N(_s_...)  Z_MOP(printf(_s_);)
#define MULTI_TRACE_NL(_s_...)   Z_MOP(printf("\n");)


#define MULTI_BASE  "simpat"


#endif /* __MULTI_H__ */
