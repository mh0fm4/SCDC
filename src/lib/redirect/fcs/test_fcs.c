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


#if HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#if USE_MPI
# include <mpi.h>
#endif

#include "z_pack.h"
#include "fcs.h"


#if LIBFCS_SCDC && LIBFCS_SCDC_PREFIX
# undef __FCS_H__
# undef MANGLE_FCS
# define MANGLE_FCS(_f_)  Z_CONCAT(libfcs_scdc_, _f_)
# include "fcs.h"
#endif


int main(int argc, char *argv[])
{
  srandom(0);

  FCS fcs;

#if USE_MPI
  MPI_Init(&argc, &argv);
#endif

  MANGLE_FCS(fcs_init)(&fcs, "direct", MPI_COMM_WORLD);

  MANGLE_FCS(fcs_destroy)(fcs);

#if USE_MPI
  MPI_Finalize();
#endif

  return 0;
}
