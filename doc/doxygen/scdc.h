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

/** Macro for standartconfiguration */
#define SCDC_INIT_DEFAULT  ""

/** @fn scdcint_t scdc_init(const char *conf, ...)
* @brief  Initialize the SCDC library using configuration string conf. Usually using SCDC_INIT_DEFAULT is sufficient in most use cases.
*
* @param  conf Configuration for start the scdc.
* @param  ... Additional parameters, but nothing specified jet.
* @return Response if the execution of the function was successful. Values: SCDC_SUCCESS/SCDC_FAILURE 
*/
scdcint_t scdc_init(const char *conf, ...);

/** @fn void scdc_release(void)
* @brief Releases the scdc context and library. Must be called at the end of the program.
*/
void scdc_release(void);

/** @fn scdcint_t scdc_log_init(const char *conf, ...)
* @brief Initializes the logging for scdc.
*
* @param conf Configuration for logging
* @return Response if the execution of the function was successful. Values: SCDC_SUCCESS/SCDC_FAILURE 
*/
scdcint_t scdc_log_init(const char *conf, ...);

/** @fn void scdc_log_release(void)
* @brief Releases the context for logging scdc and stops the logging.
*/
void scdc_log_release(void);


/* dataprov */
/*==========*/

/** @typedef typedef void *scdc_dataprov_t;
* @brief Type to declare Variables containing a handle for dataprovider
*/
typedef void *scdc_dataprov_t;
#define scdc_dataprov_fmt  "p"

/** Macro if no dataprovider is used */
#define SCDC_DATAPROV_NULL  SCDC_NULL

/** @fn scdc_dataprov_t scdc_dataprov_open(const char *base_path, const char *conf, ...)
* @brief SCDC Service: Server function for opening a new dataprovider with URI-path path and configuration conf.
*
* @param  base_path URI-path for dataprovider
* @param  conf      configuration for dataprovider (possible: fs, gen, mysql, hook)
* @param  ...       more configuration parameters if needed, for example a hookvariable with the hookfunktion registered  
* @return A handle to the new data provider or SCDC_DATAPROV_NULL if open it fails.
*/
scdc_dataprov_t scdc_dataprov_open(const char *base_path, const char *conf, ...);

/** @fn void scdc_dataprov_close(scdc_dataprov_t dataprov)
* @brief Closes the data provider with handle dataprov
*
* @param  dataprov  Handle of the data provider, which should be closed
*/
void scdc_dataprov_close(scdc_dataprov_t dataprov);


/* nodeport */
/*==========*/

/** @typedef typedef void *scdc_nodeport_t;
* @brief Type to declare Variables containing a handle for access ports (nodeports)
*/
typedef void *scdc_nodeport_t;
#define scdc_nodeport_fmt  "p"

/** Macro if no nodeport is used */
#define SCDC_NODEPORT_NULL  SCDC_NULL

/** @fn scdc_nodeport_t scdc_nodeport_open(const char *conf, ...)
* @brief Open an access port on the local node.
*
* @param  conf  string to define the transfermethode used by the SCDC service
* @param  ...   additional parameter eventually needed (for example number of max connections, etc..)
* @return A handle to the new access port or SCDC_NODEPORT_NULL if open it fails.
*/
scdc_nodeport_t scdc_nodeport_open(const char *conf, ...);

/** @fn void scdc_nodeport_close(scdc_nodeport_t nodeport)
* @brief close the access port on the local node with handle nodeport
*
* @param  nodeport  Handle of the access port to close
*/
void scdc_nodeport_close(scdc_nodeport_t nodeport);

/** @fn scdcint_t scdc_nodeport_start(scdc_nodeport_t nodeport, scdcint_t mode)
* @brief Start the access port nodeport. The mode specifies whether the function blocks in a loop until canceled or is started asynchronously. Let run the SCDC-Server.
*
* @param  nodeport  Handle of the access port.
* @return Response if the execution of the function was successful. Values: SCDC_SUCCESS/SCDC_FAILURE
*/
scdcint_t scdc_nodeport_start(scdc_nodeport_t nodeport, scdcint_t mode);

/** @fn scdcint_t scdc_nodeport_stop(scdc_nodeport_t nodeport)
* @brief Stop the access port and close the SCDC server. 
*
* @param  nodeport  Handel of acccess port
* @return Response if the execution of the function was successful. Values: SCDC_SUCCESS/SCDC_FAILURE
*/
scdcint_t scdc_nodeport_stop(scdc_nodeport_t nodeport);

/** @fn scdcint_t scdc_nodeport_cancel(scdc_nodeport_t nodeport, scdcint_t interrupt)
* @brief Cancel the access port nodeport if it was started with mode = SCDC_NODEPORT_START_LOOP_UNTIL_CANCEL, interrupt 'hard' if interrupt = 1 
*
* @param  nodeport  Handle of the access port.
* @param  interrupt Tells the function if it is a hard interrupt ("hard"=1)
* @return Response if the execution of the function was successful. Values: SCDC_SUCCESS/SCDC_FAILURE
*/
scdcint_t scdc_nodeport_cancel(scdc_nodeport_t nodeport, scdcint_t interrupt);

