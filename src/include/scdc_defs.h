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


/** @file scdc_defs.h
 @brief Definitions of macros, constants, and data types.
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

/** @hideinitializer
* @brief Macro to enable/disable deprecated functionalities. */
#define SCDC_DEPRECATED  HAVE_SCDC_DEPRECATED

/** @hideinitializer
* @brief Macro to enable/disable debug functionalities. */
#define SCDC_DEBUG  HAVE_SCDC_DEBUG

/** @cond
* Macro to provide a fallback definition of __func__, see "6.47 Function Names as Strings" in gcc-4.9 doc. */
#if __STDC_VERSION__ < 199901L && !defined(__func__)
# if __GNUC__ >= 2
#  define __func__ __FUNCTION__
# else
#  define __func__ "<unknown>"
# endif
#endif
/** @endcond */


/** @typedef typedef scdcint_t
* @brief Definition of the main integer type.
*
* Along with the type definition, the following macros define the properties of the main integer type: #scdcint_fmt, #scdcint_min, #scdcint_max, SCDCINT_IS_SHORT, SCDCINT_IS_INT, SCDCINT_IS_LONG, #SCDCINT_IS_LONG_LONG.
*/
typedef long long scdcint_t;
/** @hideinitializer
* @brief Macro to define the format specifier for a printf-like output of an #scdcint_t. */
#define scdcint_fmt  "lld"
/** @hideinitializer
* @brief Macro to return a constant of type #scdcint_t. */
#define scdcint_const(_c_)  _c_##LL
/** @hideinitializer
* @brief Macro to cast a value to type #scdcint_t. */
#define scdcint_cast(_v_)  static_cast<scdcint_t>(_v_)
/** @hideinitializer
* @brief Macro to define the minimum value that can be represented by an #scdcint_t. */
#define scdcint_min  LONG_LONG_MIN
/** @hideinitializer
* @brief Macro to define the maximum value that can be represented by an #scdcint_t. */
#define scdcint_max  LONG_LONG_MAX

#undef SCDCINT_IS_SHORT
#undef SCDCINT_IS_INT
#undef SCDCINT_IS_LONG
/** @hideinitializer
* @brief Macro to define whether #scdcint_t is a long long integer. */
#define SCDCINT_IS_LONG_LONG  1

/** @hideinitializer
* @brief Macro to define the return value of a function that was successful. */
#define SCDC_SUCCESS  (scdcint_t)1
/** @hideinitializer
* @brief Macro to define the return value of a function that failed. */
#define SCDC_FAILURE  (scdcint_t)0

/** @hideinitializer
* @brief Macro to define a NULL pointer. */
#ifdef __cplusplus
# define SCDC_NULL  0
#else
# define SCDC_NULL  (void *)0
#endif


/** @hideinitializer
* @brief Macro to define the maximum length of the string scdc_dataset_inout_t::format within the structure #scdc_dataset_inout_t. */
#define SCDC_FORMAT_MAX_SIZE  16

/** @hideinitializer
* @brief Macro to define whether multiple input buffers within the structure #scdc_dataset_inout_t are support. */
#define SCDC_DATASET_INOUT_BUF_MULTIPLE  1

/** @hideinitializer
* @brief Macro to set the field \link scdc_dataset_inout_t::total_size_given total_size_given \endlink within the structure #scdc_dataset_inout_t.
* The value specifies that the total size given in the field \link scdc_dataset_inout_t::total_size total_size \endlink is exact. */
#define SCDC_DATASET_INOUT_TOTAL_SIZE_GIVEN_EXACT     'x'
/** @hideinitializer
* @brief Macro to set the field \link scdc_dataset_inout_t::total_size_given total_size_given \endlink within the structure #scdc_dataset_inout_t.
* The value specifies that the total size given in the field \link scdc_dataset_inout_t::total_size total_size \endlink is a lower limit. */
#define SCDC_DATASET_INOUT_TOTAL_SIZE_GIVEN_AT_LEAST  'l'
/** @hideinitializer
* @brief Macro to set the field \link scdc_dataset_inout_t::total_size_given total_size_given \endlink within the structure #scdc_dataset_inout_t.
* The value specifies that the total size given in the field \link scdc_dataset_inout_t::total_size total_size \endlink is an upper limit. */
#define SCDC_DATASET_INOUT_TOTAL_SIZE_GIVEN_AT_MOST   'm'
/** @hideinitializer
* @brief Macro to set the field \link scdc_dataset_inout_t::total_size_given total_size_given \endlink within the structure #scdc_dataset_inout_t.
* The value specifies that the total size given in the field \link scdc_dataset_inout_t::total_size total_size \endlink is invalid. */
#define SCDC_DATASET_INOUT_TOTAL_SIZE_GIVEN_NONE      'n'

