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


#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "common.h"
#include "redirect_data.h"
#include "redirect_call.h"


#define TRACE_PREFIX  "redirect_call: "


#define TRACE_DATA  0

#ifndef HAVE_TRACE
# undef TRACE_DATA
#endif


static const char *FMTS[][3] = {
#define FMT_CHAR  0
  { "%c", "%c", 0 },
#define FMT_INT  1
  { "%d", "%d", 0 },
#define FMT_FLOAT  2
  { "%.8f", "%f", 0 },
#define FMT_DOUBLE  3
  { "%.16e", "%lf", 0 },
#define FMT_VOID_PTR  4
  { "%p", "%p", 0 },
#define FMT_CHAR_PTR  5
  { "%s", "%s", "*" },
};

#define PRINTF_FMT(_f_)      FMTS[_f_][0]
#define SCANF_FMT(_f_)       FMTS[_f_][1]
#define SCANF_PTR(_f_, _v_)  ((FMTS[_f_][2] == 0)?static_cast<void *>(_v_):static_cast<void *>(*reinterpret_cast<void **>(_v_)))


#if 0

template<typename T, int F>
static int sprintf_val(char *s, T v)
{
  return sprintf(s, PRINTF_FMT(F), v);
}


template<typename T>
static int sprintf_val(char *s, T v);


template<>
int sprintf_val<float>(char *s, float v)
{
  return sprintf_val<float, FMT_FLOAT>(s, v);
}


template<>
int sprintf_val<double>(char *s, double v)
{
  return sprintf_val<double, FMT_DOUBLE>(s, v);
}

#endif


static rdint_t get_blocks_cont_size(rdint_t count, rdint_t size, rdint_t stride)
{
  if (count == 0 || size == 0) return 0;

  return (count - 1) * stride + size;
}


static rdint_t get_matrix_elem_size(rdint_t nr, rdint_t nc, rdint_t ld, int rcm)
{
  switch (RCM_GET_TYPE(rcm))
  {
    case RCM_TYPE_DENSE:
    case RCM_TYPE_TRIANGULAR_UPPER:
    case RCM_TYPE_TRIANGULAR_LOWER:
      return nr * nc;
  }

  return 0;
}


static rdint_t get_matrix_cont_size(rdint_t nr, rdint_t nc, rdint_t ld, int rcm)
{
  switch (RCM_GET_TYPE(rcm))
  {
    case RCM_TYPE_DENSE:
    case RCM_TYPE_TRIANGULAR_UPPER:
    case RCM_TYPE_TRIANGULAR_LOWER:
      if (nr == 0 || nc == 0) return 0;
      if (RCM_IS_SET(rcm, RCM_ORDER_COL_MAJOR))
      {
        /* column-major order: nc-1 full columns of size ld + one column of size nr */
        return (nc - 1) * ld + nr;
      }
      if (RCM_IS_SET(rcm, RCM_ORDER_ROW_MAJOR))
      {
        /* row-major order: nr-1 full rows of size ld + one row of size nc */
        return (nr - 1) * ld + nc;
      }
  }

  return 0;
}


static rdint_t get_matrix_cont_index(rdint_t nr, rdint_t nc, rdint_t ld, int rcm, rdint_t r, rdint_t c)
{
  switch (RCM_GET_TYPE(rcm))
  {
    case RCM_TYPE_DENSE:
    case RCM_TYPE_TRIANGULAR_UPPER:
    case RCM_TYPE_TRIANGULAR_LOWER:
      if (RCM_IS_SET(rcm, RCM_ORDER_COL_MAJOR))
      {
        /* column-major order */
        return c * ld + r;
      }
      if (RCM_IS_SET(rcm, RCM_ORDER_ROW_MAJOR))
      {
        /* row-major order */
        return r * ld + c;
      }
  }

  return 0;
}


static void get_matrix_blocks(rdint_t nr, rdint_t nc, rdint_t ld, int rcm, rdint_t *count, rdint_t *size, rdint_t *stride)
{
  switch (RCM_GET_TYPE(rcm))
  {
    case RCM_TYPE_DENSE:
    case RCM_TYPE_TRIANGULAR_UPPER:
    case RCM_TYPE_TRIANGULAR_LOWER:
      if (RCM_IS_SET(rcm, RCM_ORDER_COL_MAJOR))
      {
        /* column-major order: nc-1 full columns of size ld + one column of size nr */
        *count = nc;
        *size = nr;
        *stride = ld;
      }
      if (RCM_IS_SET(rcm, RCM_ORDER_ROW_MAJOR))
      {
        /* row-major order: nr-1 full rows of size ld + one row of size nc */
        *count = nr;
        *size = nc;
        *stride = ld;
      }
      return;
  }

  *count = *size = *stride = 0;
}


static rdint_t get_vector_elem_size(rdint_t n, rdint_t inc)
{
  /* n rows of size 1 */

  return n;
}


static rdint_t get_vector_cont_size(rdint_t n, rdint_t inc)
{
  /* column-major order: n-1 full rows of size inc + one row of size 1 */

  return (n - 1) * inc + 1;
}


static rdint_t get_vector_cont_index(rdint_t n, rdint_t inc, rdint_t i)
{
  return inc * i;
}


template<typename T, int F>
static void print_vector_val(rdint_t n, T *v, rdint_t inc, bool full = false)
{
  rdint_t nx = get_vector_cont_size(n, inc);

  rdint_t i, j, k;
  for (i = 0; i < n; ++i)
  {
    char s[32];

    k = get_vector_cont_index(n, inc, i);
    sprintf(s, PRINTF_FMT(F), v[k]);
    printf("  %s", s);

    for (j = 1; j < inc; ++j)
    {
      k = get_vector_cont_index(n, inc, i) + j;
      if (k >= nx) continue;

      if (full) sprintf(s, PRINTF_FMT(F), v[k]);
      else sprintf(s, "X");
      printf("  %s", s);
    }
    printf("\n");
  }
}


#if 0
template<typename T, int (*F)(char *s, T v)>
static void print_matrix_val(rdint_t nr, rdint_t nc, T *m, rdint_t ld, int rcm)
{
  rdint_t i, j;
  for (i = 0; i < nr; ++i)
  {
    for (j = 0; j < nc; ++j)
    {
      char s[32];
      F(s, m[j * ld + i]);
      printf("  %s", s);
    }
    printf("\n");
  }
}
#endif

