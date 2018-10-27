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

#include "xxhash.h"

#include "redirect_config.h"
#include "redirect_param.h"
#include "redirect_handle.h"
#include "redirect_cache.h"
#include "redirect_types.h"
#include "redirect_data.h"
#include "redirect_call.h"
#include "common.h"


#define TRACE_PREFIX  "redirect_call: "


#define TRACE_DATA  0

#ifndef HAVE_TRACE
# undef TRACE_DATA
#endif


template<typename T>
int get_type();

#define DEFINE_TYPE(_t_, _tn_, _fmt_) \
  template<> \
  int get_type<_t_>() { \
    return _fmt_; \
  } \


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

  return sprintf(conf, "%s:%" rdint_fmt ":%.*s ", id, n, static_cast<int>(n), x);
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


static void *redirect_call_alloc_buf(redirect_call_t *rc, rdint_t size)
{
  if (rc->free_nbufs >= REDIRECT_CALL_FREE_BUFS_MAX) return NULL;

  void *buf = malloc(size);

  rc->free_bufs[rc->free_nbufs] = buf;
  ++rc->free_nbufs;

  return buf;
}


static void redirect_call_free_buf(redirect_call_t *rc, rdint_t buf_index)
{
  if (buf_index < 0 || buf_index >= rc->free_nbufs) return;

  free(rc->free_bufs[buf_index]);

  --rc->free_nbufs;

  if (buf_index != rc->free_nbufs) rc->free_bufs[buf_index] = rc->free_bufs[rc->free_nbufs];
}


static void redirect_call_free_bufs(redirect_call_t *rc)
{
#if 1
  while (rc->free_nbufs > 0) redirect_call_free_buf(rc, rc->free_nbufs - 1);
#else
  rdint_t i;
  for (i = 0; i < rc->free_nbufs; ++i) free(rc->free_bufs[i]);
  rc->free_nbufs = 0;
#endif
}

#if REDIRECT_CALL_PARAMS_CACHE

static rdint_t redirect_call_find_buf(redirect_call_t *rc, const void *buf)
{
  rdint_t i;
  for (i = 0; i < rc->free_nbufs; ++i)
  {
    if (rc->free_bufs[i] == buf) return i;
  }

  return -1;
}


static void redirect_call_remove_buf(redirect_call_t *rc, rdint_t buf_index)
{
  if (buf_index < 0 || buf_index >= rc->free_nbufs) return;

  --rc->free_nbufs;

  if (buf_index != rc->free_nbufs) rc->free_bufs[buf_index] = rc->free_bufs[rc->free_nbufs];
}

#endif /* REDIRECT_CALL_PARAMS_CACHE */


/* client side */

int redirect_call_create_scdc(redirect_call_t *rc, const char *op, const char *uri)
{
  rc->client = 1;

  strncpy(rc->op, op, sizeof(rc->op));
  if (uri) strncpy(rc->uri, uri, sizeof(rc->uri));
  else rc->uri[0] = '\0';

  rc->dataset = SCDC_DATASET_NULL;

  rc->input_conf_n = 0;
  rc->input_conf[0] = '\0';

  rc->output_conf_put_n = rc->output_conf_get_n = 0;
  rc->output_conf_put[0] = rc->output_conf_get[0] = '\0';

  rc->result_str[0] = '\0';

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

#if REDIRECT_CALL_PARAMS_CACHE
  rc->cache = 0;
#endif

  return 1;
}


void redirect_call_destroy_scdc(redirect_call_t *rc)
{
  /* consume remaining output */
  while (rc->output->next) rc->output->next(rc->output, 0);

  free(rc->input);

  redirect_call_free_bufs(rc);
}


