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


#ifndef __LOG_UNSET_HH__
#define __LOG_UNSET_HH__

/* allow following include of log_set.hh */
#undef __LOG_SET_HH__


#undef SCDC_INFO_PREFIX
#undef SCDC_INFO

#undef SCDC_TRACE_PREFIX
#undef SCDC_TRACE_DECL
#undef SCDC_TRACE_CMD
#undef SCDC_TRACE
#undef SCDC_TRACE_N
#undef SCDC_TRACE_C
#undef SCDC_TRACE_C_N
#undef SCDC_TRACE_DATASET_INPUT
#undef SCDC_TRACE_DATASET_OUTPUT
#undef SCDC_TRACE_F
#undef SCDC_TRACE_F_N

#undef SCDC_FAIL_PREFIX
#undef SCDC_FAIL
#undef SCDC_FAIL_F

#undef SCDC_ERROR_PREFIX
#undef SCDC_ERROR
#undef SCDC_ERROR_F

#undef SCDC_FATAL_PREFIX
#undef SCDC_FATAL
#undef SCDC_FATAL_F

#undef SCDC_ASSERT_PREFIX
#undef SCDC_ASSERT


#endif /* __LOG_UNSET_HH__ */
