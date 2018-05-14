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

#include <mpi.h>

#define LIBFCS_SCDC_REMOTE  1

#include "fcs_scdc_config.h"
#include "common.h"
#include "fcs.h"
#include "fcs_redirect.h"
#include "fcs_scdc.h"
#include "fcs_timing.h"


#define TRACE_PREFIX  "fcs_scdc: "


#if LIBFCS_SCDC_PREFIX

#else /* LIBFCS_SCDC_PREFIX */

# define FCS_ORIG_ENABLED  1

#endif /* LIBFCS_SCDC_PREFIX */

#include "fcs_orig.h"


#if LIBFCS_SCDC_TIMING_REMOTE
# undef FCS_TIMING_PREFIX
# define FCS_TIMING_PREFIX  TRACE_PREFIX  "TIMING: "
# define LIBFCS_SCDC_TIMING_REMOTE_X  5
int libfcs_scdc_timing_remote_i;
double libfcs_scdc_timing_remote[LIBFCS_SCDC_TIMING_REMOTE_X];
#endif
#define FCS_TIMING_INIT_()  FCS_TIMING_INIT(libfcs_scdc_timing_remote, libfcs_scdc_timing_remote_i, LIBFCS_SCDC_TIMING_REMOTE_X)

#if LIBFCS_SCDC_TIMING_REDIRECT_REMOTE
# define FCS_TIMING_REMOTE_PUT(_fc_, _t_)  FCS_CALL(put_output_conf_double)(_fc_, "TIMING", _t_);
#else
# define FCS_TIMING_REMOTE_PUT(_fc_, _t_)  Z_NOP()
#endif


static void do_fcs_init(fcs_call_t *fc)
{
  TRACE_F("%s: %p", __func__, fc);

  FCS_TIMING_INIT_();

  fcs_handle_t fh;

  FCS_CALL(get_handle)(fc, &fh);

  char method_name[256];
  FCS_CALL(get_input_conf_str)(fc, "method_name", method_name, NULL);

  TRACE_F("%s: method_name: '%s'", __func__, method_name);

  FCS fcs;

  FCS_ORIG_F_INIT(fcs_init);
  FCS_TIMING_START(libfcs_scdc_timing_remote[0]);
  FCSResult result = FCS_ORIG_F(MANGLE_FCS(fcs_init))(&fcs, method_name, MPI_COMM_WORLD);
  FCS_TIMING_STOP(libfcs_scdc_timing_remote[0]);

  TRACE_F("%s: result: %p", __func__, result);

  FCS_HANDLE(set_ptr)(&fh, fcs);

  FCS_CALL(set_handle)(fc, &fh);

  fcs_int return_code = fcs_result_get_return_code(result);
  FCS_CALL_INT(put_output_conf)(fc, "return_code", return_code);
  if (return_code != FCS_SUCCESS)
  {
    const char *message = fcs_result_get_message(result);
    FCS_CALL(set_result)(fc, message);

    TRACE_F("%s: return_code: %" FCS_LMOD_INT "d, message: '%s'", __func__, return_code, message);
  }

  FCS_TIMING_REMOTE_PUT(fc, libfcs_scdc_timing_remote[0]);

  FCS_TIMING_PRINT_F("%s: %f", __func__, libfcs_scdc_timing_remote[0]);

  TRACE_F("%s: return", __func__);
}


static void do_fcs_destroy(fcs_call_t *fc)
{
  TRACE_F("%s: %p", __func__, fc);

  FCS_TIMING_INIT_();

  fcs_handle_t fh;

  FCS_CALL(get_handle)(fc, &fh);

  FCS handle;
  FCS_HANDLE(get_ptr)(&fh, &handle);

  FCS_ORIG_F_INIT(fcs_destroy);
  FCS_TIMING_START(libfcs_scdc_timing_remote[0]);
  FCSResult result = FCS_ORIG_F(MANGLE_FCS(fcs_destroy))(handle);
  handle = FCS_NULL;
  FCS_TIMING_STOP(libfcs_scdc_timing_remote[0]);

  TRACE_F("%s: result: %p", __func__, result);

  FCS_HANDLE(set_ptr)(&fh, handle);

  FCS_CALL(set_handle)(fc, &fh);

  fcs_int return_code = fcs_result_get_return_code(result);
  FCS_CALL_INT(put_output_conf)(fc, "return_code", return_code);
  if (return_code != FCS_SUCCESS)
  {
    const char *message = fcs_result_get_message(result);
    FCS_CALL(set_result)(fc, message);

    TRACE_F("%s: return_code: %" FCS_LMOD_INT "d, message: '%s'", __func__, return_code, message);
  }

  FCS_TIMING_REMOTE_PUT(fc, libfcs_scdc_timing_remote[0]);

  FCS_TIMING_PRINT_F("%s: %f", __func__, libfcs_scdc_timing_remote[0]);

  TRACE_F("%s: return", __func__);
}