template<typename T, int F>
static void print_matrix_val(rdint_t nr, rdint_t nc, T *m, rdint_t ld, int rcm)
{
  if (nr == 0 || nc == 0) return;

  switch (RCM_GET_TYPE(rcm))
  {
    case RCM_TYPE_DENSE:
    case RCM_TYPE_TRIANGULAR_UPPER:
    case RCM_TYPE_TRIANGULAR_LOWER:
      rdint_t nx = get_matrix_cont_size(nr, nc, ld, rcm);

      rdint_t i, j, k;
      for (i = 0; i < (RCM_IS_SET(rcm, RCM_ORDER_COL_MAJOR)?ld:nr); ++i)
      {
        for (j = 0; j < (RCM_IS_SET(rcm, RCM_ORDER_ROW_MAJOR)?ld:nc); ++j)
        {
          k = get_matrix_cont_index(nr, nc, ld, rcm, i, j);
          if (k >= nx) continue;

          char s[32];
          sprintf(s, PRINTF_FMT(F), m[k]);
          printf("  %s", s);
        }
        printf("\n");
      }
  }
}


template<typename T>
static void print_matrix_val(rdint_t nr, rdint_t nc, T *m, rdint_t ld, int rcm);

#define DEFINE_PRINT_MATRIX(_t_, _tn_, _fmt_) \
  template<> \
  void print_matrix_val<_t_>(rdint_t nr, rdint_t nc, _t_ *m, rdint_t ld, int rcm) { \
    print_matrix_val<_t_, _fmt_>(nr, nc, m, ld, rcm); \
  }


template<typename T>
static void print_vector_val(rdint_t n, T *v, rdint_t inc);

#define DEFINE_PRINT_VECTOR(_t_, _tn_, _fmt_) \
  template<> \
  void print_vector_val<_t_>(rdint_t n, _t_ *v, rdint_t inc) { \
    print_vector_val<_t_, _fmt_>(n, v, inc); \
  }


template<typename T, int F>
static rdint_t redirect_call_put_inout_conf_val(char *conf, const char *id, T x)
{
  char t[32];
  sprintf(t, PRINTF_FMT(F), x);

  return sprintf(conf, "%s:%s ", id, t);
}

template<typename T>
static rdint_t redirect_call_put_inout_conf_val(char *conf, const char *id, T x);


template<typename T, int F>
static rdint_t redirect_call_get_inout_conf_val(char *conf, const char *id, T *v)
{
  char *s = strchr(conf, ':');
  char *e = strchr(s, ' ');

  e[0] = '\0';
  sscanf(s + 1, SCANF_FMT(F), SCANF_PTR(F, v));
  e[0] = ' ';

  return e + 1 - conf;
}

template<typename T>
static rdint_t redirect_call_get_inout_conf_val(char *conf, const char *id, T *v);

#define DEFINE_CONF(_t_, _tn_, _fmt_) \
  template<> \
  rdint_t redirect_call_put_inout_conf_val<_t_>(char *conf, const char *id, _t_ x) { \
    return redirect_call_put_inout_conf_val<_t_, _fmt_>(conf, id, x); \
  } \
  template<> \
  rdint_t redirect_call_get_inout_conf_val<_t_>(char *conf, const char *id, _t_ *x) { \
    return redirect_call_get_inout_conf_val<_t_, _fmt_>(conf, id, x); \
  }


/* client side */

int redirect_call_create_scdc(redirect_call_t *rc, const char *op, const char *uri)
{
  rc->client = 1;

  strncpy(rc->op, op, sizeof(rc->op));
  if (uri) strncpy(rc->uri, uri, sizeof(rc->uri));
  else rc->uri[0] = '\0';

  rc->dataset = SCDC_DATASET_NULL;

  rc->input_nconf = 0;
  rc->input_conf[0] = '\0';

  rc->output_nconf = 0;
  rc->output_conf[0] = '\0';

#if REDIRECT_CALL_PARAMS_DENSE
  rc->dense_nparams = 0;
#else
  rc->blocks_nparams = 0;
#endif

  rc->input_offset = rc->output_offset = 0;
  rc->input = static_cast<scdc_dataset_input_t *>(malloc(sizeof(scdc_dataset_input_t) + sizeof(scdc_dataset_output_t)));
  rc->output = rc->input + 1;
  scdc_dataset_input_unset(rc->input);
  scdc_dataset_output_unset(rc->output);

  rc->free_nbufs = 0;

  return 1;
}


void redirect_call_destroy_scdc(redirect_call_t *rc)
{
  /* consume remaining output */
  while (rc->output->next) rc->output->next(rc->output);

  free(rc->input);

  rdint_t i;
  for (i = 0; i < rc->free_nbufs; ++i) free(rc->free_bufs[i]);
  rc->free_nbufs = 0;
}


static scdcint_t do_input_next(scdc_dataset_input_t *input)
{
  redirect_call_t *rc = static_cast<redirect_call_t *>(input->data);

#if REDIRECT_CALL_PARAMS_DENSE
  ++rc->input_state;

  TRACE_F("%s: dense state %d of %d", __func__, rc->input_state, rc->dense_nparams);

  if (rc->input_state < rc->dense_nparams)
  {
    rdint_t i = rc->input_state;

    SCDC_DATASET_INOUT_BUF_PTR(rc->input) = rc->dense_params[i].buf;
    SCDC_DATASET_INOUT_BUF_SIZE(rc->input) = SCDC_DATASET_INOUT_BUF_CURRENT(rc->input) = rc->dense_params[i].size;

  } else
#else
  if (rc->input_state[0] < rc->blocks_nparams)
  {
    rdint_t i = rc->input_state[0];

    SCDC_DATASET_INOUT_BUF_PTR(rc->input) = rc->blocks_params[i].buf;
    SCDC_DATASET_INOUT_BUF_SIZE(rc->input) = SCDC_DATASET_INOUT_BUF_CURRENT(rc->input) = (rc->blocks_params[i].count - 1) * rc->blocks_params[i].stride + rc->blocks_params[i].size;

  } else
#endif
  {
    TRACE_F("%s: freeing %d buffers", __func__, rc->free_nbufs);

    rdint_t i;
    for (i = 0; i < rc->free_nbufs; ++i) free(rc->free_bufs[i]);
    rc->free_nbufs = 0;

    SCDC_DATASET_INOUT_BUF_PTR(rc->input) = NULL;
    SCDC_DATASET_INOUT_BUF_SIZE(rc->input) = 0;
    SCDC_DATASET_INOUT_BUF_CURRENT(rc->input) = 0;

    input->next = NULL;
    input->data = NULL;
  }

  return SCDC_SUCCESS;
}