static scdcint_t do_input_next(scdc_dataset_input_t *input, scdc_result_t *result)
{
  redirect_call_t *rc = static_cast<redirect_call_t *>(input->data);

  bool finish = true;

#if REDIRECT_CALL_PARAMS_CACHE
do_state:
#endif

#if REDIRECT_CALL_PARAMS_NEW
  TRACE_F("params state %" rdint_fmt " of %" rdint_fmt, rc->params_state[0], rc->nparams);

  if (rc->params_state[0] < rc->nparams)
  {
    rdint_t i = rc->params_state[0];

#if REDIRECT_CALL_PARAMS_CACHE
    if (rc->cache && rc->params[i].cacheable)
    {
      rdint_t entry_index = redirect_cache_get_next_param_entry_index(rc->cache);

      if (entry_index >= 0)
      {
        TRACE_F("param %" rdint_fmt ": in cache, skipping to next", i);
        ++rc->params_state[0];
        goto do_state;
      }
    }
#endif

    if (rc->params[i].type == REDIRECT_PARAM_TYPE_DENSE)
    {
      TRACE_F("param %" rdint_fmt ": dense", i);

      finish = false;

      SCDC_DATASET_INOUT_BUF_PTR(rc->input) = rc->params[i].p.dense.buf;
      SCDC_DATASET_INOUT_BUF_SIZE(rc->input) = SCDC_DATASET_INOUT_BUF_CURRENT(rc->input) = rc->params[i].p.dense.size;

      ++rc->params_state[0];

    } else if (rc->params[i].type == REDIRECT_PARAM_TYPE_BLOCKS)
    {
      TRACE_F("param %" rdint_fmt ": blocks, state: %" rdint_fmt " of %" rdint_fmt, i, rc->params_state[1], rc->params[i].p.blocks.count);

      if (rc->params_state[1] < rc->params[i].p.blocks.count)
      {
        finish = false;

        SCDC_DATASET_INOUT_BUF_PTR(rc->input) = static_cast<char *>(rc->params[i].p.blocks.buf) + (rc->params_state[1] * rc->params[i].p.blocks.stride);
        SCDC_DATASET_INOUT_BUF_SIZE(rc->input) = SCDC_DATASET_INOUT_BUF_CURRENT(rc->input) = rc->params[i].p.blocks.size;

        ++rc->params_state[1];
      }

    } else ASSERT(false);
  }
#else
  TRACE_F("dense state %d of %d", rc->input_state, rc->dense_nparams);

  if (rc->input_state < rc->dense_nparams)
  {
    finish = false;

    rdint_t i = rc->input_state;

    SCDC_DATASET_INOUT_BUF_PTR(rc->input) = rc->dense_params[i].buf;
    SCDC_DATASET_INOUT_BUF_SIZE(rc->input) = SCDC_DATASET_INOUT_BUF_CURRENT(rc->input) = rc->dense_params[i].size;

    ++rc->input_state;
  }
#endif

  if (finish)
  {
    TRACE_F("freeing %d buffers", rc->free_nbufs);

    redirect_call_free_bufs(rc);

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
#if REDIRECT_CALL_PARAMS_CACHE
  char cache_state[256];
  rdint_t cache_state_size = 0;

  if (rc->cache) redirect_cache_check(rc->cache, rc->dataset, rc->uri, cache_state, &cache_state_size);

  TRACE_F("cache_state: %.*s", static_cast<int>(cache_state_size), cache_state);
#endif

  char cmd[REDIRECT_CALL_URI_MAX_SIZE + REDIRECT_CALL_OP_MAX_SIZE + REDIRECT_CALL_CACHE_STATE_MAX_SIZE + REDIRECT_CALL_INPUT_CONF_MAX_SIZE + REDIRECT_CALL_OUTPUT_CONF_MAX_SIZE];
  rdint_t cmd_size = 0;

  if (rc->dataset == SCDC_DATASET_NULL) cmd_size += sprintf(cmd, "%s ", rc->uri);

  cmd_size += sprintf(cmd + cmd_size, "%s ", rc->op);

#if REDIRECT_CALL_PARAMS_CACHE
  cmd_size += redirect_call_put_inout_conf_str(cmd + cmd_size, "cstate", cache_state, cache_state_size);
#endif
  cmd_size += redirect_call_put_inout_conf_str(cmd + cmd_size, "iconf", rc->input_conf, rc->input_conf_n);
  cmd_size += redirect_call_put_inout_conf_str(cmd + cmd_size, "oconf", rc->output_conf_put, rc->output_conf_put_n);

  rc->input->next = do_input_next;
  rc->input->data = rc;
#if REDIRECT_CALL_PARAMS_NEW
  rc->params_state[0] = rc->params_state[1] = 0;
#else
  rc->input_state = 0;
#endif

  rc->input->next(rc->input, 0);

  TRACE_F("cmd: '%s'", cmd);

  if (scdc_dataset_cmd(rc->dataset, cmd, rc->input, rc->output) != SCDC_SUCCESS)
  {
    TRACE_F("failed");
    return 0;
  }

  /* extract result and output_conf_get from output */
  const char *s = scdc_last_result();
  rdint_t n = 0;
  n += redirect_call_get_inout_conf_str(s + n, "result", rc->result_str, 0);
  rc->output_conf_get_n = 0;
  n += redirect_call_get_inout_conf_str(s + n, "oconf", rc->output_conf_get, 0);

  TRACE_F("result: '%s'", rc->result_str);
  TRACE_F("output_conf_get: '%s'", rc->output_conf_get);

  rc->output_offset = 0;

  return 1;
}


#define DEFINE_CLI_CONF_VAL(_t_, _tn_) \
  void redirect_call_put_input_conf_ ## _tn_(redirect_call_t *rc, const char *id, _t_ x) { \
    rc->input_conf_n += redirect_call_put_inout_conf_val<_t_>(rc->input_conf + rc->input_conf_n, id, x); \
  } \
  void redirect_call_get_output_conf_ ## _tn_(redirect_call_t *rc, const char *id, _t_ *x) { \
    rc->output_conf_get_n += redirect_call_get_inout_conf_val<_t_>(rc->output_conf_get + rc->output_conf_get_n, id, x); \
  } \
  void redirect_call_put_input_conf_ ## _tn_ ## s(redirect_call_t *rc, const char *id, const _t_ *x, rdint_t n) { \
    rc->input_conf_n += redirect_call_put_inout_conf_vals<_t_>(rc->input_conf + rc->input_conf_n, id, x, n); \
  } \
  void redirect_call_get_output_conf_ ## _tn_ ## s(redirect_call_t *rc, const char *id, _t_ *x, rdint_t n) { \
    rc->output_conf_get_n += redirect_call_get_inout_conf_vals<_t_>(rc->output_conf_get + rc->output_conf_get_n, id, x, n); \
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
  rc->input_conf_n += redirect_call_put_inout_conf_str(rc->input_conf + rc->input_conf_n, id, x, n);
}


void redirect_call_get_output_conf_str(redirect_call_t *rc, const char *id, char *x, rdint_t *n)
{
  rc->output_conf_get_n += redirect_call_get_inout_conf_str(rc->output_conf_get + rc->output_conf_get_n, id, x, n);
}


/* server side */

void redirect_call_init_scdc(redirect_call_t *rc)
{
  rc->client = 0;

  rc->op[0] = rc->uri[0] = '\0';

  rc->dataset = SCDC_DATASET_NULL;

  rc->input_conf_n = 0;
  rc->input_conf[0] = '\0';
  rc->output_conf_get_n = 0;
  rc->output_conf_get[0] = '\0';
  rc->output_conf_put_n = 0;
  rc->output_conf_put[0] = '\0';

  rc->result_str[0] = '\0';

#if REDIRECT_CALL_PARAMS_NEW
  rc->nparams = 0;
#else
  rc->dense_nparams = 0;
#endif

  rc->input_offset = rc->output_offset = 0;
  rc->input = 0;
  rc->output = 0;
  rc->result = 0;

  rc->free_nbufs = 0;

#if REDIRECT_CALL_PARAMS_CACHE
  rc->cache = 0;
#endif
}