static void do_fcs_set_common(fcs_call_t *fc)
{
  TRACE_F("%s: %p", __func__, fc);

  FCS_TIMING_INIT_();

  fcs_handle_t fh;

  FCS_CALL(get_handle)(fc, &fh);

  FCS handle;
  FCS_HANDLE(get_ptr)(&fh, &handle);

  struct {
    fcs_int near_field_flag;
    fcs_float box_a[3], box_b[3], box_c[3], box_origin[3];
    fcs_int periodicity[3], total_particles;

  } *params = NULL;
  rdint_t n_params = 0;

  FCS_CALL(get_input_param_bytes)(fc, "params", (void **) &params, &n_params);

  ASSERT(n_params == sizeof(*params));

  TRACE_F("%s: handle: %p, near_field_flag: %" FCS_LMOD_INT "d, "
    "box_a: %" FCS_LMOD_FLOAT "f|%" FCS_LMOD_FLOAT "f|%" FCS_LMOD_FLOAT "f, "
    "box_b: %" FCS_LMOD_FLOAT "f|%" FCS_LMOD_FLOAT "f|%" FCS_LMOD_FLOAT "f, "
    "box_c: %" FCS_LMOD_FLOAT "f|%" FCS_LMOD_FLOAT "f|%" FCS_LMOD_FLOAT "f, "
    "box_origin: %" FCS_LMOD_FLOAT "f|%" FCS_LMOD_FLOAT "f|%" FCS_LMOD_FLOAT "f, "
    "periodicity: %" FCS_LMOD_INT "d|%" FCS_LMOD_INT "d|%" FCS_LMOD_INT "d, total_particles: %" FCS_LMOD_INT "d",
    __func__, handle, params->near_field_flag,
    params->box_a[0], params->box_a[1], params->box_a[2],
    params->box_b[0], params->box_b[1], params->box_b[2],
    params->box_c[0], params->box_c[1], params->box_c[2],
    params->box_origin[0], params->box_origin[1], params->box_origin[2],
    params->periodicity[0], params->periodicity[1], params->periodicity[2], params->total_particles
  );

  FCS_ORIG_F_INIT(fcs_set_common);
  FCS_TIMING_START(libfcs_scdc_timing_remote[0]);
  FCSResult result = FCS_ORIG_F(MANGLE_FCS(fcs_set_common))(handle,
    params->near_field_flag,
    params->box_a, params->box_b, params->box_c, params->box_origin,
    params->periodicity, params->total_particles);
  FCS_TIMING_STOP(libfcs_scdc_timing_remote[0]);

  TRACE_F("%s: result: %p", __func__, result);

  FCS_HANDLE(set_ptr)(&fh, handle);

  FCS_CALL(set_handle)(fc, &fh);

  fcs_int return_code = fcs_result_get_return_code(result);
  FCS_CALL_INT(put_output_conf)(fc, "return_code", return_code);
  if (return_code != FCS_SUCCESS)
  {
    const char *message = fcs_result_get_message(result);
    FCS_CALL(set_result)(fc, message);

    TRACE_F("%s: return_code: %" FCS_LMOD_INT "d, message: '%s'", __func__, return_code, message);
  }

  FCS_TIMING_REMOTE_PUT(fc, libfcs_scdc_timing_remote[0]);

  FCS_TIMING_PRINT_F("%s: %f", __func__, libfcs_scdc_timing_remote[0]);

  TRACE_F("%s: return", __func__);
}


