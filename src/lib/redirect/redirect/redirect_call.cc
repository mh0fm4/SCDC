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

#include "redirect_config.h"
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
static rdint_t redirect_call_get_inout_conf_val(const char *conf, const char *id, T *x)
{
  const char *s = strchr(conf, ':');

  char t[32];
  sprintf(t, "%s %%n", SCANF_FMT(F));

  int n;
  sscanf(s + 1, t, SCANF_PTR(F, x), &n);

  return s + 1 + n - conf;
}

template<typename T>
static rdint_t redirect_call_get_inout_conf_val(const char *conf, const char *id, T *x);


template<typename T, int F>
static rdint_t redirect_call_put_inout_conf_vals(char *conf, const char *id, const T *x, rdint_t n)
{
  char *s = conf;

  s += sprintf(s, "%s", id);

  rdint_t i;
  for (i = 0; i < n; ++i)
  {
    *s = ':';
    ++s;

    s += sprintf(s, PRINTF_FMT(F), x[i]);
  }

  *s = ' ';
  ++s;
  *s = '\0';

  return s - conf;
}

template<typename T>
static rdint_t redirect_call_put_inout_conf_vals(char *conf, const char *id, const T *x, rdint_t n);


template<typename T, int F>
static rdint_t redirect_call_get_inout_conf_vals(const char *conf, const char *id, T *x, rdint_t n)
{
  const char *s = conf;
  
  s = strchr(s, ':');

  char t[32];
  sprintf(t, ":%s %%n", SCANF_FMT(F));

  rdint_t i;
  for (i = 0; i < n; ++i)
  {
    int ns;
    sscanf(s, t, SCANF_PTR(F, &x[i]), &ns);
    s += ns;
  }

  return s - conf;
}

template<typename T>
static rdint_t redirect_call_get_inout_conf_vals(const char *conf, const char *id, T *x, rdint_t n);

#define DEFINE_CONF(_t_, _tn_, _fmt_) \
  template<> \
  rdint_t redirect_call_put_inout_conf_val<_t_>(char *conf, const char *id, _t_ x) { \
    return redirect_call_put_inout_conf_val<_t_, _fmt_>(conf, id, x); \
  } \
  template<> \
  rdint_t redirect_call_get_inout_conf_val<_t_>(const char *conf, const char *id, _t_ *x) { \
    return redirect_call_get_inout_conf_val<_t_, _fmt_>(conf, id, x); \
  } \
  template<> \
  rdint_t redirect_call_put_inout_conf_vals<_t_>(char *conf, const char *id, const _t_ *x, rdint_t n) { \
    return redirect_call_put_inout_conf_vals<_t_, _fmt_>(conf, id, x, n); \
  } \
  template<> \
  rdint_t redirect_call_get_inout_conf_vals<_t_>(const char *conf, const char *id, _t_ *x, rdint_t n) { \
    return redirect_call_get_inout_conf_vals<_t_, _fmt_>(conf, id, x, n); \
  }


static rdint_t redirect_call_put_inout_conf_str(char *conf, const char *id, const char *x, rdint_t n)
{
  if (n < 0) n = strlen(x);

  return sprintf(conf, "%s:%d:%.*s ", id, n, n, x);
}


