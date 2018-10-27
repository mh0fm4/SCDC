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

#include "xxhash.h"

#include "redirect_config.h"
#include "common.h"
#include "redirect_param.h"


#define TRACE_PREFIX  "redirect_param: "


#if REDIRECT_PARAM_HASH_64
# define XXHXX(_f_)  Z_CONCAT(XXH64, _f_)
#else 
# define XXHXX(_f_)  Z_CONCAT(XXH32, _f_)
#endif


const static unsigned int redirect_param_hash_seed = 0;


redirect_param_hash_t redirect_param_hash_dense(redirect_param_dense_t *param)
{
  return XXHXX()(param->buf, param->size, redirect_param_hash_seed);
}


redirect_param_hash_t redirect_param_hash_blocks(redirect_param_blocks_t *param)
{
  XXHXX(_state_t) *s = XXHXX(_createState)();
  XXHXX(_reset)(s, redirect_param_hash_seed);

  rdint_t i;
  char *buf = static_cast<char *>(param->buf);
  for (i = 0; i < param->count; ++i)
  {
    XXH32_update(s, buf, param->size);
    buf += param->stride;
  }

  XXHXX(_hash_t) h = XXHXX(_digest)(s);

  XXHXX(_freeState)(s);

  return h;
}