static void do_fcs_set_parameters(fcs_call_t *fc)
{
  TRACE_F("%s: %p", __func__, fc);

  FCS_TIMING_INIT_();

  fcs_handle_t fh;

  FCS_CALL(get_handle)(fc, &fh);

  FCS handle;
  FCS_HANDLE(get_ptr)(&fh, &handle);

  char parameters[256] = { '\0' };
  FCS_CALL(get_input_conf_str)(fc, "parameters", parameters, NULL);

  int continue_on_errors;
  FCS_CALL_INT(get_input_conf)(fc, "continue_on_errors", &continue_on_errors);

  TRACE_F("%s: handle: %p, parameters: '%s', continue_on_errors: %" FCS_LMOD_INT "d", __func__, handle, parameters, continue_on_errors);

  FCS_ORIG_F_INIT(fcs_set_common);
  FCS_TIMING_START(libfcs_scdc_timing_remote[0]);
  FCSResult result = FCS_ORIG_F(MANGLE_FCS(fcs_set_parameters))(handle, parameters, continue_on_errors);
  FCS_TIMING_STOP(libfcs_scdc_timing_remote[0]);

  TRACE_F("%s: result: %p", __func__, result);

  FCS_HANDLE(set_ptr)(&fh, handle);

  FCS_CALL(set_handle)(fc, &fh);

  fcs_int return_code = fcs_result_get_return_code(result);
  FCS_CALL_INT(put_output_conf)(fc, "return_code", return_code);
  if (return_code != FCS_SUCCESS)
  {
    const char *message = fcs_result_get_message(result);
    FCS_CALL(set_result)(fc, message);

    TRACE_F("%s: return_code: %" FCS_LMOD_INT "d, message: '%s'", __func__, return_code, message);
  }

  FCS_TIMING_REMOTE_PUT(fc, libfcs_scdc_timing_remote[0]);

  FCS_TIMING_PRINT_F("%s: %f", __func__, libfcs_scdc_timing_remote[0]);

  TRACE_F("%s: return", __func__);
}


static void do_fcs_tune(fcs_call_t *fc)
{
  TRACE_F("%s: %p", __func__, fc);

  FCS_TIMING_INIT_();

  fcs_handle_t fh;

  FCS_CALL(get_handle)(fc, &fh);

  FCS handle;
  FCS_HANDLE(get_ptr)(&fh, &handle);

  fcs_int local_particles;
  FCS_CALL(get_input_conf_int)(fc, "local_particles", &local_particles);

  fcs_float *positions = 0, *charges = 0;
  rdint_t n_positions = 0, n_charges = 0;
  FCS_CALL_FLOAT(get_input_param_array)(fc, "positions", &positions, &n_positions);
  FCS_CALL_FLOAT(get_input_param_array)(fc, "charges", &charges, &n_charges);

  ASSERT(n_positions == 3 * local_particles && n_charges == local_particles);

  TRACE_F("%s: handle: %p, local_particles: %" FCS_LMOD_INT "d, positions: %p, charges: %p", __func__, handle, local_particles, positions, charges);

  FCS_ORIG_F_INIT(fcs_set_common);
  FCS_TIMING_START(libfcs_scdc_timing_remote[0]);
  FCSResult result = FCS_ORIG_F(MANGLE_FCS(fcs_tune))(handle, local_particles, positions, charges);
  FCS_TIMING_STOP(libfcs_scdc_timing_remote[0]);

  TRACE_F("%s: result: %p", __func__, result);

  FCS_HANDLE(set_ptr)(&fh, handle);

  FCS_CALL(set_handle)(fc, &fh);

  fcs_int return_code = fcs_result_get_return_code(result);
  FCS_CALL_INT(put_output_conf)(fc, "return_code", return_code);
  if (return_code != FCS_SUCCESS)
  {
    const char *message = fcs_result_get_message(result);
    FCS_CALL(set_result)(fc, message);

    TRACE_F("%s: return_code: %" FCS_LMOD_INT "d, message: '%s'", __func__, return_code, message);
  }

  FCS_TIMING_REMOTE_PUT(fc, libfcs_scdc_timing_remote[0]);

  FCS_TIMING_PRINT_F("%s: %f", __func__, libfcs_scdc_timing_remote[0]);

  TRACE_F("%s: return", __func__);
}