/** @cond
* Forward declaration of the structure #scdc_dataset_inout_t. */
typedef struct _scdc_dataset_inout_t scdc_dataset_inout_t;
/** @endcond */

/** @brief Type definition of a next function.
*
* @param inout Dataset input or output that is or should be continued with the next function.
* @return Whether the next function was successful (#SCDC_SUCCESS) or not (#SCDC_FAILURE).
*/
typedef scdcint_t scdc_dataset_inout_next_f(scdc_dataset_inout_t *inout);

/** @brief Forward declaration of the type of an internally used member of the structure scdc_dataset_inout_t.
*/
typedef struct _scdc_dataset_inout_intern_t scdc_dataset_inout_intern_t;

/** @brief Structure of a data buffer object.
*/
typedef struct _scdc_buf_t
{
  /** Pointer to the memory location of the buffer. */
  void *ptr;
  /** Size of the buffer (bytes). */
  scdcint_t size;
  /** Current size of the data within the buffer (bytes). */
  scdcint_t current;

} scdc_buf_t;

/** @brief Structure of a dataset input/output object used to transfer data between client and service during the function scdc_dataset_cmd.
*/
typedef struct _scdc_dataset_inout_t
{
  /** String describing the format of the data (to be) transferred. */
  char format[SCDC_FORMAT_MAX_SIZE];

#if !SCDC_DEPRECATED
  /** Buffer containing the data (to be) transferred. */
  scdc_buf_t buf;
#else /* !SCDC_DEPRECATED */
  /** Pointer to the memory location storing the data (to be) transferred. */
  void *buf;
  /** Size of the memory location storing the data (to be) transferred (bytes). */
  scdcint_t buf_size;
  /** Current size of the data (to be) transferred (bytes). */
  scdcint_t current_size;
#endif /* !SCDC_DEPRECATED */

  /** Currently know total size of the data (to be) transferred (bytes). */
  scdcint_t total_size;
  /** Specifies how the given value in #total_size should be interpreted. Predefined values are #SCDC_DATASET_INOUT_TOTAL_SIZE_GIVEN_EXACT, #SCDC_DATASET_INOUT_TOTAL_SIZE_GIVEN_AT_LEAST, #SCDC_DATASET_INOUT_TOTAL_SIZE_GIVEN_AT_MOST, #SCDC_DATASET_INOUT_TOTAL_SIZE_GIVEN_NONE. */
  char total_size_given;

  /** Pointer to a next function that produces or consumes more input/output. The next function reads or modifies the fields of the provided dataset input/output object. */
  scdc_dataset_inout_next_f *next;
  /** Pointer to store arbitrary information within a dataset input/output object. */
  void *data;

  /** Pointer to an internally used field of data. (Should only be initialized with NULL, but not further modified.) */
  scdc_dataset_inout_intern_t *intern;
  /** Pointer to internally used data. (Should only be initialized with NULL, but not further modified.) */
  void *intern_data;

} scdc_dataset_inout_t;

#if !SCDC_DEPRECATED

/** @hideinitializer
* @brief Macro to return the memory location of a (single) data buffer in a dataset input/output object.
*
* @param \_inout\_ Dataset input/output object.
*/
# define SCDC_DATASET_INOUT_BUF_PTR(_inout_)      (_inout_)->buf.ptr
/** @hideinitializer
* @brief Macro to return the size of the memory location of a (single) data buffer in a dataset input/output object.
*
* @param \_inout\_ Dataset input/output object.
*/
# define SCDC_DATASET_INOUT_BUF_SIZE(_inout_)     (_inout_)->buf.size
/** @hideinitializer
* @brief Macro to return the current data size of a (single) data buffer in a dataset input/output object.
*
* @param \_inout\_ Dataset input/output object.
*/
# define SCDC_DATASET_INOUT_BUF_CURRENT(_inout_)  (_inout_)->buf.current

