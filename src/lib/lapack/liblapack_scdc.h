/*
 *  Copyright (C) 2014, 2015, 2016 Michael Hofmann
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


#ifndef __LIBLAPACK_SCDC_H__
#define __LIBLAPACK_SCDC_H__


void liblapack_scdc_init();
void liblapack_scdc_release();

void scdc_sgesv_(int *N, int *NRHS, float *A, int *LDA, int *IPIV, float *B, int *LDB, int *INFO);
void scdc_sgesv(int N, int NRHS, float *A, int LDA, int *IPIV, float *B, int LDB, int INFO);


#endif /* __LAPACK_CALL_H__ */