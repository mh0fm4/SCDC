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


#ifndef __SCDC_LOG_H__
#define __SCDC_LOG_H__


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
#ifndef __SCDC_LOG_HH__

#if HAVE_SCDC_INFO
# define SCDC_INFO_PREFIX   "SCDC-INFO: "
# define SCDC_INFO(_x_...)  scdc_cout_printf(SCDC_INFO_PREFIX SCDC_LOG_PREFIX _x_)
#else
# define SCDC_INFO(_x_...)  Z_NOP()
#endif

#if HAVE_SCDC_TRACE && !(SCDC_TRACE_NOT)
# define SCDC_TRACE_PREFIX                       "SCDC-TRACE: "
# define SCDC_TRACE(_x_...)                      scdc_cout_printf_nl(SCDC_TRACE_PREFIX SCDC_LOG_PREFIX _x_)
# define SCDC_TRACE_N(_x_...)                    scdc_cout_printf(SCDC_TRACE_PREFIX SCDC_LOG_PREFIX _x_)
# define SCDC_TRACE_C(_x_...)                    scdc_cout_printf_nl(_x_)
# define SCDC_TRACE_C_N(_x_...)                  scdc_cout_printf(_x_)
# define SCDC_TRACE_DATASET_INPUT(_d_, _x_...)   Z_MOP(SCDC_TRACE_N(_x_); scdc_dataset_input_log_cout_print(_d_); scdc_log_cout_printf("\n");)
# define SCDC_TRACE_DATASET_OUTPUT(_d_, _x_...)  Z_MOP(SCDC_TRACE_N(_x_); scdc_dataset_output_log_cout_print(_d_); scdc_log_cout_printf("\n");)
#else
# define SCDC_TRACE(_x_...)                      Z_NOP()
# define SCDC_TRACE_N(_x_...)                    Z_NOP()
# define SCDC_TRACE_C(_x_...)                    Z_NOP()
# define SCDC_TRACE_C_N(_x_...)                  Z_NOP()
# define SCDC_TRACE_DATASET_INPUT(_d_, _x_...)   Z_NOP()
# define SCDC_TRACE_DATASET_OUTPUT(_d_, _x_...)  Z_NOP()
#endif

#if HAVE_SCDC_FAIL
# define SCDC_FAIL_PREFIX   "SCDC-FAIL: "
# define SCDC_FAIL(_x_...)  scdc_cout_printf_nl(SCDC_FAIL_PREFIX SCDC_LOG_PREFIX _x_)
#else
# define SCDC_FAIL(_x_...)  Z_NOP()
#endif

#if HAVE_SCDC_ERROR
# define SCDC_ERROR_PREFIX   "SCDC-ERROR: "
# define SCDC_ERROR(_x_...)  scdc_cerr_printf_nl(SCDC_ERROR_PREFIX SCDC_LOG_PREFIX _x_)
#else
# define SCDC_ERROR(_x_...)  Z_NOP()
#endif

#if HAVE_SCDC_ASSERT
# define SCDC_ASSERT_PREFIX  "SCDC-ASSERT: "
# define SCDC_ASSERT(_t_)  Z_MOP(if (!(_t_)) scdc_cerr_printf(SCDC_ASSERT_PREFIX __FILE__ ":%d: '" #_t_ "' failed\n", __LINE__);)
#else
# define SCDC_ASSERT(_x_)  Z_NOP()
#endif

#if HAVE_SCDC_FATAL
# define SCDC_FATAL_PREFIX   "SCDC-FATAL: "
# define SCDC_FATAL(_x_...)  scdc_cerr_printf_nl(SCDC_FATAL_PREFIX SCDC_LOG_PREFIX _x_)
#else
# define SCDC_FATAL(_x_...)  Z_NOP()
#endif

#endif /* __SCDC_LOG_HH__ */


#ifdef __cplusplus
}
#endif


#endif /* __SCDC_LOG_H__ */
