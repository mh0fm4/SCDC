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


#ifndef __CONFIG_H__
#define __CONFIG_H__


#define HAVE_TRACE   1
#define HAVE_ASSERT  1
#define HAVE_TIMING  1

#define REDIRECT_CALL_BLOCKS_DENSE    1
#define REDIRECT_CALL_BLOCKS_PACK_IP  2
#define REDIRECT_CALL_BLOCKS_PACK_OP  3
#define REDIRECT_CALL_BLOCKS_PIECES   4

#if 0
#define REDIRECT_CALL_INPUT_PARAM_BLOCKS   REDIRECT_CALL_BLOCKS_DENSE
#define REDIRECT_CALL_OUTPUT_PARAM_BLOCKS  REDIRECT_CALL_BLOCKS_DENSE
#define REDIRECT_CALL_INOUT_PARAM_BLOCKS   REDIRECT_CALL_BLOCKS_DENSE
#endif

#if 0
#define REDIRECT_CALL_INPUT_PARAM_BLOCKS   REDIRECT_CALL_BLOCKS_PACK_IP
#define REDIRECT_CALL_OUTPUT_PARAM_BLOCKS  REDIRECT_CALL_BLOCKS_PACK_IP
#define REDIRECT_CALL_INOUT_PARAM_BLOCKS   REDIRECT_CALL_BLOCKS_PACK_IP
#endif

#if 1
#define REDIRECT_CALL_INPUT_PARAM_BLOCKS   REDIRECT_CALL_BLOCKS_PACK_OP
#define REDIRECT_CALL_OUTPUT_PARAM_BLOCKS  REDIRECT_CALL_BLOCKS_PACK_OP
#define REDIRECT_CALL_INOUT_PARAM_BLOCKS   REDIRECT_CALL_BLOCKS_PACK_OP
#endif

#define REDIRECT_CALL_PARAMS_DENSE  1


#endif /* __CONFIG_H__ */
