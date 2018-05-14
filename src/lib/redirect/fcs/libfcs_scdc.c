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


/* required for the later include of dlfct.h */
#define _GNU_SOURCE  1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "z_pack.h"

#include "fcs_scdc_config.h"
#include "common.h"
#include "fcs.h"
#include "fcs_redirect.h"
#include "fcs_scdc.h"
#include "fcs_timing.h"


#define TRACE_PREFIX  "libfcs_scdc: "


#if LIBFCS_SCDC_PREFIX

# undef __FCS_H__
# undef MANGLE_FCS
# define MANGLE_FCS(_f_)  Z_CONCAT(LIBFCS_SCDC_PREFIX_NAME, _f_)
# include "fcs.h"

# define LIB_F(_f_)      Z_CONCAT(LIBFCS_SCDC_PREFIX_NAME, _f_)
# define LIB_F_STR(_f_)  Z_STRINGIFY(Z_CONCAT(LIBFCS_SCDC_PREFIX_NAME, _f_))

#else /* LIBFCS_SCDC_PREFIX */

# define LIB_F(_f_)      _f_
# define LIB_F_STR(_f_)  Z_STRINGIFY(_f_)

# define FCS_ORIG_ENABLED  1

#endif /* LIBFCS_SCDC_PREFIX */

#include "fcs_orig.h"


const char *libfcs_scdc_local_base = 0;
scdc_dataprov_t libfcs_scdc_local_dataprov = SCDC_DATAPROV_NULL;

const char *libfcs_scdc_uri = 0;

int libfcs_scdc_initialized = -1;

#if LIBFCS_SCDC_TIMING
# undef FCS_TIMING_PREFIX
# define FCS_TIMING_PREFIX  TRACE_PREFIX  "TIMING: "
# define LIBFCS_SCDC_TIMING_X  5
int libfcs_scdc_timing_i;
double libfcs_scdc_timing[LIBFCS_SCDC_TIMING_X];
#endif
#define FCS_TIMING_INIT_()  FCS_TIMING_INIT(libfcs_scdc_timing, libfcs_scdc_timing_i, LIBFCS_SCDC_TIMING_X)

#if LIBFCS_SCDC_TIMING_REDIRECT_REMOTE
# define FCS_TIMING_REMOTE_GET(_bc_, _t_)  FCS_CALL(get_output_conf_double)(_bc_, "TIMING", _t_);
#else
# define FCS_TIMING_REMOTE_GET(_bc_, _t_)  Z_NOP()
#endif


struct FCSResult_t
{
  fcs_int error_code;
  char function[FCS_RESULT_MAX_FUNCTION_LENGTH];
  char message[FCS_RESULT_MAX_MESSAGE_LENGTH];

} libfcs_scdc_result_failure = { FCS_SUCCESS, "", "" };


static FCSResult libfcs_scdc_create_result(fcs_int error_code, const char *function, const char *message)
{
  libfcs_scdc_result_failure.error_code = error_code;
  strncpy(libfcs_scdc_result_failure.function, function, FCS_RESULT_MAX_FUNCTION_LENGTH);
  strncpy(libfcs_scdc_result_failure.message, message, FCS_RESULT_MAX_MESSAGE_LENGTH);

  return &libfcs_scdc_result_failure;
}


void LIB_F(fcs_result_destroy)(FCSResult result)
{
}


fcs_int LIB_F(fcs_result_get_return_code)(FCSResult result)
{
  if (result == FCS_RESULT_SUCCESS) return FCS_SUCCESS;

  return result->error_code;
}

const char *LIB_F(fcs_result_get_function)(FCSResult result)
{
  if (result == FCS_RESULT_SUCCESS) return "";

  return result->function;
}


const char *LIB_F(fcs_result_get_message)(FCSResult result)
{
  if (result == FCS_RESULT_SUCCESS) return "";

  return result->message;
}


void LIB_F(fcs_result_print_result)(FCSResult result)
{
}


#if LIBFCS_SCDC_ENABLED

void libfcs_scdc_release();

