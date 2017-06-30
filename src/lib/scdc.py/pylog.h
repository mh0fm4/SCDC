/*
 *  Copyright (C) 2014, 2015, 2016, 2017 Michael Hofmann
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


#include <stdio.h>

#include "log.h"


void pyscdc_dataset_inout_log_cout_print(PyObject *d, int str);
void pyscdc_object_log_cout_print(PyObject *o, int str);
void pyscdc_pyerr_log_cout_print(void);


#define HAVE_PYSCDC_INFO    HAVE_SCDC_INFO
#define HAVE_PYSCDC_TRACE   HAVE_SCDC_TRACE
#define HAVE_PYSCDC_FAIL    HAVE_SCDC_FAIL
#define HAVE_PYSCDC_ERROR   HAVE_SCDC_ERROR
#define HAVE_PYSCDC_ASSERT  HAVE_SCDC_ASSERT
#define HAVE_PYSCDC_FATAL   HAVE_SCDC_FATAL
#define HAVE_PYSCDC_DEBUG   HAVE_SCDC_DEBUG


#define HAVE_PYSCDC_TRACE_DATASET  1
#define HAVE_PYSCDC_TRACE_OBJECT   1
#define HAVE_PYSCDC_TRACE_PARSE    1
#define HAVE_PYSCDC_TRACE_CALL     1


#if 0
# define PYSCDC_PYERR_PRINT()  Z_MOP(fprintf(stderr, "PyErr_Print: "); PyErr_Print(); fprintf(stderr, "\n");)
#endif


#if HAVE_PYSCDC_INFO
# define PYSCDC_INFO_PREFIX   "PYSCDC-INFO: "
# define PYSCDC_INFO(_x_...)  scdc_log_cout_printf(PYSCDC_INFO_PREFIX _x_)
#else
# define PYSCDC_INFO(_x_...)  Z_NOP()
#endif

#if HAVE_PYSCDC_TRACE && !PYSCDC_TRACE_NOT
# define PYSCDC_TRACE_PREFIX                          "PYSCDC-TRACE: "
# define PYSCDC_TRACE(_x_...)                         scdc_log_cout_printf_nl(PYSCDC_TRACE_PREFIX _x_)
# define PYSCDC_TRACE_N(_x_...)                       scdc_log_cout_printf(PYSCDC_TRACE_PREFIX _x_)
# if HAVE_PYSCDC_TRACE_DATASET
#  define PYSCDC_TRACE_DATASET                        1
#  define PYSCDC_TRACE_DATASET_INPUT(_d_, _x_...)     Z_MOP(PYSCDC_TRACE_N(_x_); scdc_dataset_input_log_cout_print(_d_); scdc_log_cout_printf("\n");)
#  define PYSCDC_TRACE_DATASET_OUTPUT(_d_, _x_...)    Z_MOP(PYSCDC_TRACE_N(_x_); scdc_dataset_output_log_cout_print(_d_); scdc_log_cout_printf("\n");)
#  define PYSCDC_TRACE_DATASET_PYINPUT(_d_, _x_...)   Z_MOP(PYSCDC_TRACE_N(_x_); pyscdc_dataset_inout_log_cout_print(_d_, 1); scdc_log_cout_printf("\n");)
#  define PYSCDC_TRACE_DATASET_PYOUTPUT(_d_, _x_...)  Z_MOP(PYSCDC_TRACE_N(_x_); pyscdc_dataset_inout_log_cout_print(_d_, 1); scdc_log_cout_printf("\n");)
# else
#  define PYSCDC_TRACE_DATASET                        0
#  define PYSCDC_TRACE_DATASET_INPUT(_d_, _x_...)     Z_NOP()
#  define PYSCDC_TRACE_DATASET_OUTPUT(_d_, _x_...)    Z_NOP()
#  define PYSCDC_TRACE_DATASET_PYINPUT(_d_, _x_...)   Z_NOP()
#  define PYSCDC_TRACE_DATASET_PYOUTPUT(_d_, _x_...)  Z_NOP()
# endif
# if HAVE_PYSCDC_TRACE_OBJECT
#  define PYSCDC_TRACE_OBJECT(_o_, _x_...)            Z_MOP(PYSCDC_TRACE_N(_x_); pyscdc_object_log_cout_print(_o_, 0); scdc_log_cout_printf("\n");)
#  define PYSCDC_TRACE_OBJECT_STR(_o_, _x_...)        Z_MOP(PYSCDC_TRACE_N(_x_); pyscdc_object_log_cout_print(_o_, 1); scdc_log_cout_printf("\n");)
# else
#  define PYSCDC_TRACE_OBJECT(_o_, _x_...)            Z_NOP()
#  define PYSCDC_TRACE_OBJECT_STR(_o_, _x_...)        Z_NOP()
# endif
# if HAVE_PYSCDC_TRACE_PARSE
#  define PYSCDC_TRACE_PARSE(_s_...)                  Z_MOP(PYSCDC_TRACE_N(_s_ "python parse: %s", (pyscdc_parseret)?"OK":"FAILED: "); if (pyscdc_parseret) scdc_log_cout_printf("\n"); else pyscdc_pyerr_log_cout_print();)
# else
#  define PYSCDC_TRACE_PARSE(_s_...)                  Z_NOP()
# endif
# if HAVE_PYSCDC_TRACE_CALL
#  define PYSCDC_TRACE_CALL(_r_, _s_...)              Z_MOP(PYSCDC_TRACE_N(_s_ "python call: %s", (_r_)?"OK":"FAILED: "); if (_r_) scdc_log_cout_printf("\n"); else pyscdc_pyerr_log_cout_print();)
# else
#  define PYSCDC_TRACE_CALL(_r_, _s_...)              Z_NOP()
# endif
#else
# define PYSCDC_TRACE(_x_...)                         Z_NOP()
# define PYSCDC_TRACE_DATASET                         0
# define PYSCDC_TRACE_DATASET_INPUT(_d_, _x_...)      Z_NOP()
# define PYSCDC_TRACE_DATASET_OUTPUT(_d_, _x_...)     Z_NOP()
# define PYSCDC_TRACE_DATASET_PYINPUT(_d_, _x_...)    Z_NOP()
# define PYSCDC_TRACE_DATASET_PYOUTPUT(_d_, _x_...)   Z_NOP()
# define PYSCDC_TRACE_OBJECT(_o_, _x_...)             Z_NOP()
# define PYSCDC_TRACE_OBJECT_STR(_o_, _x_...)         Z_NOP()
# define PYSCDC_TRACE_PARSE(_s_...)                   Z_NOP()
# define PYSCDC_TRACE_CALL(_r_, _s_...)               Z_NOP()
#endif

#define PYSCDC_TRACE_DEF()  int pyscdc_parseret = 1; PyObject *pyscdc_ret = NULL;
#define PYSCDC_TRACE_DEC()  extern int pyscdc_parseret; extern PyObject *pyscdc_ret;

#if HAVE_PYSCDC_FAIL
# define PYSCDC_FAIL_PREFIX     "PYSCDC-FAIL: "
# define PYSCDC_FAIL(_x_...)    scdc_log_cout_printf_nl(PYSCDC_FAIL_PREFIX _x_)
# define PYSCDC_FAIL_N(_x_...)  scdc_log_cout_printf(PYSCDC_FAIL_PREFIX _x_)
#else
# define PYSCDC_FAIL(_x_...)    Z_NOP()
#endif

#if HAVE_PYSCDC_ERROR
# define PYSCDC_ERROR_PREFIX     "PYSCDC-ERROR: "
# define PYSCDC_ERROR(_x_...)    scdc_log_cerr_printf_nl(PYSCDC_ERROR_PREFIX _x_)
# define PYSCDC_ERROR_N(_x_...)  scdc_log_cerr_printf(PYSCDC_ERROR_PREFIX _x_)
#else
# define PYSCDC_ERROR(_x_...)    Z_NOP()
#endif

#if HAVE_PYSCDC_ASSERT
# define PYSCDC_ASSERT_PREFIX  "PYSCDC-ASSERT: "
# define PYSCDC_ASSERT(_t_)    Z_MOP(if (!(_t_)) scdc_log_cerr_printf(PYSCDC_ASSERT_PREFIX " %s:%d: '" #_t_ "' failed\n", __FILE__, (int) __LINE__);)
#else
# define PYSCDC_ASSERT(_t_)    Z_NOP()
#endif

#if HAVE_PYSCDC_FATAL
# define PYSCDC_FATAL_PREFIX     "PYSCDC-FATAL: "
# define PYSCDC_FATAL(_x_...)    scdc_log_cerr_printf_nl(PYSCDC_FATAL_PREFIX _x_)
# define PYSCDC_FATAL_N(_x_...)  scdc_log_cerr_printf(PYSCDC_FATAL_PREFIX _x_)
#else
# define PYSCDC_FATAL(_x_...)    Z_NOP()
#endif


#define PYSCDC_NEXT_IGNORE     ((void *) Py_False)
#define PYSCDC_NEXT_PACK(_n_)  (_n_)
#define PYSCDC_NEXT_UNPACK     ((void *) Py_None)

#define PYSCDC_NEXT_IS_IGNORE(_n_)  ((_n_) == PYSCDC_NEXT_IGNORE)
#define PYSCDC_NEXT_IS_PACK(_n_)    (!PYSCDC_NEXT_IS_IGNORE(_n_) && !PYSCDC_NEXT_IS_UNPACK(_n_))
#define PYSCDC_NEXT_IS_UNPACK(_n_)  ((_n_) == PYSCDC_NEXT_UNPACK)


#endif /* __LOG_H__ */
