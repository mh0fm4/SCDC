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


/** \file scdc_defs.h
    \brief A Documented file.
    
    Details.
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

/** activate debugging */
#define SCDC_DEBUG  HAVE_SCDC_DEBUG


/* basic constants and types */

/** @typedef typedef long long scdcint_t;
* @brief Definition of the intern used integer type
*/
typedef long long scdcint_t;
#define scdcint_fmt  "lld"
#define scdcint_min  LONG_LONG_MIN
#define scdcint_max  LONG_LONG_MAX

#define SCDCINT_IS_LONG_LONG  1

/** Macro to define SCDC_SUCCESS. Used as return value from scdc functions. */
#define SCDC_SUCCESS  1LL
/** Macro to define SCDC_FAILURE. Used as return value from scdc functions. */
#define SCDC_FAILURE  0LL

/** Macro to define the NULL pointer from scdc */
#ifdef __cplusplus
# define SCDC_NULL  0
#else
# define SCDC_NULL  (void *)0
#endif


/* dataset_input/output */

/** Makro to define the maximum length of the format string in the structure scdc_dataset_inout_t */
#define SCDC_FORMAT_MAX_SIZE  16

/** Makro to set the member "total_size_given" of the structure scdc_dataset_inout_t */ 
#define SCDC_DATASET_INOUT_TOTAL_SIZE_GIVEN_EXACT     'x'
#define SCDC_DATASET_INOUT_TOTAL_SIZE_GIVEN_AT_LEAST  'l'
#define SCDC_DATASET_INOUT_TOTAL_SIZE_GIVEN_AT_MOST   'm'
#define SCDC_DATASET_INOUT_TOTAL_SIZE_GIVEN_NONE      'n'

/** @struct _scdc_dataset_inout_t
* @brief Deklaration of the dataset inout object. Used for the typedef of scdc_dataset_inout_t.
*/
struct _scdc_dataset_inout_t;

/** @typedef scdc_dataset_inout_next_f;
* @brief Definition of the type for function pointer of the next function
*/
typedef scdcint_t scdc_dataset_inout_next_f(struct _scdc_dataset_inout_t *inout);

/** @struct _scdc_dataset_inout_intern_t
* @brief Deklaration of a intern used structure in the dataset inout object.
*/
struct _scdc_dataset_inout_intern_t;

/**@struct scdc_dataset_inout_t
* @brief Structure defines the input object and the output object used by scdc_dataset_cmd()
*        It is used to register the data, which should be transferred between client and scdc service.
*
* @var  format  String describing the data format. Will be transferred and can be set arbitrarily
* @var  buf_size  Size of the data buffer.
* @var  buf  Data buffer - Data in this buffer will be transferred.
* @var  current_size  Size of the currently available data within the data buffer.
* @var  total_size  Currently known total size of the input/output.
* @var  total_size_given  Defines how to interpret the given total_size value, predefined values: SCDC_DATASET_INOUT_TOTAL_SIZE_GIVEN_EXACT, SCDC_DATASET_INOUT_TOTAL_SIZE_GIVEN_AT_LEAST, SCDC_DATASET_INOUT_TOTAL_SIZE_GIVEN_AT_MOST, SCDC_DATASET_INOUT_TOTAL_SIZE_GIVEN_NONE.
* @var  next  Handle to a function to get the next input or to produce more output (It will fill the buffer with more data)
* @var  data  Arbitrary data field (e.g., to store state information). Could be used to deposit more data used later.
* @var  intern  Handle used intern by scdc. (Don't use by your self, but set to predefined value with scdc_dataset_output/input_unset() )
* @var  intern_data  Arbitrary data field for intern use. (Don't use by your self)
*/
typedef struct _scdc_dataset_inout_t
{
  char format[SCDC_FORMAT_MAX_SIZE];

  scdcint_t buf_size;
  void *buf;

  scdcint_t current_size, total_size;
  char total_size_given;

  scdc_dataset_inout_next_f *next;

  void *data;

  struct _scdc_dataset_inout_intern_t *intern;
  void *intern_data;

} scdc_dataset_inout_t;

/** @typedef typedef scdc_dataset_inout_t scdc_dataset_input_t;
* @brief Derivation from scdc_dataset_inout_t to clearly specify the input object.
*/
typedef scdc_dataset_inout_t scdc_dataset_input_t;

/** @typedef typedef scdc_dataset_inout_t scdc_dataset_output_t;
* @brief Derivation from scdc_dataset_inout_t to clearly specify the output object.
*/
typedef scdc_dataset_inout_t scdc_dataset_output_t;


/* dataprov_hook */

/** @fn void *scdc_dataprov_hook_open_f(const char *conf, va_list ap)
* @typedef typedef void *scdc_dataprov_hook_open_f(const char *conf, va_list ap);
*
* @brief Function descriptor for data provider hook open function. (Call back function)
*        Called when SCDC server opens a data provider.
*
* @param  conf  Configuration string, the string of the first argument by calling scdc_nodeport_open()
* @param  ap    A list with arbitary arguments. Includes arbitary arguments from scdc_nodeport_open()
* @return Handle of the new data provider
*/
typedef void *scdc_dataprov_hook_open_f(const char *conf, va_list ap);

/** @fn scdcint_t scdc_dataprov_hook_close_f(void *dataprov)
* @typedef typedef scdcint_t scdc_dataprov_hook_close_f(void *dataprov);
*
* @brief Function descriptor for data provider hook close function. (Call back function)
*        Called when SCDC server closes a data provider.
*
* @param  dataprov  Handle of the data provider
* @return SCDC_SUCCESS/SCDC_FAILURE
*/
typedef scdcint_t scdc_dataprov_hook_close_f(void *dataprov);


