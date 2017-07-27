/*
 *  Copyright (C) 2014, 2015, 2016, 2017 Michael Hofmann
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


/** @file scdc_defs.h
 @brief Definitions of interface functions and types.
*/

#ifndef __SCDC_H__
#define __SCDC_H__


#include "scdc_defs.h"


#ifdef __cplusplus
extern "C" {
#endif


extern const scdcint_t SCDC_USE_ZLIB;  /**< @brief Defines whether zlib support is available. */
extern const scdcint_t SCDC_USE_MYSQL; /**< @brief Defines whether MySQL support is available. */
extern const scdcint_t SCDC_USE_MPI;   /**< @brief Defines whether MPI support is available. */


/** @hideinitializer
* @brief Macro with default configuration parameters for #scdc_init. */
#define SCDC_INIT_DEFAULT  ""

/** @brief Function for initializing the SCDC library. Must be called before (almost) all other library function call.
*
* @param conf String of configuration parameters. #SCDC_INIT_DEFAULT can be used as default.
* @param ... Varying number of additional parameters.
* @return Whether the function was successful (#SCDC_SUCCESS) or not (#SCDC_FAILURE).
*/
scdcint_t scdc_init(const char *conf, ...);

/** @brief Function for releasing the SCDC library. Must be called after (almost) all other library function calls.
*/
void scdc_release(void);

/** @brief Function for initializing the logging. Can be called before #scdc_init.
*
* @param conf String of configuration parameters.
* @param ... Varying number of additional parameters.
* @return Whether the function was successful (#SCDC_SUCCESS) or not (#SCDC_FAILURE).
*/
scdcint_t scdc_log_init(const char *conf, ...);

/** @brief Function for releasing the logging. Can be called after #scdc_release.
*/
void scdc_log_release(void);


/** @typedef typedef scdc_dataprov_t
* @brief Definition of the type of a data provider handle.
*
* Along with the type definition, the macro #scdc_dataprov_fmt is defined.
*/
typedef void *scdc_dataprov_t;
/** @hideinitializer
* @brief Macro to define the format specifier for a printf-like output of a data provider handle (#scdc_dataprov_t). */
#define scdc_dataprov_fmt  "p"

/** @hideinitializer
* @brief Macro to define the null value of a data provider handle (#scdc_dataprov_t). */
#define SCDC_DATAPROV_NULL  SCDC_NULL

/** @brief Function to create a new data provider.
*
* @param base_path String specifying the base path for the URI address of the data provider.
* @param conf String of configuration parameters.
* @param ... Varying number of additional parameters.
* @return Handle of the created data provider or #SCDC_DATAPROV_NULL if the function failed.
*/
scdc_dataprov_t scdc_dataprov_open(const char *base_path, const char *conf, ...);

/** @brief Function to the close a data provider.
*
* @param dataprov Handle of the data provider.
*/
void scdc_dataprov_close(scdc_dataprov_t dataprov);


/** @typedef typedef scdc_nodeport_t
* @brief Definition of the type of a node port handle.
*
* Along with the type definition, the macro #scdc_nodeport_fmt is defined.
*/
typedef void *scdc_nodeport_t;
/** @hideinitializer
* @brief Macro to define the format specifier for a printf-like output of a node port handle (#scdc_nodeport_t). */
#define scdc_nodeport_fmt  "p"

/** @hideinitializer
* @brief Macro to define the null value of a node port handle (#scdc_nodeport_t). */
#define SCDC_NODEPORT_NULL  SCDC_NULL

/** @brief Function to create a new node port.
*
* @param conf String of configuration parameters.
* @param ... Varying number of additional parameters.
* @return Handle of the created node port or #SCDC_NODEPORT_NULL if the function failed.
*/
scdc_nodeport_t scdc_nodeport_open(const char *conf, ...);

/** @brief Function to close a node port.
*
* @param nodeport Handle of the node port.
*/
void scdc_nodeport_close(scdc_nodeport_t nodeport);

/** @brief Function to start accepting connections by a node port.
*
* @param nodeport Handle of the node port.
* @param mode Specification of the mode for starting the node port (i.e. see macros SCDC_NODEPORT_START_[@link #SCDC_NODEPORT_START_NONE NONE @endlink, @link #SCDC_NODEPORT_START_LOOP_UNTIL_CANCEL LOOP_UNTIL_CANCEL @endlink, @link #SCDC_NODEPORT_START_LOOP_UNTIL_IDLE LOOP_UNTIL_IDLE @endlink, @link #SCDC_NODEPORT_START_ASYNC_UNTIL_CANCEL ASYNC_UNTIL_CANCEL @endlink, @link #SCDC_NODEPORT_START_ASYNC_UNTIL_IDLE ASYNC_UNTIL_IDLE @endlink]).
* @return Whether the function was successful (#SCDC_SUCCESS) or not (#SCDC_FAILURE).
*/
scdcint_t scdc_nodeport_start(scdc_nodeport_t nodeport, scdcint_t mode);

/** @brief Function to stop accepting connections by a node port.
*
* @param nodeport Handle of the node port.
* @return Whether the function was successful (#SCDC_SUCCESS) or not (#SCDC_FAILURE).
*/
scdcint_t scdc_nodeport_stop(scdc_nodeport_t nodeport);

/** @brief Function to cancel accepting connections by a node port (i.e. if #scdc_nodeport_start is executed blocking).
*
* @param nodeport Handle of the node port.
* @param interrupt Specifies whether the cancelation should be performed "hard" (1) or "soft" (0).
* @return Whether the function was successful (#SCDC_SUCCESS) or not (#SCDC_FAILURE).
*/
scdcint_t scdc_nodeport_cancel(scdc_nodeport_t nodeport, scdcint_t interrupt);

/** @brief Function to construct the authority string of an URI address for a given node port and its properties.
*
* @param conf String of configuration parameters.
* @param ... Varying number of additional parameters.
* @return Pointer to a static string containing authority or #SCDC_NULL if the function failed.
*/
const char *scdc_nodeport_authority(const char *conf, ...);

/** @brief Function to determine whether a node port specified by the scheme of a given URI address is supported.
*
* @param uri String containing the scheme of the URI address. The string can include '%s' placeholders that are replaced with strings given as additional parameters.
* @param ... Varying number of additional parameters.
* @return Whether the node port is supported (#SCDC_SUCCESS) or not (#SCDC_FAILURE).
*/
scdcint_t scdc_nodeport_supported(const char *uri, ...);


/** @typedef typedef scdc_dataset_t
* @brief Definition of the type of a dataset handle.
*
* Along with the type definition, the macro #scdc_dataset_fmt is defined.
*/
typedef struct _scdc_dataset_t *scdc_dataset_t;
/** @hideinitializer
* @brief Macro to define the format specifier for a printf-like output of a dataset handle (#scdc_dataset_t). */
#define scdc_dataset_fmt  "p"

/** @hideinitializer
* @brief Macro to define the null value of a node port handle (#scdc_dataset_t). */
#define SCDC_DATASET_NULL  SCDC_NULL

/** @brief Function to the open a dataset with a given URI address.
*
* @param uri String containing the URI address. The string can include '%s' placeholders that are replaced with strings given as additional parameters. 
* @param ... Varying number of additional parameters.
* @return Handle of the opened dataset or #SCDC_DATASET_NULL if the function failed.
*/
scdc_dataset_t scdc_dataset_open(const char *uri, ...);

/** @brief Function to close a dataset.
*
* @param dataset Handle of the dataset.
*/
void scdc_dataset_close(scdc_dataset_t dataset);

/** @brief Function to execute a command on a dataset.
*
* @param dataset Handle of the dataset. The handle can be #SCDC_DATASET_NULL in which case the parameter @a cmd has to start with an URI address specifying the dataset to be used (i.e. to execute a single command without explicitely opening and closing a dataset).
* @param cmd String containing the command to be executed.
* @param input Input object with data transferred from client to server.
* @param output Output object with data transferred from server to client.
* @param ... Varying number of additional parameters.
* @return Whether the function was successful (#SCDC_SUCCESS) or not (#SCDC_FAILURE).
*/
scdcint_t scdc_dataset_cmd(scdc_dataset_t dataset, const char *cmd, scdc_dataset_input_t *input, scdc_dataset_output_t *output, ...);


/** @brief Auxiliary function to print the content of an input object.
*
* @param input Input object.
*/
void scdc_dataset_input_print(scdc_dataset_input_t *input);

/** @brief Auxiliary function to print the content of an output object.
*
* @param output Output object.
*/
void scdc_dataset_output_print(scdc_dataset_output_t *output);

/** @brief Auxiliary function to reset all fields of an input object to a (null-like) value.
*
* @param input Input object.
*/
void scdc_dataset_input_unset(scdc_dataset_input_t *input);

/** @brief Auxiliary function to reset all fields of an output object to a (null-like) value.
*
* @param output Output object.
*/
void scdc_dataset_output_unset(scdc_dataset_output_t *output);

/** @brief Auxiliary function to create an input object.
*
* @param input Pointer to an input object to be used.
* @param conf String of configuration parameters.
* @param ... Varying number of additional parameters.
* @return Pointer to the created input object (i.e. the same as @a input) or #SCDC_NULL if the function fails.
*/
scdc_dataset_input_t *scdc_dataset_input_create(scdc_dataset_input_t *input, const char *conf, ...);

/** @brief Function to destroy an input object that was created with #scdc_dataset_input_create.
*
* @param input Input object.
*/
void scdc_dataset_input_destroy(scdc_dataset_input_t *input);

/** @brief Auxiliary function to create an output object.
*
* @param output Pointer to an output object to be used.
* @param conf String of configuration parameters.
* @param ... Varying number of additional parameters.
* @return Pointer to the created output object (i.e. the same as @a output) or #SCDC_NULL if the function fails.
*/
scdc_dataset_output_t *scdc_dataset_output_create(scdc_dataset_output_t *output, const char *conf, ...);

/** @brief Function to destroy an input object that was created with #scdc_dataset_output_create.
*
* @param output Output object.
*/
void scdc_dataset_output_destroy(scdc_dataset_output_t *output);

/** @brief Auxiliary function to redirect the data of an input object.
*
* @param input Input object.
* @param conf String of configuration parameters.
* @param ... Varying number of additional parameters.
* @return Whether the function was successful (#SCDC_SUCCESS) or not (#SCDC_FAILURE).
*/
scdcint_t scdc_dataset_input_redirect(scdc_dataset_input_t *input, const char *conf, ...);

/** @brief Auxiliary function to redirect the data of an output object.
*
* @param output Output object.
* @param conf String of configuration parameters.
* @param ... Varying number of additional parameters.
* @return Whether the function was successful (#SCDC_SUCCESS) or not (#SCDC_FAILURE).
*/
scdcint_t scdc_dataset_output_redirect(scdc_dataset_output_t *output, const char *conf, ...);


#ifdef __cplusplus
}
#endif


#endif /* __SCDC_H__ */
