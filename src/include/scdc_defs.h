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


#ifndef __SCDC_DEFS_H__
#define __SCDC_DEFS_H__


#ifdef __cplusplus
# include <climits>
# include <cstdarg>
#else
# include <limits.h>
# include <stdarg.h>
#endif


#ifdef __cplusplus
extern "C" {
#endif


#define SCDC_DEPRECATED  HAVE_SCDC_DEPRECATED

#define SCDC_DEBUG  HAVE_SCDC_DEBUG


/* fallback definition, see "6.47 Function Names as Strings" in gcc-4.9 doc */
#if __STDC_VERSION__ < 199901L && !defined(__func__)
# if __GNUC__ >= 2
#  define __func__ __FUNCTION__
# else
#  define __func__ "<unknown>"
# endif
#endif


/* basic constants and types */

typedef long long scdcint_t;
#define scdcint_fmt  "lld"
#define scdcint_min  LONG_LONG_MIN
#define scdcint_max  LONG_LONG_MAX

#define SCDCINT_IS_LONG_LONG  1

#define SCDC_SUCCESS  1LL
#define SCDC_FAILURE  0LL

#ifdef __cplusplus
# define SCDC_NULL  0
#else
# define SCDC_NULL  (void *)0
#endif


/* dataset_input/output */

#define SCDC_FORMAT_MAX_SIZE  16

#define SCDC_DATASET_INOUT_BUF_MULTIPLE  0

#define SCDC_DATASET_INOUT_TOTAL_SIZE_GIVEN_EXACT     'x'
#define SCDC_DATASET_INOUT_TOTAL_SIZE_GIVEN_AT_LEAST  'l'
#define SCDC_DATASET_INOUT_TOTAL_SIZE_GIVEN_AT_MOST   'm'
#define SCDC_DATASET_INOUT_TOTAL_SIZE_GIVEN_NONE      'n'

typedef struct _scdc_buf_t
{
  void *ptr;
  scdcint_t size, current;

} scdc_buf_t;

struct _scdc_dataset_inout_t;

typedef scdcint_t scdc_dataset_inout_next_f(struct _scdc_dataset_inout_t *inout);

struct _scdc_dataset_inout_intern_t;

typedef struct _scdc_dataset_inout_t
{
  char format[SCDC_FORMAT_MAX_SIZE];

#if !SCDC_DEPRECATED
  scdc_buf_t buf;
#else /* !SCDC_DEPRECATED */
  void *buf;
  scdcint_t buf_size;
  scdcint_t current_size;
#endif /* !SCDC_DEPRECATED */

  scdcint_t total_size;
  char total_size_given;

  scdc_dataset_inout_next_f *next;

  void *data;

  struct _scdc_dataset_inout_intern_t *intern;
  void *intern_data;

} scdc_dataset_inout_t;

#if !SCDC_DEPRECATED
# define SCDC_DATASET_INOUT_BUF_PTR(_inout_)      (_inout_)->buf.ptr
# define SCDC_DATASET_INOUT_BUF_SIZE(_inout_)     (_inout_)->buf.size
# define SCDC_DATASET_INOUT_BUF_CURRENT(_inout_)  (_inout_)->buf.current
#else /* !SCDC_DEPRECATED */
# define SCDC_DATASET_INOUT_BUF_PTR(_inout_)      (_inout_)->buf
# define SCDC_DATASET_INOUT_BUF_SIZE(_inout_)     (_inout_)->buf_size
# define SCDC_DATASET_INOUT_BUF_CURRENT(_inout_)  (_inout_)->current_size
#endif /* !SCDC_DEPRECATED */
#define SCDC_DATASET_INOUT_BUF_SET_C(_inout_, _c_)  (SCDC_DATASET_INOUT_BUF_CURRENT(_inout_) = (_c_))
#define SCDC_DATASET_INOUT_BUF_GET_C(_inout_)       (SCDC_DATASET_INOUT_BUF_CURRENT(_inout_) > 0)?SCDC_DATASET_INOUT_BUF_CURRENT(_inout_):0)

#if SCDC_DATASET_INOUT_BUF_MULTIPLE
# define SCDC_DATASET_INOUT_BUF_M_PTR(_inout_, _m_)      ((scdc_buf_t *) SCDC_DATASET_INOUT_BUF_PTR(_inout_))[_m_].ptr
# define SCDC_DATASET_INOUT_BUF_M_SIZE(_inout_, _m_)     ((scdc_buf_t *) SCDC_DATASET_INOUT_BUF_PTR(_inout_))[_m_].size
# define SCDC_DATASET_INOUT_BUF_M_CURRENT(_inout_, _m_)  ((scdc_buf_t *) SCDC_DATASET_INOUT_BUF_PTR(_inout_))[_m_].current
# define SCDC_DATASET_INOUT_BUF_M_SET_C(_inout_, _c_)    (SCDC_DATASET_INOUT_BUF_CURRENT(_inout_) = -(_c_))
# define SCDC_DATASET_INOUT_BUF_M_GET_C(_inout_)         (SCDC_DATASET_INOUT_BUF_CURRENT(_inout_) < 0)?-SCDC_DATASET_INOUT_BUF_CURRENT(_inout_):0)
#endif /* SCDC_DATASET_INOUT_BUF_MULTIPLE */

typedef scdc_dataset_inout_t scdc_dataset_input_t;
typedef scdc_dataset_inout_t scdc_dataset_output_t;


/* dataprov_hook */

typedef void *scdc_dataprov_hook_open_f(const char *conf, va_list ap);
typedef scdcint_t scdc_dataprov_hook_close_f(void *dataprov);

typedef scdcint_t scdc_dataprov_hook_config_f(void *dataprov, const char *cmd, const char *param, const char *val, scdcint_t val_size, char **result, scdcint_t *result_size);

typedef void *scdc_dataprov_hook_dataset_open_f(void *dataprov, const char *path);
typedef scdcint_t scdc_dataprov_hook_dataset_close_f(void *dataprov, void *dataset);

typedef void *scdc_dataprov_hook_dataset_open_read_state_f(void *dataprov, const char *buf, scdcint_t buf_size);
typedef scdcint_t scdc_dataprov_hook_dataset_close_write_state_f(void *dataprov, void *dataset, char *buf, scdcint_t buf_size);

typedef scdcint_t scdc_dataprov_hook_dataset_cmd_f(void *dataprov, void *dataset, const char *cmd, const char *params, scdc_dataset_input_t *input, scdc_dataset_output_t *output);

typedef struct _scdc_dataprov_hook_t
{
  scdc_dataprov_hook_open_f *open;
  scdc_dataprov_hook_close_f *close;
  scdc_dataprov_hook_config_f *config;

  scdc_dataprov_hook_dataset_open_f *dataset_open;
  scdc_dataprov_hook_dataset_close_f *dataset_close;

  scdc_dataprov_hook_dataset_open_read_state_f *dataset_open_read_state;
  scdc_dataprov_hook_dataset_close_write_state_f *dataset_close_write_state;

  scdc_dataprov_hook_dataset_cmd_f *dataset_cmd;

} scdc_dataprov_hook_t;


/* misc. constants and types */

#define SCDC_NODEPORT_START_NONE                0x0
#define SCDC_NODEPORT_START_LOOP_UNTIL_CANCEL   0x1
#define SCDC_NODEPORT_START_LOOP_UNTIL_IDLE     0x2
#define SCDC_NODEPORT_START_ASYNC_UNTIL_CANCEL  0x4
#define SCDC_NODEPORT_START_ASYNC_UNTIL_IDLE    0x8


typedef scdcint_t scdc_handler_f(void *data);

typedef scdcint_t scdc_log_handler_f(void *data, const char *buf, scdcint_t buf_size);

typedef scdcint_t scdc_nodeport_cmd_handler_f(void *data, const char *cmd, const char *params, scdcint_t params_size);

typedef scdcint_t scdc_nodeport_timer_handler_f(void *data);

typedef scdcint_t scdc_nodeport_loop_handler_f(void *data, scdcint_t l);

typedef scdcint_t scdc_dataprov_jobrun_handler_f(void *data, const char *jobid, const char *cmd, const char *params, scdc_dataset_input_t *input, scdc_dataset_output_t *output);


#ifdef __cplusplus
}
#endif


#endif /* __SCDC_DEFS_H__ */