void libfcs_scdc_init()
{
  TRACE_F("%s: libfcs_scdc_initialized: %d", __func__, libfcs_scdc_initialized);

  if (libfcs_scdc_initialized < 0)
  {
    /* on first init */
    TRACE_F("%s: first init", __func__);

    FCS_ORIG_INIT();

    scdc_init(SCDC_INIT_DEFAULT);

#if LIBFCS_SCDC_LOCAL
    libfcs_scdc_local_base = getenv("LIFCS_SCDC_LOCAL_BASE");
    if (!libfcs_scdc_local_base) libfcs_scdc_local_base = LIBFCS_SCDC_LOCAL_BASE;

    libfcs_scdc_local_dataprov = scdc_dataprov_open(libfcs_scdc_local_base, "hook", &fcs_scdc_hook);
#endif

    libfcs_scdc_uri = getenv("LIFCS_SCDC_URI");

    if (!libfcs_scdc_uri)
    {
#if defined(LIBFCS_SCDC_URI_DEFAULT)
      libfcs_scdc_uri = LIBFCS_SCDC_URI_DEFAULT;
#elif LIBFCS_SCDC_LOCAL
      libfcs_scdc_uri = LIBFCS_SCDC_LOCAL_URI LIBFCS_SCDC_LOCAL_BASE;
#endif
    }

    /* register release handler */
    atexit(libfcs_scdc_release);

    ++libfcs_scdc_initialized;
  }

  if (libfcs_scdc_initialized == 0)
  {
    /* on every init */
    TRACE_F("%s: every init", __func__);
  }

  ++libfcs_scdc_initialized;

  TRACE_F("%s: return: libfcs_scdc_initialized: %d", __func__, libfcs_scdc_initialized);
}


void libfcs_scdc_release()
{
  --libfcs_scdc_initialized;

  if (libfcs_scdc_initialized == 0)
  {
    /* on every release */
    TRACE_F("%s: every release", __func__);
  }

  if (libfcs_scdc_initialized < 0)
  {
    /* on last release */
    TRACE_F("%s: last release", __func__);

#if LIBFCS_SCDC_LOCAL
    scdc_dataprov_close(libfcs_scdc_local_dataprov);
    libfcs_scdc_local_dataprov = SCDC_DATAPROV_NULL;
#endif

    scdc_release();

    FCS_ORIG_RELEASE();
  }
}

#endif /* LIBFCS_SCDC_ENABLED */


FCSResult LIB_F(fcs_init)(FCS *new_handle, const char* method_name, MPI_Comm communicator)
{
  TRACE_F("%s: method_name: '%s'", __func__, method_name);

  libfcs_scdc_init();

  fcs_handle_t *fh = malloc(sizeof(fcs_handle_t));

  if (!FCS_HANDLE(create_scdc)(fh, method_name, libfcs_scdc_uri)) goto do_return;

  fcs_call_t fc;

  if (!FCS_CALL(create_scdc)(&fc, "fcs_init", NULL)) goto do_return;

  FCS_CALL(set_handle)(&fc, fh);
  FCS_CALL(put_input_conf_str)(&fc, "method_name", method_name, -1);

  if (!FCS_CALL(execute)(&fc)) goto do_return;

  FCS_CALL(get_handle)(&fc, fh);

  fcs_int return_code;
  FCS_CALL_INT(get_output_conf)(&fc, "return_code", &return_code);

  FCSResult res = FCS_RESULT_SUCCESS;
  if (return_code != FCS_SUCCESS)
  {
    res = libfcs_scdc_create_result(return_code, __func__, FCS_CALL(get_result(&fc, NULL, 0)));
    TRACE_F("%s: return_code: %" FCS_LMOD_INT "d, function: '%s', message: '%s'", __func__, res->error_code, res->function, res->message);
  }

  FCS_CALL(destroy_scdc)(&fc);

  libfcs_scdc_release();

  *new_handle = fh;

do_return:

  if (res != FCS_RESULT_SUCCESS)
  {
    FCS_HANDLE(destroy_scdc)(fh);
    free(fh);
  }

  TRACE_F("%s: return: %p", __func__, res);

  return res;
}


FCSResult LIB_F(fcs_destroy)(FCS handle)
{
  TRACE_F("%s: handle: %p", __func__, handle);

  libfcs_scdc_init();

  fcs_handle_t *fh = handle;

  fcs_call_t fc;

  if (!FCS_CALL(create_scdc)(&fc, "fcs_destroy", NULL)) goto do_return;

  FCS_CALL(set_handle)(&fc, fh);

  if (!FCS_CALL(execute)(&fc)) goto do_return;

  FCS_CALL(get_handle)(&fc, fh);

  fcs_int return_code;
  FCS_CALL_INT(get_output_conf)(&fc, "return_code", &return_code);

  FCSResult res = FCS_RESULT_SUCCESS;
  if (return_code != FCS_SUCCESS)
  {
    res = libfcs_scdc_create_result(return_code, __func__, FCS_CALL(get_result(&fc, NULL, 0)));
    TRACE_F("%s: return_code: %" FCS_LMOD_INT "d, function: '%s', message: '%s'", __func__, res->error_code, res->function, res->message);
  }

  FCS_CALL(destroy_scdc)(&fc);

  FCS_HANDLE(destroy_scdc)(fh);
  free(fh);

  libfcs_scdc_release();

do_return:

  TRACE_F("%s: return: %p", __func__, res);

  return res;
}


