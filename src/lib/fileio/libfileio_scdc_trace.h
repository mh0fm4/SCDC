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


#ifndef __LIBFILEIO_SCDC_TRACE_H__
#define __LIBFILEIO_SCDC_TRACE_H__


#if !LIBFILEIO_SCDC_TRACE
# undef LIBFILEIO_SCDC_TRACE_INTRO
# undef LIBFILEIO_SCDC_TRACE_INTRO_CTRL
# undef LIBFILEIO_SCDC_TRACE_INTRO_RD
# undef LIBFILEIO_SCDC_TRACE_INTRO_WR
# undef LIBFILEIO_SCDC_TRACE_OUTRO
# undef LIBFILEIO_SCDC_TRACE_OUTRO_CTRL
# undef LIBFILEIO_SCDC_TRACE_OUTRO_RD
# undef LIBFILEIO_SCDC_TRACE_OUTRO_WR
# undef LIBFILEIO_SCDC_TRACE_BODY
# undef LIBFILEIO_SCDC_TRACE_BODY_CTRL
# undef LIBFILEIO_SCDC_TRACE_BODY_RD
# undef LIBFILEIO_SCDC_TRACE_BODY_WR
# undef LIBFILEIO_SCDC_TRACE_ORIG
# undef LIBFILEIO_SCDC_TRACE_ORIG_CTRL
# undef LIBFILEIO_SCDC_TRACE_ORIG_RD
# undef LIBFILEIO_SCDC_TRACE_ORIG_WR
# undef LIBFILEIO_SCDC_TRACE_SCDC
# undef LIBFILEIO_SCDC_TRACE_SCDC_CTRL
# undef LIBFILEIO_SCDC_TRACE_SCDC_RD
# undef LIBFILEIO_SCDC_TRACE_SCDC_WR
#endif


/* INTRO */
#ifndef LIBFILEIO_SCDC_TRACE_INTRO
# define LIBFILEIO_SCDC_TRACE_INTRO  0
#endif

#ifndef LIBFILEIO_SCDC_TRACE_INTRO_CTRL
# define LIBFILEIO_SCDC_TRACE_INTRO_CTRL  LIBFILEIO_SCDC_TRACE_INTRO
#endif
#if LIBFILEIO_SCDC_TRACE_INTRO_CTRL
# define TRACE_F_CTRL_INTRO(_x_...)  TRACE_F(_x_)
#else
# define TRACE_F_CTRL_INTRO(_x_...)  Z_NOP() 
#endif

#ifndef LIBFILEIO_SCDC_TRACE_INTRO_RD
# define LIBFILEIO_SCDC_TRACE_INTRO_RD  LIBFILEIO_SCDC_TRACE_INTRO
#endif
#if LIBFILEIO_SCDC_TRACE_INTRO_RD
# define TRACE_F_RD_INTRO(_x_...)  TRACE_F(_x_)
#else
# define TRACE_F_RD_INTRO(_x_...)  Z_NOP() 
#endif

#ifndef LIBFILEIO_SCDC_TRACE_INTRO_WR
# define LIBFILEIO_SCDC_TRACE_INTRO_WR  LIBFILEIO_SCDC_TRACE_INTRO
#endif
#if LIBFILEIO_SCDC_TRACE_INTRO_WR
# define TRACE_F_WR_INTRO(_x_...)  TRACE_F(_x_)
#else
# define TRACE_F_WR_INTRO(_x_...)  Z_NOP() 
#endif

/* OUTRO */
#ifndef LIBFILEIO_SCDC_TRACE_OUTRO
# define LIBFILEIO_SCDC_TRACE_OUTRO  0
#endif

#ifndef LIBFILEIO_SCDC_TRACE_OUTRO_CTRL
# define LIBFILEIO_SCDC_TRACE_OUTRO_CTRL  LIBFILEIO_SCDC_TRACE_OUTRO
#endif
#if LIBFILEIO_SCDC_TRACE_OUTRO_CTRL
# define TRACE_F_CTRL_OUTRO(_x_...)  TRACE_F(_x_)
#else
# define TRACE_F_CTRL_OUTRO(_x_...)  Z_NOP() 
#endif

#ifndef LIBFILEIO_SCDC_TRACE_OUTRO_RD
# define LIBFILEIO_SCDC_TRACE_OUTRO_RD  LIBFILEIO_SCDC_TRACE_OUTRO
#endif
#if LIBFILEIO_SCDC_TRACE_OUTRO_RD
# define TRACE_F_RD_OUTRO(_x_...)  TRACE_F(_x_)
#else
# define TRACE_F_RD_OUTRO(_x_...)  Z_NOP() 
#endif

#ifndef LIBFILEIO_SCDC_TRACE_OUTRO_WR
# define LIBFILEIO_SCDC_TRACE_OUTRO_WR  LIBFILEIO_SCDC_TRACE_OUTRO
#endif
#if LIBFILEIO_SCDC_TRACE_OUTRO_WR
# define TRACE_F_WR_OUTRO(_x_...)  TRACE_F(_x_)
#else
# define TRACE_F_WR_OUTRO(_x_...)  Z_NOP() 
#endif

/* BODY */
#ifndef LIBFILEIO_SCDC_TRACE_BODY
# define LIBFILEIO_SCDC_TRACE_BODY  0
#endif

