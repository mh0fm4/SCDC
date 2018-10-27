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
#include <string.h>

#include "z_pack.h"

#include "lapack.h"
#if LIBLAPACK_SCDC
# include "liblapack_scdc.h"
#endif

#include "config.h"
#include "parnum.h"
#include "test_blas_common.h"


#define REP_BENCH(_f_, _n_, _i_)  Z_MOP(for (j = 0; j < (_n_); ++j) _f_;)

#if LIBLAPACK_SCDC
# define LIBLAPACK_SCDC_CMD(_cmd_)  Z_MOP(_cmd_)
#else
# define LIBLAPACK_SCDC_CMD(_cmd_)  Z_NOP()
#endif


void lapack_bench_n()
{
  const int N[] = { LAPACK_BENCH_N };
  const int ns = sizeof(N) / sizeof(int);

	float *A = malloc(LAPACK_BENCH_MAX * LAPACK_BENCH_MAX * sizeof(float));
	int *IPIV = malloc(LAPACK_BENCH_MAX * sizeof(int));
  float *B = malloc(LAPACK_BENCH_MAX * LAPACK_BENCH_MAX * sizeof(float));

  int INFO = 0;

  int i, j;
  double t[4 * 2];

  printf("#N  idamax  X  dcopy  X  dscal  X  daxpy  X\n");

  for (i = 0; i < ns; ++i)
  {
    const int NRHS = N[i];

    const int LDA = N[i];
    const int LDB = N[i];

    fmatrix_cmo_init_rand(N[i], N[i], A, LDA);
    fmatrix_cmo_init_rand(N[i], NRHS, B, LDB);

  	MANGLE_LAPACK(sgesv_)(&N[i], &NRHS, A, &LDA, IPIV, B, &LDB, &INFO);
    t[0] = z_time_wtime();
  	REP_BENCH(MANGLE_LAPACK(sgesv_)(&N[i], &NRHS, A, &LDA, IPIV, B, &LDB, &INFO), LAPACK_NREP, j);
    t[0] = (z_time_wtime() - t[0]) / LAPACK_NREP;
    LIBLAPACK_SCDC_CMD(t[1] = liblapack_scdc_timing[1];);

    printf("%4d  %f  %f\n", N[i], t[0], t[1]);
  }

  free(A);
  free(IPIV);
  free(B);
}


void parnum_lapack(int argc, char *argv[])
{
  --argc; ++argv;

  if (argc < 1) return;

#if LIBLAPACK_SCDC
  liblapack_scdc_init();

#if 0
  liblapack_scdc_uri = "scdc+uds://liblapack_scdc/lapack";
#endif
#endif

  if (strcmp(argv[0], "n") == 0)
  {
    lapack_bench_n();
  }

#if LIBLAPACK_SCDC
  liblapack_scdc_release();
#endif
}