int redirect_call_execute(redirect_call_t *rc)
{
  char cmd[1024];

  if (rc->dataset == SCDC_DATASET_NULL) sprintf(cmd, "%s %s %s", rc->uri, rc->op, rc->input_conf);
  else sprintf(cmd, "%s %s", rc->op, rc->input_conf);

  rc->input->next = do_input_next;
  rc->input->data = rc;
#if REDIRECT_CALL_PARAMS_DENSE
  rc->input_state = -1;
#else
  rc->input_state[0] = rc->input_state[1] = -1;
#endif

  rc->input->next(rc->input);

  TRACE_F("%s: cmd: '%s'", __func__, cmd);

  if (scdc_dataset_cmd(rc->dataset, cmd, rc->input, rc->output) != SCDC_SUCCESS)
  {
    TRACE_F("%s: failed", __func__);
    return 0;
  }

  /* extract conf output from output */
  const char *s = static_cast<char *>(SCDC_DATASET_INOUT_BUF_PTR(rc->output));
  const char *e = strchr(s, '|');

  ASSERT(e);

  const rdint_t n = e - s;

  rc->output_nconf = 0;
  strncpy(rc->output_conf, s, n);
  rc->output_conf[n] = '\0';

  TRACE_F("%s: output: '%s'", __func__, rc->output_conf);

  rc->output_offset = n + 1;

  return 1;
}


#define DEFINE_CLI_CONF(_t_, _tn_) \
  void redirect_call_put_input_conf_ ## _tn_(redirect_call_t *rc, const char *id, _t_ x) { \
    rc->input_nconf += redirect_call_put_inout_conf_val<_t_>(rc->input_conf + rc->input_nconf, id, x); \
  } \
  void redirect_call_get_output_conf_ ## _tn_(redirect_call_t *rc, const char *id, _t_ *x) { \
    rc->output_nconf += redirect_call_get_inout_conf_val<_t_>(rc->output_conf + rc->output_nconf, id, x); \
  }


/* server side */

void redirect_call_init_scdc(redirect_call_t *rc, const char *params, scdc_dataset_input_t *input, scdc_dataset_output_t *output)
{
  rc->client = 0;

  rc->dataset = SCDC_DATASET_NULL;

  rc->input_nconf = 0;
  strcpy(rc->input_conf, params);

  rc->output_nconf = 0;
  rc->output_conf[0] = '\0';

  rc->dense_nparams = 0;

  rc->input_offset = rc->output_offset = 0;
  rc->input = input;
  rc->output = output;

  rc->free_nbufs = 0;
}


static scdcint_t release_scdc_output_next(scdc_dataset_output_t *output)
{
  redirect_call_t *rc = static_cast<redirect_call_t *>(output->data);

#if REDIRECT_CALL_PARAMS_DENSE
  ++rc->output_state;

  TRACE_F("%s: dense state %d of %d", __func__, rc->output_state, rc->dense_nparams);

  if (rc->output_state < rc->dense_nparams)
  {
    rdint_t i = rc->output_state;

    SCDC_DATASET_INOUT_BUF_PTR(rc->output) = rc->dense_params[i].buf;
    SCDC_DATASET_INOUT_BUF_SIZE(rc->output) = rc->dense_params[i].size;
    SCDC_DATASET_INOUT_BUF_CURRENT(rc->output) = rc->dense_params[i].size;

  } else
#else
# error
#endif
  {
    TRACE_F("%s: freeing %d buffers", __func__, rc->free_nbufs);

    rdint_t i;
    for (i = 0; i < rc->free_nbufs; ++i) free(rc->free_bufs[i]);
    rc->free_nbufs = 0;

    SCDC_DATASET_INOUT_BUF_PTR(rc->output) = NULL;
    SCDC_DATASET_INOUT_BUF_SIZE(rc->output) = 0;
    SCDC_DATASET_INOUT_BUF_CURRENT(rc->output) = 0;

    output->next = NULL;
    output->data = NULL;
  }

  return SCDC_SUCCESS;
}


void redirect_call_release_scdc(redirect_call_t *rc)
{
  /* consume remaining input */
  while (rc->input->next) rc->input->next(rc->input);

  strcpy(rc->output_conf + rc->output_nconf, "|");
  rc->output_nconf += 1;

  SCDC_DATASET_INOUT_BUF_PTR(rc->output) = rc->output_conf;
  SCDC_DATASET_INOUT_BUF_SIZE(rc->output) = rc->output_nconf;

  SCDC_DATASET_INOUT_BUF_CURRENT(rc->output) = rc->output_nconf;

  rc->output->next = release_scdc_output_next;
  rc->output->data = rc;
#if REDIRECT_CALL_PARAMS_DENSE
  rc->output_state = -1;
#else
  rc->output_state[0] = rc->output_state[1] = -1;
#endif
}


#define DEFINE_SRV_CONF(_t_, _tn_) \
  void redirect_call_get_input_conf_ ## _tn_(redirect_call_t *rc, const char *id, _t_ *x) { \
    rc->input_nconf += redirect_call_get_inout_conf_val<_t_>(rc->input_conf + rc->input_nconf, id, x); \
  } \
  void redirect_call_put_output_conf_ ## _tn_(redirect_call_t *rc, const char *id, _t_ x) { \
    rc->output_nconf += redirect_call_put_inout_conf_val<_t_>(rc->output_conf + rc->output_nconf, id, x); \
  }


/* client and server side */

/* handle */

void redirect_call_set_handle(redirect_call_t *rc, redirect_handle_t *rh)
{
  if (rc->client)
  {
    rc->dataset = rh->dataset;
    redirect_call_put_input_conf_char_p(rc, "hdl_id", rh->id);
    redirect_call_put_input_conf_void_p(rc, "hdl_ptr", rh->ptr);

  } else
  {
    redirect_call_put_output_conf_char_p(rc, "hdl_id", rh->id);
    redirect_call_put_output_conf_void_p(rc, "hdl_ptr", rh->ptr);
  }
}


void redirect_call_get_handle(redirect_call_t *rc, redirect_handle_t *rh)
{
  rh->client = rc->client;

  if (rc->client)
  {
    rh->dataset = rc->dataset;
    redirect_call_get_output_conf_char_p(rc, "hdl_id", reinterpret_cast<char **>(&rh->id));
    redirect_call_get_output_conf_void_p(rc, "hdl_ptr", &rh->ptr);

  } else
  {
    redirect_call_get_input_conf_char_p(rc, "hdl_id", reinterpret_cast<char **>(&rh->id));
    redirect_call_get_input_conf_void_p(rc, "hdl_ptr", &rh->ptr);
  }
}


/* dense data */

static void redirect_call_put_input_param_dense(redirect_call_t *rc, const char *id, rdint_t n, void *b)
{
  /* client only */
  ASSERT(rc->client);

  rc->dense_params[rc->dense_nparams].buf = b;
  rc->dense_params[rc->dense_nparams].size = n;

  ++rc->dense_nparams;
}