#else /* !SCDC_DEPRECATED */

/** @hideinitializer
* @brief Macro to return the memory location of the data buffer in a dataset input/output object.
*
* @param \_inout\_ Dataset input/output object.
*/
# define SCDC_DATASET_INOUT_BUF_PTR(_inout_)      (_inout_)->buf
/** @hideinitializer
* @brief Macro to return the size of the memory location of a (single) data buffer in a dataset input/output object.
*
* @param \_inout\_ Dataset input/output object.
*/
# define SCDC_DATASET_INOUT_BUF_SIZE(_inout_)     (_inout_)->buf_size
/** @hideinitializer
* @brief Macro to return the current data size of a (single) data buffer in a dataset input/output object.
*
* @param \_inout\_ Dataset input/output object.
*/
# define SCDC_DATASET_INOUT_BUF_CURRENT(_inout_)  (_inout_)->current_size

#endif /* !SCDC_DEPRECATED */

/** @hideinitializer
* @brief Macro to set the memory location of a (single) data buffer in a dataset input/output object.
*
* @param \_inout\_ Dataset input/output object.
* @param \_c\_ New memory location.
*/
#define SCDC_DATASET_INOUT_BUF_SET_P(_inout_, _p_)  (SCDC_DATASET_INOUT_BUF_PTR(_inout_) = (_p_))
/** @hideinitializer
* @brief Macro to get the memory location of a (single) data buffer in a dataset input/output object.
*
* @param \_inout\_ Dataset input/output object.
* @return Memory location of a (single) data buffer or 0 in case of a multiple data buffer.
*/
#define SCDC_DATASET_INOUT_BUF_GET_P(_inout_)       ((SCDC_DATASET_INOUT_BUF_SIZE(_inout_) >= 0)?SCDC_DATASET_INOUT_BUF_PTR(_inout_):SCDC_NULL)

/** @hideinitializer
* @brief Macro to set the size of the memory location of a (single) data buffer in a dataset input/output object.
*
* @param \_inout\_ Dataset input/output object.
* @param \_c\_ New size of the memory location.
*/
#define SCDC_DATASET_INOUT_BUF_SET_S(_inout_, _s_)  (SCDC_DATASET_INOUT_BUF_SIZE(_inout_) = (_s_))
/** @hideinitializer
* @brief Macro to get the size of the memory location of a (single) data buffer in a dataset input/output object.
*
* @param \_inout\_ Dataset input/output object.
* @return Size of the memory location of a (single) data buffer or 0 in case of a multiple data buffer.
*/
#define SCDC_DATASET_INOUT_BUF_GET_S(_inout_)       ((SCDC_DATASET_INOUT_BUF_SIZE(_inout_) >= 0)?SCDC_DATASET_INOUT_BUF_SIZE(_inout_):0)

/** @hideinitializer
* @brief Macro to set the current data size of a (single) data buffer in a dataset input/output object.
*
* @param \_inout\_ Dataset input/output object.
* @param \_c\_ New current data size.
*/
#define SCDC_DATASET_INOUT_BUF_SET_C(_inout_, _c_)  (SCDC_DATASET_INOUT_BUF_CURRENT(_inout_) = (_c_))
/** @hideinitializer
* @brief Macro to get the current data size of a (single) data buffer in a dataset input/output object.
*
* @param \_inout\_ Dataset input/output object.
* @return Current data size of a (single) data buffer or 0 in case of a multiple data buffer.
*/
#define SCDC_DATASET_INOUT_BUF_GET_C(_inout_)       ((SCDC_DATASET_INOUT_BUF_CURRENT(_inout_) >= 0)?SCDC_DATASET_INOUT_BUF_CURRENT(_inout_):0)

