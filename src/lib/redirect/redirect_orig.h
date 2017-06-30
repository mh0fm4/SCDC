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


#ifndef __REDIRECT_ORIG_H__
#define __REDIRECT_ORIG_H__


#include <dlfcn.h>
#include "z_pack.h"


#ifndef REDIRECT_ORIG
# define REDIRECT_ORIG  redirect_originals
#endif


#define REDIRECT_ORIG_DECLARE_INTRO() \
  static struct { \
    void *this_is_not_an_empty_struct

#define REDIRECT_ORIG_DECLARE(_f_) \
  _f_ ## _f *_f_, *_f_ ## _

#define REDIRECT_ORIG_DECLARE_OUTRO() \
  } REDIRECT_ORIG = { NULL };


#define REDIRECT_ORIG_INIT()        Z_MOP(int i; for (i = 0; i < sizeof(REDIRECT_ORIG) / sizeof(void *); ++i) ((void **) &REDIRECT_ORIG)[i] = NULL;)
#define REDIRECT_ORIG_RELEASE()     Z_NOP()
#define REDIRECT_ORIG_F_INIT(_f_)   Z_MOP(if (REDIRECT_ORIG._f_ == NULL) *((void **) &REDIRECT_ORIG._f_) = dlsym(RTLD_NEXT, #_f_);)
#define REDIRECT_ORIG_F(_f_)        REDIRECT_ORIG._f_
#define REDIRECT_ORIG_F_INIT_(_f_)  Z_MOP(if (REDIRECT_ORIG.Z_CONCAT(_f_, _) == NULL) *((void **) &REDIRECT_ORIG.Z_CONCAT(_f_, _)) = dlsym(RTLD_NEXT, #_f_);)
#define REDIRECT_ORIG_F_(_f_)       REDIRECT_ORIG.Z_CONCAT(_f_, _)


#endif /* __REDIRECT_ORIG_H__ */
