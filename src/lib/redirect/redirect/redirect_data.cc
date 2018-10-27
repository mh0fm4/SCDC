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


#define TRACE_PREFIX  "redirect_data: "


void redirect_data_blocks_print_val(rdint_t count, rdint_t size, rdint_t stride, double *b)
{
  rdint_t i, j;

  for (i = 0; i < count; ++i)
  {
    for (j = 0; j < stride; ++j)
    {
      printf("  %f", b[i * stride + j]);

      if (i == count - 1) break;
    }
    printf("  |");
  }

  printf("\n");
}


template<typename T>
rdint_t redirect_data_blocks_pack_val(rdint_t count, rdint_t size, rdint_t stride, T *b, T **bout)
{
#if PRINT_BLOCKS
  redirect_data_blocks_print_val(count, size, stride, (double *) b);
#endif

  const rdint_t total_size = (count - 1) * z_min(stride, size) + size;

  if (bout && !(*bout)) *bout = static_cast<T *>(malloc(total_size * sizeof(T)));

  if (stride <= size)
  {
    /* nothing to pack, just copy */
    if (bout) memcpy(*bout, b, total_size * sizeof(T));

  } else
  {
    rdint_t i, j;

    if (bout)
    {
      /* out-of-place */
      T *br = b;
      T *bw = *bout;

      for (i = 0; i < count; ++i)
      {
        for (j = 0; j < size; ++j) bw[j] = br[j];

        br += stride;
        bw += size;
      }

    } else
    {
      /* in-place */
      T *br = b + stride;
      T *bw = b + size;

      for (i = 1; i < count; ++i)
      {
        for (j = 0; j < size; ++j) bw[j] = br[j];

        br += stride;
        bw += size;
      }
    }
  }

#if PRINT_BLOCKS
  redirect_data_blocks_print_val(count, size, 1, (double *) b);
#endif

  return total_size;
}


template<typename T>
rdint_t redirect_data_blocks_unpack_val(rdint_t count, rdint_t size, rdint_t stride, T *b, T **bout)
{
#if PRINT_BLOCKS
  redirect_data_blocks_print_val(count, size, 1, (double *) b);
#endif

  const rdint_t total_size = (count - 1) * z_max(stride, size) + size;

  if (bout && !(*bout)) *bout = static_cast<T *>(malloc(total_size * sizeof(T)));

  if (stride <= size)
  {
    /* nothing to unpack, just copy */
    if (bout) memcpy(*bout, b, total_size * sizeof(T));

  } else
  {
    rdint_t i, j;

    if (bout)
    {
      /* out-of-place */
      T *br = b + (count - 1) * size;
      T *bw = *bout + (count - 1) * stride;;

      for (i = count - 1; i >= 0; --i)
      {
        for (j = size - 1; j >= 0; --j) bw[j] = br[j];

        br -= size;
        bw -= stride;
      }

    } else
    {
      /* in-place */
      T *br = b + (count - 1) * size;
      T *bw = b + (count - 1) * stride;

      for (i = count - 1; i >= 1; --i)
      {
        for (j = size - 1; j >= 0; --j) bw[j] = br[j];

        br -= size;
        bw -= stride;
      }
    }
  }

#if PRINT_BLOCKS
  redirect_data_blocks_print_val(count, size, stride, (double *) b);
#endif

  return total_size;
}


template<typename T>
rdint_t redirect_data_blocks_transform_val(rdint_t count, rdint_t size, rdint_t stride_in, T *b_in, rdint_t stride_out, T *b_out)
{
#if PRINT_BLOCKS
  redirect_data_blocks_print_val(count, size, stride_in, (double *) b);
#endif

  if (stride_in <= 0 || stride_out <= 0) return 0;

  const rdint_t total_size = (count - 1) * z_max(stride_out, size) + size;

  /* no transform? */
  if (stride_in == stride_out)
  {
    if (b_out) memcpy(b_out, b_in, total_size * sizeof(T));

    return total_size;
  }

  /* pack in */
  if (stride_out == 1)
  {
    return redirect_data_blocks_pack_val(count, size, stride_in, b_in, (b_out)?&b_out:NULL);
  }

  /* pack out */
  if (stride_in == 1)
  {
    return redirect_data_blocks_unpack_val(count, size, stride_out, b_in, (b_out)?&b_out:NULL);
  }

  rdint_t i, j;

  if (b_out)
  {
    /* out-of-place */
    T *br = b_in;
    T *bw = b_out;

    for (i = 0; i < count; ++i)
    {
      for (j = 0; j < size; ++j) bw[j] = br[j];

      br += stride_in;
      bw += stride_out;
    }

  } else
  {
    /* in-place */
    if (stride_in > stride_out)
    {
      /* forward */
      T *br = b_in + stride_in;
      T *bw = b_in + stride_out;

      for (i = 1; i < count; ++i)
      {
        for (j = 0; j < size; ++j) bw[j] = br[j];

        br += stride_in;
        bw += stride_out;
      }

    } else
    {
      /* backward */
      T *br = b_in + (count - 1) * stride_in;
      T *bw = b_in + (count - 1) * stride_out;

      for (i = count - 1; i >= 1; --i)
      {
        for (j = size - 1; j >= 0; --j) bw[j] = br[j];

        br -= stride_in;
        bw -= stride_out;
      }
    }
  }

#if PRINT_BLOCKS
  redirect_data_blocks_print_val(count, size, stride_out, (double *) b);
#endif

  return total_size;
}


template<typename T>
T *redirect_data_blocks_alloc_val(rdint_t count, rdint_t size, rdint_t stride)
{
  if (count <= 0 || stride <= 0) return NULL;

  return malloc(((count - 1) * stride + size) * sizeof(T));
}


template<typename T>
void redirect_data_blocks_free_val(T *b)
{
  free(b);
}


#define DEFINE_BLOCKS(_t_, _tn_) \
  rdint_t redirect_data_blocks_pack_ ## _tn_(rdint_t count, rdint_t size, rdint_t stride, _t_ *b, _t_ **bout) { \
    return redirect_data_blocks_pack_val<_t_>(count, size, stride, b, bout); \
  } \
  rdint_t redirect_data_blocks_unpack_ ## _tn_(rdint_t count, rdint_t size, rdint_t stride, _t_ *b, _t_ **bout) { \
    return redirect_data_blocks_unpack_val<_t_>(count, size, stride, b, bout); \
  } \
  rdint_t redirect_data_blocks_transform_ ## _tn_(rdint_t count, rdint_t size, rdint_t stride_in, _t_ *b_in, rdint_t stride_out, _t_ *b_out) { \
    return redirect_data_blocks_transform_val<_t_>(count, size, stride_in, b_in, stride_out, b_out); \
  } \
  void redirect_data_blocks_free_ ## _tn_(_t_ *b) { \
    return redirect_data_blocks_free_val<_t_>(b); \
  }


DEFINE_BLOCKS(float, float)
DEFINE_BLOCKS(double, double)

DEFINE_BLOCKS(char, char)
DEFINE_BLOCKS(int, int)
