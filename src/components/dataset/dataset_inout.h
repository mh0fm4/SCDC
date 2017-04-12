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


#ifndef __DATASET_INOUT_H__
#define __DATASET_INOUT_H__


#ifdef __cplusplus
extern "C" {
#endif


#include "z_pack.h"
#include "scdc_defs.h"


typedef void scdc_dataset_inout_destroy_f(scdc_dataset_inout_t *inout);

typedef struct _scdc_dataset_inout_intern_t
{
  scdcint_t alloc_size, type;
  void *buf, *data;

  scdc_dataset_inout_destroy_f *destroy;

} scdc_dataset_inout_intern_t;

extern const scdc_dataset_inout_intern_t scdc_dataset_inout_intern_null;
#define SCDC_DATASET_INPUT_INTERN_NULL   scdc_dataset_inout_intern_null
#define SCDC_DATASET_OUTPUT_INTERN_NULL  scdc_dataset_inout_intern_null


#define SCDC_DATASET_OUTPUT_STR(_d_)                    ((_d_)?((SCDC_DATASET_INOUT_BUF_PTR(_d_))?std::string(static_cast<char *>(SCDC_DATASET_INOUT_BUF_PTR(_d_)), SCDC_DATASET_INOUT_BUF_CURRENT(_d_)):""):"<null>")
#define SCDC_DATASET_OUTPUT_CLEAR(_d_)                  Z_MOP(\
  if (_d_) { \
    SCDC_DATASET_INOUT_BUF_CURRENT(_d_) = 0; \
    (_d_)->total_size = 0; \
    (_d_)->total_size_given = SCDC_DATASET_INOUT_TOTAL_SIZE_GIVEN_NONE; \
    (_d_)->data = 0; \
    (_d_)->next = 0; \
  } )
#define SCDC_DATASET_OUTPUT_PRINTF(_d_, _s_...)         scdc_dataset_output_printf(0, _d_, _s_)
#define SCDC_DATASET_OUTPUT_PRINTF_APPEND(_d_, _s_...)  scdc_dataset_output_printf(1, _d_, _s_)
void scdc_dataset_output_printf(int append, scdc_dataset_output_t *output, const char *fmt, ...);


void scdc_dataset_input_log_cout_print(scdc_dataset_input_t *input);
void scdc_dataset_output_log_cout_print(scdc_dataset_output_t *output);

void scdc_dataset_input_print(scdc_dataset_input_t *input);
void scdc_dataset_output_print(scdc_dataset_output_t *output);

extern const scdc_dataset_inout_t scdc_dataset_inout_none;
#define SCDC_DATASET_INPUT_NONE   (&scdc_dataset_inout_none)
#define SCDC_DATASET_OUTPUT_NONE  (&scdc_dataset_inout_none)
extern const scdc_dataset_inout_t scdc_dataset_inout_endl;
#define SCDC_DATASET_INPUT_ENDL   (&scdc_dataset_inout_endl)
#define SCDC_DATASET_OUTPUT_ENDL  (&scdc_dataset_inout_endl)

void scdc_dataset_inout_unset(scdc_dataset_inout_t *inout);
void scdc_dataset_input_unset(scdc_dataset_input_t *input);
void scdc_dataset_output_unset(scdc_dataset_output_t *output);

scdc_dataset_input_t *scdc_dataset_input_create_intern(scdc_dataset_input_t *input, const char *conf, scdc_args_t *args);
scdc_dataset_input_t *scdc_dataset_input_create(scdc_dataset_input_t *input, const char *conf, ...);
void scdc_dataset_input_destroy(scdc_dataset_input_t *input);

scdc_dataset_output_t *scdc_dataset_output_create_intern(scdc_dataset_output_t *output, const char *conf, scdc_args_t *args);
scdc_dataset_output_t *scdc_dataset_output_create(scdc_dataset_output_t *output, const char *conf, ...);
void scdc_dataset_output_destroy(scdc_dataset_output_t *output);

scdcint_t scdc_dataset_input_redirect_intern(scdc_dataset_input_t *input, const char *conf, scdc_args_t *args);
scdcint_t scdc_dataset_input_redirect(scdc_dataset_input_t *input, const char *conf, ...);

scdcint_t scdc_dataset_output_redirect_intern(scdc_dataset_output_t *output, const char *conf, scdc_args_t *args);
scdcint_t scdc_dataset_output_redirect(scdc_dataset_output_t *output, const char *conf, ...);


#ifdef __cplusplus
}
#endif


#endif /* __DATASET_INOUT_H__ */
