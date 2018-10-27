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


#ifndef __LOG_H__
#define __LOG_H__


#ifdef __cplusplus
extern "C" {
#endif


#include "scdc_defs.h"


void scdc_dataset_input_log_cout_print(scdc_dataset_input_t *input);
void scdc_dataset_output_log_cout_print(scdc_dataset_output_t *output);

int scdc_log_cout_printf(const char *format, ...);
int scdc_log_cout_printf_nl(const char *format, ...);
int scdc_log_cerr_printf(const char *format, ...);
int scdc_log_cerr_printf_nl(const char *format, ...);


/* the C++ header will define it's own macros! */
#ifndef __LOG_HH__

#if HAVE_SCDC_INFO
# define SCDC_INFO_PREFIX   "SCDC-INFO: "
# define SCDC_INFO(...)  scdc_cout_printf(SCDC_INFO_PREFIX SCDC_LOG_PREFIX __VA_ARGS__)
#else
# define SCDC_INFO(...)  Z_NOP()
#endif

#if HAVE_SCDC_TRACE && !(SCDC_TRACE_NOT)
# define SCDC_TRACE_PREFIX                       "SCDC-TRACE: "
# define SCDC_TRACE(...)                      scdc_log_cout_printf_nl(SCDC_TRACE_PREFIX SCDC_LOG_PREFIX __VA_ARGS__)
# define SCDC_TRACE_N(...)                    scdc_log_cout_printf(SCDC_TRACE_PREFIX SCDC_LOG_PREFIX __VA_ARGS__)
# define SCDC_TRACE_C(...)                    scdc_log_cout_printf_nl(__VA_ARGS__)
# define SCDC_TRACE_C_N(...)                  scdc_log_cout_printf(__VA_ARGS__)
# define SCDC_TRACE_DATASET_INPUT(_d_, ...)   Z_MOP(SCDC_TRACE_N(__VA_ARGS__); scdc_dataset_input_log_cout_print(_d_); scdc_log_cout_printf("\n");)
# define SCDC_TRACE_DATASET_OUTPUT(_d_, ...)  Z_MOP(SCDC_TRACE_N(__VA_ARGS__); scdc_dataset_output_log_cout_print(_d_); scdc_log_cout_printf("\n");)
#else
# define SCDC_TRACE(...)                      Z_NOP()
# define SCDC_TRACE_N(...)                    Z_NOP()
# define SCDC_TRACE_C(...)                    Z_NOP()
# define SCDC_TRACE_C_N(...)                  Z_NOP()
# define SCDC_TRACE_DATASET_INPUT(_d_, ...)   Z_NOP()
# define SCDC_TRACE_DATASET_OUTPUT(_d_, ...)  Z_NOP()
#endif

#if HAVE_SCDC_FAIL
# define SCDC_FAIL_PREFIX   "SCDC-FAIL: "
# define SCDC_FAIL(...)  scdc_log_cout_printf_nl(SCDC_FAIL_PREFIX SCDC_LOG_PREFIX __VA_ARGS__)
#else
# define SCDC_FAIL(...)  Z_NOP()
#endif

#if HAVE_SCDC_ERROR
# define SCDC_ERROR_PREFIX   "SCDC-ERROR: "
# define SCDC_ERROR(...)  scdc_log_cerr_printf_nl(SCDC_ERROR_PREFIX SCDC_LOG_PREFIX __VA_ARGS__)
#else
# define SCDC_ERROR(...)  Z_NOP()
#endif

#if HAVE_SCDC_ASSERT
# define SCDC_ASSERT_PREFIX  "SCDC-ASSERT: "
# define SCDC_ASSERT(_t_)  Z_MOP(if (!(_t_)) scdc_log_cerr_printf(SCDC_ASSERT_PREFIX __FILE__ ":%d: '" #_t_ "' failed\n", __LINE__);)
#else
# define SCDC_ASSERT(_t_)  Z_NOP()
#endif

#if HAVE_SCDC_FATAL
# define SCDC_FATAL_PREFIX   "SCDC-FATAL: "
# define SCDC_FATAL(...)  scdc_log_cerr_printf_nl(SCDC_FATAL_PREFIX SCDC_LOG_PREFIX __VA_ARGS__)
#else
# define SCDC_FATAL(...)  Z_NOP()
#endif

#endif /* __LOG_HH__ */


#ifdef __cplusplus
}
#endif


#endif /* __LOG_H__ */
