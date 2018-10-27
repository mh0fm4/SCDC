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


#ifndef __FCS_SCDC_H__
#define __FCS_SCDC_H__


#include "scdc.h"


void *fcs_scdc_open(const char *conf, va_list ap);
scdcint_t fcs_scdc_close(void *dataprov);
/*void *fcs_scdc_dataset_open(void *dataprov, const char *path);
scdcint_t fcs_scdc_dataset_close(void *dataprov, void *dataset);*/
scdcint_t fcs_scdc_dataset_cmd(void *dataprov, void *dataset, const char *cmd, const char *params, scdc_dataset_input_t *input, scdc_dataset_output_t *output, scdc_result_t *result);

const static scdc_dataprov_hook_t fcs_scdc_hook = {
  fcs_scdc_open, /* open */
  fcs_scdc_close, /* close */
  0, /* config */
  0, /* dataset_open */
  0, /* dataset_close */
  0, /* dataset_open_read_state */
  0, /* dataset_close_write_state */
  fcs_scdc_dataset_cmd, /* dataset_cmd */
};


#endif /* __FCS_SCDC_H__ */