static rdint_t redirect_call_get_inout_conf_str(const char *conf, const char *id, char *x, rdint_t *n)
{
  rdint_t _n = 0;

  if (!n) n = &_n;

  const char *s = strchr(conf, ':');
  sscanf(s + 1, "%" rdint_fmt ":", n);

  s = strchr(s + 1, ':');
  strncpy(x, s + 1, *n);
  x[*n] = '\0';

  return s + 1 + *n + 1 - conf;
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

  rc->result[0] = '\0';

#if REDIRECT_CALL_PARAMS_NEW
  rc->nparams = 0;
#else
  rc->dense_nparams = 0;
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

#if REDIRECT_CALL_PARAMS_NEW
# error FIXME
  // if (rc->[0] < rc->blocks_nparams)
  // {
  //   rdint_t i = rc->input_state[0];
  // 
  //   SCDC_DATASET_INOUT_BUF_PTR(rc->input) = rc->blocks_params[i].buf;
  //   SCDC_DATASET_INOUT_BUF_SIZE(rc->input) = SCDC_DATASET_INOUT_BUF_CURRENT(rc->input) = (rc->blocks_params[i].count - 1) * rc->blocks_params[i].stride + rc->blocks_params[i].size;
  // 
  // } else
#else
  TRACE_F("%s: dense state %d of %d", __func__, rc->input_state, rc->dense_nparams);

  if (rc->input_state < rc->dense_nparams)
  {
    rdint_t i = rc->input_state;

    SCDC_DATASET_INOUT_BUF_PTR(rc->input) = rc->dense_params[i].buf;
    SCDC_DATASET_INOUT_BUF_SIZE(rc->input) = SCDC_DATASET_INOUT_BUF_CURRENT(rc->input) = rc->dense_params[i].size;

    ++rc->input_state;

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
#if REDIRECT_CALL_PARAMS_NEW
  rc->params_state[0] = rc->params_state[1] = 0;
#else
  rc->input_state = 0;
#endif

  rc->input->next(rc->input);

  TRACE_F("%s: cmd: '%s'", __func__, cmd);

  if (scdc_dataset_cmd(rc->dataset, cmd, rc->input, rc->output) != SCDC_SUCCESS)
  {
    TRACE_F("%s: failed", __func__);
    return 0;
  }

  /* extract conf output from output */
  const char *conf = static_cast<char *>(SCDC_DATASET_INOUT_BUF_PTR(rc->output));

  const char *s = strchr(conf, ':');
  rdint_t n = 0;
  sscanf(s + 1, "%" rdint_fmt ":", &n);

  s = strchr(s + 1, ':');
  strncpy(rc->output_conf, s + 1, n);
  rc->output_conf[n] = '\0';
  rc->output_nconf = 0;

  n += s + 1 - conf;

  rc->output_offset = n;

  rdint_t nresult = sizeof(rc->result);
  redirect_call_get_output_conf_str(rc, "result", rc->result, &nresult);

  TRACE_F("%s: conf: '%s'", __func__, rc->output_conf);
  TRACE_F("%s: result: '%s', output_conf: '%s', ", __func__, rc->result, rc->output_conf + rc->output_nconf);

  return 1;
}


#define DEFINE_CLI_CONF_VAL(_t_, _tn_) \
  void redirect_call_put_input_conf_ ## _tn_(redirect_call_t *rc, const char *id, _t_ x) { \
    rc->input_nconf += redirect_call_put_inout_conf_val<_t_>(rc->input_conf + rc->input_nconf, id, x); \
  } \
  void redirect_call_get_output_conf_ ## _tn_(redirect_call_t *rc, const char *id, _t_ *x) { \
    rc->output_nconf += redirect_call_get_inout_conf_val<_t_>(rc->output_conf + rc->output_nconf, id, x); \
  } \
  void redirect_call_put_input_conf_ ## _tn_ ## s(redirect_call_t *rc, const char *id, const _t_ *x, rdint_t n) { \
    rc->input_nconf += redirect_call_put_inout_conf_vals<_t_>(rc->input_conf + rc->input_nconf, id, x, n); \
  } \
  void redirect_call_get_output_conf_ ## _tn_ ## s(redirect_call_t *rc, const char *id, _t_ *x, rdint_t n) { \
    rc->output_nconf += redirect_call_get_inout_conf_vals<_t_>(rc->output_conf + rc->output_nconf, id, x, n); \
  }


void redirect_call_put_input_conf_rdint(redirect_call_t *rc, const char *id, rdint_t x)
{
  Z_CONCAT(redirect_call_put_input_conf_, rdint_type)(rc, id, x);
}


void redirect_call_get_output_conf_rdint(redirect_call_t *rc, const char *id, rdint_t *x)
{
  Z_CONCAT(redirect_call_get_output_conf_, rdint_type)(rc, id, x);
}


void redirect_call_put_input_conf_rdints(redirect_call_t *rc, const char *id, const rdint_t *x, rdint_t n)
{
  Z_CONCONCAT(redirect_call_put_input_conf_, rdint_type, s)(rc, id, x, n);
}


void redirect_call_get_output_conf_rdints(redirect_call_t *rc, const char *id, rdint_t *x, rdint_t n)
{
  Z_CONCONCAT(redirect_call_get_output_conf_, rdint_type, s)(rc, id, x, n);
}


void redirect_call_put_input_conf_str(redirect_call_t *rc, const char *id, const char *x, rdint_t n)
{
  rc->input_nconf += redirect_call_put_inout_conf_str(rc->input_conf + rc->input_nconf, id, x, n);
}


void redirect_call_get_output_conf_str(redirect_call_t *rc, const char *id, char *x, rdint_t *n)
{
  rc->output_nconf += redirect_call_get_inout_conf_str(rc->output_conf + rc->output_nconf, id, x, n);
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

  rc->result[0] = '\0';

#if REDIRECT_CALL_PARAMS_NEW
  rc->nparams = 0;
#else
  rc->dense_nparams = 0;
#endif

  rc->input_offset = rc->output_offset = 0;
  rc->input = input;
  rc->output = output;

  rc->free_nbufs = 0;
}


