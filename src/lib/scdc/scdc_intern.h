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


#ifndef __SCDC_INTERN_H__
#define __SCDC_INTERN_H__


#include "scdc_defs.h"
#include "scdc_args.h"


#ifdef __cplusplus
extern "C" {
#endif


void scdc_main_log_init(const char *conf, ...);

scdcint_t scdc_init_intern(const char *conf, scdc_args_t *args);
void scdc_release_intern(void);

scdcint_t scdc_log_init_intern(const char *conf, scdc_args_t *args);
void scdc_log_release_intern(void);

scdc_dataprov_t scdc_dataprov_open_intern(const char *base_path, const char *conf, scdc_args_t *args);

scdc_nodeport_t scdc_nodeport_open_intern(const char *conf, scdc_args_t *args);

const char *scdc_nodeport_authority_intern(const char *conf, scdc_args_t *args);

scdcint_t scdc_nodeport_supported_intern(const char *uri, scdc_args_t *args);

scdc_dataset_t scdc_dataset_open_intern(const char *uri, scdc_args_t *args);

scdcint_t scdc_dataset_cmd_intern(scdc_dataset_t dataset, const char *cmd, scdc_dataset_input_t *input, scdc_dataset_output_t *output, scdc_args_t *args);

scdc_dataset_input_t *scdc_dataset_input_create_intern(scdc_dataset_input_t *input, const char *conf, scdc_args_t *args);

scdc_dataset_output_t *scdc_dataset_output_create_intern(scdc_dataset_output_t *output, const char *conf, scdc_args_t *args);

scdcint_t scdc_dataset_input_redirect_intern(scdc_dataset_input_t *input, const char *conf, scdc_args_t *args);

scdcint_t scdc_dataset_output_redirect_intern(scdc_dataset_output_t *output, const char *conf, scdc_args_t *args);


#ifdef __cplusplus
}
#endif


#endif /* __SCDC_INTERN_H__ */