static void redirect_call_get_input_param_dense(redirect_call_t *rc, const char *id, rdint_t n, void **b)
{
  /* server only */
  ASSERT(!rc->client);

  if (*b == NULL)
  {
    /* puffer allokieren */
    *b = static_cast<char *>(malloc(n));

    rc->free_bufs[rc->free_nbufs] = *b;
    ++rc->free_nbufs;
  }

  /* daten aus input in puffer kopieren */

  char *src = static_cast<char *>(SCDC_DATASET_INOUT_BUF_PTR(rc->input));
  char *dst = static_cast<char *>(*b);

  do
  {
    rdint_t t = z_min(n, SCDC_DATASET_INOUT_BUF_CURRENT(rc->input) - rc->input_offset);
    if (dst != src + rc->output_offset) memcpy(dst, src + rc->input_offset, t);
    rc->input_offset += t;
    dst += t;
    n -= t;

    if (n <= 0 || rc->input->next == 0 || rc->input->next(rc->input) != SCDC_SUCCESS) break;

    src = static_cast<char *>(SCDC_DATASET_INOUT_BUF_PTR(rc->input));
    rc->input_offset = 0;

  } while (1);
}


static void redirect_call_put_output_param_dense(redirect_call_t *rc, const char *id, rdint_t n, void *b)
{
  if (rc->client)
  {
    /* puffer in output hinterlegen */
    /* FIXME */

  } else
  {
    /* ausgabe in output schreiben */
    rc->dense_params[rc->dense_nparams].buf = b;
    rc->dense_params[rc->dense_nparams].size = n;

    ++rc->dense_nparams;
  }
}


void redirect_call_get_output_param_dense(redirect_call_t *rc, const char *id, rdint_t n, void **b)
{
  if (rc->client)
  {
    ASSERT(*b);

    /* daten aus output in puffer kopieren */

    char *src = static_cast<char *>(SCDC_DATASET_INOUT_BUF_PTR(rc->output));
    char *dst = static_cast<char *>(*b);

    do
    {
      rdint_t t = z_min(n, SCDC_DATASET_INOUT_BUF_CURRENT(rc->output) - rc->output_offset);
      if (dst != src + rc->output_offset) memcpy(dst, src + rc->output_offset, t);
      rc->output_offset += t;
      dst += t;
      n -= t;

      if (n <= 0 || rc->output->next == 0 || rc->output->next(rc->output) != SCDC_SUCCESS) break;

      src = static_cast<char *>(SCDC_DATASET_INOUT_BUF_PTR(rc->output));
      rc->output_offset = 0;

    } while (1);

  } else
  {
    if (*b == NULL)
    {
#if 0
      if (SCDC_DATASET_INOUT_BUF_PTR(rc->output) && SCDC_DATASET_INOUT_BUF_SIZE(rc->output) - rc->output_offset >= s)
      {
        /* puffer aus output.buf verwenden */
        /* FIXME */
        /* output_offset hochsetzen */

      } else
#endif
      {
        /* puffer allokieren */
        *b = static_cast<char *>(malloc(n));

        rc->free_bufs[rc->free_nbufs] = *b;
        ++rc->free_nbufs;
      }
    }
  }
}


static void redirect_call_put_inout_param_dense(redirect_call_t *rc, const char *id, rdint_t n, void *b)
{
  redirect_call_put_input_param_dense(rc, id, n, b);
  redirect_call_put_output_param_dense(rc, id, n, b);
}


static void redirect_call_get_inout_param_dense(redirect_call_t *rc, const char *id, rdint_t n, void **b)
{
  redirect_call_get_input_param_dense(rc, id, n, b);
  redirect_call_get_output_param_dense(rc, id, n, b);
}


/* blocks data */

template<typename T>
static void redirect_call_put_input_param_blocks_val(redirect_call_t *rc, const char *id, rdint_t count, rdint_t size, rdint_t stride, T *b)
{
  /* client only */
  ASSERT(rc->client);

#if REDIRECT_CALL_INPUT_PARAM_BLOCKS == REDIRECT_CALL_BLOCKS_DENSE
  /* nothing to be done here */
#elif REDIRECT_CALL_INPUT_PARAM_BLOCKS == REDIRECT_CALL_BLOCKS_PACK_IP
  if (stride > size)
  {
    TRACE_F("%s: size: %" rdint_fmt ", stride: %" rdint_fmt ", transform in-place to stride: %" rdint_fmt, __func__, size, stride, size);
    redirect_data_blocks_transform_val<T>(count, size, stride, b, size, NULL);
    stride = size;
  }
#elif REDIRECT_CALL_INPUT_PARAM_BLOCKS == REDIRECT_CALL_BLOCKS_PACK_OP
  if (stride > size)
  {
    TRACE_F("%s: size: %" rdint_fmt ", stride: %" rdint_fmt ", transform out-of-place to stride: %" rdint_fmt, __func__, size, stride, size);

    T *b_tmp = static_cast<T *>(malloc(count * size * sizeof(T)));

    rc->free_bufs[rc->free_nbufs] = b_tmp;
    ++rc->free_nbufs;

    redirect_data_blocks_transform_val<T>(count, size, stride, b, size, b_tmp);
    stride = size;
    b = b_tmp;
  }
#elif REDIRECT_CALL_INPUT_PARAM_BLOCKS == REDIRECT_CALL_BLOCKS_PIECES
  if (stride > size)
  {
# error TODO
  } else
#endif
  {
    redirect_call_put_input_conf_int(rc, "bstride", stride);

    redirect_call_put_input_param_dense(rc, id, get_blocks_cont_size(count, size, stride) * sizeof(T), static_cast<void *>(b));
  }
}


template<typename T>
static void redirect_call_get_input_param_blocks_val(redirect_call_t *rc, const char *id, rdint_t count, rdint_t size, rdint_t *stride, T **b)
{
  /* server only */
  ASSERT(!rc->client);

  rdint_t stride_in;

  redirect_call_get_input_conf_int(rc, "bstride", &stride_in);

  if (*stride == 0) *stride = stride_in;

#if REDIRECT_CALL_INPUT_PARAM_BLOCKS == REDIRECT_CALL_BLOCKS_PACK_OP
  if (*stride != stride_in && *b != NULL)
  {
    T *b_tmp = static_cast<T *>(malloc(get_blocks_cont_size(count, size, stride_in) * sizeof(T)));

    redirect_call_get_input_param_dense(rc, id, get_blocks_cont_size(count, size, stride_in) * sizeof(T), reinterpret_cast<void **>(&b_tmp));

    TRACE_F("%s: size: %" rdint_fmt ", stride: %" rdint_fmt ", transform out-of-place to stride: %" rdint_fmt , __func__, size, stride_in, *stride);
    redirect_data_blocks_transform_val<T>(count, size, stride_in, b_tmp, *stride, *b);

    free(b_tmp);

  } else
#endif
  {
    /* currently: if the buffer is allocated using stride_in, then we can not transform into a larger stride afterwards */
    ASSERT(!(*b == NULL && *stride > stride_in));

    redirect_call_get_input_param_dense(rc, id, get_blocks_cont_size(count, size, stride_in) * sizeof(T), reinterpret_cast<void **>(b));

    if (*stride != stride_in)
    {
      TRACE_F("%s: size: %" rdint_fmt ", stride: %" rdint_fmt ", transform in-place to stride: %" rdint_fmt , __func__, size, stride_in, *stride);
      redirect_data_blocks_transform_val<T>(count, size, stride_in, *b, *stride, NULL);
    }
  }
}


