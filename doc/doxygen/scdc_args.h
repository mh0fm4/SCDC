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


#ifndef __SCDC_ARGS_H__
#define __SCDC_ARGS_H__


#include "scdc_defs.h"


#ifdef __cplusplus
extern "C" {
#endif


#define SCDC_ARG_REF_NULL  NULL
#define SCDC_ARG_REF_NONE  ((void *) 1)
typedef void *scdc_arg_ref_t;
#define scdc_arg_ref_fmt  "p"
typedef scdc_arg_ref_t scdc_args_get_f(void *data, scdcint_t type, void *v);
typedef scdc_arg_ref_t scdc_args_set_f(void *data, scdcint_t type, void *v, scdc_arg_ref_t ref);
typedef void scdc_args_free_f(void *data, scdcint_t type, void *v, scdc_arg_ref_t ref);

typedef struct _scdc_args_t
{
  void *data;
  scdc_args_get_f *get;
  scdc_args_set_f *set;
  scdc_args_free_f *free;
  
} scdc_args_t;


typedef struct { scdc_handler_f *handler; void *data; } scdc_handler_args_t;

typedef struct { scdc_log_handler_f *handler; void *data; } scdc_log_handler_args_t;

typedef struct { scdc_nodeport_cmd_handler_f *handler; void *data; } scdc_nodeport_cmd_handler_args_t;

typedef struct { scdc_nodeport_timer_handler_f *handler; void *data; } scdc_nodeport_timer_handler_args_t;

typedef struct { scdc_nodeport_loop_handler_f *handler; void *data; } scdc_nodeport_loop_handler_args_t;

typedef struct { scdc_dataprov_jobrun_handler_f *handler; void *data; } scdc_dataprov_jobrun_handler_args_t;


typedef struct
{
  scdcint_t buf_size;
  void *buf;

} scdc_args_buf_t;


typedef struct
{
  scdc_dataprov_hook_t hook;
  void *(*hook_open_intern)(void *intern_data, const char *conf, scdc_args_t *args, scdcint_t *ret);
  void *intern_data;

} scdc_args_dataprov_hook_t;


#define SCDC_ARGS_TYPE_NULL                    0
#define SCDC_ARGS_TYPE_INT                     1
#define SCDC_ARGS_TYPE_SCDCINT                 2
#define SCDC_ARGS_TYPE_DOUBLE                  3
#define SCDC_ARGS_TYPE_CSTR                    4
#define SCDC_ARGS_TYPE_PTR                     5
#define SCDC_ARGS_TYPE_IN_STREAM               6
#define SCDC_ARGS_TYPE_OUT_STREAM              7
#define SCDC_ARGS_TYPE_BUF                     8
#define SCDC_ARGS_TYPE_DATASET_INPUT           9
#define SCDC_ARGS_TYPE_DATASET_OUTPUT          10
#define SCDC_ARGS_TYPE_DATASET_INPUT_PTR       11
#define SCDC_ARGS_TYPE_DATASET_OUTPUT_PTR      12
#define SCDC_ARGS_TYPE_DATASET_INOUT_REDIRECT  13
#define SCDC_ARGS_TYPE_DATASET_INOUT_REDIRECT_PTR  14
#define SCDC_ARGS_TYPE_DATAPROV_HOOK           15
#define SCDC_ARGS_TYPE_DATAPROV_JOBRUN_HANDLER 16
#define SCDC_ARGS_TYPE_LOG_HANDLER             17
#define SCDC_ARGS_TYPE_NODEPORT_CMD_HANDLER    18
#define SCDC_ARGS_TYPE_NODEPORT_TIMER_HANDLER  19
#define SCDC_ARGS_TYPE_NODEPORT_LOOP_HANDLER   20
#define SCDC_ARGS_TYPE_NODEPORT_LOOP_HANDLER_DUMMY  21
#define SCDC_ARGS_TYPE_LAST                    100


#ifdef __cplusplus
}
#endif


#endif /* __SCDC_ARGS_H__ */
