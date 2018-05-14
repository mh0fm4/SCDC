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


#ifndef __REDIRECT_CALL_VECTOR_TCC__
#define __REDIRECT_CALL_VECTOR_TCC__


/* typed vectors with stride/inc */

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
static void print_vector_val(const T *v, rdint_t n, rdint_t inc, bool full = false)
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


template<typename T>
static void print_vector_val(const T *v, rdint_t n, rdint_t inc);

#define DEFINE_PRINT_VECTOR(_t_, _tn_, _fmt_) \
  template<> \
  void print_vector_val<_t_>(const _t_ *v, rdint_t n, rdint_t inc) { \
    print_vector_val<_t_, _fmt_>(v, n, inc); \
  }


template<typename T>
void redirect_call_print_param_vector_val(const T *v, rdint_t n, rdint_t inc)
{
  print_vector_val<T>(v, n, inc);
}


template<typename T>
void redirect_call_put_input_param_vector_val(redirect_call_t *rc, const char *id, const T *v, rdint_t n, rdint_t inc)
{
  TRACE_F("%s: rc: %p, id: '%s', v: %p, n: %" rdint_fmt ", inc: %" rdint_fmt, __func__, rc, id, v, n, inc);

  /* client only */
  ASSERT(rc->client);

#if TRACE_DATA
  TRACE_F("%s:", __func__);
  print_vector_val<T>(v, n, inc);
#endif

  const rdint_t x[2] = { n, inc };
  redirect_call_put_input_conf_rdints(rc, "v", x, 2);

  redirect_call_put_input_param_blocks_val<T>(rc, id, v, n, 1, inc);
}


template<typename T>
void redirect_call_get_input_param_vector_val(redirect_call_t *rc, const char *id, T **v, rdint_t *n, rdint_t *inc)
{
  TRACE_F("%s: rc: %p, id: '%s', v: %p, n: %" rdint_fmt ", inc: %" rdint_fmt, __func__, rc, id, *v, *n, *inc);

  /* server only */
  ASSERT(!rc->client);

  rdint_t x[2];
  redirect_call_get_input_conf_rdints(rc, "v", x, 2);
  *n = x[0];
  /* the original inc in x[1] is ignored, use the locally given inc or the stride from the transfer (if inc == 0) */

  rdint_t stride = *inc;
  redirect_call_get_input_param_blocks_val<T>(rc, id, v, *n, 1, &stride);
  *inc = stride;

#if TRACE_DATA
  TRACE_F("%s:", __func__);
  print_vector_val<T>(*v, *n, *inc);
#endif
}


template<typename T>
void redirect_call_put_output_param_vector_val(redirect_call_t *rc, const char *id, const T *v, rdint_t n, rdint_t inc)
{
  TRACE_F("%s: rc: %p, id: '%s', v: %p, n: %" rdint_fmt ", inc: %" rdint_fmt, __func__, rc, id, v, n, inc);

  if (!rc->client)
  {
#if TRACE_DATA
    TRACE_F("%s:", __func__);
    print_vector_val<T>(v, n, inc);
#endif
  }

  const rdint_t x[2] = { n, inc };
  if (rc->client) redirect_call_put_input_conf_rdints(rc, "v", x, 2);
  else redirect_call_put_output_conf_rdints(rc, "v", x, 2);

  redirect_call_put_output_param_blocks_val<T>(rc, id, v, n, 1, inc);
}


template<typename T>
void redirect_call_get_output_param_vector_val(redirect_call_t *rc, const char *id, T **v, rdint_t *n, rdint_t *inc)
{
  TRACE_F("%s: rc: %p, id: '%s', v: %p, n: %" rdint_fmt ", inc: %" rdint_fmt, __func__, rc, id, *v, *n, *inc);

  rdint_t x[2];
  if (rc->client) redirect_call_get_output_conf_rdints(rc, "v", x, 2);
  else redirect_call_get_input_conf_rdints(rc, "v", x, 2);
  *n = x[0];
  /* the original inc in x[1] is ignored, use the locally given inc or the stride from the transfer (if inc == 0) */

  rdint_t stride = *inc;
  redirect_call_get_output_param_blocks_val<T>(rc, id, v, *n, 1, &stride);
  *inc = stride;

  if (rc->client)
  {
#if TRACE_DATA
    TRACE_F("%s:", __func__);
    print_vector_val<T>(*v, *n, *inc);
#endif
  }
}


template<typename T>
void redirect_call_put_inout_param_vector_val(redirect_call_t *rc, const char *id, const T *v, rdint_t n, rdint_t inc)
{
  TRACE_F("%s: rc: %p, id: '%s', v: %p, n: %" rdint_fmt ", inc: %" rdint_fmt, __func__, rc, id, v, n, inc);

  /* client only */
  ASSERT(rc->client);

#if TRACE_DATA
  TRACE_F("%s:", __func__);
  print_vector_val<T>(v, n, inc);
#endif

  const rdint_t x[2] = { n, inc };
  if (rc->client) redirect_call_put_input_conf_rdints(rc, "v", x, 2);
  else redirect_call_put_output_conf_rdints(rc, "v", x, 2);

  redirect_call_put_inout_param_blocks_val<T>(rc, id, v, n, 1, inc);
}


template<typename T>
void redirect_call_get_inout_param_vector_val(redirect_call_t *rc, const char *id, T **v, rdint_t *n, rdint_t *inc)
{
  TRACE_F("%s: rc: %p, id: '%s', v: %p, n: %" rdint_fmt ", inc: %" rdint_fmt, __func__, rc, id, *v, *n, *inc);

  /* server only */
  ASSERT(!rc->client);

  rdint_t x[2];
  if (rc->client) redirect_call_get_output_conf_rdints(rc, "v", x, 2);
  else redirect_call_get_input_conf_rdints(rc, "v", x, 2);
  *n = x[0];
  /* the original inc in x[1] is ignored, use the locally given inc or the stride from the transfer (if inc == 0) */

  rdint_t stride = *inc;
  redirect_call_get_inout_param_blocks_val<T>(rc, id, v, *n, 1, &stride);
  *inc = stride;

#if TRACE_DATA
  TRACE_F("%s:", __func__);
  print_vector_val<T>(*v, *n, *inc);
#endif
}


#endif /* __REDIRECT_CALL_VECTOR_TCC__ */