template<typename T>
static void redirect_call_put_output_param_blocks_val(redirect_call_t *rc, const char *id, rdint_t count, rdint_t size, rdint_t stride, T *b)
{
#if REDIRECT_CALL_OUTPUT_PARAM_BLOCKS == REDIRECT_CALL_BLOCKS_DENSE
  /* nothing to be done here */
#elif (REDIRECT_CALL_OUTPUT_PARAM_BLOCKS == REDIRECT_CALL_BLOCKS_PACK_IP) || (REDIRECT_CALL_OUTPUT_PARAM_BLOCKS == REDIRECT_CALL_BLOCKS_PACK_OP)
  if (stride > size)
  {
    if (rc->client)
    {
      /* client-side */
      TRACE_F("%s: size: %" rdint_fmt ", stride: %" rdint_fmt ", request stride: %" rdint_fmt, __func__, size, stride, size);
      stride = size;

    } else
    {
      /* server-side */
#if REDIRECT_CALL_OUTPUT_PARAM_BLOCKS == REDIRECT_CALL_BLOCKS_PACK_IP
      TRACE_F("%s: size: %" rdint_fmt ", stride: %" rdint_fmt ", transform in-place to stride: %" rdint_fmt, __func__, size, stride, size);
      redirect_data_blocks_transform_val<T>(count, size, stride, b, size, NULL);
      stride = size;
#endif
#if REDIRECT_CALL_OUTPUT_PARAM_BLOCKS == REDIRECT_CALL_BLOCKS_PACK_OP
      TRACE_F("%s: size: %" rdint_fmt ", stride: %" rdint_fmt ", transform out-of-place to stride: %" rdint_fmt, __func__, size, stride, size);

      T *b_tmp = static_cast<T *>(malloc(count * size * sizeof(T)));

      rc->free_bufs[rc->free_nbufs] = b_tmp;
      ++rc->free_nbufs;

      redirect_data_blocks_transform_val<T>(count, size, stride, b, size, b_tmp);
      stride = size;
      b = b_tmp;
#endif
    }
  }
#elif REDIRECT_CALL_OUTPUT_PARAM_BLOCKS == REDIRECT_CALL_BLOCKS_PIECES
  if (stride > size)
  {
# error TODO
  } else
#endif
  {
    if (rc->client)
    {
      /* client-side */
      redirect_call_put_input_conf_int(rc, "bstride", stride);

    } else
    {
      /* server-side */
      redirect_call_put_output_conf_int(rc, "bstride", stride);
    }

    redirect_call_put_output_param_dense(rc, id, get_blocks_cont_size(count, size, stride) * sizeof(T), static_cast<void *>(b));
  }
}


template<typename T>
void redirect_call_get_output_param_blocks_val(redirect_call_t *rc, const char *id, rdint_t count, rdint_t size, rdint_t *stride, T **b)
{
  if (!rc->client)
  {
    /* server-side */
    rdint_t stride_in;

    redirect_call_get_input_conf_int(rc, "bstride", &stride_in);

    if (*stride == 0) *stride = stride_in;

    redirect_call_get_output_param_dense(rc, id, get_blocks_cont_size(count, size, stride_in) * sizeof(T), reinterpret_cast<void **>(b));

  } else
  {
    /* client-side */
    rdint_t stride_in;

    redirect_call_get_output_conf_int(rc, "bstride", &stride_in);

    if (*stride == 0) *stride = stride_in;

#if REDIRECT_CALL_OUTPUT_PARAM_BLOCKS == REDIRECT_CALL_BLOCKS_PACK_OP
    if (*stride != stride_in && *b != NULL)
    {
      T *b_tmp = static_cast<T *>(malloc(get_blocks_cont_size(count, size, stride_in) * sizeof(T)));

      redirect_call_get_output_param_dense(rc, id, get_blocks_cont_size(count, size, stride_in) * sizeof(T), reinterpret_cast<void **>(&b_tmp));

      TRACE_F("%s: size: %" rdint_fmt ", stride: %" rdint_fmt ", transform out-of-place to stride: %" rdint_fmt , __func__, size, stride_in, *stride);
      redirect_data_blocks_transform_val<T>(count, size, stride_in, b_tmp, *stride, *b);

      free(b_tmp);

    } else
#endif
    {
      /* currently: if the buffer is allocated using stride_in, then we can not transform into a larger stride afterwards */
      ASSERT(!(*b == NULL && *stride > stride_in));

      redirect_call_get_output_param_dense(rc, id, get_blocks_cont_size(count, size, stride_in) * sizeof(T), reinterpret_cast<void **>(b));

      if (*stride != stride_in)
      {
        TRACE_F("%s: size: %" rdint_fmt ", stride: %" rdint_fmt ", transform in-place to stride: %" rdint_fmt , __func__, size, stride_in, *stride);
        redirect_data_blocks_transform_val<T>(count, size, stride_in, *b, *stride, NULL);
      }
    }
  }
}


