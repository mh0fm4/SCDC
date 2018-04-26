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

  char method_name[256], *method_name_ptr = method_name; /* need separate pointer to get pointer to pointer */
  FCS_CALL(get_input_conf_char_p)(fc, "method_name", &method_name_ptr);

  TRACE_F("%s: method_name: %s", __func__, method_name);

  FCS fcs;

  FCS_ORIG_F_INIT(fcs_init);
  FCS_TIMING_START(libfcs_scdc_timing_remote[0]);
  FCSResult result = FCS_ORIG_F(MANGLE_FCS(fcs_init))(&fcs, method_name, MPI_COMM_WORLD);
  FCS_TIMING_STOP(libfcs_scdc_timing_remote[0]);

  TRACE_F("%s: result: %p", __func__, result);

  FCS_HANDLE(set_ptr)(&fh, fcs);

  FCS_CALL(set_handle)(fc, &fh);

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

  } params, *params_ptr = &params;

  FCS_CALL(get_input_param_vector_char)(fc, "params", sizeof(params), (char **) &params_ptr);

  TRACE_F("%s: handle: %p, near_field_flag: %" FCS_LMOD_INT "d, "
    "box_a: %" FCS_LMOD_FLOAT "f|%" FCS_LMOD_FLOAT "f|%" FCS_LMOD_FLOAT "f, "
    "box_b: %" FCS_LMOD_FLOAT "f|%" FCS_LMOD_FLOAT "f|%" FCS_LMOD_FLOAT "f, "
    "box_c: %" FCS_LMOD_FLOAT "f|%" FCS_LMOD_FLOAT "f|%" FCS_LMOD_FLOAT "f, "
    "box_origin: %" FCS_LMOD_FLOAT "f|%" FCS_LMOD_FLOAT "f|%" FCS_LMOD_FLOAT "f, "
    "periodicity: %" FCS_LMOD_INT "d|%" FCS_LMOD_INT "d|%" FCS_LMOD_INT "d, total_particles: %" FCS_LMOD_INT "d",
    __func__, handle, params.near_field_flag,
    params.box_a[0], params.box_a[1], params.box_a[2],
    params.box_b[0], params.box_b[1], params.box_b[2],
    params.box_c[0], params.box_c[1], params.box_c[2],
    params.box_origin[0], params.box_origin[1], params.box_origin[2],
    params.periodicity[0], params.periodicity[1], params.periodicity[2], params.total_particles
  );

  FCS_ORIG_F_INIT(fcs_set_common);
  FCS_TIMING_START(libfcs_scdc_timing_remote[0]);
  FCSResult result = FCS_ORIG_F(MANGLE_FCS(fcs_set_common))(handle,
    params.near_field_flag,
    params.box_a, params.box_b, params.box_c, params.box_origin,
    params.periodicity, params.total_particles);
  FCS_TIMING_STOP(libfcs_scdc_timing_remote[0]);

  TRACE_F("%s: result: %p", __func__, result);

  FCS_HANDLE(set_ptr)(&fh, handle);

  FCS_CALL(set_handle)(fc, &fh);

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
  {
    TRACE_F("%s: command '%s' not supported!", __func__, cmd);
  }

  FCS_CALL(release_scdc)(fc);

  TRACE_F("%s: return", __func__);

  return SCDC_SUCCESS;
}
