/*
 *  Copyright (C) 2014, 2015, 2016, 2017 Michael Hofmann
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


#ifndef __REDIRECT_DATA_HH__
#define __REDIRECT_DATA_HH__


#include "redirect_config.h"


template<typename T>
rdint_t redirect_data_blocks_pack_val(rdint_t count, rdint_t size, rdint_t stride, T *b, T **bout);

template<typename T>
rdint_t redirect_data_blocks_unpack_val(rdint_t count, rdint_t size, rdint_t stride, T *b, T **bout);

template<typename T>
rdint_t redirect_data_blocks_transform_val(rdint_t count, rdint_t size, rdint_t stride_in, T *b_in, rdint_t stride_out, T *b_out);

template<typename T>
void redirect_data_blocks_free_val(T *b);


#endif /* __REDIRECT_DATA_HH__ */
