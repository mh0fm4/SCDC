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


#define CHECK_FCS_RESULT(_f_, _r_) do { \
    if ((_r_) != FCS_SUCCESS) printf("%s: failed\n", (_f_)); \
  } while (0)


int main(int argc, char *argv[])
{
  srand(0);

#if USE_MPI
  MPI_Init(&argc, &argv);

  int comm_size, comm_rank;
  MPI_Comm comm = MPI_COMM_WORLD;

  MPI_Comm_size(comm, &comm_size);
  MPI_Comm_rank(comm, &comm_rank);
#endif

  FCS fcs;
  FCSResult result;

  result = MANGLE_FCS(fcs_init)(&fcs, "direct", MPI_COMM_WORLD);
  CHECK_FCS_RESULT("fcs_init", result);

  fcs_float box_a[] = { 1.0, 0.0, 0.0 };
  fcs_float box_b[] = { 0.0, 1.0, 0.0 };
  fcs_float box_c[] = { 0.0, 0.0, 1.0 };
  fcs_float box_origin[] = { 0.0, 0.0, 0.0 };
  fcs_int periodicity[] = { 0, 0, 0 };
  fcs_int total_particles = 100;

  result = MANGLE_FCS(fcs_set_common)(fcs, 1, box_a, box_b, box_c, box_origin, periodicity, total_particles);
  CHECK_FCS_RESULT("fcs_set_common", result);

  result = MANGLE_FCS(fcs_set_parameters)(fcs, "", 0);
  CHECK_FCS_RESULT("fcs_set_parameters", result);

  fcs_int local_particles = total_particles;
#if USE_MPI
  local_particles = total_particles / comm_size + ((total_particles % comm_size < comm_rank)?1:0);
#endif

  fcs_float *positions  = malloc(local_particles * 3 * sizeof(fcs_float));
  fcs_float *charges    = malloc(local_particles * 1 * sizeof(fcs_float));
  fcs_float *field      = malloc(local_particles * 3 * sizeof(fcs_float));
  fcs_float *potentials = malloc(local_particles * 1 * sizeof(fcs_float));

  fcs_int i;
  for (i = 0; i < local_particles; ++i)
  {
    positions[3 * i + 0] = ((fcs_float) rand() / RAND_MAX) * box_a[0];
    positions[3 * i + 1] = ((fcs_float) rand() / RAND_MAX) * box_b[1];
    positions[3 * i + 2] = ((fcs_float) rand() / RAND_MAX) * box_c[2];

    charges[i] = 1.0;

    field[3 * i + 0] = field[3 * i + 1] = field[3 * i + 2] = potentials[i] = -0.0;
  }

  result = MANGLE_FCS(fcs_tune)(fcs, local_particles, positions, charges);
  CHECK_FCS_RESULT("fcs_tune", result);

  result = MANGLE_FCS(fcs_run)(fcs, local_particles, positions, charges, field, potentials);
  CHECK_FCS_RESULT("fcs_run", result);

  i = 0;
  printf("%" FCS_LMOD_INT "d: positions: %" FCS_LMOD_FLOAT "f|%" FCS_LMOD_FLOAT "f|%" FCS_LMOD_FLOAT "f, charges: %" FCS_LMOD_FLOAT "f, field: %" FCS_LMOD_FLOAT "f|%" FCS_LMOD_FLOAT "f|%" FCS_LMOD_FLOAT "f, potentials: %" FCS_LMOD_FLOAT "f\n", i,
    positions[3 * i + 0], positions[3 * i + 1], positions[3 * i + 2], charges[i],
    field[3 * i + 0], field[3 * i + 1], field[3 * i + 2], potentials[i]);

  free(positions);
  free(charges);
  free(field);
  free(potentials);

  result = MANGLE_FCS(fcs_destroy)(fcs);
  CHECK_FCS_RESULT("fcs_destroy", result);

#if USE_MPI
  MPI_Finalize();
#endif

  return 0;
}
