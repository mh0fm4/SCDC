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


#ifndef __REDIRECT_TEST_H__
#define __REDIRECT_TEST_H__


#include "scdc.h"


#define REDIRECT_TEST_SCDC_PATH  "redirect"


void *redirect_test_open(const char *conf, va_list ap);
scdcint_t redirect_test_close(void *dataprov);
/*void *redirect_test_dataset_open(void *dataprov, const char *path);
scdcint_t redirect_test_dataset_close(void *dataprov, void *dataset);*/
scdcint_t redirect_test_dataset_cmd(void *dataprov, void *dataset, const char *cmd, const char *params, scdc_dataset_input_t *input, scdc_dataset_output_t *output, scdc_result_t *result);

const static scdc_dataprov_hook_t redirect_test_hook = {
  redirect_test_open, /* open */
  redirect_test_close, /* close */
  0, /* config */
  0, /* dataset_open */
  0, /* dataset_close */
  0, /* dataset_open_read_state */
  0, /* dataset_close_write_state */
  redirect_test_dataset_cmd, /* dataset_cmd */
};


#define REDIRECT_TEST_PARAM_VECTOR_INPUT   1
#define REDIRECT_TEST_PARAM_VECTOR_OUTPUT  1
#define REDIRECT_TEST_PARAM_VECTOR_INOUT   1


#endif /* __REDIRECT_TEST_H__ */
