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


#ifndef __FCS_H__
#define __FCS_H__


#if USE_MPI

# include <mpi.h>

#else /* USE_MPI */

# include "redirect_mpi.h"
typedef redirect_MPI_Comm MPI_Comm;
#define MPI_COMM_NULL   REDIRECT_MPI_COMM_NULL
#define MPI_COMM_WORLD  REDIRECT_MPI_COMM_WORLD

#endif /* USE_MPI */


#include "fcs_config.h"


#ifndef MANGLE_FCS
# define MANGLE_FCS(_f_)  _f_
#endif


#define DECLARE_PROTOTYPE(_r_, _f_, _p_...) \
  typedef _r_ MANGLE_FCS(_f_ ## _f)(_p_); \
  MANGLE_FCS(_f_ ## _f) MANGLE_FCS(_f_), MANGLE_FCS(_f_ ## _)


typedef void *FCS;
#define FCS_NULL  NULL

#define FCS_RESULT_MAX_FUNCTION_LENGTH   64
#define FCS_RESULT_MAX_MESSAGE_LENGTH   512

struct FCSResult_t;

typedef struct FCSResult_t *FCSResult;

extern struct FCSResult_t libfcs_scdc_result_failure;

#define FCS_RESULT_SUCCESS  NULL
#define FCS_RESULT_FAILURE  (FCSResult)&libfcs_scdc_result_failure

#define FCS_ERROR    -1
#define FCS_SUCCESS  0


DECLARE_PROTOTYPE(FCSResult, fcs_init, FCS *new_handle, const char* method_name, MPI_Comm communicator);

DECLARE_PROTOTYPE(FCSResult, fcs_destroy, FCS handle);

DECLARE_PROTOTYPE(FCSResult, fcs_set_common, FCS handle,
  fcs_int near_field_flag,
  const fcs_float *box_a, const fcs_float *box_b, const fcs_float *box_c,
  const fcs_float *box_origin,
  const fcs_int *periodicity, fcs_int total_particles);

DECLARE_PROTOTYPE(FCSResult, fcs_set_parameters, FCS handle, const char *parameters, fcs_int continue_on_errors);

DECLARE_PROTOTYPE(FCSResult, fcs_tune, FCS handle, fcs_int local_particles, fcs_float *positions, fcs_float *charges);

DECLARE_PROTOTYPE(FCSResult, fcs_run, FCS handle, fcs_int local_particles, fcs_float *positions, fcs_float *charges, fcs_float *field, fcs_float *potentials);

DECLARE_PROTOTYPE(void, fcs_result_destroy, FCSResult result);
DECLARE_PROTOTYPE(fcs_int, fcs_result_get_return_code, FCSResult result);
DECLARE_PROTOTYPE(const char *, fcs_result_get_function, FCSResult result);
DECLARE_PROTOTYPE(const char *, fcs_result_get_message, FCSResult result);
DECLARE_PROTOTYPE(void, fcs_result_print_result, FCSResult result);


#endif /* __FCS_H__ */
