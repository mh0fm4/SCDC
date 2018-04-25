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


#ifndef __REDIRECT_DATA_H__
#define __REDIRECT_DATA_H__


#include "redirect_config.h"

#ifdef __cplusplus
# include "redirect_data.hh"
#endif


#ifdef __cplusplus
extern "C" {
#endif


#define DECLARE_BLOCKS(_t_, _tn_) \
  rdint_t redirect_data_blocks_pack_ ## _tn_(rdint_t count, rdint_t size, rdint_t stride, _t_ *b, _t_ **bout); \
  rdint_t redirect_data_blocks_unpack_ ## _tn_(rdint_t count, rdint_t size, rdint_t stride, _t_ *b, _t_ **bout); \
  void redirect_data_blocks_free_ ## _tn_(_t_ *b);


DECLARE_BLOCKS(float, float)
DECLARE_BLOCKS(double, double)


#undef DECLARE_BLOCKS


#ifdef __cplusplus
}
#endif


#endif /* __REDIRECT_DATA_H__ */