static scdcint_t release_scdc_output_next(scdc_dataset_output_t *output, scdc_result_t *result)
{
  redirect_call_t *rc = static_cast<redirect_call_t *>(output->data);

  bool finish = true;

#if REDIRECT_CALL_PARAMS_NEW
  TRACE_F("params state %" rdint_fmt " of %" rdint_fmt, rc->params_state[0], rc->nparams);

  if (rc->params_state[0] < rc->nparams)
  {
    rdint_t i = rc->params_state[0];

    if (rc->params[i].type == REDIRECT_PARAM_TYPE_DENSE)
    {
      finish = false;

      SCDC_DATASET_INOUT_BUF_PTR(rc->output) = rc->params[i].p.dense.buf;
      SCDC_DATASET_INOUT_BUF_SIZE(rc->output) = SCDC_DATASET_INOUT_BUF_CURRENT(rc->output) = rc->params[i].p.dense.size;

      ++rc->params_state[0];

    } else if (rc->params[i].type == REDIRECT_PARAM_TYPE_BLOCKS)
    {
      TRACE_F("blocks state %" rdint_fmt " of %" rdint_fmt, rc->params_state[1], rc->params[i].p.blocks.count);

      if (rc->params_state[1] < rc->params[i].p.blocks.count)
      {
        finish = false;

        SCDC_DATASET_INOUT_BUF_PTR(rc->output) = static_cast<char *>(rc->params[i].p.blocks.buf) + (rc->params_state[1] * rc->params[i].p.blocks.stride);
        SCDC_DATASET_INOUT_BUF_SIZE(rc->output) = SCDC_DATASET_INOUT_BUF_CURRENT(rc->output) = rc->params[i].p.blocks.size;

        ++rc->params_state[1];
      }

    } else ASSERT(false);
  }
#else
  TRACE_F("dense state %d of %d", rc->output_state, rc->dense_nparams);

  if (rc->output_state < rc->dense_nparams)
  {
    finish = false;

    rdint_t i = rc->output_state;

    SCDC_DATASET_INOUT_BUF_PTR(rc->output) = rc->dense_params[i].buf;
    SCDC_DATASET_INOUT_BUF_SIZE(rc->output) = SCDC_DATASET_INOUT_BUF_CURRENT(rc->output) = rc->dense_params[i].size;

    ++rc->output_state;
  }
#endif

  if (finish)
  {
    TRACE_F("freeing %d buffers", rc->free_nbufs);

    redirect_call_free_bufs(rc);

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
  if (rc->input)
  {
    /* consume remaining input */
    while (rc->input->next) rc->input->next(rc->input, 0);
  }

  if (rc->result)
  {
    /* prepare result with result_str and output_conf_put */
    char *s = SCDC_RESULT_STR(rc->result);
    rdint_t n = 0;
    n += redirect_call_put_inout_conf_str(s + n, "result", rc->result_str, -1);
    n += redirect_call_put_inout_conf_str(s + n, "oconf", rc->output_conf_put, rc->output_conf_put_n);

    ASSERT(n < SCDC_RESULT_SIZE(rc->result));
  }

  if (rc->output)
  {
    /* prepare parameter output */
    rc->output->next = release_scdc_output_next;
    rc->output->data = rc;
#if REDIRECT_CALL_PARAMS_NEW
    rc->params_state[0] = rc->params_state[1] = 0;
#else
    rc->output_state = 0;
#endif

    /* set first paramter to output */
    rc->output->next(rc->output, 0);
  }
}


int redirect_call_cmd(redirect_call_t *rc, const char *cmd, const char *params, scdc_dataset_input_t *input, scdc_dataset_output_t *output, scdc_result_t *result)
{
  TRACE_F("cmd: '%s', params: '%s', input: %p, output: %p, result: %p", cmd, params, input, output, result);

#if REDIRECT_CALL_PARAMS_CACHE
  if (redirect_cache_check_cmd(rc->cache, cmd, params, result)) return 0;
#endif

  rdint_t n = 0;
#if REDIRECT_CALL_PARAMS_CACHE
  char cache_state[REDIRECT_CALL_CACHE_STATE_MAX_SIZE] = { '\0' };
  rdint_t cache_state_size = 0;
  n += redirect_call_get_inout_conf_str(params + n, "cstate", cache_state, &cache_state_size);
  if (rc->cache) redirect_cache_check_state(rc->cache, cache_state, cache_state_size);
#endif
  rc->input_conf_n = 0;
  n += redirect_call_get_inout_conf_str(params + n, "iconf", rc->input_conf, 0);
  rc->output_conf_get_n = 0;
  n += redirect_call_get_inout_conf_str(params + n, "oconf", rc->output_conf_get, 0);

  rc->input = input;
  rc->output = output;
  rc->result = result;

  return 1;
}


#define DEFINE_SRV_CONF_VAL(_t_, _tn_) \
  void redirect_call_get_input_conf_ ## _tn_(redirect_call_t *rc, const char *id, _t_ *x) { \
    rc->input_conf_n += redirect_call_get_inout_conf_val<_t_>(rc->input_conf + rc->input_conf_n, id, x); \
  } \
  void redirect_call_put_output_conf_ ## _tn_(redirect_call_t *rc, const char *id, _t_ x) { \
    rc->output_conf_put_n += redirect_call_put_inout_conf_val<_t_>(rc->output_conf_put + rc->output_conf_put_n, id, x); \
  } \
  void redirect_call_get_input_conf_ ## _tn_ ## s(redirect_call_t *rc, const char *id, _t_ *x, rdint_t n) { \
    rc->input_conf_n += redirect_call_get_inout_conf_vals<_t_>(rc->input_conf + rc->input_conf_n, id, x, n); \
  } \
  void redirect_call_put_output_conf_ ## _tn_ ## s(redirect_call_t *rc, const char *id, const _t_ *x, rdint_t n) { \
    rc->output_conf_put_n += redirect_call_put_inout_conf_vals<_t_>(rc->output_conf_put + rc->output_conf_put_n, id, x, n); \
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
  rc->input_conf_n += redirect_call_get_inout_conf_str(rc->input_conf + rc->input_conf_n, id, x, n);
}


void redirect_call_put_output_conf_str(redirect_call_t *rc, const char *id, const char *x, rdint_t n)
{
  rc->output_conf_put_n += redirect_call_put_inout_conf_str(rc->output_conf_put + rc->output_conf_put_n, id, x, n);
}


/* client and server side */

/* cache */
#if REDIRECT_CALL_PARAMS_CACHE

void redirect_call_set_cache_ptr(redirect_call_t *rc, redirect_cache_t *cache)
{
  rc->cache = cache;
}


void redirect_call_get_cache_ptr(redirect_call_t *rc, redirect_cache_t **cache)
{
  *cache = rc->cache;
}

#endif