static scdcint_t release_scdc_output_next(scdc_dataset_output_t *output)
{
  redirect_call_t *rc = static_cast<redirect_call_t *>(output->data);

#if REDIRECT_CALL_PARAMS_NEW
# error FIXME
#else
  TRACE_F("%s: dense state %d of %d", __func__, rc->output_state, rc->dense_nparams);

  if (rc->output_state < rc->dense_nparams)
  {
    rdint_t i = rc->output_state;

    SCDC_DATASET_INOUT_BUF_PTR(rc->output) = rc->dense_params[i].buf;
    SCDC_DATASET_INOUT_BUF_SIZE(rc->output) = rc->dense_params[i].size;
    SCDC_DATASET_INOUT_BUF_CURRENT(rc->output) = rc->dense_params[i].size;

    ++rc->output_state;

  } else
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

  /* prepend result to output_conf (TODO: use scdc_result if available) */
  char output_conf[REDIRECT_CALL_OUTPUT_CONF_MAX_SIZE];
  rdint_t output_nconf = rc->output_nconf;
  strncpy(output_conf, rc->output_conf, rc->output_nconf);

  rc->output_nconf = 0;
  rc->output_conf[0] = '\0';

  redirect_call_put_output_conf_str(rc, "result", rc->result, -1);

  strncpy(rc->output_conf + rc->output_nconf, output_conf, output_nconf);
  rc->output_nconf += output_nconf;

  /* prepend size */
  output_nconf = rc->output_nconf;
  strncpy(output_conf, rc->output_conf, rc->output_nconf);

  rc->output_nconf = sprintf(rc->output_conf, "conf:%" rdint_fmt ":%.*s", output_nconf, (int) output_nconf, output_conf);

  /* start output with output_conf */
  SCDC_DATASET_INOUT_BUF_PTR(rc->output) = rc->output_conf;
  SCDC_DATASET_INOUT_BUF_SIZE(rc->output) = rc->output_nconf;

  SCDC_DATASET_INOUT_BUF_CURRENT(rc->output) = rc->output_nconf;

  rc->output->next = release_scdc_output_next;
  rc->output->data = rc;
#if REDIRECT_CALL_PARAMS_NEW
  rc->params_state[0] = rc->params_state[1] = 0;
#else
  rc->output_state = 0;
#endif
}


#define DEFINE_SRV_CONF_VAL(_t_, _tn_) \
  void redirect_call_get_input_conf_ ## _tn_(redirect_call_t *rc, const char *id, _t_ *x) { \
    rc->input_nconf += redirect_call_get_inout_conf_val<_t_>(rc->input_conf + rc->input_nconf, id, x); \
  } \
  void redirect_call_put_output_conf_ ## _tn_(redirect_call_t *rc, const char *id, _t_ x) { \
    rc->output_nconf += redirect_call_put_inout_conf_val<_t_>(rc->output_conf + rc->output_nconf, id, x); \
  } \
  void redirect_call_get_input_conf_ ## _tn_ ## s(redirect_call_t *rc, const char *id, _t_ *x, rdint_t n) { \
    rc->input_nconf += redirect_call_get_inout_conf_vals<_t_>(rc->input_conf + rc->input_nconf, id, x, n); \
  } \
  void redirect_call_put_output_conf_ ## _tn_ ## s(redirect_call_t *rc, const char *id, const _t_ *x, rdint_t n) { \
    rc->output_nconf += redirect_call_put_inout_conf_vals<_t_>(rc->output_conf + rc->output_nconf, id, x, n); \
  }


void redirect_call_get_input_conf_rdint(redirect_call_t *rc, const char *id, rdint_t *x)
{
  Z_CONCAT(redirect_call_get_input_conf_, rdint_type)(rc, id, x);
}


void redirect_call_put_output_conf_rdint(redirect_call_t *rc, const char *id, rdint_t x)
{
  Z_CONCAT(redirect_call_put_output_conf_, rdint_type)(rc, id, x);
}


void redirect_call_get_input_conf_rdints(redirect_call_t *rc, const char *id, rdint_t *x, rdint_t n)
{
  Z_CONCONCAT(redirect_call_get_input_conf_, rdint_type, s)(rc, id, x, n);
}


void redirect_call_put_output_conf_rdints(redirect_call_t *rc, const char *id, const rdint_t *x, rdint_t n)
{
  Z_CONCONCAT(redirect_call_put_output_conf_, rdint_type, s)(rc, id, x, n);
}


void redirect_call_get_input_conf_str(redirect_call_t *rc, const char *id, char *x, rdint_t *n)
{
  rc->input_nconf += redirect_call_get_inout_conf_str(rc->input_conf + rc->input_nconf, id, x, n);
}


