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


#ifndef __SCDCMODULE_H__
#define __SCDCMODULE_H__


#include <stdint.h>

#include "scdc.h"


#if SCDCINT_IS_INT
# define PYSCDCINT_FMT  "i"
#elif SCDCINT_IS_LONG
# define PYSCDCINT_FMT  "l"
#elif SCDCINT_IS_LONG_LONG
# define PYSCDCINT_FMT  "L"
#else
# error scdcint_t type not supported!
#endif

typedef long long pyscdc_cptr_t;
#define PYSCDC_CPTR_FMT         "L"
#define PYSCDC_CPTR_TO_PY(_x_)  ((pyscdc_cptr_t) (intptr_t) (_x_))
#define PYSCDC_CPTR_TO_C(_x_)   ((void *) (intptr_t) (_x_))
#define PYSCDC_CPTR_NULL        0

typedef pyscdc_cptr_t pyscdc_dataprov_t;
#define PYSCDC_DATAPROV_FMT         PYSCDC_CPTR_FMT
#define PYSCDC_DATAPROV_TO_PY(_x_)  PYSCDC_CPTR_TO_PY(_x_)
#define PYSCDC_DATAPROV_TO_C(_x_)   ((scdc_dataprov_t) PYSCDC_CPTR_TO_C(_x_))
#define PYSCDC_DATAPROV_NULL        PYSCDC_CPTR_NULL

typedef pyscdc_cptr_t pyscdc_nodeport_t;
#define PYSCDC_NODEPORT_FMT         PYSCDC_CPTR_FMT
#define PYSCDC_NODEPORT_TO_PY(_x_)  PYSCDC_CPTR_TO_PY(_x_)
#define PYSCDC_NODEPORT_TO_C(_x_)   ((scdc_dataprov_t) PYSCDC_CPTR_TO_C(_x_))
#define PYSCDC_NODEPORT_NULL        PYSCDC_CPTR_NULL

typedef pyscdc_cptr_t pyscdc_dataset_t;
#define PYSCDC_DATASET_FMT         PYSCDC_CPTR_FMT
#define PYSCDC_DATASET_TO_PY(_x_)  PYSCDC_CPTR_TO_PY(_x_)
#define PYSCDC_DATASET_TO_C(_x_)   ((scdc_dataset_t) PYSCDC_CPTR_TO_C(_x_))
#define PYSCDC_DATASET_NULL        PYSCDC_CPTR_NULL

/*typedef pyscdc_cptr_t pyscdc_dataset_inout_t;
#define PYSCDC_DATASET_INOUT_FMT         PYSCDC_CPTR_FMT
#define PYSCDC_DATASET_INOUT_TO_PY(_x_)  PYSCDC_CPTR_TO_PY(_x_)
#define PYSCDC_DATASET_INOUT_TO_C(_x_)   PYSCDC_CPTR_TO_C(_x_)
#define PYSCDC_DATASET_INOUT_NULL        PYSCDC_CPTR_NULL*/

#define PYSCDC_ARGS_MAX  16

#define PYSCDC_PYBUF_DEFAULT_SIZE  256

#ifdef SCDC_LOGFILE
# define PYSCDC_LOGFILE  SCDC_LOGFILE
#endif

#define PYSCDC_LOG_HANDLER_LOCK      0

#define PYSCDC_LIBSCDC_ALLOW_THREADS  1


#endif /* __SCDCMODULE_H__ */