template<typename T>
static void redirect_call_put_inout_param_blocks_val(redirect_call_t *rc, const char *id, rdint_t count, rdint_t size, rdint_t stride, T *b)
{
  /* client only */
  ASSERT(rc->client);

#if REDIRECT_CALL_INOUT_PARAM_BLOCKS == REDIRECT_CALL_BLOCKS_DENSE
  /* nothing to be done here */
#elif REDIRECT_CALL_INOUT_PARAM_BLOCKS == REDIRECT_CALL_BLOCKS_PACK_IP
  if (stride > size)
  {
    TRACE_F("%s: size: %" rdint_fmt ", stride: %" rdint_fmt ", transform in-place to stride: %" rdint_fmt, __func__, size, stride, size);
    redirect_data_blocks_transform_val<T>(count, size, stride, b, size, NULL);
    stride = size;
  }
#elif REDIRECT_CALL_INOUT_PARAM_BLOCKS == REDIRECT_CALL_BLOCKS_PACK_OP
  if (stride > size)
  {
    TRACE_F("%s: size: %" rdint_fmt ", stride: %" rdint_fmt ", transform out-of-place to stride: %" rdint_fmt, __func__, size, stride, size);

    T *b_tmp = static_cast<T *>(malloc(count * size * sizeof(T)));

    rc->free_bufs[rc->free_nbufs] = b_tmp;
    ++rc->free_nbufs;

    redirect_data_blocks_transform_val<T>(count, size, stride, b, size, b_tmp);
    stride = size;
    b = b_tmp;
  }
#elif REDIRECT_CALL_INOUT_PARAM_BLOCKS == REDIRECT_CALL_BLOCKS_PIECES
  if (stride > size)
  {
# error TODO
  } else
#endif
  {
    redirect_call_put_input_conf_int(rc, "bstride", stride);

    redirect_call_put_inout_param_dense(rc, id, get_blocks_cont_size(count, size, stride) * sizeof(T), static_cast<void *>(b));
  }
}


template<typename T>
static void redirect_call_get_inout_param_blocks_val(redirect_call_t *rc, const char *id, rdint_t count, rdint_t size, rdint_t *stride, T **b)
{
  /* server only */
  ASSERT(!rc->client);

  rdint_t stride_in;

  redirect_call_get_input_conf_int(rc, "bstride", &stride_in);

  if (*stride == 0) *stride = stride_in;

#if REDIRECT_CALL_INOUT_PARAM_BLOCKS == REDIRECT_CALL_BLOCKS_PACK_OP
  if (*stride != stride_in && *b != NULL)
  {
    T *b_tmp = static_cast<T *>(malloc(get_blocks_cont_size(count, size, stride_in) * sizeof(T)));

    redirect_call_get_inout_param_dense(rc, id, get_blocks_cont_size(count, size, stride_in) * sizeof(T), reinterpret_cast<void **>(&b_tmp));

    TRACE_F("%s: size: %" rdint_fmt ", stride: %" rdint_fmt ", transform out-of-place to stride: %" rdint_fmt , __func__, size, stride_in, *stride);
    redirect_data_blocks_transform_val<T>(count, size, stride_in, b_tmp, *stride, *b);

    free(b_tmp);

  } else
#endif
  {
    /* currently: if the buffer is allocated using stride_in, then we can not transform into a larger stride afterwards */
    ASSERT(!(*b == NULL && *stride > stride_in));

    redirect_call_get_inout_param_dense(rc, id, get_blocks_cont_size(count, size, stride_in) * sizeof(T), reinterpret_cast<void **>(b));

    if (*stride != stride_in)
    {
      TRACE_F("%s: size: %" rdint_fmt ", stride: %" rdint_fmt ", transform in-place to stride: %" rdint_fmt , __func__, size, stride_in, *stride);
      redirect_data_blocks_transform_val<T>(count, size, stride_in, *b, *stride, NULL);
    }
  }
}


/* plain bytes */

void redirect_call_put_input_param_bytes(redirect_call_t *rc, const char *id, rdint_t n, char *b)
{
  redirect_call_put_input_param_dense(rc, id, n, b);
}


void redirect_call_get_input_param_bytes(redirect_call_t *rc, const char *id, rdint_t n, char **b)
{
  redirect_call_get_input_param_dense(rc, id, n, reinterpret_cast<void **>(b));
}


void redirect_call_put_output_param_bytes(redirect_call_t *rc, const char *id, rdint_t n, char *b)
{
  redirect_call_put_output_param_dense(rc, id, n, b);
}


void redirect_call_get_output_param_bytes(redirect_call_t *rc, const char *id, rdint_t n, char **b)
{
  redirect_call_get_output_param_dense(rc, id, n, reinterpret_cast<void **>(b));
}


void redirect_call_put_inout_param_bytes(redirect_call_t *rc, const char *id, rdint_t n, char *b)
{
  redirect_call_put_inout_param_dense(rc, id, n, b);
}


void redirect_call_get_inout_param_bytes(redirect_call_t *rc, const char *id, rdint_t n, char **b)
{
  redirect_call_get_inout_param_dense(rc, id, n, reinterpret_cast<void **>(b));
}


/* typed vectors with stride */

template<typename T>
void redirect_call_print_param_vector_val(rdint_t n, T *v, rdint_t inc)
{
  print_vector_val<T>(n, v, inc);
}


template<typename T>
void redirect_call_put_input_param_vector_val(redirect_call_t *rc, const char *id, rdint_t n, T *v, rdint_t inc)
{
  /* client only */
  ASSERT(rc->client);

#if TRACE_DATA
  TRACE_F("%s:", __func__);
  print_vector_val<T>(n, v, inc);
#endif

  redirect_call_put_input_param_blocks_val<T>(rc, id, n, 1, inc, v);
}


template<typename T>
void redirect_call_get_input_param_vector_val(redirect_call_t *rc, const char *id, rdint_t n, T **v, rdint_t *inc)
{
  TRACE_F("%s: rc: %p, id: '%s', n: %" rdint_fmt ", v: %p, inc: %" rdint_fmt, __func__, rc, id, n, *v, *inc);

  /* server only */
  ASSERT(!rc->client);

  redirect_call_get_input_param_blocks_val<T>(rc, id, n, 1, inc, v);

#if TRACE_DATA
  TRACE_F("%s:", __func__);
  print_vector_val<T>(n, *v, *inc);
#endif
}


template<typename T>
void redirect_call_put_output_param_vector_val(redirect_call_t *rc, const char *id, rdint_t n, T *v, rdint_t inc)
{
  if (!rc->client)
  {
#if TRACE_DATA
    TRACE_F("%s:", __func__);
    print_vector_val<T>(n, v, inc);
#endif
  }

  redirect_call_put_output_param_blocks_val<T>(rc, id, n, 1, inc, v);
}


template<typename T>
void redirect_call_get_output_param_vector_val(redirect_call_t *rc, const char *id, rdint_t n, T **v, rdint_t *inc)
{
  TRACE_F("%s: rc: %p, id: '%s', n: %" rdint_fmt ", v: %p, inc: %" rdint_fmt, __func__, rc, id, n, *v, *inc);

  redirect_call_get_output_param_blocks_val<T>(rc, id, n, 1, inc, v);

  if (rc->client)
  {
#if TRACE_DATA
    TRACE_F("%s:", __func__);
    print_vector_val<T>(n, *v, *inc);
#endif
  }
}