void redirect_call_put_output_conf_str(redirect_call_t *rc, const char *id, const char *x, rdint_t n)
{
  rc->output_nconf += redirect_call_put_inout_conf_str(rc->output_conf + rc->output_nconf, id, x, n);
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

  char *rh_id_ptr = rh->id;

  if (rc->client)
  {
    rh->dataset = rc->dataset;
    redirect_call_get_output_conf_char_p(rc, "hdl_id", reinterpret_cast<char **>(&rh_id_ptr));
    redirect_call_get_output_conf_void_p(rc, "hdl_ptr", &rh->ptr);

  } else
  {
    redirect_call_get_input_conf_char_p(rc, "hdl_id", reinterpret_cast<char **>(&rh_id_ptr));
    redirect_call_get_input_conf_void_p(rc, "hdl_ptr", &rh->ptr);
  }
}


/* result */

void redirect_call_set_result(redirect_call_t *rc, const char *result)
{
  strncpy(rc->result, result, sizeof(rc->result));
}


const char * redirect_call_get_result(redirect_call_t *rc, char *result, rdint_t n)
{
  if (result) strncpy(result, rc->result, n);

  return rc->result;
}


/* dense data */

static void redirect_call_put_input_param_dense(redirect_call_t *rc, const char *id, const void *b, rdint_t n)
{
  /* client only */
  ASSERT(rc->client);

  /* register dense input */
#if REDIRECT_CALL_PARAMS_NEW
  ASSERT(rc->nparams < REDIRECT_CALL_PARAMS_MAX);

  rc->params[rc->nparams].type = REDIRECT_PARAM_TYPE_DENSE;
  rc->params[rc->nparams].p.dense.buf = const_cast<void *>(b);
  rc->params[rc->nparams].p.dense.size = n;

  ++rc->nparams;
#else
  rc->dense_params[rc->dense_nparams].buf = const_cast<void *>(b);
  rc->dense_params[rc->dense_nparams].size = n;

  ++rc->dense_nparams;
#endif
}


static void redirect_call_get_input_param_dense(redirect_call_t *rc, const char *id, void **b, rdint_t n)
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


static void redirect_call_put_output_param_dense(redirect_call_t *rc, const char *id, const void *b, rdint_t n)
{
  if (rc->client)
  {
    /* puffer in output hinterlegen */
    /* FIXME */

  } else
  {
    /* register dense output */
#if REDIRECT_CALL_PARAMS_NEW
    rc->params[rc->nparams].type = REDIRECT_PARAM_TYPE_DENSE;
    rc->params[rc->nparams].p.dense.buf = const_cast<void *>(b);
    rc->params[rc->nparams].p.dense.size = n;

    ++rc->nparams;
#else
    rc->dense_params[rc->dense_nparams].buf = const_cast<void *>(b);
    rc->dense_params[rc->dense_nparams].size = n;

    ++rc->dense_nparams;
#endif
  }
}


void redirect_call_get_output_param_dense(redirect_call_t *rc, const char *id, void **b, rdint_t n)
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


static void redirect_call_put_inout_param_dense(redirect_call_t *rc, const char *id, const void *b, rdint_t n)
{
  redirect_call_put_input_param_dense(rc, id, b, n);
  redirect_call_put_output_param_dense(rc, id, b, n);
}


static void redirect_call_get_inout_param_dense(redirect_call_t *rc, const char *id, void **b, rdint_t n)
{
  redirect_call_get_input_param_dense(rc, id, b, n);
  redirect_call_get_output_param_dense(rc, id, b, n);
}


/* blocks data */

template<typename T>
static void redirect_call_put_input_param_blocks_val(redirect_call_t *rc, const char *id, const T *b, rdint_t count, rdint_t size, rdint_t stride)
{
  /* client only */
  ASSERT(rc->client);

#if REDIRECT_CALL_INPUT_PARAM_BLOCKS == REDIRECT_CALL_BLOCKS_DENSE
  /* nothing to be done here */
#elif REDIRECT_CALL_INPUT_PARAM_BLOCKS == REDIRECT_CALL_BLOCKS_PACK_IP
  if (stride > size)
  {
    TRACE_F("%s: size: %" rdint_fmt ", stride: %" rdint_fmt ", transform in-place to stride: %" rdint_fmt, __func__, size, stride, size);
    redirect_data_blocks_transform_val<T>(count, size, stride, const_cast<T *>(b), size, NULL);
    stride = size;
  }
#elif REDIRECT_CALL_INPUT_PARAM_BLOCKS == REDIRECT_CALL_BLOCKS_PACK_OP
  if (stride > size)
  {
    TRACE_F("%s: size: %" rdint_fmt ", stride: %" rdint_fmt ", transform out-of-place to stride: %" rdint_fmt, __func__, size, stride, size);

    T *b_tmp = static_cast<T *>(malloc(count * size * sizeof(T)));

    rc->free_bufs[rc->free_nbufs] = b_tmp;
    ++rc->free_nbufs;

    redirect_data_blocks_transform_val<T>(count, size, stride, const_cast<T *>(b), size, b_tmp);
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

    redirect_call_put_input_param_dense(rc, id, b, get_blocks_cont_size(count, size, stride) * sizeof(T));
  }
}