/** @fn const char *scdc_nodeport_authority(const char *conf, ...)
* @brief Return an authority string for the access port specified with configuration string conf 
*        and the subsequent arguments (see scdc_nodeport_open)
*
* @param  conf  Configuration for the access port.
* @param  ...   Additional Arguments if needed
* @return C string containing the authority
*/
const char *scdc_nodeport_authority(const char *conf, ...);

/** @fn scdcint_t scdc_nodeport_supported(const char *uri, ...)
* @brief  ??
*
* @param  uri  uri of the access port
* @param  ...  Additional Arguments
* @return Response if the access port with uri is supported Values: SCDC_SUCCESS/SCDC_FAILURE
*/
scdcint_t scdc_nodeport_supported(const char *uri, ...);

/* dataset */
/*=========*/

/** @typedef typedef struct _scdc_dataset_t *scdc_dataset_t;
* @brief Type to declare Variables containing a handle for datasets
*/
typedef struct _scdc_dataset_t *scdc_dataset_t;
#define scdc_dataset_fmt  "p"

/** Macro if no dataset is used */
#define SCDC_DATASET_NULL  SCDC_NULL

/** @fn scdc_dataset_t scdc_dataset_open(const char *uri, ...)
* @brief Open a data set with URI uri. If the authority part of the URI is '%s', then the authority is read as string from an optional argument
*
* @param  uri  The URI adress of the the data set
* @param  ...  If uri contains '%s', then the additional string argument is included in the place of '%s'. It is possible to give other additional arguments to the function, but they are not used now, if the do not correspond to a '%s'.
* @return  A handle to the new opened dataset or SCDC_DATASET_NULL if it fails.
*/
scdc_dataset_t scdc_dataset_open(const char *uri, ...);

/** @fn void scdc_dataset_close(scdc_dataset_t dataset)
* @brief Close the data set with the handle dataset
*
* @param Handle of the data set to close
*/
void scdc_dataset_close(scdc_dataset_t dataset);

/** @fn scdcint_t scdc_dataset_cmd(scdc_dataset_t dataset, const char *cmd, scdc_dataset_input_t *input, scdc_dataset_output_t *output, ...)
* @brief Execute a command cmd on the data set with the handle dataset. Input and output are given through the parameters input and output. 
*        All the data is automaticly sended to the dataprovider. If handle dataset is SCDC_DATASET_NULL, then the first part of 
*        cmd (i.e. up to the first space) is used as URI for the dataset on which the remaining part of cmd is executed as command, 
*        the authority of this URI can be specified with an optional argument (see scdc_dataset_open)
*        Example: Bsp: scdc_dataset_cmd(SCDC_DATASET_NULL, "scdc:/hookdemo CMD PARAM1 PARAM2 PARAM3", &input, &output);
*
* @param  dataset  Handle of the data set
* @param  cmd      A string with the command wich should be executed on the data set.
* @param  input    The input data for the command
* @param  output   The output data the command generates
* @param  ...      additional parameters
* @return   Response if the execution of the function was successful. Values: SCDC_SUCCESS/SCDC_FAILURE  
*/
scdcint_t scdc_dataset_cmd(scdc_dataset_t dataset, const char *cmd, scdc_dataset_input_t *input, scdc_dataset_output_t *output, ...);


/* dataset input/output */
/*======================*/

/** @fn void scdc_dataset_input_print(scdc_dataset_input_t *input
* @brief Print the content of input that is used to transfer data with scdc_cmd() to a dataprovider in a nerd readable way.
*
* @param[in]  input  Varaible which olds in its structure the input data  
*/
void scdc_dataset_input_print(scdc_dataset_input_t *input);

/** @fn void scdc_dataset_output_print(scdc_dataset_output_t *output);
* @brief Print the content of output that is used by scdc_cmd() to transfer data back to the client from a dataprovider in a nerd readable way.
*
* @param[in]  output  Varaible which holds in its structure the output data
*/
void scdc_dataset_output_print(scdc_dataset_output_t *output);

/** @fn scdc_dataset_input_unset(scdc_dataset_input_t *input)
* @brief Set all member of the structure for the given input to predefined null values. This function clears the input object.
*
* @param  input  Pointer to the input variable
*/
void scdc_dataset_input_unset(scdc_dataset_input_t *input);

/** @fn scdc_dataset_output_unset(scdc_dataset_output_t *output)
* @brief Set all member of the structure for the given output to predefined null values. This function clears the output object.
*
* @param  input  Pointer to the output variable
*/
void scdc_dataset_output_unset(scdc_dataset_output_t *output);