template<typename T>
void redirect_call_put_inout_param_vector_val(redirect_call_t *rc, const char *id, rdint_t n, T *v, rdint_t inc)
{
  /* client only */
  ASSERT(rc->client);

#if TRACE_DATA
  TRACE_F("%s:", __func__);
  print_vector_val<T>(n, v, inc);
#endif

  redirect_call_put_inout_param_blocks_val<T>(rc, id, n, 1, inc, v);
}


template<typename T>
void redirect_call_get_inout_param_vector_val(redirect_call_t *rc, const char *id, rdint_t n, T **v, rdint_t *inc)
{
  /* server only */
  ASSERT(!rc->client);

  redirect_call_get_inout_param_blocks_val<T>(rc, id, n, 1, inc, v);

#if TRACE_DATA
  TRACE_F("%s:", __func__);
  print_vector_val<T>(n, *v, *inc);
#endif
}


/* typed matrices with leading dimension */

template<typename T>
void redirect_call_print_param_matrix_val(rdint_t nr, rdint_t nc, T *m, rdint_t ld, int rcm)
{
  print_matrix_val<T>(nr, nc, m, ld, rcm);
}


template<typename T>
void redirect_call_put_input_param_matrix_val(redirect_call_t *rc, const char *id, rdint_t nr, rdint_t nc, T *m, rdint_t ld, int rcm)
{
  /* client only */
  ASSERT(rc->client);

#if TRACE_DATA
  TRACE_F("%s:", __func__);
  print_matrix_val<T>(nr, nc, m, ld, rcm);
#endif

  redirect_call_put_input_conf_int(rc, "mrcm", rcm);

  rdint_t count, size, stride;
  get_matrix_blocks(nr, nc, ld, rcm, &count, &size, &stride);
  redirect_call_put_input_param_blocks_val<T>(rc, id, count, size, stride, m);
}


template<typename T>
void redirect_call_get_input_param_matrix_val(redirect_call_t *rc, const char *id, rdint_t nr, rdint_t nc, T **m, rdint_t *ld, int *rcm)
{
  /* server only */
  ASSERT(!rc->client);

  redirect_call_get_input_conf_int(rc, "mrcm", rcm);

  rdint_t count, size, stride;
  get_matrix_blocks(nr, nc, *ld, *rcm, &count, &size, &stride);
  redirect_call_get_input_param_blocks_val<T>(rc, id, count, size, &stride, m);
  *ld = stride;

#if TRACE_DATA
  TRACE_F("%s:", __func__);
  print_matrix_val<T>(nr, nc, *m, *ld, *rcm);
#endif
}


template<typename T>
void redirect_call_put_output_param_matrix_val(redirect_call_t *rc, const char *id, rdint_t nr, rdint_t nc, T *m, rdint_t ld, int rcm)
{
  if (!rc->client)
  {
#if TRACE_DATA
    TRACE_F("%s:", __func__);
    print_matrix_val<T>(nr, nc, m, ld, rcm);
#endif
  }

  if (rc->client) redirect_call_put_input_conf_int(rc, "mrcm", rcm);
  else redirect_call_put_output_conf_int(rc, "mrcm", rcm);

  rdint_t count, size, stride;
  get_matrix_blocks(nr, nc, ld, rcm, &count, &size, &stride);
  redirect_call_put_output_param_blocks_val<T>(rc, id, count, size, stride, m);
}


template<typename T>
void redirect_call_get_output_param_matrix_val(redirect_call_t *rc, const char *id, rdint_t nr, rdint_t nc, T **m, rdint_t *ld, int *rcm)
{
  if (!rc->client) redirect_call_get_input_conf_int(rc, "mrcm", rcm);
  else redirect_call_get_output_conf_int(rc, "mrcm", rcm);

  rdint_t count, size, stride;
  get_matrix_blocks(nr, nc, *ld, *rcm, &count, &size, &stride);
  redirect_call_get_output_param_blocks_val<T>(rc, id, count, size, &stride, m);
  *ld = stride;

  if (rc->client)
  {
#if TRACE_DATA
    TRACE_F("%s:", __func__);
    print_matrix_val<T>(nr, nc, *m, *ld, *rcm);
#endif
  }
}


template<typename T>
void redirect_call_put_inout_param_matrix_val(redirect_call_t *rc, const char *id, rdint_t nr, rdint_t nc, T *m, rdint_t ld, int rcm)
{
  /* client only */
  ASSERT(rc->client);

#if TRACE_DATA
  TRACE_F("%s:", __func__);
  print_matrix_val<T>(nr, nc, m, ld, rcm);
#endif

  redirect_call_put_input_conf_int(rc, "mrcm", rcm);

  rdint_t count, size, stride;
  get_matrix_blocks(nr, nc, ld, rcm, &count, &size, &stride);
  redirect_call_put_inout_param_blocks_val<T>(rc, id, count, size, stride, m);
}


template<typename T>
void redirect_call_get_inout_param_matrix_val(redirect_call_t *rc, const char *id, rdint_t nr, rdint_t nc, T **m, rdint_t *ld, int *rcm)
{
  /* server only */
  ASSERT(!rc->client);

  redirect_call_get_input_conf_int(rc, "mrcm", rcm);

  rdint_t count, size, stride;
  get_matrix_blocks(nr, nc, *ld, *rcm, &count, &size, &stride);
  redirect_call_get_inout_param_blocks_val<T>(rc, id, count, size, &stride, m);
  *ld = stride;

#if TRACE_DATA
  TRACE_F("%s:", __func__);
  print_matrix_val<T>(nr, nc, *m, *ld, *rcm);
#endif
}


/* vectors */

#define DEFINE_FLOAT_VECTOR(_t_, _tn_) \
  void redirect_call_print_param_vector_ ## _tn_(rdint_t n, _t_ *v, rdint_t inc) { \
    redirect_call_print_param_vector_val<_t_>(n, v, inc); \
  } \
  void redirect_call_put_input_param_vector_ ## _tn_(redirect_call_t *rc, const char *id, rdint_t n, _t_ *v, rdint_t inc) { \
    redirect_call_put_input_param_vector_val<_t_>(rc, id, n, v, inc); \
  } \
  void redirect_call_get_input_param_vector_ ## _tn_(redirect_call_t *rc, const char *id, rdint_t n, _t_ **v, rdint_t *inc) { \
    redirect_call_get_input_param_vector_val<_t_>(rc, id, n, v, inc); \
  } \
  void redirect_call_put_output_param_vector_ ## _tn_(redirect_call_t *rc, const char *id, rdint_t n, _t_ *v, rdint_t inc) { \
    redirect_call_put_output_param_vector_val<_t_>(rc, id, n, v, inc); \
  } \
  void redirect_call_get_output_param_vector_ ## _tn_(redirect_call_t *rc, const char *id, rdint_t n, _t_ **v, rdint_t *inc) { \
    redirect_call_get_output_param_vector_val<_t_>(rc, id, n, v, inc); \
  } \
  void redirect_call_put_inout_param_vector_ ## _tn_(redirect_call_t *rc, const char *id, rdint_t n, _t_ *v, rdint_t inc) { \
    redirect_call_put_inout_param_vector_val<_t_>(rc, id, n, v, inc); \
  } \
  void redirect_call_get_inout_param_vector_ ## _tn_(redirect_call_t *rc, const char *id, rdint_t n, _t_ **v, rdint_t *inc) { \
    redirect_call_get_inout_param_vector_val<_t_>(rc, id, n, v, inc); \
  }


