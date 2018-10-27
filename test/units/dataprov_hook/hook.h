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


#ifndef __HOOK_H__
#define __HOOK_H__


#ifdef __cplusplus
extern "C" {
#endif


#include "scdc.h"


void *hook_open(const char *conf, va_list ap);
scdcint_t hook_close(void *dataprov);

scdcint_t hook_config(void *dataprov, const char *cmd, const char *param, const char *input, scdcint_t input_size, scdc_result_t *result);

void *hook_dataset_open(void *dataprov, const char *path, scdc_result_t *result);
scdcint_t hook_dataset_close(void *dataprov, void *dataset, scdc_result_t *result);

void *hook_dataset_open_read_state(void *dataprov, const void *state, scdcint_t state_size, scdc_result_t *result);
scdcint_t hook_dataset_close_write_state(void *dataprov, void *dataset, void *state, scdcint_t state_size, scdc_result_t *result);

scdcint_t hook_dataset_cmd(void *dataprov, void *dataset, const char *cmd, const char *params, scdc_dataset_input_t *input, scdc_dataset_output_t *output, scdc_result_t *result);

#define USE_HOOK_CONFIG  1
#define USE_HOOK_STATE   0

const static scdc_dataprov_hook_t hook = {
  hook_open,
  hook_close,
#if USE_HOOK_CONFIG
  hook_config,
#else /* USE_HOOK_CONFIG */
  0,
#endif /* USE_HOOK_CONFIG */
  hook_dataset_open,
  hook_dataset_close,
#if USE_HOOK_STATE
  hook_dataset_open_read_state,
  hook_dataset_close_write_state,
#else /* USE_HOOK_STATE */
  0, 0,
#endif /* USE_HOOK_STATE */
  hook_dataset_cmd,
};


#ifdef __cplusplus
}
#endif


#endif /* __HOOK_H__ */
