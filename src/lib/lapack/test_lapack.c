/*
 *  Copyright (C) 2014, 2015, 2016, 2017 Florian Polster
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
#include <math.h>

#include "z_pack.h"
#include "lapack.h"


#if LIBLAPACK_SCDC && LIBLAPACK_SCDC_PREFIX
# undef __LAPACK_H__
# undef MANGLE_LAPACK
# define MANGLE_LAPACK(_f_)  Z_CONCAT(liblapack_scdc_, _f_)
# include "lapack.h"
#endif


#define RANDOM  0

void init_vector(float *v, int n, int inc)
{
  int i, j;

  for (i = 0; i < n; ++i)
  {
    v[i * inc + 0] =
#if RANDOM
      random() / (float) RAND_MAX;
#else
      i + 1.0;
#endif

    for (j = 1; j < inc; ++j)
      v[i * inc + j] = NAN;
  }
}


void print_vector(float *v, int n, int inc)
{
  int i, j;

  for (i = 0; i < n; ++i)
  {
    for (j = 0; j < inc; ++j)
      printf("  %8f", v[i * inc + j]);

    printf("\n");
  }
}


void init_matrix_cmo(float *m, int nrows, int ncols, int ld)
{
  int i, j;

  for (i = 0; i < ld; ++i)
  {
    for (j = 0; j < ncols; ++j)
    {
      if (i < nrows)
      {
        m[j * ld + i] =
#if RANDOM
          random() / (float) RAND_MAX;
#else
    			(j <= i)?1.0:0.0;
#endif

      } else m[j * ld + i] = NAN;
    }
  }
}


void print_matrix_cmo(float *m, int nrows, int ncols, int ld)
{
  int i, j;

  for (i = 0; i < ld; ++i)
  {
    for (j = 0; j < ncols; ++j)
      printf("  %8f", m[j * ld + i]);

    printf("\n");
  }
}


/* solving the matrix equation A*x = b using LAPACKs sgesv */
void test_sgesv()
{
  printf("%s:\n", __func__);

  const int N = 10;
  const int NRHS = N;

  const int LDA = N + 1;
  const int LDB = N + 2;

	float *A = malloc(LDA * N * sizeof(float));
	int *IPIV = malloc(N * sizeof(int));
  float *B = malloc(LDB * NRHS * sizeof(float));

  int INFO = 0;

  init_matrix_cmo(A, N, N, LDA);
  init_matrix_cmo(B, N, NRHS, LDB);

  printf("A: %d x %d\n", N, N);
  print_matrix_cmo(A, N, N, LDA);
  printf("B: %d x %d\n", N, NRHS);
  print_matrix_cmo(B, N, NRHS, LDB);

	MANGLE_LAPACK(sgesv_)(&N, &NRHS, A, &LDA, IPIV, B, &LDB, &INFO);

  printf("A: %d x %d\n", N, N);
  print_matrix_cmo(A, N, N, LDA);
  printf("B: %d x %d\n", N, NRHS);
  print_matrix_cmo(B, N, NRHS, LDB);

  free(A);
  free(IPIV);
  free(B);
}


int main(int argc, char *argv[])
{
  srandom(0);

#if 1
  test_sgesv();
#endif

  return 0;
}