/* handle */

void redirect_call_set_handle(redirect_call_t *rc, redirect_handle_t *rh)
{
  if (rc->client)
  {
    rc->dataset = rh->dataset;
    redirect_call_put_input_conf_str(rc, "hdl_id", rh->id, -1);
    redirect_call_put_input_conf_void_p(rc, "hdl_ptr", rh->ptr);

  } else
  {
    redirect_call_put_output_conf_str(rc, "hdl_id", rh->id, -1);
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
    redirect_call_get_output_conf_str(rc, "hdl_id", rh_id_ptr, 0);
    redirect_call_get_output_conf_void_p(rc, "hdl_ptr", &rh->ptr);

  } else
  {
    redirect_call_get_input_conf_str(rc, "hdl_id", rh_id_ptr, 0);
    redirect_call_get_input_conf_void_p(rc, "hdl_ptr", &rh->ptr);
  }
}


/* result */

void redirect_call_set_result_str(redirect_call_t *rc, const char *result_str)
{
  strncpy(rc->result_str, result_str, sizeof(rc->result_str));
}


const char *redirect_call_get_result_str(redirect_call_t *rc, char *result_str, rdint_t n)
{
  if (result_str) strncpy(result_str, rc->result_str, n);

  return rc->result_str;
}


/* dense data */

template<typename T>
static void register_param_dense(redirect_call_t *rc, const T *buf, rdint_t size)
{
#if REDIRECT_CALL_PARAMS_NEW
  ASSERT(rc->nparams < REDIRECT_CALL_PARAMS_MAX);

  rc->params[rc->nparams].type = REDIRECT_PARAM_TYPE_DENSE;
#if REDIRECT_CALL_PARAMS_CACHE
  rc->params[rc->nparams].cacheable = 1;
#endif
  rc->params[rc->nparams].p.dense.buf = const_cast<T *>(buf);
  rc->params[rc->nparams].p.dense.size = size * sizeof(T);

  ++rc->nparams;
#else
  rc->dense_params[rc->dense_nparams].buf = const_cast<void *>(buf);
  rc->dense_params[rc->dense_nparams].size = size * sizeof(T);

  ++rc->dense_nparams;
#endif
}


template<typename T>
static void consume_input_dense(redirect_call_t *rc, T *buf, rdint_t size)
{
  rdint_t size_left = size * sizeof(T);

  char *src = static_cast<char *>(SCDC_DATASET_INOUT_BUF_PTR(rc->input));
  char *dst = reinterpret_cast<char *>(buf);

  do
  {
    rdint_t t = z_min(size_left, SCDC_DATASET_INOUT_BUF_CURRENT(rc->input) - rc->input_offset);
    if (dst != src + rc->input_offset) memcpy(dst, src + rc->input_offset, t);
    rc->input_offset += t;
    dst += t;
    size_left -= t;

    /* done */
    if (size_left <= 0) break;

    /* have no next or next fails? */
    if (rc->input->next == 0 || rc->input->next(rc->input, 0) != SCDC_SUCCESS) break;

    src = static_cast<char *>(SCDC_DATASET_INOUT_BUF_PTR(rc->input));
    rc->input_offset = 0;

  } while (1);

#if REDIRECT_CALL_PARAMS_CACHE
  /* try to add an entry to the cache (dense input is cacheable) */
  if (rc->cache)
  {
    rdint_t size_cache = size * sizeof(T);
    rdint_t buf_index = redirect_call_find_buf(rc, buf);
    rdint_t entry_index = redirect_cache_find_entry(rc->cache, get_type<T>(), size_cache, buf);

    /* caching entry if buffer is allocated or entry is already cached */
    if (buf_index >= 0 || entry_index >= 0)
    {
      if (entry_index >= 0)
      {
        TRACE_F("updating entry %" rdint_fmt " of cache", entry_index);
        redirect_cache_update_entry(rc->cache, entry_index);

      } else
      {
        TRACE_F("adding entry [%d,%" rdint_fmt ",%p] to cache", get_type<T>(), size_cache, buf);
        redirect_cache_put_entry(rc->cache, get_type<T>(), size_cache, buf);
      }

      if (buf_index >= 0) redirect_call_remove_buf(rc, buf_index);
    }
  }
#endif
}


template<typename T>
static void consume_output_dense(redirect_call_t *rc, T *buf, rdint_t size)
{
  rdint_t size_left = size * sizeof(T);

  char *src = static_cast<char *>(SCDC_DATASET_INOUT_BUF_PTR(rc->output));
  char *dst = reinterpret_cast<char *>(buf);

  do
  {
    rdint_t t = z_min(size_left, SCDC_DATASET_INOUT_BUF_CURRENT(rc->output) - rc->output_offset);
    if (dst != src + rc->output_offset) memcpy(dst, src + rc->output_offset, t);
    rc->output_offset += t;
    dst += t;
    size_left -= t;

    /* done */
    if (size_left <= 0) break;

    /* have no next or next fails? */
    if (rc->output->next == 0 || rc->output->next(rc->output, 0) != SCDC_SUCCESS) break;

    src = static_cast<char *>(SCDC_DATASET_INOUT_BUF_PTR(rc->output));
    rc->output_offset = 0;

  } while (1);
}


template<typename T>
static void redirect_call_put_input_param_dense_val(redirect_call_t *rc, const char *id, const T *b, rdint_t n)
{
  TRACE_F("rc: %p, id: '%s', b: %p, n: %" rdint_fmt, rc, id, b, n);

  /* client only */
  ASSERT(rc->client);

  /* register dense input */
  register_param_dense<T>(rc, b, n);

#if REDIRECT_CALL_PARAMS_CACHE
  if (rc->cache) redirect_cache_register_param_dense(rc->cache, "unknown", get_type<T>(), &rc->params[rc->nparams - 1].p.dense);
#endif
}