template<typename T>
static void redirect_call_get_input_param_blocks_val(redirect_call_t *rc, const char *id, T **b, rdint_t count, rdint_t size, rdint_t *stride)
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

    redirect_call_get_input_param_dense(rc, id, reinterpret_cast<void **>(&b_tmp), get_blocks_cont_size(count, size, stride_in) * sizeof(T));

    TRACE_F("%s: size: %" rdint_fmt ", stride: %" rdint_fmt ", transform out-of-place to stride: %" rdint_fmt , __func__, size, stride_in, *stride);
    redirect_data_blocks_transform_val<T>(count, size, stride_in, b_tmp, *stride, *b);

    free(b_tmp);

  } else
#endif
  {
    /* currently: if the buffer is allocated using stride_in, then we can not transform into a larger stride afterwards */
    ASSERT(!(*b == NULL && *stride > stride_in));

    redirect_call_get_input_param_dense(rc, id, reinterpret_cast<void **>(b), get_blocks_cont_size(count, size, stride_in) * sizeof(T));

    if (*stride != stride_in)
    {
      TRACE_F("%s: size: %" rdint_fmt ", stride: %" rdint_fmt ", transform in-place to stride: %" rdint_fmt , __func__, size, stride_in, *stride);
      redirect_data_blocks_transform_val<T>(count, size, stride_in, *b, *stride, NULL);
    }
  }
}


template<typename T>
static void redirect_call_put_output_param_blocks_val(redirect_call_t *rc, const char *id, const T *b, rdint_t count, rdint_t size, rdint_t stride)
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
      redirect_data_blocks_transform_val<T>(count, size, stride, const_cast<T *>(b), size, NULL);
      stride = size;
#endif
#if REDIRECT_CALL_OUTPUT_PARAM_BLOCKS == REDIRECT_CALL_BLOCKS_PACK_OP
      TRACE_F("%s: size: %" rdint_fmt ", stride: %" rdint_fmt ", transform out-of-place to stride: %" rdint_fmt, __func__, size, stride, size);

      T *b_tmp = static_cast<T *>(malloc(count * size * sizeof(T)));

      rc->free_bufs[rc->free_nbufs] = b_tmp;
      ++rc->free_nbufs;

      redirect_data_blocks_transform_val<T>(count, size, stride, const_cast<T *>(b), size, b_tmp);
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

    redirect_call_put_output_param_dense(rc, id, b, get_blocks_cont_size(count, size, stride) * sizeof(T));
  }
}


template<typename T>
void redirect_call_get_output_param_blocks_val(redirect_call_t *rc, const char *id, T **b, rdint_t count, rdint_t size, rdint_t *stride)
{
  if (!rc->client)
  {
    /* server-side */
    rdint_t stride_in;

    redirect_call_get_input_conf_int(rc, "bstride", &stride_in);

    if (*stride == 0) *stride = stride_in;

    redirect_call_get_output_param_dense(rc, id, reinterpret_cast<void **>(b), get_blocks_cont_size(count, size, stride_in) * sizeof(T));

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

      redirect_call_get_output_param_dense(rc, id, reinterpret_cast<void **>(&b_tmp), get_blocks_cont_size(count, size, stride_in) * sizeof(T));

      TRACE_F("%s: size: %" rdint_fmt ", stride: %" rdint_fmt ", transform out-of-place to stride: %" rdint_fmt , __func__, size, stride_in, *stride);
      redirect_data_blocks_transform_val<T>(count, size, stride_in, b_tmp, *stride, *b);

      free(b_tmp);

    } else
#endif
    {
      /* currently: if the buffer is allocated using stride_in, then we can not transform into a larger stride afterwards */
      ASSERT(!(*b == NULL && *stride > stride_in));

      redirect_call_get_output_param_dense(rc, id, reinterpret_cast<void **>(b), get_blocks_cont_size(count, size, stride_in) * sizeof(T));

      if (*stride != stride_in)
      {
        TRACE_F("%s: size: %" rdint_fmt ", stride: %" rdint_fmt ", transform in-place to stride: %" rdint_fmt , __func__, size, stride_in, *stride);
        redirect_data_blocks_transform_val<T>(count, size, stride_in, *b, *stride, NULL);
      }
    }
  }
}


