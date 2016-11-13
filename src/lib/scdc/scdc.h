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


#ifndef __SCDC_H__
#define __SCDC_H__


#include "scdc_defs.h"


#ifdef __cplusplus
extern "C" {
#endif


#define SCDC_INIT_DEFAULT  ""

scdcint_t scdc_init(const char *conf, ...);
void scdc_release(void);

scdcint_t scdc_log_init(const char *conf, ...);
void scdc_log_release(void);


/* dataprov */
typedef void *scdc_dataprov_t;
#define scdc_dataprov_fmt  "p"

#define SCDC_DATAPROV_NULL  SCDC_NULL

scdc_dataprov_t scdc_dataprov_open(const char *base_path, const char *conf, ...);
void scdc_dataprov_close(scdc_dataprov_t dataprov);

/* nodeport */
typedef void *scdc_nodeport_t;
#define scdc_nodeport_fmt  "p"

#define SCDC_NODEPORT_NULL  SCDC_NULL

scdc_nodeport_t scdc_nodeport_open(const char *conf, ...);
void scdc_nodeport_close(scdc_nodeport_t nodeport);
scdcint_t scdc_nodeport_start(scdc_nodeport_t nodeport, scdcint_t mode);
scdcint_t scdc_nodeport_stop(scdc_nodeport_t nodeport);
scdcint_t scdc_nodeport_cancel(scdc_nodeport_t nodeport, scdcint_t interrupt);

const char *scdc_nodeport_authority(const char *conf, ...);
scdcint_t scdc_nodeport_supported(const char *uri, ...);

/* dataset */
typedef struct _scdc_dataset_t *scdc_dataset_t;
#define scdc_dataset_fmt  "p"

#define SCDC_DATASET_NULL  SCDC_NULL

scdc_dataset_t scdc_dataset_open(const char *uri, ...);
void scdc_dataset_close(scdc_dataset_t dataset);
scdcint_t scdc_dataset_cmd(scdc_dataset_t dataset, const char *cmd, scdc_dataset_input_t *input, scdc_dataset_output_t *output, ...);


/* dataset input/output */
void scdc_dataset_input_print(scdc_dataset_input_t *input);
void scdc_dataset_output_print(scdc_dataset_output_t *output);

void scdc_dataset_input_unset(scdc_dataset_input_t *input);
void scdc_dataset_output_unset(scdc_dataset_output_t *output);

scdc_dataset_input_t *scdc_dataset_input_create(scdc_dataset_input_t *input, const char *conf, ...);
void scdc_dataset_input_destroy(scdc_dataset_input_t *input);

scdc_dataset_output_t *scdc_dataset_output_create(scdc_dataset_output_t *output, const char *conf, ...);
void scdc_dataset_output_destroy(scdc_dataset_output_t *output);

scdcint_t scdc_dataset_input_redirect(scdc_dataset_input_t *input, const char *conf, ...);
scdcint_t scdc_dataset_output_redirect(scdc_dataset_output_t *output, const char *conf, ...);


#ifdef __cplusplus
}
#endif


#endif /* __SCDC_H__ */