template<typename T>
static void redirect_call_get_input_param_dense_val(redirect_call_t *rc, const char *id, T **b, rdint_t n)
{
  TRACE_F("rc: %p, id: '%s', b: %p, n: %" rdint_fmt, rc, id, *b, n);

  /* server only */
  ASSERT(!rc->client);

#if REDIRECT_CALL_PARAMS_CACHE
  if (rc->cache)
  {
    rdint_t entry_index = redirect_cache_get_next_param_entry_index(rc->cache);

    if (entry_index >= 0)
    {
      TRACE_F("using cache entry %" rdint_fmt, entry_index);
      if (*b)
      {
        /* copy cache entry to buffer */
        memcpy(*b, redirect_cache_get_entry_buf(rc->cache, entry_index), n * sizeof(T));

      } else
      {
        /* reuse buffer of cache entry */
        *b = static_cast<T *>(redirect_cache_get_entry_buf(rc->cache, entry_index));
      }

      return;
    }
  }
#endif

  if (*b == NULL)
  {
    /* allocate buffer */
    *b = static_cast<T *>(redirect_call_alloc_buf(rc, n * sizeof(T)));
  }

  /* copy data from input to buffer */
  consume_input_dense<T>(rc, *b, n);
}


template<typename T>
static void redirect_call_put_output_param_dense_val(redirect_call_t *rc, const char *id, const T *b, rdint_t n)
{
  TRACE_F("rc: %p, id: '%s', b: %p, n: %" rdint_fmt, rc, id, b, n);

  if (rc->client) /* client */
  {
    /* FIXME: use buffer for output */

  } else /* server */
  {
    /* register dense output */
    register_param_dense<T>(rc, b, n);

#if REDIRECT_CALL_PARAMS_CACHE
    /* try to add an entry to the cache (dense output is cacheable) */
    if (rc->cache)
    {
      rdint_t buf_index = redirect_call_find_buf(rc, b);
      rdint_t entry_index = redirect_cache_find_entry(rc->cache, get_type<T>(), n * sizeof(T), b);

      /* caching entry if buffer is allocated or entry is already cached */
      if (buf_index >= 0 || entry_index >= 0)
      {
        if (entry_index >= 0)
        {
          TRACE_F("updating entry %" rdint_fmt " of cache", entry_index);
          redirect_cache_update_entry(rc->cache, entry_index);

        } else
        {
          TRACE_F("adding entry [%d,%" rdint_fmt ",%p] to cache", get_type<T>(), static_cast<rdint_t>(n * sizeof(T)), b);
          redirect_cache_put_entry(rc->cache, get_type<T>(), n * sizeof(T), b);
        }

        if (buf_index >= 0) redirect_call_remove_buf(rc, buf_index);
      }
    }
#endif
  }
}


template<typename T>
void redirect_call_get_output_param_dense_val(redirect_call_t *rc, const char *id, T **b, rdint_t n)
{
  TRACE_F("rc: %p, id: '%s', b: %p, n: %" rdint_fmt, rc, id, *b, n);

  if (rc->client) /* client */
  {
    if (*b == NULL)
    {
      /* allocate buffer */
      *b = static_cast<T *>(redirect_call_alloc_buf(rc, n * sizeof(T)));
    }

    /* copy data from output to buffer */
    consume_output_dense<T>(rc, *b, n);

  } else /* server */
  {
    if (*b == NULL)
    {
      /* FIXME: use output instead of allocation */

      /* allocate buffer */
      *b = static_cast<T *>(redirect_call_alloc_buf(rc, n * sizeof(T)));
    }
  }
}


template<typename T>
static void redirect_call_put_inout_param_dense_val(redirect_call_t *rc, const char *id, const T *b, rdint_t n)
{
  TRACE_F("rc: %p, id: '%s', b: %p, n: %" rdint_fmt, rc, id, b, n);

  /* FIXME: unroll and simplify the two calls */
  redirect_call_put_input_param_dense_val<T>(rc, id, b, n);
  redirect_call_put_output_param_dense_val<T>(rc, id, b, n);
}


template<typename T>
static void redirect_call_get_inout_param_dense_val(redirect_call_t *rc, const char *id, T **b, rdint_t n)
{
  TRACE_F("rc: %p, id: '%s', b: %p, n: %" rdint_fmt, rc, id, *b, n);

  /* FIXME: unroll and simplify the two calls */
  redirect_call_get_input_param_dense_val<T>(rc, id, b, n);
  redirect_call_get_output_param_dense_val<T>(rc, id, b, n);
}


/* blocks data */

template<typename T>
static void register_param_blocks(redirect_call_t *rc, const T *buf, rdint_t count, rdint_t size, rdint_t stride)
{
  ASSERT(rc->nparams < REDIRECT_CALL_PARAMS_MAX);

  rc->params[rc->nparams].type = REDIRECT_PARAM_TYPE_BLOCKS;
#if REDIRECT_CALL_PARAMS_CACHE
  rc->params[rc->nparams].cacheable = 0;
#endif
  rc->params[rc->nparams].p.blocks.buf = const_cast<T *>(buf);
  rc->params[rc->nparams].p.blocks.count = count;
  rc->params[rc->nparams].p.blocks.size = size * sizeof(T);
  rc->params[rc->nparams].p.blocks.stride = stride * sizeof(T);

  ++rc->nparams;
}


template<typename T>
static void consume_input_blocks(redirect_call_t *rc, void *buf, rdint_t count, rdint_t size, rdint_t stride)
{
  char *src = static_cast<char *>(SCDC_DATASET_INOUT_BUF_PTR(rc->input));
  char *dst = static_cast<char *>(buf);

  rdint_t stride_skip = (stride - size) * sizeof(T);
  rdint_t size_left = size * sizeof(T);

  do
  {
do_block:
    rdint_t t = z_min(size_left, SCDC_DATASET_INOUT_BUF_CURRENT(rc->input) - rc->input_offset);
    if (dst != src + rc->input_offset) memcpy(dst, src + rc->input_offset, t);
    rc->input_offset += t;
    dst += t;
    size_left -= t;

    /* block done? */
    if (size_left <= 0)
    {
      dst += stride_skip;
      size_left = size * sizeof(T);
      --count;
    }

    /* all blocks done? */
    if (count <= 0) break;

    /* data left for next block? */
    if (rc->input_offset < SCDC_DATASET_INOUT_BUF_CURRENT(rc->input)) goto do_block;

    /* have no next or next fails? */
    if (rc->input->next == 0 || rc->input->next(rc->input, 0) != SCDC_SUCCESS) break;

    src = static_cast<char *>(SCDC_DATASET_INOUT_BUF_PTR(rc->input));
    rc->input_offset = 0;

  } while (1);
}