#define DEFINE_INT_VECTOR(_t_, _tn_) \
  void redirect_call_print_param_vector_ ## _tn_(rdint_t n, _t_ *v) { \
    redirect_call_print_param_vector_val<_t_>(n, v, 1); \
  } \
  void redirect_call_put_input_param_vector_ ## _tn_(redirect_call_t *rc, const char *id, rdint_t n, _t_ *v) { \
    redirect_call_put_input_param_vector_val<_t_>(rc, id, n, v, 1); \
  } \
  void redirect_call_get_input_param_vector_ ## _tn_(redirect_call_t *rc, const char *id, rdint_t n, _t_ **v) { \
    rdint_t inc = 1; \
    redirect_call_get_input_param_vector_val<_t_>(rc, id, n, v, &inc); \
    ASSERT(inc == 1); \
  } \
  void redirect_call_put_output_param_vector_ ## _tn_(redirect_call_t *rc, const char *id, rdint_t n, _t_ *v) { \
    redirect_call_put_output_param_vector_val<_t_>(rc, id, n, v, 1); \
  } \
  void redirect_call_get_output_param_vector_ ## _tn_(redirect_call_t *rc, const char *id, rdint_t n, _t_ **v) { \
    rdint_t inc = 1; \
    redirect_call_get_output_param_vector_val<_t_>(rc, id, n, v, &inc); \
    ASSERT(inc == 1); \
  } \
  void redirect_call_put_inout_param_vector_ ## _tn_(redirect_call_t *rc, const char *id, rdint_t n, _t_ *v) { \
    redirect_call_put_inout_param_vector_val<_t_>(rc, id, n, v, 1); \
  } \
  void redirect_call_get_inout_param_vector_ ## _tn_(redirect_call_t *rc, const char *id, rdint_t n, _t_ **v) { \
    rdint_t inc = 1; \
    redirect_call_get_inout_param_vector_val<_t_>(rc, id, n, v, &inc); \
    ASSERT(inc == 1); \
  }


/* matrices */

#define DEFINE_FLOAT_MATRIX(_t_, _tn_) \
  void redirect_call_print_param_matrix_ ## _tn_(rdint_t nr, rdint_t nc, _t_ *m, rdint_t ld, int rcm) { \
    redirect_call_print_param_matrix_val<_t_>(nr, nc, m, ld, rcm); \
  } \
  void redirect_call_put_input_param_matrix_ ## _tn_(redirect_call_t *rc, const char *id, rdint_t nr, rdint_t nc, _t_ *m, rdint_t ld, int rcm) { \
    redirect_call_put_input_param_matrix_val<_t_>(rc, id, nr, nc, m, ld, rcm); \
  } \
  void redirect_call_get_input_param_matrix_ ## _tn_(redirect_call_t *rc, const char *id, rdint_t nr, rdint_t nc, _t_ **m, rdint_t *ld, int *rcm) { \
    redirect_call_get_input_param_matrix_val<_t_>(rc, id, nr, nc, m, ld, rcm); \
  } \
  void redirect_call_put_output_param_matrix_ ## _tn_(redirect_call_t *rc, const char *id, rdint_t nr, rdint_t nc, _t_ *m, rdint_t ld, int rcm) { \
    redirect_call_put_output_param_matrix_val<_t_>(rc, id, nr, nc, m, ld, rcm); \
  } \
  void redirect_call_get_output_param_matrix_ ## _tn_(redirect_call_t *rc, const char *id, rdint_t nr, rdint_t nc, _t_ **m, rdint_t *ld, int *rcm) { \
    redirect_call_get_output_param_matrix_val<_t_>(rc, id, nr, nc, m, ld, rcm); \
  } \
  void redirect_call_put_inout_param_matrix_ ## _tn_(redirect_call_t *rc, const char *id, rdint_t nr, rdint_t nc, _t_ *m, rdint_t ld, int rcm) { \
    redirect_call_put_inout_param_matrix_val<_t_>(rc, id, nr, nc, m, ld, rcm); \
  } \
  void redirect_call_get_inout_param_matrix_ ## _tn_(redirect_call_t *rc, const char *id, rdint_t nr, rdint_t nc, _t_ **m, rdint_t *ld, int *rcm) { \
    redirect_call_get_inout_param_matrix_val<_t_>(rc, id, nr, nc, m, ld, rcm); \
  }


#define DEFINE_FLOAT(_t_, _tn_, _fmt_) \
  DEFINE_CONF(_t_, _tn_, _fmt_); \
  DEFINE_CLI_CONF(_t_, _tn_); \
  DEFINE_SRV_CONF(_t_, _tn_); \
  DEFINE_PRINT_VECTOR(_t_, _tn_, _fmt_); \
  DEFINE_PRINT_MATRIX(_t_, _tn_, _fmt_); \
  DEFINE_FLOAT_VECTOR(_t_, _tn_); \
  DEFINE_FLOAT_MATRIX(_t_, _tn_);

DEFINE_FLOAT(float, float, FMT_FLOAT);
DEFINE_FLOAT(double, double, FMT_DOUBLE);

#define DEFINE_INT(_t_, _tn_, _fmt_) \
  DEFINE_CONF(_t_, _tn_, _fmt_); \
  DEFINE_CLI_CONF(_t_, _tn_); \
  DEFINE_SRV_CONF(_t_, _tn_); \
  DEFINE_PRINT_VECTOR(_t_, _tn_, _fmt_); \
  DEFINE_INT_VECTOR(_t_, _tn_); \

DEFINE_INT(char, char, FMT_CHAR);
DEFINE_INT(int, int, FMT_INT);

DEFINE_CONF(void *, void_p, FMT_VOID_PTR);
DEFINE_CLI_CONF(void *, void_p);
DEFINE_SRV_CONF(void *, void_p);
DEFINE_CONF(char *, char_p, FMT_CHAR_PTR);
DEFINE_CLI_CONF(char *, char_p);
DEFINE_SRV_CONF(char *, char_p);
