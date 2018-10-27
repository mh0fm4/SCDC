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


#ifndef __LOG_SET_HH__
#define __LOG_SET_HH__

/* allow following include of log_unset.hh */
#undef __LOG_UNSET_HH__


#if HAVE_SCDC_INFO
# define SCDC_INFO_PREFIX  "SCDC-INFO: "
# define SCDC_INFO(_x_)    SCDC_LOG_COUT(SCDC_INFO_PREFIX << SCDC_LOG_TIMESTAMP << SCDC_LOG_PREFIX << _x_ << std::endl)
#else
# define SCDC_INFO(_x_)    Z_NOP()
#endif

#if HAVE_SCDC_TRACE && !(SCDC_TRACE_NOT)
# define SCDC_TRACE_PREFIX                       "SCDC-TRACE: "
# define SCDC_TRACE_DECL(_x_)                    _x_
# define SCDC_TRACE_CMD(_x_)                     Z_MOP(_x_)
# define SCDC_TRACE(_x_)                         SCDC_LOG_COUT(SCDC_TRACE_PREFIX << SCDC_LOG_PREFIX << _x_ << std::endl)
# define SCDC_TRACE_N(_x_)                       SCDC_LOG_COUT(SCDC_TRACE_PREFIX << SCDC_LOG_PREFIX << _x_)
# define SCDC_TRACE_C(_x_)                       SCDC_LOG_COUT(_x_ << std::endl)
# define SCDC_TRACE_C_N(_x_)                     SCDC_LOG_COUT(_x_)
# define SCDC_TRACE_DATASET_INPUT(_d_, _x_...)   Z_MOP(SCDC_TRACE_N(_x_); scdc_dataset_input_log_cout_print(_d_); SCDC_TRACE_C("");)
# define SCDC_TRACE_DATASET_OUTPUT(_d_, _x_...)  Z_MOP(SCDC_TRACE_N(_x_); scdc_dataset_output_log_cout_print(_d_); SCDC_TRACE_C("");)
#else
# define SCDC_TRACE_DECL(_x_)
# define SCDC_TRACE_CMD(_x_)                     Z_NOP()
# define SCDC_TRACE(_x_)                         Z_NOP()
# define SCDC_TRACE_N(_x_)                       Z_NOP()
# define SCDC_TRACE_C(_x_)                       Z_NOP()
# define SCDC_TRACE_C_N(_x_)                     Z_NOP()
# define SCDC_TRACE_INOUT(_d_, _x_)              Z_NOP()
# define SCDC_TRACE_INPUT(_d_, _x_)              Z_NOP()
# define SCDC_TRACE_OUTPUT(_d_, _x_)             Z_NOP()
# define SCDC_TRACE_DATASET_INPUT(_d_, _x_...)   Z_NOP()
# define SCDC_TRACE_DATASET_OUTPUT(_d_, _x_...)  Z_NOP()
#endif
#define SCDC_TRACE_F(_x_)    SCDC_TRACE(__func__ << ": " << _x_)
#define SCDC_TRACE_F_N(_x_)  SCDC_TRACE_N(__func__ << ": " << _x_)

#if HAVE_SCDC_FAIL
# define SCDC_FAIL_PREFIX  "SCDC-FAIL: "
# define SCDC_FAIL(_x_)    SCDC_LOG_COUT(SCDC_FAIL_PREFIX << SCDC_LOG_PREFIX << _x_ << std::endl)
#else
# define SCDC_FAIL(_x_)    Z_NOP()
#endif
#define SCDC_FAIL_F(_x_)  SCDC_FAIL(__func__ << ": " << _x_)

#if HAVE_SCDC_ERROR
# define SCDC_ERROR_PREFIX  "SCDC-ERROR: "
# define SCDC_ERROR(_x_)    SCDC_LOG_CERR(SCDC_ERROR_PREFIX << SCDC_LOG_PREFIX << _x_ << std::endl)
#else
# define SCDC_ERROR(_x_)    Z_NOP()
#endif
#define SCDC_ERROR_F(_x_)   SCDC_ERROR(__func__ << ": " << _x_)

#if HAVE_SCDC_FATAL
# define SCDC_FATAL_PREFIX  "SCDC-FATAL: "
# define SCDC_FATAL(_x_)    SCDC_LOG_CERR(SCDC_FATAL_PREFIX << SCDC_LOG_PREFIX << _x_ << std::endl)
#else
# define SCDC_FATAL(_x_)    Z_NOP()
#endif
#define SCDC_FATAL_F(_x_)   SCDC_FATAL(__func__ << ": " << _x_)

#if HAVE_SCDC_ASSERT
# define SCDC_ASSERT_PREFIX  "SCDC-ASSERT: "
# define SCDC_ASSERT(_t_)    Z_MOP(if (!(_t_)) SCDC_LOG_CERR(SCDC_ASSERT_PREFIX << __FILE__ << ":" << __LINE__ << ": '" #_t_ << "' failed" << std::endl);)
#else
# define SCDC_ASSERT(_x_)    Z_NOP()
#endif


#endif /* __LOG_SET_HH__ */
