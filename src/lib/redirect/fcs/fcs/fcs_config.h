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

/* fcs_config.h.  Generated from fcs_config.h.in by configure.  */
/* fcs_config.h.in.  Public config header for ScaFaCoS FCS.  */

/* Define to the scanf conversion that corresponds to the floating type to use
   for FCS. */
#define FCS_CONV_FLOAT "l"

/* Define to the scanf conversion that corresponds to the integer type to use
   for FCS. */
#define FCS_CONV_INT /**/

/* Whether solver method direct is enabled. */
#define FCS_ENABLE_DIRECT 1

/* Whether solver method ewald is enabled. */
#define FCS_ENABLE_EWALD 1

/* Whether solver method fmm is enabled. */
/* #undef FCS_ENABLE_FMM */

/* Whether solver method memd is enabled. */
#define FCS_ENABLE_MEMD 1

/* Whether solver method mmm1d is enabled. */
#define FCS_ENABLE_MMM1D 1

/* Whether solver method mmm2d is enabled. */
#define FCS_ENABLE_MMM2D 1

/* Whether solver method p2nfft is enabled. */
#define FCS_ENABLE_P2NFFT 1

/* Whether solver method p3m is enabled. */
#define FCS_ENABLE_P3M 1

/* Whether solver method pepc is enabled. */
/* #undef FCS_ENABLE_PEPC */

/* Whether solver method pp3mg is enabled. */
#define FCS_ENABLE_PP3MG 1

/* Whether solver method vmg is enabled. */
#define FCS_ENABLE_VMG 1

/* Define whether fcs_float is double. */
#define FCS_FLOAT_IS_DOUBLE 1

/* Define whether fcs_float is float. */
/* #undef FCS_FLOAT_IS_FLOAT */

/* Define whether fcs_float is long double. */
/* #undef FCS_FLOAT_IS_LONG_DOUBLE */

/* Define whether fcs_int is int. */
#define FCS_INT_IS_INT 1

/* Define whether fcs_int is long. */
/* #undef FCS_INT_IS_LONG */

/* Define whether fcs_int is long long. */
/* #undef FCS_INT_IS_LONG_LONG */

/* Define whether fcs_int is short. */
/* #undef FCS_INT_IS_SHORT */

/* Define to the printf length modifier that corresponds to the floating type
   to use for FCS. */
#define FCS_LMOD_FLOAT ""

/* Define to the printf length modifier that corresponds to the integer type
   to use for FCS. */
#define FCS_LMOD_INT ""

/* Define to the MPI datatype that corresponds to the floating type to use for
   FCS. */
#define FCS_MPI_FLOAT MPI_DOUBLE

/* Define to the MPI datatype that corresponds to the integer type to use for
   FCS. */
#define FCS_MPI_INT MPI_INT

/* Define to the floating type to use for FCS. */
#define fcs_float double

/* Define to the integer type to use for FCS. */
#define fcs_int int

/* Define to a macro mangling the given C identifier (in lower and upper
   case), which must not contain underscores, for linking with Fortran. */
#define FC_FUNC(name,NAME) name ## _

/* As FC_FUNC, but for C identifiers containing underscores. */
#define FC_FUNC_(name,NAME) name ## _
