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


#ifndef __FCS_CALL_H__
#define __FCS_CALL_H__


#include "z_pack.h"
#include "redirect_handle.h"
#include "redirect_call.h"


typedef redirect_handle_t fcs_handle_t;

#define FCS_HANDLE(_x_)  Z_CONCAT(redirect_handle_, _x_)


typedef redirect_call_t fcs_call_t;

#define FCS_CALL(_x_)  Z_CONCAT(redirect_call_, _x_)


#endif /* __FCS_CALL_H__ */
