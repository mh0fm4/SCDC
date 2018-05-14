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


#ifndef __REDIRECT_CALL_MATRIX_TCC__
#define __REDIRECT_CALL_MATRIX_TCC__


/* typed matrices with leading dimension */

static rdint_t get_matrix_cont_size(rdint_t nr, rdint_t nc, rdint_t ld, rdint_t rcm)
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


static rdint_t get_matrix_cont_index(rdint_t nr, rdint_t nc, rdint_t ld, rdint_t rcm, rdint_t r, rdint_t c)
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


static void get_matrix_blocks(rdint_t nr, rdint_t nc, rdint_t ld, rdint_t rcm, rdint_t *count, rdint_t *size, rdint_t *stride)
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


#if 0
template<typename T, int (*F)(char *s, T v)>
static void print_matrix_val(rdint_t nr, rdint_t nc, T *m, rdint_t ld, rdint_t rcm)
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
static void print_matrix_val(const T *m, rdint_t nr, rdint_t nc, rdint_t ld, rdint_t rcm)
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
static void print_matrix_val(const T *m, rdint_t nr, rdint_t nc, rdint_t ld, rdint_t rcm);

#define DEFINE_PRINT_MATRIX(_t_, _tn_, _fmt_) \
  template<> \
  void print_matrix_val<_t_>(const _t_ *m, rdint_t nr, rdint_t nc, rdint_t ld, rdint_t rcm) { \
    print_matrix_val<_t_, _fmt_>(m, nr, nc, ld, rcm); \
  }


template<typename T>
void redirect_call_print_param_matrix_val(const T *m, rdint_t nr, rdint_t nc, rdint_t ld, rdint_t rcm)
{
  print_matrix_val<T>(m, nr, nc, ld, rcm);
}


template<typename T>
void redirect_call_put_input_param_matrix_val(redirect_call_t *rc, const char *id, const T *m, rdint_t nr, rdint_t nc, rdint_t ld, rdint_t rcm)
{
  TRACE_F("%s: rc: %p, id: '%s', m: %p, nr: %" rdint_fmt ", nc: %" rdint_fmt ", ld: %" rdint_fmt ", rcm: %" rdint_fmt, __func__, rc, id, m, nr, nc, ld, rcm);

  /* client only */
  ASSERT(rc->client);

#if TRACE_DATA
  TRACE_F("%s:", __func__);
  print_matrix_val<T>(m, nr, nc, ld, rcm);
#endif

  const rdint_t x[4] = { nr, nc, ld, rcm };
  redirect_call_put_input_conf_rdints(rc, "m", x, 4);

  rdint_t count, size, stride;
  get_matrix_blocks(nr, nc, ld, rcm, &count, &size, &stride);
  redirect_call_put_input_param_blocks_val<T>(rc, id, m, count, size, stride);
}


template<typename T>
void redirect_call_get_input_param_matrix_val(redirect_call_t *rc, const char *id, T **m, rdint_t *nr, rdint_t *nc, rdint_t *ld, rdint_t *rcm)
{
  TRACE_F("%s: rc: %p, id: '%s', m: %p, nr: %" rdint_fmt ", nc: %" rdint_fmt ", ld: %" rdint_fmt ", rcm: %" rdint_fmt, __func__, rc, id, *m, *nr, *nc, *ld, *rcm);

  /* server only */
  ASSERT(!rc->client);

  rdint_t x[4];
  redirect_call_get_input_conf_rdints(rc, "m", x, 4);
  *nr = x[0];
  *nc = x[1];
  /* the original ld in x[2] is ignored, use the locally given ld or the stride from the transfer (if ld == 0) */
  *rcm = x[3];

  rdint_t count, size, stride;
  get_matrix_blocks(*nr, *nc, *ld, *rcm, &count, &size, &stride);
  redirect_call_get_input_param_blocks_val<T>(rc, id, m, count, size, &stride);
  *ld = stride;

#if TRACE_DATA
  TRACE_F("%s:", __func__);
  print_matrix_val<T>(*m, *nr, *nc, *ld, *rcm);
#endif
}