/** @fn scdcint_t scdc_dataprov_hook_config_f(void *dataprov, const char *cmd, const char *param, const char *val, scdcint_t val_size, char **result, scdcint_t *result_size)
* @typedef typedef scdcint_t scdc_dataprov_hook_config_f(void *dataprov, const char *cmd, const char *param, const char *val, scdcint_t val_size, char **result, scdcint_t *result_size);
*
* @brief Function descriptor for data provider hook config function. (Call back function)
*        Called when a configuration interface of the SCDC server is used.
*
* @param  dataprov  Handle of the data provider
* @param  cmd  String containing the configuration command
* @param  param String containing the additional paraneter of the command
* @param  val  specify the configuration value
* @param  val_size  size of the string val
* @param  result  string for returning the result of the command
* @param  result_size  the lenght of result
* @return SCDC_SUCCESS/SCDC_FAILURE
*/
typedef scdcint_t scdc_dataprov_hook_config_f(void *dataprov, const char *cmd, const char *param, const char *val, scdcint_t val_size, char **result, scdcint_t *result_size);


/** @fn void *scdc_dataprov_hook_dataset_open_f(void *dataprov, const char *path)
* @typedef typedef void *scdc_dataprov_hook_dataset_open_f(void *dataprov, const char *path);
*
* @brief Function descriptor for data set hook open function. (Call back function)
*        Called when client opens a dataset on the data provider.
*
* @param  dataprov  Handle of the data provider
* @param  path  the last part of the URI describing the data set
* @return Handle of the new data set
*/
typedef void *scdc_dataprov_hook_dataset_open_f(void *dataprov, const char *path);

/** @fn scdcint_t scdc_dataprov_hook_dataset_close_f(void *dataprov, void *dataset)
* @typedef typedef scdcint_t scdc_dataprov_hook_dataset_close_f(void *dataprov, void *dataset);
*
* @brief Function descriptor for data set hook close function of an data provider. (Call back function)
*        Called on SCDC server when client closes a data set.
*
* @param  dataprov  Handle of the data provider
* @param  dataset  Handle of the data set
* @return SCDC_SUCCESS/SCDC_FAILURE
*/
typedef scdcint_t scdc_dataprov_hook_dataset_close_f(void *dataprov, void *dataset);


/** @fn void *scdc_dataprov_hook_dataset_open_read_state_f(void *dataprov, const char *buf, scdcint_t buf_size)
* @typedef typedef void *scdc_dataprov_hook_dataset_open_read_state_f(void *dataprov, const char *buf, scdcint_t buf_size);
*
* @brief Function descriptor for data set hook open read state function. (Call back function)
*
* @param  dataprov  Handle of the data provider
* @param  buf  Pointer to a buffer to store a string with the state
* @param  buf_size  buffer size
* @return ??
*/
typedef void *scdc_dataprov_hook_dataset_open_read_state_f(void *dataprov, const char *buf, scdcint_t buf_size);

/** @fn scdcint_t scdc_dataprov_hook_dataset_close_write_state_f(void *dataprov, void *dataset, char *buf, scdcint_t buf_size)
* @typedef typedef scdcint_t scdc_dataprov_hook_dataset_close_write_state_f(void *dataprov, void *dataset, char *buf, scdcint_t buf_size);
*
* @brief Function descriptor for data set hook close write state function. (Call back function)
*
* @param  dataprov  Handle of the data provider
* @param  dataset  Handle of the data set
* @param  buf  Pointer to a string buffer with the state 
* @param  buf_size  buffer size
* @return SCDC_SUCCESS/SCDC_FAILURE
*/
typedef scdcint_t scdc_dataprov_hook_dataset_close_write_state_f(void *dataprov, void *dataset, char *buf, scdcint_t buf_size);

/** @fn scdcint_t scdc_dataprov_hook_dataset_cmd_f(void *dataprov, void *dataset, const char *cmd, const char *params, scdc_dataset_input_t *input, scdc_dataset_output_t *output)
* @typedef typedef scdcint_t scdc_dataprov_hook_dataset_cmd_f(void *dataprov, void *dataset, const char *cmd, const char *params, scdc_dataset_input_t *input, scdc_dataset_output_t *output);
*
* @brief Function descriptor for data set hook cmd function. (Call back function)
*        Corresponding function to scdc_dataset_cmd() on client.
*        Called on SCDC server when client execute scdc_dataset_cmd()
*
* @param  dataprov  Handle of the data provider
* @param  dataset  Handle of the data set
* @param  cmd  String contains the command
* @param  params  String with additional and partly arbitrary values 
* @param  input  input object provides the data transferred from client to server
* @param  output  output object, for transfer data back to client
* @return SCDC_SUCCESS/SCDC_FAILURE
*/
typedef scdcint_t scdc_dataprov_hook_dataset_cmd_f(void *dataprov, void *dataset, const char *cmd, const char *params, scdc_dataset_input_t *input, scdc_dataset_output_t *output);

/**@struct scdc_dataprov_hook_t
* @brief Struktur to register the callback functions to the data provider.
*        To declare the functions to the data provider a variable of this type is defined and then 
*        given as first arbitary argument to the function scdc_dataprov_open() when conf includes "hook"
*        
*
* @var  open  Handle to register a hook open function for data provider.
* @var  close  Handle to register a hook close function for data provider.
* @var  config  Handle to register a hook config function for data provider.
* @var  dataset_open  Handle to register a hook dataset open function for data provider.
* @var  dataset_close  Handle to register a hook dataset close function for data provider.
* @var  dataset_open_read_state  Handle to register a hook dataset open read state function for data provider.
* @var  dataset_close_write_state  Handle to register a hook close write state function for data provider.
* @var  dataset_cmd  Handle to register a hook dataset command function for data provider.
*/
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

/** Makro to define the access port behavior while running*/
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
