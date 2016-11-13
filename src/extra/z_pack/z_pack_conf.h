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


#ifndef __Z_PACK_CONF_H__
#define __Z_PACK_CONF_H__


#include "scdc_defs.h"


typedef scdcint_t z_int_t;
#define z_int_fmt  scdcint_fmt


#define Z_PACK_NUMERIC

#define HAVE_STDDEF_H  1
#define HAVE_SYS_TIME_H  1
#define Z_PACK_TIME  1

#define HAVE_SYS_STAT_H  1
#define HAVE_SYS_TYPES_H  1
#define HAVE_STDLIB_H  1
#define HAVE_STRING_H  1
#define HAVE_STDIO_H  1
#define HAVE_UNISTD_H 1
#define HAVE_FTW_H  1
#define Z_PACK_FS

#define HAVE_STDIO_H  1
#define HAVE_STRING_H  1
#define HAVE_STDARG_H  1
#define Z_PACK_STDIO  1


#endif /* __Z_PACK_CONF_H__ */