FCSResult LIB_F(fcs_set_common)(FCS handle,
  fcs_int near_field_flag,
  const fcs_float *box_a, const fcs_float *box_b, const fcs_float *box_c,
  const fcs_float *box_origin,
  const fcs_int *periodicity, fcs_int total_particles)
{
  TRACE_F("%s: handle: %p, near_field_flag: %" FCS_LMOD_INT "d, "
    "box_a: %" FCS_LMOD_FLOAT "f|%" FCS_LMOD_FLOAT "f|%" FCS_LMOD_FLOAT "f, "
    "box_b: %" FCS_LMOD_FLOAT "f|%" FCS_LMOD_FLOAT "f|%" FCS_LMOD_FLOAT "f, "
    "box_c: %" FCS_LMOD_FLOAT "f|%" FCS_LMOD_FLOAT "f|%" FCS_LMOD_FLOAT "f, "
    "box_origin: %" FCS_LMOD_FLOAT "f|%" FCS_LMOD_FLOAT "f|%" FCS_LMOD_FLOAT "f, "
    "periodicity: %" FCS_LMOD_INT "d|%" FCS_LMOD_INT "d|%" FCS_LMOD_INT "d, total_particles: %" FCS_LMOD_INT "d",
    __func__, handle, near_field_flag,
    box_a[0], box_a[1], box_a[2], box_b[0], box_b[1], box_b[2], box_c[0], box_c[1], box_c[2],
    box_origin[0], box_origin[1], box_origin[2],
    periodicity[0], periodicity[1], periodicity[2], total_particles
  );

  libfcs_scdc_init();

  fcs_handle_t *fh = handle;

  fcs_call_t fc;

  if (!FCS_CALL(create_scdc)(&fc, "fcs_set_common", NULL)) goto do_return;

  FCS_CALL(set_handle)(&fc, fh);

  struct {
    fcs_int near_field_flag;
    fcs_float box_a[3], box_b[3], box_c[3], box_origin[3];
    fcs_int periodicity[3], total_particles;

  } params;

  params.near_field_flag = near_field_flag;
  params.box_a[0] = box_a[0];  params.box_a[1] = box_a[1];  params.box_a[2] = box_a[2];
  params.box_b[0] = box_b[0];  params.box_b[1] = box_b[1];  params.box_b[2] = box_b[2];
  params.box_c[0] = box_c[0];  params.box_c[1] = box_c[1];  params.box_c[2] = box_c[2];
  params.box_origin[0] = box_origin[0];  params.box_origin[1] = box_origin[1];  params.box_origin[2] = box_origin[2];
  params.periodicity[0] = periodicity[0];  params.periodicity[1] = periodicity[1];  params.periodicity[2] = periodicity[2];
  params.total_particles = total_particles;

  FCS_CALL(put_input_param_bytes)(&fc, "params", &params, sizeof(params));

  if (!FCS_CALL(execute)(&fc)) goto do_return;

  FCS_CALL(get_handle)(&fc, fh);

  fcs_int return_code;
  FCS_CALL_INT(get_output_conf)(&fc, "return_code", &return_code);

  FCSResult res = FCS_RESULT_SUCCESS;
  if (return_code != FCS_SUCCESS)
  {
    res = libfcs_scdc_create_result(return_code, __func__, FCS_CALL(get_result(&fc, NULL, 0)));
    TRACE_F("%s: return_code: %" FCS_LMOD_INT "d, function: '%s', message: '%s'", __func__, res->error_code, res->function, res->message);
  }

  FCS_CALL(destroy_scdc)(&fc);

  libfcs_scdc_release();

do_return:

  TRACE_F("%s: return: %p", __func__, res);

  return res;
}


FCSResult LIB_F(fcs_set_parameters)(FCS handle, const char *parameters, fcs_int continue_on_errors)
{
  TRACE_F("%s: handle: %p, parameters: '%s', continue_on_errors: %" FCS_LMOD_INT "d", __func__, handle, parameters, continue_on_errors);

  libfcs_scdc_init();

  fcs_handle_t *fh = handle;

  fcs_call_t fc;

  if (!FCS_CALL(create_scdc)(&fc, "fcs_set_parameters", NULL)) goto do_return;

  FCS_CALL(set_handle)(&fc, fh);

  FCS_CALL(put_input_conf_str)(&fc, "parameters", parameters, -1);
  FCS_CALL_INT(put_input_conf)(&fc, "continue_on_errors", continue_on_errors);

  if (!FCS_CALL(execute)(&fc)) goto do_return;

  FCS_CALL(get_handle)(&fc, fh);

  fcs_int return_code;
  FCS_CALL_INT(get_output_conf)(&fc, "return_code", &return_code);

  FCSResult res = FCS_RESULT_SUCCESS;
  if (return_code != FCS_SUCCESS)
  {
    res = libfcs_scdc_create_result(return_code, __func__, FCS_CALL(get_result(&fc, NULL, 0)));
    TRACE_F("%s: return_code: %" FCS_LMOD_INT "d, function: '%s', message: '%s'", __func__, res->error_code, res->function, res->message);
  }

  FCS_CALL(destroy_scdc)(&fc);

  libfcs_scdc_release();

do_return:

  TRACE_F("%s: return: %p", __func__, res);

  return res;
}