/** @fn scdc_dataset_input_t *scdc_dataset_input_create(scdc_dataset_input_t *input, const char *conf, ...)
* @brief  A additional function to create a dataset input object according to the give configuration. Only for special cases.\\
*         With this function it is possible to read the input data from different places.
*         supported types and optional parameters:
*          - "none": no input
*          - "alloc": allocate an input buffer (without content), if the optional parameter "size" is specified, then the buffer size can be specified with the list of variable arguments
*          - "buffer": use an existing memory buffer as input, the pointer and the size of the memory buffer have to be specified with the list of variable arguments
*          - "stream": use an I/O stream as input, the pointer to the corresponding FILE object has to be specified with the list of variable arguments
*          - "file": use a local file as input, the path of the file has to be specified with the list of variable arguments
*          - "fs": use a file system object (e.g., file or directory) as input, the path of the object has to be specified with the list of variable arguments
*          - "fslist": use a string listing local files and directories as input, the path to the directory to be listed has to be specified with the list of variable arguments
*
* @param  input  Input object
* @param  conf  Configuration of the created input. Paramter conf is a colon-separated list that contains the type of the input to create and further optional parameters
* @param  ...  Additional parameter depending on the given confguration. (e.g. Filepointer)
* @return  The pointer of the input object or NULL.
*/
scdc_dataset_input_t *scdc_dataset_input_create(scdc_dataset_input_t *input, const char *conf, ...);

/** @fn void scdc_dataset_input_destroy(scdc_dataset_input_t *input)
* @brief  Destroy an dataset input object that was created with scdc_dataset_input_create()
*
* @param  input  Pointer of the input object
*/
void scdc_dataset_input_destroy(scdc_dataset_input_t *input);


/** @fn scdc_dataset_output_t *scdc_dataset_output_create(scdc_dataset_output_t *output, const char *conf, ...)
* @brief  A additional function to create a dataset output object according to the give configuration. Only for special cases.\\
*         With this function it is possible to store the output data to different places.
*         supported types and optional parameters:
*          - "none": no output
*          - "alloc": allocate an output buffer, if the optional parameter "size" is specified, then the buffer size can be specified with the list of variable arguments
*          - "buffer": use an existing memory buffer as output, the pointer and the size of the memory buffer have to be specified with the list of variable arguments
*          - "stream": use an I/O stream as output, the pointer to the corresponding FILE object has to be specified with the list of variable arguments
*          - "file": use a local file as output, the path of the file has to be specified with the list of variable arguments
*          - "fs": use a local directory to store file system objects (e.g., file or directory) given as output, the path of the object has to be specified with the list of variable arguments
*          - "consume" or "none": consume the given output without further processing
*
* @param  output  Output object
* @param  conf  Configuration of the created output. Paramter conf is a colon-separated list that contains the type of the output to create and further optional parameters
* @param  ...  Additional parameter depending on the given confguration. (e.g. Filepointer)
* @return  The pointer of the output object or NULL.
*/
scdc_dataset_output_t *scdc_dataset_output_create(scdc_dataset_output_t *output, const char *conf, ...);

/** @fn void scdc_dataset_output_destroy(scdc_dataset_output_t *output)
* @brief  Destroy an dataset output object that was created with scdc_dataset_output_create()
*
* @param  output  Pointer of the output object
*/
void scdc_dataset_output_destroy(scdc_dataset_output_t *output);


/** @fn  scdcint_t scdc_dataset_input_redirect(scdc_dataset_input_t *input, const char *conf, ...)
* @brief  Redirect an existing dataset input object according to the given configuration conf.\\
*         The configuration paramter conf is a colon-separated list describing the redirection. \\
*          If the first list entry is "from", then the existing input is replaced with a new input source, 
*         the subsequent entries describing the source can be the same as for scdc_dataset_input_create().\\
*          If the first list entry is "to" (or at least not "from"), then the existing input is redirected to 
*         the given destination, the subsequent entries describing the destination can be the same as for scdc_dataset_output_create().
*
* @param  input  Input object to redirect
* @param  conf   Colon-separated list describing the redirection.
* @param  ...    Additional paramters
* @return Response if the execution of the function was successful. Values: SCDC_SUCCESS/SCDC_FAILURE
*/
scdcint_t scdc_dataset_input_redirect(scdc_dataset_input_t *input, const char *conf, ...);

/** @fn scdcint_t scdc_dataset_output_redirect(scdc_dataset_output_t *output, const char *conf, ...)
* @brief  Redirect an existing dataset output object according to the given configuration conf.\\
*         The configuration paramter conf is a colon-separated list describing the redirection. \\
*          If the first list entry is "from", then the existing output is replaced with a new output source, 
*         the subsequent entries describing the source can be the same as for scdc_dataset_input_create().\\
*          If the first list entry is "to" (or at least not "from"), then the existing output is redirected to 
*         the given destination, the subsequent entries describing the destination can be the same as for scdc_dataset_output_create().
*
* @param  output  Output object to redirect
* @param  conf    Colon-separated list describing the redirection.
* @param   ...    Additional paramters
* @return Response if the execution of the function was successful. Values: SCDC_SUCCESS/SCDC_FAILURE
*/
scdcint_t scdc_dataset_output_redirect(scdc_dataset_output_t *output, const char *conf, ...);


#ifdef __cplusplus
}
#endif


#endif /* __SCDC_H__ */