#define SCDC_DATASET_INOUT_BUF_SET(_inout_, _p_, _s_, _c_)  ( \
  SCDC_DATASET_INOUT_BUF_SET_P(_inout_, _p_), \
  SCDC_DATASET_INOUT_BUF_SET_S(_inout_, _s_), \
  SCDC_DATASET_INOUT_BUF_SET_C(_inout_, _c_) \
)

#define SCDC_DATASET_INOUT_BUF_ASSIGN(_inout_, _rhs_)  ( \
  SCDC_DATASET_INOUT_BUF_SET_P(_inout_, SCDC_DATASET_INOUT_BUF_GET_P(_rhs_)), \
  SCDC_DATASET_INOUT_BUF_SET_S(_inout_, SCDC_DATASET_INOUT_BUF_GET_S(_rhs_)), \
  SCDC_DATASET_INOUT_BUF_SET_C(_inout_, SCDC_DATASET_INOUT_BUF_GET_C(_rhs_)) \
)

#if SCDC_DATASET_INOUT_BUF_MULTIPLE

# define SCDC_DATASET_INOUT_MBUF_ISSET(_inout_)               (SCDC_DATASET_INOUT_BUF_SIZE(_inout_) < 0)
# define SCDC_DATASET_INOUT_MBUF_SET_P(_inout_, _p_)          (SCDC_DATASET_INOUT_BUF_PTR(_inout_) = (_p_))
# define SCDC_DATASET_INOUT_MBUF_GET_P(_inout_)               (SCDC_DATASET_INOUT_MBUF_ISSET(_inout_)?SCDC_DATASET_INOUT_BUF_PTR(_inout_):SCDC_NULL)
# define SCDC_DATASET_INOUT_MBUF_SET_S(_inout_, _s_)          (SCDC_DATASET_INOUT_BUF_SIZE(_inout_) = -(_s_))
# define SCDC_DATASET_INOUT_MBUF_GET_S(_inout_)               (SCDC_DATASET_INOUT_MBUF_ISSET(_inout_)?-SCDC_DATASET_INOUT_BUF_SIZE(_inout_):0)
# define SCDC_DATASET_INOUT_MBUF_SET_C(_inout_, _c_)          (SCDC_DATASET_INOUT_BUF_CURRENT(_inout_) = -(_c_))
# define SCDC_DATASET_INOUT_MBUF_GET_C(_inout_)               (SCDC_DATASET_INOUT_MBUF_ISSET(_inout_)?-SCDC_DATASET_INOUT_BUF_CURRENT(_inout_):0)
# define SCDC_DATASET_INOUT_MBUF_SET(_inout_, _p_, _s_, _c_)  (SCDC_DATASET_INOUT_MBUF_SET_P(_inout_, _p_), SCDC_DATASET_INOUT_MBUF_SET_S(_inout_, _s_), SCDC_DATASET_INOUT_MBUF_SET_C(_inout_, _c_))
# define SCDC_DATASET_INOUT_MBUF_INIT(_inout_, _p_, _b_)      SCDC_DATASET_INOUT_MBUF_SET(_inout_, _p_, ((scdcint_t) (_b_)) / ((scdcint_t) sizeof(scdc_buf_t)), 0)

# define SCDC_DATASET_INOUT_MBUF_M(_inout_, _m_)                     (&((scdc_buf_t *) SCDC_DATASET_INOUT_BUF_PTR(_inout_))[_m_])
# define SCDC_DATASET_INOUT_MBUF_M_PTR(_inout_, _m_)                 SCDC_DATASET_INOUT_MBUF_M(_inout_, _m_)->ptr
# define SCDC_DATASET_INOUT_MBUF_M_SIZE(_inout_, _m_)                SCDC_DATASET_INOUT_MBUF_M(_inout_, _m_)->size
# define SCDC_DATASET_INOUT_MBUF_M_CURRENT(_inout_, _m_)             SCDC_DATASET_INOUT_MBUF_M(_inout_, _m_)->current
# define SCDC_DATASET_INOUT_MBUF_M_SET(_inout_, _m_, _p_, _s_, _c_)  (SCDC_DATASET_INOUT_MBUF_M_PTR(_inout_, _m_) = (_p_), SCDC_DATASET_INOUT_MBUF_M_SIZE(_inout_, _m_) = (_s_), SCDC_DATASET_INOUT_MBUF_M_CURRENT(_inout_, _m_) = (_c_))

