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


#ifndef __CONFIG_H__
#define __CONFIG_H__


#define HAVE_TRACE   1
#define HAVE_ASSERT  1
#define HAVE_TIMING  1


#define LIBFCS_SCDC_ENABLED  1

#define LIBFCS_SCDC_PREFIX       1
#define LIBFCS_SCDC_PREFIX_NAME  libfcs_scdc_

#define LIBFCS_SCDC_LOCAL       1
#define LIBFCS_SCDC_LOCAL_BASE  "fcs"
#define LIBFCS_SCDC_LOCAL_URI   "scdc:/"

#if 1
# define LIBFCS_SCDC_URI_DEFAULT  "scdc:/fcs"
#elif 1
# define LIBFCS_SCDC_URI_DEFAULT  "scdc+uds://libfcs_scdc/fcs"
#elif 1
# define LIBFCS_SCDC_URI_DEFAULT  "scdc+tcp://localhost/fcs"
#endif

#define LIBFCS_SCDC_TRACE_DATA  0

#define LIBFCS_SCDC_CACHE  (REDIRECT_CALL_PARAMS_CACHE && 1)

#define LIBFCS_SCDC_TIMING                  0
#define LIBFCS_SCDC_TIMING_REMOTE           1
#define LIBFCS_SCDC_TIMING_REDIRECT_REMOTE  1
#define LIBFCS_SCDC_TIMING_PRINT            1
#define LIBFCS_SCDC_TIMING_PRINT_REMOTE     1


#endif /* __CONFIG_H__ */