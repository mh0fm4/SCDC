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


#ifndef __FCS_CONFIG_H__
#define __FCS_CONFIG_H__


/* Define to the scanf conversion that corresponds to the floating type to use
   for FCS. */
#define FCS_CONV_FLOAT "l"

/* Define to the scanf conversion that corresponds to the integer type to use
   for FCS. */
#define FCS_CONV_INT /**/

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


#endif /* __FCS_CONFIG_H__ */