#ifndef LIBFILEIO_SCDC_TRACE_BODY_CTRL
# define LIBFILEIO_SCDC_TRACE_BODY_CTRL  LIBFILEIO_SCDC_TRACE_BODY
#endif
#if LIBFILEIO_SCDC_TRACE_BODY_CTRL
# define TRACE_F_CTRL_BODY(_x_...)  TRACE_F(_x_)
#else
# define TRACE_F_CTRL_BODY(_x_...)  Z_NOP() 
#endif

#ifndef LIBFILEIO_SCDC_TRACE_BODY_RD
# define LIBFILEIO_SCDC_TRACE_BODY_RD  LIBFILEIO_SCDC_TRACE_BODY
#endif
#if LIBFILEIO_SCDC_TRACE_BODY_RD
# define TRACE_F_RD_BODY(_x_...)  TRACE_F(_x_)
#else
# define TRACE_F_RD_BODY(_x_...)  Z_NOP() 
#endif

#ifndef LIBFILEIO_SCDC_TRACE_BODY_WR
# define LIBFILEIO_SCDC_TRACE_BODY_WR  LIBFILEIO_SCDC_TRACE_BODY
#endif
#if LIBFILEIO_SCDC_TRACE_BODY_WR
# define TRACE_F_WR_BODY(_x_...)  TRACE_F(_x_)
#else
# define TRACE_F_WR_BODY(_x_...)  Z_NOP() 
#endif

/* ORIG */
#ifndef LIBFILEIO_SCDC_TRACE_ORIG
# define LIBFILEIO_SCDC_TRACE_ORIG  0
#endif

#ifndef LIBFILEIO_SCDC_TRACE_ORIG_CTRL
# define LIBFILEIO_SCDC_TRACE_ORIG_CTRL  LIBFILEIO_SCDC_TRACE_ORIG
#endif
#if LIBFILEIO_SCDC_TRACE_ORIG_CTRL
# define TRACE_F_CTRL_ORIG(_x_...)       TRACE_F("orig: " _x_)
# define TRACE_F_CTRL_ORIG_FAIL(_x_...)  TRACE_F("orig failed: " _x_)
#else
# define TRACE_F_CTRL_ORIG(_x_...)       Z_NOP()
# define TRACE_F_CTRL_ORIG_FAIL(_x_...)  Z_NOP()
#endif

#ifndef LIBFILEIO_SCDC_TRACE_ORIG_RD
# define LIBFILEIO_SCDC_TRACE_ORIG_RD  LIBFILEIO_SCDC_TRACE_ORIG
#endif
#if LIBFILEIO_SCDC_TRACE_ORIG_RD
# define TRACE_F_RD_ORIG(_x_...)  TRACE_F("orig: " _x_)
#else
# define TRACE_F_RD_ORIG(_x_...)  Z_NOP() 
#endif

#ifndef LIBFILEIO_SCDC_TRACE_ORIG_WR
# define LIBFILEIO_SCDC_TRACE_ORIG_WR  LIBFILEIO_SCDC_TRACE_ORIG
#endif
#if LIBFILEIO_SCDC_TRACE_ORIG_WR
# define TRACE_F_WR_ORIG(_x_...)  TRACE_F("orig: " _x_)
#else
# define TRACE_F_WR_ORIG(_x_...)  Z_NOP() 
#endif

/* SCDC */
#ifndef LIBFILEIO_SCDC_TRACE_SCDC
# define LIBFILEIO_SCDC_TRACE_SCDC  0
#endif

#ifndef LIBFILEIO_SCDC_TRACE_SCDC_CTRL
# define LIBFILEIO_SCDC_TRACE_SCDC_CTRL  LIBFILEIO_SCDC_TRACE_SCDC
#endif
#if LIBFILEIO_SCDC_TRACE_SCDC_CTRL
# define TRACE_F_CTRL_SCDC(_x_...)       TRACE_F("scdc: " _x_)
# define TRACE_F_CTRL_SCDC_FAIL(_x_...)  TRACE_F("scdc failed: " _x_)
#else
# define TRACE_F_CTRL_SCDC(_x_...)       Z_NOP()
# define TRACE_F_CTRL_SCDC_FAIL(_x_...)  Z_NOP()
#endif

#ifndef LIBFILEIO_SCDC_TRACE_SCDC_RD
# define LIBFILEIO_SCDC_TRACE_SCDC_RD  LIBFILEIO_SCDC_TRACE_SCDC
#endif
#if LIBFILEIO_SCDC_TRACE_SCDC_RD
# define TRACE_F_RD_SCDC(_x_...)  TRACE_F("scdc: " _x_)
#else
# define TRACE_F_RD_SCDC(_x_...)  Z_NOP() 
#endif

#ifndef LIBFILEIO_SCDC_TRACE_SCDC_WR
# define LIBFILEIO_SCDC_TRACE_SCDC_WR  LIBFILEIO_SCDC_TRACE_SCDC
#endif
#if LIBFILEIO_SCDC_TRACE_SCDC_WR
# define TRACE_F_WR_SCDC(_x_...)  TRACE_F("scdc: " _x_)
#else
# define TRACE_F_WR_SCDC(_x_...)  Z_NOP() 
#endif


#endif /* __LIBFILEIO_SCDC_TRACE_H__ */
