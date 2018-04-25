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

#include "common.h"
#include "z_pack.h"
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
#if defined(LIBFCS_SCDC_URI)
      libfcs_scdc_uri = LIBFCS_SCDC_URI;
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
  TRACE_F("%s: method_name: %s", __func__, method_name);

  FCSResult res = FCS_RESULT_FAILURE;

  libfcs_scdc_init();

  fcs_handle_t *fh = malloc(sizeof(fcs_handle_t));

  if (!FCS_HANDLE(create_scdc)(fh, method_name, libfcs_scdc_uri)) goto do_return;

  fcs_call_t fc;

  if (!FCS_CALL(create_scdc)(&fc, "fcs_init", NULL)) goto do_return;

  FCS_CALL(set_handle)(&fc, fh);
  FCS_CALL(put_input_conf_char_p)(&fc, "method_name", (char *) method_name);

  if (!FCS_CALL(execute)(&fc)) goto do_return;

  FCS_CALL(get_handle)(&fc, fh);

  FCS_CALL(destroy_scdc)(&fc);

  libfcs_scdc_release();

  res = FCS_RESULT_SUCCESS;
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

  FCSResult res = FCS_RESULT_FAILURE;

  libfcs_scdc_init();

  fcs_handle_t *fh = handle;

  fcs_call_t fc;

  if (!FCS_CALL(create_scdc)(&fc, "fcs_destroy", NULL)) goto do_return;

  FCS_CALL(set_handle)(&fc, fh);

  if (!FCS_CALL(execute)(&fc)) goto do_return;

  FCS_CALL(get_handle)(&fc, fh);

  FCS_CALL(destroy_scdc)(&fc);

  FCS_HANDLE(destroy_scdc)(fh);
  free(fh);

  libfcs_scdc_release();

  res = FCS_RESULT_SUCCESS;

do_return:

  TRACE_F("%s: return: %p", __func__, res);

  return res;
}