template<typename T>
void redirect_call_put_output_param_matrix_val(redirect_call_t *rc, const char *id, const T *m, rdint_t nr, rdint_t nc, rdint_t ld, rdint_t rcm)
{
  TRACE_F("%s: rc: %p, id: '%s', m: %p, nr: %" rdint_fmt ", nc: %" rdint_fmt ", ld: %" rdint_fmt ", rcm: %" rdint_fmt, __func__, rc, id, m, nr, nc, ld, rcm);

  if (!rc->client)
  {
#if TRACE_DATA
    TRACE_F("%s:", __func__);
    print_matrix_val<T>(m, nr, nc, ld, rcm);
#endif
  }

  const rdint_t x[4] = { nr, nc, ld, rcm };
  if (rc->client) redirect_call_put_input_conf_rdints(rc, "m", x, 4);
  else redirect_call_put_output_conf_rdints(rc, "m", x, 4);

  rdint_t count, size, stride;
  get_matrix_blocks(nr, nc, ld, rcm, &count, &size, &stride);
  redirect_call_put_output_param_blocks_val<T>(rc, id, m, count, size, stride);
}


template<typename T>
void redirect_call_get_output_param_matrix_val(redirect_call_t *rc, const char *id, T **m, rdint_t *nr, rdint_t *nc, rdint_t *ld, rdint_t *rcm)
{
  TRACE_F("%s: rc: %p, id: '%s', m: %p, nr: %" rdint_fmt ", nc: %" rdint_fmt ", ld: %" rdint_fmt ", rcm: %" rdint_fmt, __func__, rc, id, *m, *nr, *nc, *ld, *rcm);

  rdint_t x[4];
  if (rc->client) redirect_call_get_output_conf_rdints(rc, "m", x, 4);
  else redirect_call_get_input_conf_rdints(rc, "m", x, 4);
  *nr = x[0];
  *nc = x[1];
  /* the original ld in x[2] is ignored, use the locally given ld or the stride from the transfer (if ld == 0) */
  *rcm = x[3];

  rdint_t count, size, stride;
  get_matrix_blocks(*nr, *nc, *ld, *rcm, &count, &size, &stride);
  redirect_call_get_output_param_blocks_val<T>(rc, id, m, count, size, &stride);
  *ld = stride;

  if (rc->client)
  {
#if TRACE_DATA
    TRACE_F("%s:", __func__);
    print_matrix_val<T>(*m, *nr, *nc, *ld, *rcm);
#endif
  }
}


template<typename T>
void redirect_call_put_inout_param_matrix_val(redirect_call_t *rc, const char *id, const T *m, rdint_t nr, rdint_t nc, rdint_t ld, rdint_t rcm)
{
  TRACE_F("%s: rc: %p, id: '%s', m: %p, nr: %" rdint_fmt ", nc: %" rdint_fmt ", ld: %" rdint_fmt ", rcm: %" rdint_fmt, __func__, rc, id, m, nr, nc, ld, rcm);

  /* client only */
  ASSERT(rc->client);

#if TRACE_DATA
  TRACE_F("%s:", __func__);
  print_matrix_val<T>(m, nr, nc, ld, rcm);
#endif

  const rdint_t x[4] = { nr, nc, ld, rcm };
  if (rc->client) redirect_call_put_input_conf_rdints(rc, "m", x, 4);
  else redirect_call_put_output_conf_rdints(rc, "m", x, 4);

  rdint_t count, size, stride;
  get_matrix_blocks(nr, nc, ld, rcm, &count, &size, &stride);
  redirect_call_put_inout_param_blocks_val<T>(rc, id, m, count, size, stride);
}


template<typename T>
void redirect_call_get_inout_param_matrix_val(redirect_call_t *rc, const char *id, T **m, rdint_t *nr, rdint_t *nc, rdint_t *ld, rdint_t *rcm)
{
  TRACE_F("%s: rc: %p, id: '%s', m: %p, nr: %" rdint_fmt ", nc: %" rdint_fmt ", ld: %" rdint_fmt ", rcm: %" rdint_fmt, __func__, rc, id, *m, *nr, *nc, *ld, *rcm);

  /* server only */
  ASSERT(!rc->client);

  rdint_t x[4];
  if (rc->client) redirect_call_get_output_conf_rdints(rc, "m", x, 4);
  else redirect_call_get_input_conf_rdints(rc, "m", x, 4);
  *nr = x[0];
  *nc = x[1];
  /* the original ld in x[2] is ignored, use the locally given ld or the stride from the transfer (if ld == 0) */
  *rcm = x[3];

  rdint_t count, size, stride;
  get_matrix_blocks(*nr, *nc, *ld, *rcm, &count, &size, &stride);
  redirect_call_get_inout_param_blocks_val<T>(rc, id, m, count, size, &stride);
  *ld = stride;

#if TRACE_DATA
  TRACE_F("%s:", __func__);
  print_matrix_val<T>(*m, *nr, *nc, *ld, *rcm);
#endif
}


#endif /* __REDIRECT_CALL_MATRIX_TCC__ */