#endif /* SCDC_DATASET_INOUT_BUF_MULTIPLE */

/** @brief Structure of a dataset input object derived from #scdc_dataset_inout_t. */
typedef scdc_dataset_inout_t scdc_dataset_input_t;

/** @brief Structure of a dataset output object derived from #scdc_dataset_inout_t. */
typedef scdc_dataset_inout_t scdc_dataset_output_t;


/** @brief Type definition of the callback function for opening a hook data provider.
* The function is (optionally) called when a server creates a data provider with #scdc_dataprov_open.
*
* @param conf Configuration string provided to the function #scdc_dataprov_open.
* @param ap A variable argument list containing the arguments provided to the function #scdc_dataprov_open.
* @return Handle of the created data provider.
*/
typedef void *scdc_dataprov_hook_open_f(const char *conf, va_list ap);

/** @brief Type definition of the callback function for closing a hook data provider.
* The function is (optionally) called when a server closes a data provider with #scdc_dataprov_close.
*
* @param dataprov Handle of the data provider.
* @return Whether the function was successful (#SCDC_SUCCESS) or not (#SCDC_FAILURE).
*/
typedef scdcint_t scdc_dataprov_hook_close_f(void *dataprov);

/** @brief Type definition of the callback function for configuring a hook data provider.
* The function is (optionally) called when a server configures a data data provider.
*
* @param dataprov Handle of the data provider.
* @param cmd String specifying the configuration command.
* @param param String specifying the parameter to be configured.
* @param val String specifying the value to be set for the parameter.
* @param val_size Size of the string specified in @a val.
* @param result String for storing a result of the configuration command.
* @param result_size Size of the result string stored in @a result.
* @return Whether the function was successful (#SCDC_SUCCESS) or not (#SCDC_FAILURE).
*/
typedef scdcint_t scdc_dataprov_hook_config_f(void *dataprov, const char *cmd, const char *param, const char *val, scdcint_t val_size, char **result, scdcint_t *result_size);

/** @brief Type definition of the callback function for opening a dataset of a hook data provider.
* The function is (optionally) called when a client opens a dataset with #scdc_dataset_open.
*
* @param dataprov Handle of the data provider.
* @param path Path of the dataset within the URI address.
* @return Handle of the opened dataset.
*/
typedef void *scdc_dataprov_hook_dataset_open_f(void *dataprov, const char *path);

/** @brief Type definition of the callback function for closing a dataset of a hook data provider.
* The function is (optionally) called when a client closes a dataset with #scdc_dataset_close.
*
* @param dataprov Handle of the data provider.
* @param dataset Handle of the dataset.
* @return Whether the function was successful (#SCDC_SUCCESS) or not (#SCDC_FAILURE).
*/
typedef scdcint_t scdc_dataprov_hook_dataset_close_f(void *dataprov, void *dataset);

/** Type definition of the callback function for closing a dataset and writing its state to a memory location.
* The function is (optionally) called when a server finished working with an open dataset and does not want to permanently allocate the resources of that dataset.
*
* @param dataprov Handle of the data provider.
* @param dataset Handle of the dataset.
* @param buf Memory location for storing the state of the dataset.
* @param buf_size Size of the memory location.
* @return Whether the function was successful (#SCDC_SUCCESS) or not (#SCDC_FAILURE).
*/
typedef scdcint_t scdc_dataprov_hook_dataset_close_write_state_f(void *dataprov, void *dataset, char *buf, scdcint_t buf_size);

/** Type definition of the callback function for opening a dataset and reading its state from a memory location.
* The function is (optionally) called when a server continues working with an open dataset without having its resources permanently allocated.
*
* @param dataprov Handle of the data provider.
* @param buf Memory location of the stored state of the dataset.
* @param buf_size Size of the memory location.
* @return Handle of the opened dataset.
*/
typedef void *scdc_dataprov_hook_dataset_open_read_state_f(void *dataprov, const char *buf, scdcint_t buf_size);

