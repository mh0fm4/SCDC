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


#ifndef __FCS_ORIG_H__
#define __FCS_ORIG_H__


#if FCS_ORIG_ENABLED

#include "fcs.h"
#include "redirect_orig.h"

REDIRECT_ORIG_DECLARE_INTRO();

REDIRECT_ORIG_DECLARE(fcs_init);
REDIRECT_ORIG_DECLARE(fcs_destroy);

REDIRECT_ORIG_DECLARE_OUTRO();

#define FCS_ORIG_INIT()        REDIRECT_ORIG_INIT()
#define FCS_ORIG_RELEASE()     REDIRECT_ORIG_RELEASE()
#define FCS_ORIG_F_INIT(_f_)   REDIRECT_ORIG_F_INIT(_f_)
#define FCS_ORIG_F(_f_)        REDIRECT_ORIG_F(_f_)
#define FCS_ORIG_F_INIT_(_f_)  REDIRECT_ORIG_F_INIT_(_f_)
#define FCS_ORIG_F_(_f_)       REDIRECT_ORIG_F_(_f_)

#else /* FCS_ORIG_ENABLED */

#define FCS_ORIG_INIT()        Z_NOP()
#define FCS_ORIG_RELEASE()     Z_NOP()
#define FCS_ORIG_F_INIT(_f_)   Z_NOP()
#define FCS_ORIG_F(_f_)        _f_
#define FCS_ORIG_F_INIT_(_f_)  Z_NOP()
#define FCS_ORIG_F_(_f_)       _f_

#endif /* FCS_ORIG_ENABLED */


#endif /* __FCS_ORIG_H__ */