template<typename T>
static void consume_output_blocks(redirect_call_t *rc, void *buf, rdint_t count, rdint_t size, rdint_t stride)
{
  char *src = static_cast<char *>(SCDC_DATASET_INOUT_BUF_PTR(rc->output));
  char *dst = static_cast<char *>(buf);

  rdint_t stride_skip = (stride - size) * sizeof(T);
  rdint_t size_left = size * sizeof(T);

  do
  {
do_block:
    rdint_t t = z_min(size_left, SCDC_DATASET_INOUT_BUF_CURRENT(rc->output) - rc->output_offset);
    if (dst != src + rc->output_offset) memcpy(dst, src + rc->output_offset, t);
    rc->output_offset += t;
    dst += t;
    size_left -= t;

    /* block done? */
    if (size_left <= 0)
    {
      dst += stride_skip;
      size_left = size * sizeof(T);
      --count;
    }

    /* all blocks done? */
    if (count <= 0) break;

    /* data left for next block? */
    if (rc->output_offset < SCDC_DATASET_INOUT_BUF_CURRENT(rc->output)) goto do_block;

    /* have no next or next fails? */
    if (rc->output->next == 0 || rc->output->next(rc->output, 0) != SCDC_SUCCESS) break;

    src = static_cast<char *>(SCDC_DATASET_INOUT_BUF_PTR(rc->output));
    rc->output_offset = 0;

  } while (1);
}


template<typename T>
static void redirect_call_put_input_param_blocks_val(redirect_call_t *rc, const char *id, const T *b, rdint_t count, rdint_t size, rdint_t stride)
{
  TRACE_F("rc: %p, id: '%s', b: %p, count: %" rdint_fmt ", size: %" rdint_fmt ", stride: %" rdint_fmt, rc, id, b, count, size, stride);

  /* client only */
  ASSERT(rc->client);

  bool transfer_dense = false;
  rdint_t send_stride = stride;
  rdint_t transfer_stride = stride;

  if (size < stride)
  {
#if REDIRECT_CALL_INPUT_PARAM_BLOCKS == REDIRECT_CALL_BLOCKS_PLAIN
    TRACE_F("transfer plain");
    transfer_dense = true;
    send_stride = stride;
    transfer_stride = stride;
#elif REDIRECT_CALL_INPUT_PARAM_BLOCKS == REDIRECT_CALL_BLOCKS_PACK_IP
    TRACE_F("transform in-place to stride %" rdint_fmt, size);
    redirect_data_blocks_transform_val<T>(count, size, stride, const_cast<T *>(b), size, NULL);
    transfer_dense = true;
    send_stride = size;
    transfer_stride = size;
#elif REDIRECT_CALL_INPUT_PARAM_BLOCKS == REDIRECT_CALL_BLOCKS_PACK_OP
    TRACE_F("transform out-of-place to stride %" rdint_fmt, size);
    T *b_tmp = static_cast<T *>(redirect_call_alloc_buf(rc, count * size * sizeof(T)));
    redirect_data_blocks_transform_val<T>(count, size, stride, const_cast<T *>(b), size, b_tmp);
    b = b_tmp;
    transfer_dense = true;
    send_stride = size;
    transfer_stride = size;
#elif REDIRECT_CALL_INPUT_PARAM_BLOCKS == REDIRECT_CALL_BLOCKS_PIECES
    TRACE_F("process in pieces");
    transfer_dense = false;
    send_stride = stride;
    transfer_stride = size;
#endif
  }

  TRACE_F("send stride: %" rdint_fmt ", transfer stride: %" rdint_fmt, send_stride, transfer_stride);
  redirect_call_put_input_conf_rdint(rc, "stride", transfer_stride);

  if (size >= send_stride || transfer_dense)
  {
    TRACE_F("dense: %" rdint_fmt, get_blocks_cont_size(count, size, send_stride));
    register_param_dense<T>(rc, b, get_blocks_cont_size(count, size, send_stride));

  } else
  {
    TRACE_F("blocks: %" rdint_fmt " x %" rdint_fmt " / %" rdint_fmt, count, size, send_stride);
    register_param_blocks<T>(rc, b, count, size, send_stride);
  }
}


