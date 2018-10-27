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


#include <stdio.h>
#include <stdlib.h>

#include "redirect_test_config.h"
#include "common.h"
#include "redirect_call.h"
#include "redirect_common.h"


#define TRACE_PREFIX  "redirect_common: "


void vector_init(double *v, rdint_t n, rdint_t inc, double xv, double xn)
{
  int i, j;

  for (i = 0; i < n; ++i)
  {
    v[i * inc + 0] = xv;
    if (i + 1 == n) break;
    for (j = 1; j < inc; ++j) v[i * inc + j] = xn;
  }
}
