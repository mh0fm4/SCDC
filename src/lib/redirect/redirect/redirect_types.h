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


#ifndef __REDIRECT_TYPES_H__
#define __REDIRECT_TYPES_H__


#include "redirect_config.h"


#if 1

static const struct {
  const char *printf_fmt, *scanf_fmt;
  int size_of;
} FMTS[] = {
#define FMT_CHAR  0
  { .printf_fmt = "%c", .scanf_fmt = "%c", .size_of = sizeof(char) },
#define FMT_INT  1
  { .printf_fmt = "%d", .scanf_fmt = "%d", .size_of = sizeof(int) },
#define FMT_LONG  2
  { .printf_fmt = "%ld", .scanf_fmt = "%ld", .size_of = sizeof(long) },
#define FMT_FLOAT  3
  { .printf_fmt = "%.8f", .scanf_fmt = "%f", .size_of = sizeof(float) },
#define FMT_DOUBLE  4
  { .printf_fmt = "%.16e", .scanf_fmt = "%lf", .size_of = sizeof(double) },
#define FMT_VOID_PTR  5
  { .printf_fmt = "%p", .scanf_fmt = "%p", .size_of = sizeof(void *) },
};

#define PRINTF_FMT(_f_)      FMTS[_f_].printf_fmt
#define SCANF_FMT(_f_)       FMTS[_f_].scanf_fmt
#define SCANF_PTR(_f_, _v_)  (_v_)
#define SIZE_OF(_f_)         FMTS[_f_].size_of

#else

static const char *FMTS[][2] = {
#define FMT_CHAR  0
  { "%c", "%c" },
#define FMT_INT  1
  { "%d", "%d" },
#define FMT_LONG  2
  { "%ld", "%ld" },
#define FMT_FLOAT  3
  { "%.8f", "%f" },
#define FMT_DOUBLE  4
  { "%.16e", "%lf" },
#define FMT_VOID_PTR  5
  { "%p", "%p" },
};

#define PRINTF_FMT(_f_)      FMTS[_f_][0]
#define SCANF_FMT(_f_)       FMTS[_f_][1]
#define SCANF_PTR(_f_, _v_)  (_v_)

#endif


#endif /* __REDIRECT_TYPES_H__ */