template<typename T>
static void redirect_call_get_input_param_blocks_val(redirect_call_t *rc, const char *id, T **b, rdint_t count, rdint_t size, rdint_t *stride)
{
  TRACE_F("rc: %p, id: '%s', b: %p, count: %" rdint_fmt ", size: %" rdint_fmt ", stride: %" rdint_fmt, rc, id, *b, count, size, *stride);

  /* server only */
  ASSERT(!rc->client);

  bool transfer_dense = false;
  rdint_t transfer_stride;

  redirect_call_get_input_conf_rdint(rc, "stride", &transfer_stride);
  TRACE_F("transfer stride: %" rdint_fmt, transfer_stride);

  if (*stride == 0) *stride = transfer_stride;

  rdint_t alloc_stride, recv_stride;

#if REDIRECT_CALL_INPUT_PARAM_BLOCKS == REDIRECT_CALL_BLOCKS_PLAIN
  transfer_dense = true;
  alloc_stride = transfer_stride;
  recv_stride = transfer_stride;
#elif REDIRECT_CALL_INPUT_PARAM_BLOCKS == REDIRECT_CALL_BLOCKS_PACK_IP
  transfer_dense = true;
  alloc_stride = z_max(*stride, transfer_stride);
  recv_stride = transfer_stride;
#elif REDIRECT_CALL_INPUT_PARAM_BLOCKS == REDIRECT_CALL_BLOCKS_PACK_OP
  transfer_dense = true;
  alloc_stride = *stride;
  recv_stride = transfer_stride;
#elif REDIRECT_CALL_INPUT_PARAM_BLOCKS == REDIRECT_CALL_BLOCKS_PIECES
  transfer_dense = false;
  alloc_stride = *stride;
  recv_stride = *stride;
#endif

  TRACE_F("alloc stride: %" rdint_fmt ", recv stride: %" rdint_fmt, alloc_stride, recv_stride);

  if (*b == NULL) *b = static_cast<T *>(redirect_call_alloc_buf(rc, get_blocks_cont_size(count, size, alloc_stride) * sizeof(T)));

  T* b_recv = *b;

#if REDIRECT_CALL_INPUT_PARAM_BLOCKS == REDIRECT_CALL_BLOCKS_PACK_OP
  T *b_tmp = 0;
  if (recv_stride != *stride) b_recv = b_tmp = static_cast<T *>(malloc(get_blocks_cont_size(count, size, transfer_stride) * sizeof(T)));
#endif

  if (recv_stride == transfer_stride || transfer_dense)
  {
    TRACE_F("consume dense: %" rdint_fmt, get_blocks_cont_size(count, size, recv_stride));
    consume_input_dense<T>(rc, b_recv, get_blocks_cont_size(count, size, recv_stride));

  } else
  {
    TRACE_F("consume blocks: %" rdint_fmt " x %" rdint_fmt " / %" rdint_fmt, count, size, recv_stride);
    consume_input_blocks<T>(rc, b_recv, count, size, recv_stride);
  }

  if (recv_stride != *stride)
  {
#if REDIRECT_CALL_INPUT_PARAM_BLOCKS == REDIRECT_CALL_BLOCKS_PLAIN
    TRACE_F("set stride to %" rdint_fmt, recv_stride);
    *stride = recv_stride;
#elif REDIRECT_CALL_INPUT_PARAM_BLOCKS == REDIRECT_CALL_BLOCKS_PACK_IP
    TRACE_F("transform in-place to stride %" rdint_fmt, *stride);
    redirect_data_blocks_transform_val<T>(count, size, recv_stride, b_recv, *stride, NULL);
#elif REDIRECT_CALL_INPUT_PARAM_BLOCKS == REDIRECT_CALL_BLOCKS_PACK_OP
    TRACE_F("transform out-of-place to stride %" rdint_fmt, *stride);
    redirect_data_blocks_transform_val<T>(count, size, recv_stride, b_recv, *stride, *b);
    free(b_tmp);
#elif REDIRECT_CALL_INPUT_PARAM_BLOCKS == REDIRECT_CALL_BLOCKS_PIECES
    /* this should never occur, because pieces are received with the correct stride */
    ASSERT(false);
#endif
  }
}


template<typename T>
static void redirect_call_put_output_param_blocks_val(redirect_call_t *rc, const char *id, const T *b, rdint_t count, rdint_t size, rdint_t stride)
{
  TRACE_F("rc: %p, id: '%s', b: %p, count: %" rdint_fmt ", size: %" rdint_fmt ", stride: %" rdint_fmt, rc, id, b, count, size, stride);

  if (rc->client) /* client */
  {
    /* FIXME: use buffer for output */

  } else /* server */
  {
    bool transfer_dense = false;
    rdint_t send_stride = stride;
    rdint_t transfer_stride = stride;

    if (size < stride)
    {
#if REDIRECT_CALL_OUTPUT_PARAM_BLOCKS == REDIRECT_CALL_BLOCKS_PLAIN
      TRACE_F("transfer plain");
      transfer_dense = true;
      send_stride = stride;
      transfer_stride = stride;
#elif REDIRECT_CALL_OUTPUT_PARAM_BLOCKS == REDIRECT_CALL_BLOCKS_PACK_IP
      TRACE_F("transform in-place to stride: %" rdint_fmt, size);
      redirect_data_blocks_transform_val<T>(count, size, stride, const_cast<T *>(b), size, NULL);
      transfer_dense = true;
      send_stride = size;
      transfer_stride = size;
#elif REDIRECT_CALL_OUTPUT_PARAM_BLOCKS == REDIRECT_CALL_BLOCKS_PACK_OP
      TRACE_F("transform out-of-place to stride: %" rdint_fmt, size);
      T *b_tmp = static_cast<T *>(redirect_call_alloc_buf(rc, count * size * sizeof(T)));
      redirect_data_blocks_transform_val<T>(count, size, stride, const_cast<T *>(b), size, b_tmp);
      b = b_tmp;
      transfer_dense = true;
      send_stride = size;
      transfer_stride = size;
#elif REDIRECT_CALL_OUTPUT_PARAM_BLOCKS == REDIRECT_CALL_BLOCKS_PIECES
      TRACE_F("process in pieces");
      transfer_dense = false;
      send_stride = stride;
      transfer_stride = size;
#endif
    }

    TRACE_F("send stride: %" rdint_fmt ", transfer stride: %" rdint_fmt, send_stride, transfer_stride);
    redirect_call_put_output_conf_rdint(rc, "stride", transfer_stride);

    if (size >= send_stride || transfer_dense)
    {
      TRACE_F("dense: %" rdint_fmt, get_blocks_cont_size(count, size, send_stride));
      register_param_dense<T>(rc, b, get_blocks_cont_size(count, size, send_stride));

    } else
    {
      TRACE_F("blocks: %" rdint_fmt " x %" rdint_fmt " / %" rdint_fmt, count, size, send_stride);
      register_param_blocks<T>(rc, b, count, size, send_stride);
    }
  }
}