/** Type definition of the callback function for executing a command on a dataset of a hook data provider.
* The function is (optionally) called when a client executes a command with #scdc_dataset_cmd.
*
* @param dataprov Handle of the data provider.
* @param dataset Handle of the dataset.
* @param cmd String specifying the command.
* @param params String specifying additional parameters of the command.
* @param input Input object with data transferred from client to server.
* @param output Output object with data transferred from server to client.
* @return Whether the function was successful (#SCDC_SUCCESS) or not (#SCDC_FAILURE).
*/
typedef scdcint_t scdc_dataprov_hook_dataset_cmd_f(void *dataprov, void *dataset, const char *cmd, const char *params, scdc_dataset_input_t *input, scdc_dataset_output_t *output);

/**@brief Structure to specify the callback functions of a hook data provider.
* A pointer to such a structure has to be provided as a paramter to when creating the data provider with #scdc_dataprov_open.
*/
typedef struct _scdc_dataprov_hook_t
{
  /** Pointer to the callback function for opening the hook data provider. */
  scdc_dataprov_hook_open_f *open;
  /** Pointer to the callback function for closing the hook data provider. */
  scdc_dataprov_hook_close_f *close;
  /** Pointer to the callback function for configuring the hook data provider. */
  scdc_dataprov_hook_config_f *config;

  /** Pointer to the callback function for opening a dataset. */
  scdc_dataprov_hook_dataset_open_f *dataset_open;
  /** Pointer to the callback function for closing a dataset. */
  scdc_dataprov_hook_dataset_close_f *dataset_close;

  /** Pointer to the callback function for closing a dataset and writing its state to a memory location. */
  scdc_dataprov_hook_dataset_close_write_state_f *dataset_close_write_state;
  /** Pointer to the callback function for opening a dataset and reading its state from a memory location. */
  scdc_dataprov_hook_dataset_open_read_state_f *dataset_open_read_state;

  /** Type definition of the callback function for executing a command on a dataset. */
  scdc_dataprov_hook_dataset_cmd_f *dataset_cmd;

} scdc_dataprov_hook_t;


/** @hideinitializer
* @brief Macro to specify the mode for starting a node port with #scdc_nodeport_start.
* The value specifies that the node port is not started. */
#define SCDC_NODEPORT_START_NONE                0x0
/** @hideinitializer
* @brief Macro to specify the mode for starting a node port with #scdc_nodeport_start.
* The value specifies that the node port is started blocking until it is canceled with #scdc_nodeport_cancel. */
#define SCDC_NODEPORT_START_LOOP_UNTIL_CANCEL   0x1
/** @hideinitializer
* @brief Macro to specify the mode for starting a node port with #scdc_nodeport_start.
* The value specifies that the node port is started blocking until all connections are finished. */
#define SCDC_NODEPORT_START_LOOP_UNTIL_IDLE     0x2
/** @hideinitializer
* @brief Macro to specify the mode for starting a node port with #scdc_nodeport_start.
* The value specifies that the node port is started asynchronously and runs until it is canceled with #scdc_nodeport_cancel. */
#define SCDC_NODEPORT_START_ASYNC_UNTIL_CANCEL  0x4
/** @hideinitializer
* @brief Macro to specify the mode for starting a node port with #scdc_nodeport_start.
* The value specifies that the node port is started asynchronously until all connections are finished. */
#define SCDC_NODEPORT_START_ASYNC_UNTIL_IDLE    0x8

/** @cond */
typedef scdcint_t scdc_handler_f(void *data);

typedef scdcint_t scdc_log_handler_f(void *data, const char *buf, scdcint_t buf_size);

typedef scdcint_t scdc_nodeport_cmd_handler_f(void *data, const char *cmd, const char *params, scdcint_t params_size);

typedef scdcint_t scdc_nodeport_timer_handler_f(void *data);

typedef scdcint_t scdc_nodeport_loop_handler_f(void *data, scdcint_t l);

typedef scdcint_t scdc_dataprov_jobrun_handler_f(void *data, const char *jobid, const char *cmd, const char *params, scdc_dataset_input_t *input, scdc_dataset_output_t *output);
/** @endcond */


#ifdef __cplusplus
}
#endif


#endif /* __SCDC_DEFS_H__ */