template<typename T>
static void redirect_call_put_inout_param_blocks_val(redirect_call_t *rc, const char *id, const T *b, rdint_t count, rdint_t size, rdint_t stride)
{
  /* client only */
  ASSERT(rc->client);

#if REDIRECT_CALL_INOUT_PARAM_BLOCKS == REDIRECT_CALL_BLOCKS_DENSE
  /* nothing to be done here */
#elif REDIRECT_CALL_INOUT_PARAM_BLOCKS == REDIRECT_CALL_BLOCKS_PACK_IP
  if (stride > size)
  {
    TRACE_F("%s: size: %" rdint_fmt ", stride: %" rdint_fmt ", transform in-place to stride: %" rdint_fmt, __func__, size, stride, size);
    redirect_data_blocks_transform_val<T>(count, size, stride, const_cast<T *>(b), size, NULL);
    stride = size;
  }
#elif REDIRECT_CALL_INOUT_PARAM_BLOCKS == REDIRECT_CALL_BLOCKS_PACK_OP
  if (stride > size)
  {
    TRACE_F("%s: size: %" rdint_fmt ", stride: %" rdint_fmt ", transform out-of-place to stride: %" rdint_fmt, __func__, size, stride, size);

    T *b_tmp = static_cast<T *>(malloc(count * size * sizeof(T)));

    rc->free_bufs[rc->free_nbufs] = b_tmp;
    ++rc->free_nbufs;

    redirect_data_blocks_transform_val<T>(count, size, stride, const_cast<T *>(b), size, b_tmp);
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

    redirect_call_put_inout_param_dense(rc, id, b, get_blocks_cont_size(count, size, stride) * sizeof(T));
  }
}


template<typename T>
static void redirect_call_get_inout_param_blocks_val(redirect_call_t *rc, const char *id, T **b, rdint_t count, rdint_t size, rdint_t *stride)
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

    redirect_call_get_inout_param_dense(rc, id, reinterpret_cast<void **>(&b_tmp), get_blocks_cont_size(count, size, stride_in) * sizeof(T));

    TRACE_F("%s: size: %" rdint_fmt ", stride: %" rdint_fmt ", transform out-of-place to stride: %" rdint_fmt , __func__, size, stride_in, *stride);
    redirect_data_blocks_transform_val<T>(count, size, stride_in, b_tmp, *stride, *b);

    free(b_tmp);

  } else
#endif
  {
    /* currently: if the buffer is allocated using stride_in, then we can not transform into a larger stride afterwards */
    ASSERT(!(*b == NULL && *stride > stride_in));

    redirect_call_get_inout_param_dense(rc, id, reinterpret_cast<void **>(b), get_blocks_cont_size(count, size, stride_in) * sizeof(T));

    if (*stride != stride_in)
    {
      TRACE_F("%s: size: %" rdint_fmt ", stride: %" rdint_fmt ", transform in-place to stride: %" rdint_fmt , __func__, size, stride_in, *stride);
      redirect_data_blocks_transform_val<T>(count, size, stride_in, *b, *stride, NULL);
    }
  }
}


/* plain bytes */

void redirect_call_put_input_param_bytes(redirect_call_t *rc, const char *id, const void *b, rdint_t n)
{
  redirect_call_put_input_conf_rdint(rc, "bn", n);
  redirect_call_put_input_param_dense(rc, id, b, n);
}


void redirect_call_get_input_param_bytes(redirect_call_t *rc, const char *id, void **b, rdint_t *n)
{
  redirect_call_get_input_conf_rdint(rc, "bn", n);
  redirect_call_get_input_param_dense(rc, id, b, *n);
}


void redirect_call_put_output_param_bytes(redirect_call_t *rc, const char *id, rdint_t n, const void *b)
{
  if (rc->client) redirect_call_put_input_conf_rdint(rc, "bn", n);
  else redirect_call_put_output_conf_rdint(rc, "bn", n);
  redirect_call_put_output_param_dense(rc, id, b, n);
}


void redirect_call_get_output_param_bytes(redirect_call_t *rc, const char *id, void **b, rdint_t *n)
{
  if (rc->client) redirect_call_get_output_conf_rdint(rc, "bn", n);
  else redirect_call_get_input_conf_rdint(rc, "bn", n);
  redirect_call_get_output_param_dense(rc, id, b, *n);
}


void redirect_call_put_inout_param_bytes(redirect_call_t *rc, const char *id, const void *b, rdint_t n)
{
  if (rc->client) redirect_call_put_input_conf_rdint(rc, "bn", n);
  else redirect_call_put_output_conf_rdint(rc, "bn", n);
  redirect_call_put_inout_param_dense(rc, id, b, n);
}


void redirect_call_get_inout_param_bytes(redirect_call_t *rc, const char *id, void **b, rdint_t *n)
{
  if (rc->client) redirect_call_get_output_conf_rdint(rc, "bn", n);
  else redirect_call_get_input_conf_rdint(rc, "bn", n);
  redirect_call_get_inout_param_dense(rc, id, b, *n);
}


#include "redirect_call_array.tcc"
#include "redirect_call_vector.tcc"
#include "redirect_call_matrix.tcc"


/* float */

