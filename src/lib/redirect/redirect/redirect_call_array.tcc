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


#ifndef __REDIRECT_CALL_ARRAY_TCC__
#define __REDIRECT_CALL_ARRAY_TCC__


/* typed arrays */

template<typename T, int F>
static void print_array_val(const T *a, rdint_t n)
{
  rdint_t i;
  for (i = 0; i < n; ++i)
  {
    char s[32];

    sprintf(s, PRINTF_FMT(F), a[i]);
    printf("  %s", s);

    printf("\n");
  }
}


template<typename T>
static void print_array_val(const T *a, rdint_t n);

#define DEFINE_PRINT_ARRAY(_t_, _tn_, _fmt_) \
  template<> \
  void print_array_val<_t_>(const _t_ *a, rdint_t n) { \
    print_array_val<_t_, _fmt_>(a, n); \
  }


template<typename T>
void redirect_call_print_param_array_val(const T *a, rdint_t n)
{
  print_array_val<T>(a, n);
}


template<typename T>
redirect_param_hash_t redirect_call_hash_param_array_val(const T *a, rdint_t n)
{
  redirect_param_dense_t p = { const_cast<T *>(a), static_cast<rdint_t>(n * sizeof(T)) };
  return redirect_param_hash_dense(&p);
}


template<typename T>
void redirect_call_put_input_param_array_val(redirect_call_t *rc, const char *id, const T *a, rdint_t n)
{
  TRACE_F("rc: %p, id: '%s', a: %p, n: %" rdint_fmt, rc, id, a, n);

  /* client only */
  ASSERT(rc->client);

#if TRACE_DATA
  TRACE_F("");
  print_array_val<T>(a, n);
#endif

  redirect_call_put_input_conf_rdint(rc, "a", n);
  redirect_call_put_input_param_dense_val<T>(rc, id, a, n);
}


template<typename T>
void redirect_call_get_input_param_array_val(redirect_call_t *rc, const char *id, T **a, rdint_t *n)
{
  TRACE_F("rc: %p, id: '%s', a: %p, n: %" rdint_fmt, rc, id, *a, *n);

  /* server only */
  ASSERT(!rc->client);

  redirect_call_get_input_conf_rdint(rc, "a", n);
  redirect_call_get_input_param_dense_val<T>(rc, id, a, *n);

#if TRACE_DATA
  TRACE_F("");
  print_array_val<T>(*a, *n);
#endif
}


template<typename T>
void redirect_call_put_output_param_array_val(redirect_call_t *rc, const char *id, const T *a, rdint_t n)
{
  TRACE_F("rc: %p, id: '%s', a: %p, n: %" rdint_fmt, rc, id, a, n);

  if (!rc->client)
  {
#if TRACE_DATA
    TRACE_F("");
    print_array_val<T>(a, n);
#endif
  }

  redirect_call_put_output_conf_rdint(rc, "a", n);
  redirect_call_put_output_param_dense_val<T>(rc, id, a, n);
}


template<typename T>
void redirect_call_get_output_param_array_val(redirect_call_t *rc, const char *id, T **a, rdint_t *n)
{
  TRACE_F("rc: %p, id: '%s', a: %p, n: %" rdint_fmt, rc, id, *a, *n);

  redirect_call_get_output_conf_rdint(rc, "a", n);
  redirect_call_get_output_param_dense_val<T>(rc, id, a, *n);

  if (rc->client)
  {
#if TRACE_DATA
    TRACE_F("");
    print_array_val<T>(*a, *n);
#endif
  }
}


template<typename T>
void redirect_call_put_inout_param_array_val(redirect_call_t *rc, const char *id, const T *a, rdint_t n)
{
  TRACE_F("rc: %p, id: '%s', a: %p, n: %" rdint_fmt, rc, id, a, n);

  /* client only */
  ASSERT(rc->client);

#if TRACE_DATA
  TRACE_F("");
  print_array_val<T>(a, n);
#endif

  redirect_call_put_input_conf_rdint(rc, "a", n);
  redirect_call_put_inout_param_dense_val<T>(rc, id, a, n);
}


template<typename T>
void redirect_call_get_inout_param_array_val(redirect_call_t *rc, const char *id, T **a, rdint_t *n)
{
  TRACE_F("rc: %p, id: '%s', a: %p, n: %" rdint_fmt, rc, id, *a, *n);

  /* server only */
  ASSERT(!rc->client);

  redirect_call_get_input_conf_rdint(rc, "a", n);
  redirect_call_get_inout_param_dense_val<T>(rc, id, a, *n);

#if TRACE_DATA
  TRACE_F("");
  print_array_val<T>(*a, *n);
#endif
}


#endif /* __REDIRECT_CALL_ARRAY_TCC__ */
