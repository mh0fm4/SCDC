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


#ifndef __REPOH_H__
#define __REPOH_H__


#ifdef __cplusplus
extern "C" {
#endif


#include "scdc.h"


void *repoH_open(const char *conf, va_list ap);
scdcint_t repoH_close(void *dataprov);

scdcint_t repoH_config(void *dataprov, const char *cmd, const char *param, const char *input, scdcint_t input_size, char **output, scdcint_t *output_size);

void *repoH_dataset_open(void *dataprov, const char *path);
scdcint_t repoH_dataset_close(void *dataprov, void *dataset);

void *repoH_dataset_open_read_state(void *dataprov, const char *buf, scdcint_t buf_size);
scdcint_t repoH_dataset_close_write_state(void *dataprov, void *dataset, char *buf, scdcint_t buf_size);

scdcint_t repoH_dataset_cmd(void *dataprov, void *dataset, const char *cmd, const char *params, scdc_dataset_input_t *input, scdc_dataset_output_t *output);

#define USE_REPOH_CONFIG  1
#define USE_REPOH_STATE   0

const static scdc_dataprov_hook_t repoH = {
  repoH_open,
  repoH_close,
#if USE_REPOH_CONFIG
  repoH_config,
#else /* USE_REPOH_CONFIG */
  0,
#endif /* USE_REPOH_CONFIG */
  repoH_dataset_open,
  repoH_dataset_close,
#if USE_REPOH_STATE
  repoH_dataset_open_read_state,
  repoH_dataset_close_write_state,
#else /* USE_REPOH_STATE */
  0, 0,
#endif /* USE_REPOH_STATE */
  repoH_dataset_cmd,
};


#ifdef __cplusplus
}
#endif


#endif /* __REPOH_H__ */