#define DEFINE_FLOAT_ARRAY(_t_, _tn_) \
  void redirect_call_print_param_array_ ## _tn_(const _t_ *a, rdint_t n) { \
    redirect_call_print_param_array_val<_t_>(a, n); \
  } \
  void redirect_call_put_input_param_array_ ## _tn_(redirect_call_t *rc, const char *id, const _t_ *a, rdint_t n) { \
    redirect_call_put_input_param_array_val<_t_>(rc, id, a, n); \
  } \
  void redirect_call_get_input_param_array_ ## _tn_(redirect_call_t *rc, const char *id, _t_ **a, rdint_t *n) { \
    redirect_call_get_input_param_array_val<_t_>(rc, id, a, n); \
  } \
  void redirect_call_put_output_param_array_ ## _tn_(redirect_call_t *rc, const char *id, const _t_ *a, rdint_t n) { \
    redirect_call_put_output_param_array_val<_t_>(rc, id, a, n); \
  } \
  void redirect_call_get_output_param_array_ ## _tn_(redirect_call_t *rc, const char *id, _t_ **a, rdint_t *n) { \
    redirect_call_get_output_param_array_val<_t_>(rc, id, a, n); \
  } \
  void redirect_call_put_inout_param_array_ ## _tn_(redirect_call_t *rc, const char *id, const _t_ *a, rdint_t n) { \
    redirect_call_put_inout_param_array_val<_t_>(rc, id, a, n); \
  } \
  void redirect_call_get_inout_param_array_ ## _tn_(redirect_call_t *rc, const char *id, _t_ **a, rdint_t *n) { \
    redirect_call_get_inout_param_array_val<_t_>(rc, id, a, n); \
  }

#define DEFINE_FLOAT_VECTOR(_t_, _tn_) \
  void redirect_call_print_param_vector_ ## _tn_(const _t_ *v, rdint_t n, rdint_t inc) { \
    redirect_call_print_param_vector_val<_t_>(v, n, inc); \
  } \
  void redirect_call_put_input_param_vector_ ## _tn_(redirect_call_t *rc, const char *id, const _t_ *v, rdint_t n, rdint_t inc) { \
    redirect_call_put_input_param_vector_val<_t_>(rc, id, v, n, inc); \
  } \
  void redirect_call_get_input_param_vector_ ## _tn_(redirect_call_t *rc, const char *id, _t_ **v, rdint_t *n, rdint_t *inc) { \
    redirect_call_get_input_param_vector_val<_t_>(rc, id, v, n, inc); \
  } \
  void redirect_call_put_output_param_vector_ ## _tn_(redirect_call_t *rc, const char *id, const _t_ *v, rdint_t n, rdint_t inc) { \
    redirect_call_put_output_param_vector_val<_t_>(rc, id, v, n, inc); \
  } \
  void redirect_call_get_output_param_vector_ ## _tn_(redirect_call_t *rc, const char *id, _t_ **v, rdint_t *n, rdint_t *inc) { \
    redirect_call_get_output_param_vector_val<_t_>(rc, id, v, n, inc); \
  } \
  void redirect_call_put_inout_param_vector_ ## _tn_(redirect_call_t *rc, const char *id, const _t_ *v, rdint_t n, rdint_t inc) { \
    redirect_call_put_inout_param_vector_val<_t_>(rc, id, v, n, inc); \
  } \
  void redirect_call_get_inout_param_vector_ ## _tn_(redirect_call_t *rc, const char *id, _t_ **v, rdint_t *n, rdint_t *inc) { \
    redirect_call_get_inout_param_vector_val<_t_>(rc, id, v, n, inc); \
  }

#define DEFINE_FLOAT_MATRIX(_t_, _tn_) \
  void redirect_call_print_param_matrix_ ## _tn_(const _t_ *m, rdint_t nr, rdint_t nc, rdint_t ld, rdint_t rcm) { \
    redirect_call_print_param_matrix_val<_t_>(m, nr, nc, ld, rcm); \
  } \
  void redirect_call_put_input_param_matrix_ ## _tn_(redirect_call_t *rc, const char *id, const _t_ *m, rdint_t nr, rdint_t nc, rdint_t ld, rdint_t rcm) { \
    redirect_call_put_input_param_matrix_val<_t_>(rc, id, m, nr, nc, ld, rcm); \
  } \
  void redirect_call_get_input_param_matrix_ ## _tn_(redirect_call_t *rc, const char *id, _t_ **m, rdint_t *nr, rdint_t *nc, rdint_t *ld, rdint_t *rcm) { \
    redirect_call_get_input_param_matrix_val<_t_>(rc, id, m, nr, nc, ld, rcm); \
  } \
  void redirect_call_put_output_param_matrix_ ## _tn_(redirect_call_t *rc, const char *id, const _t_ *m, rdint_t nr, rdint_t nc, rdint_t ld, rdint_t rcm) { \
    redirect_call_put_output_param_matrix_val<_t_>(rc, id, m, nr, nc, ld, rcm); \
  } \
  void redirect_call_get_output_param_matrix_ ## _tn_(redirect_call_t *rc, const char *id, _t_ **m, rdint_t *nr, rdint_t *nc, rdint_t *ld, rdint_t *rcm) { \
    redirect_call_get_output_param_matrix_val<_t_>(rc, id, m, nr, nc, ld, rcm); \
  } \
  void redirect_call_put_inout_param_matrix_ ## _tn_(redirect_call_t *rc, const char *id, const _t_ *m, rdint_t nr, rdint_t nc, rdint_t ld, rdint_t rcm) { \
    redirect_call_put_inout_param_matrix_val<_t_>(rc, id, m, nr, nc, ld, rcm); \
  } \
  void redirect_call_get_inout_param_matrix_ ## _tn_(redirect_call_t *rc, const char *id, _t_ **m, rdint_t *nr, rdint_t *nc, rdint_t *ld, rdint_t *rcm) { \
    redirect_call_get_inout_param_matrix_val<_t_>(rc, id, m, nr, nc, ld, rcm); \
  }


