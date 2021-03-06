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


#ifndef __REDIRECT_CONFIG__
#define __REDIRECT_CONFIG__


#if 1
#define rdint_type  long
typedef rdint_type  rdint_t;
#define rdint_fmt   "ld"
#else
#define rdint_type  int
typedef rdint_type rdint_t;
#define rdint_fmt  "d"
#endif


#define HAVE_TRACE   1
#define HAVE_ASSERT  1
#define HAVE_TIMING  1


#define REDIRECT_PARAM_HASH_64  0

#define REDIRECT_CALL_OP_MAX_SIZE              64
#define REDIRECT_CALL_URI_MAX_SIZE            256
#define REDIRECT_CALL_CACHE_REQUEST_MAX_SIZE  256
#define REDIRECT_CALL_CACHE_STATE_MAX_SIZE    256
#define REDIRECT_CALL_INPUT_CONF_MAX_SIZE     512
#define REDIRECT_CALL_OUTPUT_CONF_MAX_SIZE    512
#define REDIRECT_CALL_RESULT_STR_MAX_SIZE     256
#define REDIRECT_CALL_PARAMS_MAX               16
#define REDIRECT_CALL_FREE_BUFS_MAX            16

#define REDIRECT_CACHE_PARAMS_MAX        REDIRECT_CALL_PARAMS_MAX
#define REDIRECT_CACHE_ENTRIES_MAX       16
#define REDIRECT_CACHE_FIND_ENTRIES_MAX  16

#define REDIRECT_CALL_PARAMS_NEW  1

#define REDIRECT_CALL_PARAMS_CACHE  1

#define REDIRECT_CALL_VECTOR_DENSE  1
#define REDIRECT_CALL_MATRIX_DENSE  1

#define REDIRECT_CALL_BLOCKS_PLAIN    1
#define REDIRECT_CALL_BLOCKS_PACK_IP  2
#define REDIRECT_CALL_BLOCKS_PACK_OP  3
#define REDIRECT_CALL_BLOCKS_PIECES   4

#if 1
#define REDIRECT_CALL_INPUT_PARAM_BLOCKS   REDIRECT_CALL_BLOCKS_PLAIN
#define REDIRECT_CALL_OUTPUT_PARAM_BLOCKS  REDIRECT_CALL_BLOCKS_PLAIN
#define REDIRECT_CALL_INOUT_PARAM_BLOCKS   REDIRECT_CALL_BLOCKS_PLAIN
#elif 1
#define REDIRECT_CALL_INPUT_PARAM_BLOCKS   REDIRECT_CALL_BLOCKS_PACK_IP
#define REDIRECT_CALL_OUTPUT_PARAM_BLOCKS  REDIRECT_CALL_BLOCKS_PACK_IP
#define REDIRECT_CALL_INOUT_PARAM_BLOCKS   REDIRECT_CALL_BLOCKS_PACK_IP
#elif 1
#define REDIRECT_CALL_INPUT_PARAM_BLOCKS   REDIRECT_CALL_BLOCKS_PACK_OP
#define REDIRECT_CALL_OUTPUT_PARAM_BLOCKS  REDIRECT_CALL_BLOCKS_PACK_OP
#define REDIRECT_CALL_INOUT_PARAM_BLOCKS   REDIRECT_CALL_BLOCKS_PACK_OP
#elif 1
#define REDIRECT_CALL_INPUT_PARAM_BLOCKS   REDIRECT_CALL_BLOCKS_PIECES
#define REDIRECT_CALL_OUTPUT_PARAM_BLOCKS  REDIRECT_CALL_BLOCKS_PIECES
#define REDIRECT_CALL_INOUT_PARAM_BLOCKS   REDIRECT_CALL_BLOCKS_PIECES
#endif


#endif /* __REDIRECT_CONFIG__ */