static void do_fcs_run(fcs_call_t *fc)
{
  TRACE_F("%s: %p", __func__, fc);

  FCS_TIMING_INIT_();

  fcs_handle_t fh;

  FCS_CALL(get_handle)(fc, &fh);

  FCS handle;
  FCS_HANDLE(get_ptr)(&fh, &handle);

  fcs_int local_particles;
  FCS_CALL(get_input_conf_int)(fc, "local_particles", &local_particles);

  fcs_float *positions = 0, *charges = 0;
  rdint_t n_positions = 0, n_charges = 0;
  FCS_CALL_FLOAT(get_input_param_array)(fc, "positions", &positions, &n_positions);
  FCS_CALL_FLOAT(get_input_param_array)(fc, "charges", &charges, &n_charges);

  ASSERT(n_positions == 3 * local_particles && n_charges == local_particles);

  fcs_float *field = 0, *potentials = 0;
  rdint_t n_field = 0, n_potentials = 0;
  FCS_CALL_FLOAT(get_output_param_array)(fc, "field", &field, &n_field);
  FCS_CALL_FLOAT(get_output_param_array)(fc, "potentials", &potentials, &n_potentials);

  ASSERT(n_field == 3 * local_particles && n_potentials == local_particles);

  TRACE_F("%s: handle: %p, local_particles: %" FCS_LMOD_INT "d, positions: %p, charges: %p, field: %p, potentials: %p", __func__, handle, local_particles, positions, charges, field, potentials);

  FCS_ORIG_F_INIT(fcs_set_common);
  FCS_TIMING_START(libfcs_scdc_timing_remote[0]);
  FCSResult result = FCS_ORIG_F(MANGLE_FCS(fcs_run))(handle, local_particles, positions, charges, field, potentials);
  FCS_TIMING_STOP(libfcs_scdc_timing_remote[0]);

  TRACE_F("%s: result: %p", __func__, result);

  FCS_CALL_FLOAT(put_output_param_array)(fc, "field", field, 3 * local_particles);
  FCS_CALL_FLOAT(put_output_param_array)(fc, "potentials", potentials, local_particles);

  FCS_HANDLE(set_ptr)(&fh, handle);

  FCS_CALL(set_handle)(fc, &fh);

  fcs_int return_code = fcs_result_get_return_code(result);
  FCS_CALL_INT(put_output_conf)(fc, "return_code", return_code);
  if (return_code != FCS_SUCCESS)
  {
    const char *message = fcs_result_get_message(result);
    FCS_CALL(set_result)(fc, message);

    TRACE_F("%s: return_code: %" FCS_LMOD_INT "d, message: '%s'", __func__, return_code, message);
  }

  FCS_TIMING_REMOTE_PUT(fc, libfcs_scdc_timing_remote[0]);

  FCS_TIMING_PRINT_F("%s: %f", __func__, libfcs_scdc_timing_remote[0]);

  TRACE_F("%s: return", __func__);
}


void *fcs_scdc_open(const char *conf, va_list ap)
{
  TRACE_F("%s: conf: '%s'", __func__, conf);

  FCS_ORIG_INIT();

  fcs_call_t *fc = malloc(sizeof(fcs_call_t));

  TRACE_F("%s: return: %p", __func__, fc);

  return fc;
}


scdcint_t fcs_scdc_close(void *dataprov)
{
  TRACE_F("%s: dataprov: %p", __func__, dataprov);

  fcs_call_t *fc = dataprov;

  free(fc);

  FCS_ORIG_RELEASE();

  TRACE_F("%s: return", __func__);

  return SCDC_SUCCESS;
}


/*void *fcs_scdc_dataset_open(void *dataprov, const char *path)
{
  TRACE_F("%s:fcs_scdc_dataset_open:\n");
  TRACE_F("%s:  dataprov: '%p'\n", dataprov);
  TRACE_F("%s:  path: '%s'\n", path);

  fcs_call_t *fc = malloc(sizeof(fcs_call_t));

  TRACE_F("%s:  fc: '%p'\n", path);

  return fc;
}


scdcint_t fcs_scdc_dataset_close(void *dataprov, void *dataset)
{
  TRACE_F("%s:fcs_scdc_dataset_close:\n");
  TRACE_F("%s:  dataprov: '%p'\n", dataprov);
  TRACE_F("%s:  dataset: '%p'\n", dataset);

  fcs_call_t *fc = dataset;

  free(fc);

  return SCDC_SUCCESS;
}*/


scdcint_t fcs_scdc_dataset_cmd(void *dataprov, void *dataset, const char *cmd, const char *params, scdc_dataset_input_t *input, scdc_dataset_output_t *output)
{
  TRACE_F("%s: dataprov: %p, dataset: %p, cmd: '%s', params: '%s', input: %p, output: %p", __func__, dataprov, dataset, cmd, params, input, output);

  printf("input: "); scdc_dataset_input_print(input); printf("\n");
  printf("output: "); scdc_dataset_output_print(output); printf("\n");

  fcs_call_t *fc = dataprov;

  FCS_CALL(init_scdc)(fc, params, input, output);

  if (strcmp(cmd, "fcs_init") == 0) do_fcs_init(fc); else
  if (strcmp(cmd, "fcs_destroy") == 0) do_fcs_destroy(fc); else
  if (strcmp(cmd, "fcs_set_common") == 0) do_fcs_set_common(fc); else
  if (strcmp(cmd, "fcs_set_parameters") == 0) do_fcs_set_parameters(fc); else
  if (strcmp(cmd, "fcs_tune") == 0) do_fcs_tune(fc); else
  if (strcmp(cmd, "fcs_run") == 0) do_fcs_run(fc); else
  {
    TRACE_F("%s: command '%s' not supported!", __func__, cmd);
  }

  FCS_CALL(release_scdc)(fc);

  TRACE_F("%s: return", __func__);

  return SCDC_SUCCESS;
}
