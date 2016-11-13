/*
 *  Copyright (C) 2014, 2015, 2016 Florian Polster
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

/* solving the matrix equation A*x=b using LAPACK */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <string.h>

#include "liblapack_scdc.h"

int main()
{
    int size = 10;
    int nrhs = 2;
    int lda = size+2;
    int ldb = size+2;
    int info;

	int pivot[size];
	float A[lda][size], b[ldb*nrhs], AT[lda*size];
/*    memset(A, 0, lda * size * sizeof(float));
    memset(b, 0, ldb * nrhs * sizeof(float));
    memset(AT, 0, lda * size * sizeof(float));*/

    int i, j;
	clock_t start;

	//Matrix-Init
	for(i=0; i < lda; i++) {
		for(j=0; j < size; j++) {
			A[i][j] = (j <= i) ? 1.0 : 0.0;
		}
	}
	for(i=0; i < size; i++) {
		b[i]=1.0;
        b[ldb+i] = 1.0 + i;
	}

	//Row-major Matrix --> Column-major Array
	for (i=0; i<size; i++)
	{
		for(j=0; j<lda; j++) AT[j+lda*i]=A[j][i];
	}

	start = clock();
	scdc_sgesv_(&size, &nrhs, AT, &lda, pivot, b, &ldb, &info);

	float time = (double) (clock() - start) / CLOCKS_PER_SEC;
	printf ("%fsec fuer die Berechnung von %d Variablen.\n\n", time, size);

	//for (j=0; j<size; j++) printf("%e\n", b[j]); printf("\n");
}
