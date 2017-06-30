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


#ifndef __FILEIO_SCDC_TRACE_H__
#define __FILEIO_SCDC_TRACE_H__


#if !FILEIO_SCDC_TRACE
# undef FILEIO_SCDC_TRACE_INTRO
# undef FILEIO_SCDC_TRACE_INTRO_CTRL
# undef FILEIO_SCDC_TRACE_INTRO_RD
# undef FILEIO_SCDC_TRACE_INTRO_WR
# undef FILEIO_SCDC_TRACE_OUTRO
# undef FILEIO_SCDC_TRACE_OUTRO_CTRL
# undef FILEIO_SCDC_TRACE_OUTRO_RD
# undef FILEIO_SCDC_TRACE_OUTRO_WR
# undef FILEIO_SCDC_TRACE_BODY
# undef FILEIO_SCDC_TRACE_BODY_CTRL
# undef FILEIO_SCDC_TRACE_BODY_RD
# undef FILEIO_SCDC_TRACE_BODY_WR
# undef FILEIO_SCDC_TRACE_PERF
# undef FILEIO_SCDC_TRACE_PERF_CTRL
# undef FILEIO_SCDC_TRACE_PERF_RD
# undef FILEIO_SCDC_TRACE_PERF_WR
# undef FILEIO_SCDC_TRACE_SCDC
# undef FILEIO_SCDC_TRACE_SCDC_CTRL
# undef FILEIO_SCDC_TRACE_SCDC_RD
# undef FILEIO_SCDC_TRACE_SCDC_WR
#endif


/* INTRO */
#ifndef FILEIO_SCDC_TRACE_INTRO
# define FILEIO_SCDC_TRACE_INTRO  0
#endif

#ifndef FILEIO_SCDC_TRACE_INTRO_CTRL
# define FILEIO_SCDC_TRACE_INTRO_CTRL  FILEIO_SCDC_TRACE_INTRO
#endif
#if FILEIO_SCDC_TRACE_INTRO_CTRL
# define TRACE_F_CTRL_INTRO(_x_...)  TRACE_F(_x_)
#else
# define TRACE_F_CTRL_INTRO(_x_...)  Z_NOP() 
#endif

#ifndef FILEIO_SCDC_TRACE_INTRO_RD
# define FILEIO_SCDC_TRACE_INTRO_RD  FILEIO_SCDC_TRACE_INTRO
#endif
#if FILEIO_SCDC_TRACE_INTRO_RD
# define TRACE_F_RD_INTRO(_x_...)  TRACE_F(_x_)
#else
# define TRACE_F_RD_INTRO(_x_...)  Z_NOP() 
#endif

#ifndef FILEIO_SCDC_TRACE_INTRO_WR
# define FILEIO_SCDC_TRACE_INTRO_WR  FILEIO_SCDC_TRACE_INTRO
#endif
#if FILEIO_SCDC_TRACE_INTRO_WR
# define TRACE_F_WR_INTRO(_x_...)  TRACE_F(_x_)
#else
# define TRACE_F_WR_INTRO(_x_...)  Z_NOP() 
#endif

/* OUTRO */
#ifndef FILEIO_SCDC_TRACE_OUTRO
# define FILEIO_SCDC_TRACE_OUTRO  0
#endif

#ifndef FILEIO_SCDC_TRACE_OUTRO_CTRL
# define FILEIO_SCDC_TRACE_OUTRO_CTRL  FILEIO_SCDC_TRACE_OUTRO
#endif
#if FILEIO_SCDC_TRACE_OUTRO_CTRL
# define TRACE_F_CTRL_OUTRO(_x_...)  TRACE_F(_x_)
#else
# define TRACE_F_CTRL_OUTRO(_x_...)  Z_NOP() 
#endif

#ifndef FILEIO_SCDC_TRACE_OUTRO_RD
# define FILEIO_SCDC_TRACE_OUTRO_RD  FILEIO_SCDC_TRACE_OUTRO
#endif
#if FILEIO_SCDC_TRACE_OUTRO_RD
# define TRACE_F_RD_OUTRO(_x_...)  TRACE_F(_x_)
#else
# define TRACE_F_RD_OUTRO(_x_...)  Z_NOP() 
#endif

#ifndef FILEIO_SCDC_TRACE_OUTRO_WR
# define FILEIO_SCDC_TRACE_OUTRO_WR  FILEIO_SCDC_TRACE_OUTRO
#endif
#if FILEIO_SCDC_TRACE_OUTRO_WR
# define TRACE_F_WR_OUTRO(_x_...)  TRACE_F(_x_)
#else
# define TRACE_F_WR_OUTRO(_x_...)  Z_NOP() 
#endif