template<typename T>
void redirect_call_get_output_param_blocks_val(redirect_call_t *rc, const char *id, T **b, rdint_t count, rdint_t size, rdint_t *stride)
{
  TRACE_F("rc: %p, id: '%s', b: %p, count: %" rdint_fmt ", size: %" rdint_fmt ", stride: %" rdint_fmt, rc, id, *b, count, size, *stride);

  if (rc->client) /* client */
  {
    bool transfer_dense = false;
    rdint_t transfer_stride;

    redirect_call_get_output_conf_rdint(rc, "stride", &transfer_stride);
    TRACE_F("transfer stride: %" rdint_fmt, transfer_stride);

    if (*stride == 0) *stride = transfer_stride;

    rdint_t alloc_stride, recv_stride;

#if REDIRECT_CALL_OUTPUT_PARAM_BLOCKS == REDIRECT_CALL_BLOCKS_PLAIN
    transfer_dense = true;
    alloc_stride = transfer_stride;
    recv_stride = transfer_stride;
#elif REDIRECT_CALL_OUTPUT_PARAM_BLOCKS == REDIRECT_CALL_BLOCKS_PACK_IP
    transfer_dense = true;
    alloc_stride = z_max(*stride, transfer_stride);
    recv_stride = transfer_stride;
#elif REDIRECT_CALL_OUTPUT_PARAM_BLOCKS == REDIRECT_CALL_BLOCKS_PACK_OP
    transfer_dense = true;
    alloc_stride = *stride;
    recv_stride = transfer_stride;
#elif REDIRECT_CALL_OUTPUT_PARAM_BLOCKS == REDIRECT_CALL_BLOCKS_PIECES
    transfer_dense = false;
    alloc_stride = *stride;
    recv_stride = *stride;
#endif

    TRACE_F("alloc stride: %" rdint_fmt ", recv stride: %" rdint_fmt, alloc_stride, recv_stride);

    if (*b == NULL) *b = static_cast<T *>(redirect_call_alloc_buf(rc, get_blocks_cont_size(count, size, alloc_stride) * sizeof(T)));

    T* b_recv = *b;

#if REDIRECT_CALL_INPUT_PARAM_BLOCKS == REDIRECT_CALL_BLOCKS_PACK_OP
    T *b_tmp = 0;
    if (recv_stride != *stride) b_recv = b_tmp = static_cast<T *>(malloc(get_blocks_cont_size(count, size, transfer_stride) * sizeof(T)));
#endif

    if (recv_stride == transfer_stride || transfer_dense)
    {
      TRACE_F("consume dense: %" rdint_fmt, get_blocks_cont_size(count, size, recv_stride));
      consume_output_dense<T>(rc, b_recv, get_blocks_cont_size(count, size, recv_stride));

    } else
    {
      TRACE_F("consume blocks: %" rdint_fmt " x %" rdint_fmt " / %" rdint_fmt, count, size, recv_stride);
      consume_output_blocks<T>(rc, b_recv, count, size, recv_stride);
    }

    if (recv_stride != *stride)
    {
#if REDIRECT_CALL_INPUT_PARAM_BLOCKS == REDIRECT_CALL_BLOCKS_PLAIN
      TRACE_F("set stride %" rdint_fmt, recv_stride);
      *stride = recv_stride;
#elif REDIRECT_CALL_INPUT_PARAM_BLOCKS == REDIRECT_CALL_BLOCKS_PACK_IP
      TRACE_F("transform in-place to stride %" rdint_fmt, *stride);
      redirect_data_blocks_transform_val<T>(count, size, recv_stride, b_recv, *stride, NULL);
#elif REDIRECT_CALL_INPUT_PARAM_BLOCKS == REDIRECT_CALL_BLOCKS_PACK_OP
      TRACE_F("transform out-of-place to stride: %" rdint_fmt, *stride);
      redirect_data_blocks_transform_val<T>(count, size, recv_stride, b_recv, *stride, *b);
      free(b_tmp);
#elif REDIRECT_CALL_INPUT_PARAM_BLOCKS == REDIRECT_CALL_BLOCKS_PIECES
      /* this should never occur, because pieces are received with the correct stride */
      ASSERT(false);
#endif
    }

  } else /* server */
  {
    if (*b == NULL)
    {
      /* default stride is size */
      if (*stride == 0) *stride = size;

      /* FIXME: use output instead of allocation */

      /* allocate buffer */
      *b = static_cast<T *>(redirect_call_alloc_buf(rc, get_blocks_cont_size(count, size, *stride) * sizeof(T)));
    }
  }
}


template<typename T>
static void redirect_call_put_inout_param_blocks_val(redirect_call_t *rc, const char *id, const T *b, rdint_t count, rdint_t size, rdint_t stride)
{
  TRACE_F("rc: %p, id: '%s', b: %p, count: %" rdint_fmt ", size: %" rdint_fmt ", stride: %" rdint_fmt, rc, id, b, count, size, stride);

  /* FIXME: unroll and simplify the two calls */
  redirect_call_put_input_param_blocks_val<T>(rc, id, b, count, size, stride);
  redirect_call_put_output_param_blocks_val<T>(rc, id, b, count, size, stride);
}


template<typename T>
static void redirect_call_get_inout_param_blocks_val(redirect_call_t *rc, const char *id, T **b, rdint_t count, rdint_t size, rdint_t *stride)
{
  TRACE_F("rc: %p, id: '%s', b: %p, count: %" rdint_fmt ", size: %" rdint_fmt ", stride: %" rdint_fmt, rc, id, *b, count, size, *stride);

  /* FIXME: unroll and simplify the two calls */
  redirect_call_get_input_param_blocks_val<T>(rc, id, b, count, size, stride);
  redirect_call_get_output_param_blocks_val<T>(rc, id, b, count, size, stride);
}


#include "redirect_call_bytes.tcc"
#include "redirect_call_array.tcc"
#include "redirect_call_vector.tcc"
#include "redirect_call_matrix.tcc"


/* float */

#define DEFINE_FLOAT_ARRAY(_t_, _tn_) \
  void redirect_call_print_param_array_ ## _tn_(const _t_ *a, rdint_t n) { \
    redirect_call_print_param_array_val<_t_>(a, n); \
  } \
  redirect_param_hash_t redirect_call_hash_param_array_ ## _tn_(const _t_ *a, rdint_t n) { \
    return redirect_call_hash_param_array_val<_t_>(a, n); \
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
  DEFINE_TYPE(_t_, _tn_, _fmt_); \
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
  DEFINE_TYPE(_t_, _tn_, _fmt_); \
  DEFINE_CONF(_t_, _tn_, _fmt_); \
  DEFINE_CLI_CONF_VAL(_t_, _tn_); \
  DEFINE_SRV_CONF_VAL(_t_, _tn_); \
  DEFINE_PRINT_ARRAY(_t_, _tn_, _fmt_); \
  DEFINE_INT_ARRAY(_t_, _tn_); \

DEFINE_INT(char, char, FMT_CHAR);
DEFINE_INT(int, int, FMT_INT);
DEFINE_INT(long, long, FMT_LONG);

DEFINE_CONF(void_p, void_p, FMT_VOID_PTR);
DEFINE_CLI_CONF_VAL(void_p, void_p);
DEFINE_SRV_CONF_VAL(void_p, void_p);