/* int */

#define DEFINE_INT_ARRAY(_t_, _tn_) \
  void redirect_call_print_param_array_ ## _tn_(const _t_ *a, rdint_t n) { \
    redirect_call_print_param_array_val<_t_>(a, n); \
  } \
  void redirect_call_put_input_param_array_ ## _tn_(redirect_call_t *rc, const char *id, const _t_ *a, rdint_t n) { \
    redirect_call_put_input_param_array_val<_t_>(rc, id, a, n); \
  } \
  void redirect_call_get_input_param_array_ ## _tn_(redirect_call_t *rc, const char *id, _t_ **a, rdint_t *n) { \
    redirect_call_get_input_param_array_val<_t_>(rc, id, a, n); \
  } \
  void redirect_call_put_output_param_array_ ## _tn_(redirect_call_t *rc, const char *id, const _t_ *a, rdint_t n) { \
    redirect_call_put_output_param_array_val<_t_>(rc, id, a, n); \
  } \
  void redirect_call_get_output_param_array_ ## _tn_(redirect_call_t *rc, const char *id, _t_ **a, rdint_t *n) { \
    redirect_call_get_output_param_array_val<_t_>(rc, id, a, n); \
  } \
  void redirect_call_put_inout_param_array_ ## _tn_(redirect_call_t *rc, const char *id, const _t_ *a, rdint_t n) { \
    redirect_call_put_inout_param_array_val<_t_>(rc, id, a, n); \
  } \
  void redirect_call_get_inout_param_array_ ## _tn_(redirect_call_t *rc, const char *id, _t_ **a, rdint_t *n) { \
    redirect_call_get_inout_param_array_val<_t_>(rc, id, a, n); \
  }


#define DEFINE_FLOAT(_t_, _tn_, _fmt_) \
  DEFINE_CONF(_t_, _tn_, _fmt_); \
  DEFINE_CLI_CONF_VAL(_t_, _tn_); \
  DEFINE_SRV_CONF_VAL(_t_, _tn_); \
  DEFINE_PRINT_ARRAY(_t_, _tn_, _fmt_); \
  DEFINE_PRINT_VECTOR(_t_, _tn_, _fmt_); \
  DEFINE_PRINT_MATRIX(_t_, _tn_, _fmt_); \
  DEFINE_FLOAT_ARRAY(_t_, _tn_); \
  DEFINE_FLOAT_VECTOR(_t_, _tn_); \
  DEFINE_FLOAT_MATRIX(_t_, _tn_);

DEFINE_FLOAT(float, float, FMT_FLOAT);
DEFINE_FLOAT(double, double, FMT_DOUBLE);

#define DEFINE_INT(_t_, _tn_, _fmt_) \
  DEFINE_CONF(_t_, _tn_, _fmt_); \
  DEFINE_CLI_CONF_VAL(_t_, _tn_); \
  DEFINE_SRV_CONF_VAL(_t_, _tn_); \
  DEFINE_PRINT_ARRAY(_t_, _tn_, _fmt_); \
  DEFINE_INT_ARRAY(_t_, _tn_); \

DEFINE_INT(char, char, FMT_CHAR);
DEFINE_INT(int, int, FMT_INT);

DEFINE_CONF(void_p, void_p, FMT_VOID_PTR);
DEFINE_CLI_CONF_VAL(void_p, void_p);
DEFINE_SRV_CONF_VAL(void_p, void_p);
DEFINE_CONF(char_p, char_p, FMT_CHAR_PTR);
DEFINE_CLI_CONF_VAL(char_p, char_p);
DEFINE_SRV_CONF_VAL(char_p, char_p);
