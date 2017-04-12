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


#ifndef __BLAS_ORIG_H__
#define __BLAS_ORIG_H__


#if BLAS_ORIG_ENABLED

#include "blas.h"
#include "redirect_orig.h"

REDIRECT_ORIG_DECLARE_INTRO();

#if BLAS_LEVEL1
# if BLAS_IDAMAX
REDIRECT_ORIG_DECLARE(idamax);
# endif
# if BLAS_DCOPY
REDIRECT_ORIG_DECLARE(dcopy);
# endif
# if BLAS_DSCAL
REDIRECT_ORIG_DECLARE(dscal);
# endif
# if BLAS_DAXPY
REDIRECT_ORIG_DECLARE(daxpy);
# endif
#endif
#if BLAS_LEVEL2
# if BLAS_DGER
REDIRECT_ORIG_DECLARE(dger);
# endif
# if BLAS_DGEMV
REDIRECT_ORIG_DECLARE(dgemv);
# endif
# if BLAS_DTRSV
REDIRECT_ORIG_DECLARE(dtrsv);
# endif
#endif
#if BLAS_LEVEL3
# if BLAS_DGEMM
REDIRECT_ORIG_DECLARE(dgemm);
# endif
# if BLAS_DGEMM
REDIRECT_ORIG_DECLARE(dtrsm);
# endif
#endif

REDIRECT_ORIG_DECLARE_OUTRO();

#define BLAS_ORIG_INIT()        REDIRECT_ORIG_INIT()
#define BLAS_ORIG_RELEASE()     REDIRECT_ORIG_RELEASE()
#define BLAS_ORIG_F_INIT(_f_)   REDIRECT_ORIG_F_INIT(_f_)
#define BLAS_ORIG_F(_f_)        REDIRECT_ORIG_F(_f_)
#define BLAS_ORIG_F_INIT_(_f_)  REDIRECT_ORIG_F_INIT_(_f_)
#define BLAS_ORIG_F_(_f_)       REDIRECT_ORIG_F_(_f_)

#else /* BLAS_ORIG_ENABLED */

#define BLAS_ORIG_INIT()        Z_NOP()
#define BLAS_ORIG_RELEASE()     Z_NOP()
#define BLAS_ORIG_F_INIT(_f_)   Z_NOP()
#define BLAS_ORIG_F(_f_)        _f_
#define BLAS_ORIG_F_INIT_(_f_)  Z_NOP()
#define BLAS_ORIG_F_(_f_)       _f_

#endif /* BLAS_ORIG_ENABLED */


#endif /* __BLAS_ORIG_H__ */
