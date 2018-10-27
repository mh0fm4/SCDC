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


#ifndef __REDIRECT_PARAM_H__
#define __REDIRECT_PARAM_H__


#include "redirect_config.h"


/* matrix */
#define RCM_TYPE                   0x000F
#define RCM_TYPE_DENSE             0x0000
#define RCM_TYPE_TRIANGULAR_UPPER  0x0001
#define RCM_TYPE_TRIANGULAR_LOWER  0x0002
#define RCM_GET_TYPE(_x_)          ((_x_) & RCM_TYPE)

#define RCM_ORDER                  0x00F0
#define RCM_ORDER_COL_MAJOR        0x0000
#define RCM_ORDER_ROW_MAJOR        0x0010
#define RCM_GET_ORDER(_x_)         ((_x_) & RCM_ORDER)

#define RCM_DIAG_NOT               0x0000
#define RCM_DIAG                   0x0100
#define RCM_GET_DIAG(_x_)          ((_x_) & RCM_DIAG)

#define RCM_IS_SET(_x_, _y_)       (((_x_) & (_y_)) == (_y_))


typedef struct _redirect_param_dense_t
{
  void *buf;
  rdint_t size;

} redirect_param_dense_t, redirect_params_dense_t;

typedef struct _redirect_param_blocks_t
{
  void *buf;
  rdint_t count, size, stride;

} redirect_param_blocks_t, redirect_params_blocks_t;

#define REDIRECT_PARAM_TYPE_DENSE   0
#define REDIRECT_PARAM_TYPE_BLOCKS  1

typedef struct _redirect_param_t
{
  int type;
#if REDIRECT_CALL_PARAMS_CACHE
  int cacheable;
#endif
  union
  {
    redirect_param_dense_t dense;
    redirect_param_blocks_t blocks;
  } p;
  
} redirect_param_t;


#if REDIRECT_PARAM_HASH_64
typedef unsigned long long redirect_param_hash_t;
#define redirect_param_hash_fmt        ".16llX"
#define redirect_param_hash_fmt_scanf  "X"
#else /* REDIRECT_PARAM_HASH_64 */
typedef unsigned int redirect_param_hash_t;
#define redirect_param_hash_fmt        ".8X"
#define redirect_param_hash_fmt_scanf  "X"
#endif /* REDIRECT_PARAM_HASH_64 _*/

redirect_param_hash_t redirect_param_hash_dense(redirect_param_dense_t *param);
redirect_param_hash_t redirect_param_hash_blocks(redirect_param_dense_t *param);


#endif /* __REDIRECT_PARAM_H__ */