FCSResult LIB_F(fcs_tune)(FCS handle, fcs_int local_particles, fcs_float *positions, fcs_float *charges)
{
  TRACE_F("%s: handle: %p, local_particles: %" FCS_LMOD_INT "d, positions: %p, charges: %p", __func__, handle, local_particles, positions, charges);

  libfcs_scdc_init();

  fcs_handle_t *fh = handle;

  fcs_call_t fc;

  if (!FCS_CALL(create_scdc)(&fc, "fcs_tune", NULL)) goto do_return;

  FCS_CALL(set_handle)(&fc, fh);

  FCS_CALL(put_input_conf_int)(&fc, "local_particles", local_particles);
  FCS_CALL_FLOAT(put_input_param_array)(&fc, "positions", positions, 3 * local_particles);
  FCS_CALL_FLOAT(put_input_param_array)(&fc, "charges", charges, local_particles);

  if (!FCS_CALL(execute)(&fc)) goto do_return;

  FCS_CALL(get_handle)(&fc, fh);

  fcs_int return_code;
  FCS_CALL_INT(get_output_conf)(&fc, "return_code", &return_code);

  FCSResult res = FCS_RESULT_SUCCESS;
  if (return_code != FCS_SUCCESS)
  {
    res = libfcs_scdc_create_result(return_code, __func__, FCS_CALL(get_result(&fc, NULL, 0)));
    TRACE_F("%s: return_code: %" FCS_LMOD_INT "d, function: '%s', message: '%s'", __func__, res->error_code, res->function, res->message);
  }

  FCS_CALL(destroy_scdc)(&fc);

  libfcs_scdc_release();

do_return:

  TRACE_F("%s: return: %p", __func__, res);

  return res;
}


FCSResult LIB_F(fcs_run)(FCS handle, fcs_int local_particles, fcs_float *positions, fcs_float *charges, fcs_float *field, fcs_float *potentials)
{
  TRACE_F("%s: handle: %p, local_particles: %" FCS_LMOD_INT "d, positions: %p, charges: %p, field: %p, potentials: %p", __func__, handle, local_particles, positions, charges, field, potentials);

  libfcs_scdc_init();

  fcs_handle_t *fh = handle;

  fcs_call_t fc;

  if (!FCS_CALL(create_scdc)(&fc, "fcs_run", NULL)) goto do_return;

  FCS_CALL(set_handle)(&fc, fh);

  FCS_CALL(put_input_conf_int)(&fc, "local_particles", local_particles);

  FCS_CALL_FLOAT(put_input_param_array)(&fc, "positions", positions, 3 * local_particles);
  FCS_CALL_FLOAT(put_input_param_array)(&fc, "charges", charges, local_particles);

  FCS_CALL_FLOAT(put_output_param_array)(&fc, "field", field, 3 * local_particles);
  FCS_CALL_FLOAT(put_output_param_array)(&fc, "potentials", potentials, local_particles);

  if (!FCS_CALL(execute)(&fc)) goto do_return;

  fcs_float *field_ = field,  *potentials_ = potentials;
  rdint_t n_field_ = 3 * local_particles;
  rdint_t n_potentials_ = local_particles;
  FCS_CALL_FLOAT(get_output_param_array)(&fc, "field", &field_, &n_field_);
  FCS_CALL_FLOAT(get_output_param_array)(&fc, "potentials", &potentials_, &n_potentials_);

  ASSERT(n_field_ == 3 * local_particles && field_ == field);
  ASSERT(n_potentials_ == local_particles && potentials_ == potentials);

  FCS_CALL(get_handle)(&fc, fh);

  fcs_int return_code;
  FCS_CALL_INT(get_output_conf)(&fc, "return_code", &return_code);

  FCSResult res = FCS_RESULT_SUCCESS;
  if (return_code != FCS_SUCCESS)
  {
    res = libfcs_scdc_create_result(return_code, __func__, FCS_CALL(get_result(&fc, NULL, 0)));
    TRACE_F("%s: return_code: %" FCS_LMOD_INT "d, function: '%s', message: '%s'", __func__, res->error_code, res->function, res->message);
  }

  FCS_CALL(destroy_scdc)(&fc);

  libfcs_scdc_release();

do_return:

  TRACE_F("%s: return: %p", __func__, res);

  return res;
}