/* BODY */
#ifndef FILEIO_SCDC_TRACE_BODY
# define FILEIO_SCDC_TRACE_BODY  0
#endif

#ifndef FILEIO_SCDC_TRACE_BODY_CTRL
# define FILEIO_SCDC_TRACE_BODY_CTRL  FILEIO_SCDC_TRACE_BODY
#endif
#if FILEIO_SCDC_TRACE_BODY_CTRL
# define TRACE_F_CTRL_BODY(_x_...)  TRACE_F(_x_)
#else
# define TRACE_F_CTRL_BODY(_x_...)  Z_NOP() 
#endif

#ifndef FILEIO_SCDC_TRACE_BODY_RD
# define FILEIO_SCDC_TRACE_BODY_RD  FILEIO_SCDC_TRACE_BODY
#endif
#if FILEIO_SCDC_TRACE_BODY_RD
# define TRACE_F_RD_BODY(_x_...)  TRACE_F(_x_)
#else
# define TRACE_F_RD_BODY(_x_...)  Z_NOP() 
#endif

#ifndef FILEIO_SCDC_TRACE_BODY_WR
# define FILEIO_SCDC_TRACE_BODY_WR  FILEIO_SCDC_TRACE_BODY
#endif
#if FILEIO_SCDC_TRACE_BODY_WR
# define TRACE_F_WR_BODY(_x_...)  TRACE_F(_x_)
#else
# define TRACE_F_WR_BODY(_x_...)  Z_NOP() 
#endif

/* PERF */
#ifndef FILEIO_SCDC_TRACE_PERF
# define FILEIO_SCDC_TRACE_PERF  0
#endif

#ifndef FILEIO_SCDC_TRACE_PERF_CTRL
# define FILEIO_SCDC_TRACE_PERF_CTRL  FILEIO_SCDC_TRACE_PERF
#endif
#if FILEIO_SCDC_TRACE_PERF_CTRL
# define TRACE_F_CTRL_PERF(_x_...)  TRACE_F(_x_)
#else
# define TRACE_F_CTRL_PERF(_x_...)  Z_NOP() 
#endif

#ifndef FILEIO_SCDC_TRACE_PERF_RD
# define FILEIO_SCDC_TRACE_PERF_RD  FILEIO_SCDC_TRACE_PERF
#endif
#if FILEIO_SCDC_TRACE_PERF_RD
# define TRACE_F_RD_PERF(_x_...)  TRACE_F(_x_)
#else
# define TRACE_F_RD_PERF(_x_...)  Z_NOP() 
#endif

#ifndef FILEIO_SCDC_TRACE_PERF_WR
# define FILEIO_SCDC_TRACE_PERF_WR  FILEIO_SCDC_TRACE_PERF
#endif
#if FILEIO_SCDC_TRACE_PERF_WR
# define TRACE_F_WR_PERF(_x_...)  TRACE_F(_x_)
#else
# define TRACE_F_WR_PERF(_x_...)  Z_NOP() 
#endif

/* SCDC */
#ifndef FILEIO_SCDC_TRACE_SCDC
# define FILEIO_SCDC_TRACE_SCDC  0
#endif

#ifndef FILEIO_SCDC_TRACE_SCDC_CTRL
# define FILEIO_SCDC_TRACE_SCDC_CTRL  FILEIO_SCDC_TRACE_SCDC
#endif
#if FILEIO_SCDC_TRACE_SCDC_CTRL
# define TRACE_F_CTRL_SCDC(_x_...)  TRACE_F(_x_)
#else
# define TRACE_F_CTRL_SCDC(_x_...)  Z_NOP() 
#endif

#ifndef FILEIO_SCDC_TRACE_SCDC_RD
# define FILEIO_SCDC_TRACE_SCDC_RD  FILEIO_SCDC_TRACE_SCDC
#endif
#if FILEIO_SCDC_TRACE_SCDC_RD
# define TRACE_F_RD_SCDC(_x_...)  TRACE_F(_x_)
#else
# define TRACE_F_RD_SCDC(_x_...)  Z_NOP() 
#endif

#ifndef FILEIO_SCDC_TRACE_SCDC_WR
# define FILEIO_SCDC_TRACE_SCDC_WR  FILEIO_SCDC_TRACE_SCDC
#endif
#if FILEIO_SCDC_TRACE_SCDC_WR
# define TRACE_F_WR_SCDC(_x_...)  TRACE_F(_x_)
#else
# define TRACE_F_WR_SCDC(_x_...)  Z_NOP() 
#endif


#endif /* __FILEIO_SCDC_TRACE_H__ */
