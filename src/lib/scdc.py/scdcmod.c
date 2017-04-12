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


#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include <stdio.h>

#include "z_pack.h"

#include "scdc.h"
#include "scdc_intern.h"

#define PYSCDC_TRACE_NOT  1

#include "pylog.h"
#include "scdcmod.h"

#if PYSCDC_LOG_HANDLER_LOCK
# include <pthread.h>
#endif


PYSCDC_TRACE_DEF()


int pyscdc_threading = 1;

#if PYSCDC_LIBSCDC_ALLOW_THREADS
# define PYSCDC_THREADING(_x_)      Z_MOP(if (pyscdc_threading) { _x_ })
# define PYSCDC_LIBSCDC_DEF()       PyThreadState *_tstate = NULL;
# define PYSCDC_LIBSCDC_PY_DEF()    PyGILState_STATE _gstate = 0;
#else
# define PYSCDC_THREADING(_x_)      Z_NOP()
# define PYSCDC_LIBSCDC_DEF()
# define PYSCDC_LIBSCDC_PY_DEF()
#endif
#define PYSCDC_LIBSCDC_BEGIN()     PYSCDC_THREADING(_tstate = PyEval_SaveThread();)
#define PYSCDC_LIBSCDC_END()       PYSCDC_THREADING(PyEval_RestoreThread(_tstate);)
#define PYSCDC_LIBSCDC_PY_BEGIN()  PYSCDC_THREADING(_gstate = PyGILState_Ensure();)
#define PYSCDC_LIBSCDC_PY_END()    PYSCDC_THREADING(PyGILState_Release(_gstate);)


#define PYSCDC_OBJ_SCDC            0
#define PYSCDC_OBJ_SCDCMOD         1
#define PYSCDC_OBJ_DATAPROV        2
#define PYSCDC_OBJ_NODEPORT        3 
#define PYSCDC_OBJ_DATASET         4
#define PYSCDC_OBJ_CBUF            5
#define PYSCDC_OBJ_DATASET_INPUT   6
#define PYSCDC_OBJ_DATASET_OUTPUT  7
#define PYSCDC_OBJ_LAST            8

PyObject *pyscdc_objs[PYSCDC_OBJ_LAST];

#define PYSCDC_RETURN_OBJECT(_o_)  Z_MOP( \
  if ((_o_) == Py_None) Py_RETURN_NONE; \
  else if ((_o_) == Py_True) Py_RETURN_TRUE; \
  else if ((_o_) == Py_False) Py_RETURN_FALSE; \
  else return (_o_); \
)

#define PYSCDC_RETURN_BOOL(_r_)  Z_MOP( \
  if ((_r_) == SCDC_SUCCESS) Py_RETURN_TRUE; \
  else Py_RETURN_FALSE; \
)


#define MODULE_NAME     scdcmod
#define MODULE_NAMESTR  "scdcmod"


static PyObject *pyscdc_log_cout_write(PyObject *self, PyObject *pyargs);
static PyObject *pyscdc_log_cerr_write(PyObject *self, PyObject *pyargs);

static PyObject *pyscdc_CONFIG(PyObject *self, PyObject *pyargs);
static PyObject *pyscdc_CONSTANTS(PyObject *self, PyObject *pyargs);

static PyObject *pyscdc_cptr2py(PyObject *self, PyObject *pyargs);
static PyObject *pyscdc_py2cptr(PyObject *self, PyObject *pyargs);

static PyObject *pyscdc_pybuf(PyObject *self, PyObject *pyargs);

static PyObject *pyscdc_init(PyObject *self, PyObject *pyargs);
static PyObject *pyscdc_release(PyObject *self, PyObject *pyargs);

static PyObject *pyscdc_log_init(PyObject *self, PyObject *pyargs);
static PyObject *pyscdc_log_release(PyObject *self, PyObject *pyargs);

static PyObject *pyscdc_dataprov_open(PyObject *self, PyObject *pyargs);
static PyObject *pyscdc_dataprov_close(PyObject *self, PyObject *pyargs);

static PyObject *pyscdc_nodeport_open(PyObject *self, PyObject *pyargs);
static PyObject *pyscdc_nodeport_close(PyObject *self, PyObject *pyargs);
static PyObject *pyscdc_nodeport_start(PyObject *self, PyObject *pyargs);
static PyObject *pyscdc_nodeport_stop(PyObject *self, PyObject *pyargs);
static PyObject *pyscdc_nodeport_cancel(PyObject *self, PyObject *pyargs);
static PyObject *pyscdc_nodeport_authority(PyObject *self, PyObject *pyargs);
static PyObject *pyscdc_nodeport_supported(PyObject *self, PyObject *pyargs);

static PyObject *pyscdc_dataset_open(PyObject *self, PyObject *pyargs);
static PyObject *pyscdc_dataset_close(PyObject *self, PyObject *pyargs);
static PyObject *pyscdc_dataset_cmd(PyObject *self, PyObject *pyargs);

static PyObject *pyscdc_dataset_input_create(PyObject *self, PyObject *pyargs);
static PyObject *pyscdc_dataset_input_destroy(PyObject *self, PyObject *pyargs);

static PyObject *pyscdc_dataset_output_create(PyObject *self, PyObject *pyargs);
static PyObject *pyscdc_dataset_output_destroy(PyObject *self, PyObject *pyargs);

static PyObject *pyscdc_dataset_input_redirect(PyObject *self, PyObject *pyargs);
static PyObject *pyscdc_dataset_output_redirect(PyObject *self, PyObject *pyargs);

static PyObject *pyscdc_dataset_input_next_class(PyObject *self, PyObject *pyargs);
static PyObject *pyscdc_dataset_output_next_class(PyObject *self, PyObject *pyargs);

static PyMethodDef scdc_methods[] = {
  {"log_cout_write", pyscdc_log_cout_write, METH_VARARGS, "Write string to cout log."},
  {"log_cerr_write", pyscdc_log_cerr_write, METH_VARARGS, "Write string to cerr log."},
  {"CONFIG", pyscdc_CONFIG, METH_VARARGS, "Return config flags."},
  {"CONSTANTS", pyscdc_CONSTANTS, METH_VARARGS, "Return C constants."},
  {"cptr2py", pyscdc_cptr2py, METH_VARARGS, "Convert a given cptr value to a Python object."},
  {"py2cptr", pyscdc_py2cptr, METH_VARARGS, "Convert a given Python object to a cptr value."},
  {"pybuf", pyscdc_pybuf, METH_VARARGS, "Create a buffer for input or output objects."},
  {"init", pyscdc_init, METH_VARARGS, "Init the SCDC library."},
  {"release", pyscdc_release, METH_VARARGS, "Release the SCDC library."},
  {"log_init", pyscdc_log_init, METH_VARARGS, "Init logging of the SCDC library."},
  {"log_release", pyscdc_log_release, METH_VARARGS, "Release logging of the SCDC library."},
  {"dataprov_open", pyscdc_dataprov_open, METH_VARARGS, "Open a new data provider."},
  {"dataprov_close", pyscdc_dataprov_close, METH_VARARGS, "Close a data provider."},
  {"nodeport_open", pyscdc_nodeport_open, METH_VARARGS, "Open a new node port."},
  {"nodeport_close", pyscdc_nodeport_close, METH_VARARGS, "Close a node port."},
  {"nodeport_start", pyscdc_nodeport_start, METH_VARARGS, "Start a node port."},
  {"nodeport_stop", pyscdc_nodeport_stop, METH_VARARGS, "Stop a node port."},
  {"nodeport_cancel", pyscdc_nodeport_cancel, METH_VARARGS, "Cancel a node port."},
  {"nodeport_authority", pyscdc_nodeport_authority, METH_VARARGS, "Create a node port authority string."},
  {"nodeport_supported", pyscdc_nodeport_supported, METH_VARARGS, "Check whether an uri address is supported."},
  {"dataset_open", pyscdc_dataset_open, METH_VARARGS, "Open a new dataset."},
  {"dataset_close", pyscdc_dataset_close, METH_VARARGS, "Close a dataset."},
  {"dataset_cmd", pyscdc_dataset_cmd, METH_VARARGS, "Execute a dataset command."},
  {"dataset_input_create", pyscdc_dataset_input_create, METH_VARARGS, "Create a dataset input object."},
  {"dataset_input_destroy", pyscdc_dataset_input_destroy, METH_VARARGS, "Destroy a dataset input object."},
  {"dataset_output_create", pyscdc_dataset_output_create, METH_VARARGS, "Create a dataset output object."},
  {"dataset_output_destroy", pyscdc_dataset_output_destroy, METH_VARARGS, "Destroy a dataset output object."},
  {"dataset_input_redirect", pyscdc_dataset_input_redirect, METH_VARARGS, "Redirect a dataset input object."},
  {"dataset_output_redirect", pyscdc_dataset_output_redirect, METH_VARARGS, "Redirect a dataset output object."},
  {"dataset_input_next_class", pyscdc_dataset_input_next_class, METH_VARARGS, "Execute next handler of a dataset input class."},
  {"dataset_output_next_class", pyscdc_dataset_output_next_class, METH_VARARGS, "Execute next handler of a dataset output class."},
  {NULL, NULL, 0, NULL} 
};


typedef struct _pyscdc_args_data_py_t
{
  scdcint_t nargs, arg;
  PyObject *args[PYSCDC_ARGS_MAX];

} pyscdc_args_data_py_t;


#ifndef PYSCDC_LOGFILE

#if PYSCDC_LOG_HANDLER_LOCK
static pthread_mutex_t pyscdc_log_handler_lock = PTHREAD_MUTEX_INITIALIZER;
#endif

#define SYS_WRITE_MAX_SIZE  1000

scdcint_t pyscdc_log_handler_sys_writestdout(void *data, const char *buf, scdcint_t buf_size)
{
  int n;

  PYSCDC_LIBSCDC_PY_DEF()


  if (!Py_IsInitialized()) return SCDC_FAILURE;

  PYSCDC_LIBSCDC_PY_BEGIN();

#if PYSCDC_LOG_HANDLER_LOCK
  pthread_mutex_lock(&pyscdc_log_handler_lock);
#endif

  while (buf_size > 0)
  {
    n = z_min(buf_size, SYS_WRITE_MAX_SIZE);

    PySys_WriteStdout("%.*s", n, buf);

    buf += n;
    buf_size -= n;
  }

#if PYSCDC_LOG_HANDLER_LOCK
  pthread_mutex_unlock(&pyscdc_log_handler_lock);
#endif

  PYSCDC_LIBSCDC_PY_END();

  return SCDC_SUCCESS;
}


scdcint_t pyscdc_log_handler_sys_writestderr(void *data, const char *buf, scdcint_t buf_size)
{
  int n;

  PYSCDC_LIBSCDC_PY_DEF()


  if (!Py_IsInitialized()) return SCDC_FAILURE;

  PYSCDC_LIBSCDC_PY_BEGIN();

#if PYSCDC_LOG_HANDLER_LOCK
  pthread_mutex_lock(&pyscdc_log_handler_lock);
#endif

  while (buf_size > 0)
  {
    n = z_min(buf_size, SYS_WRITE_MAX_SIZE);

    PySys_WriteStderr("%.*s", n, buf);

    buf += n;
    buf_size -= n;
  }

#if PYSCDC_LOG_HANDLER_LOCK
  pthread_mutex_unlock(&pyscdc_log_handler_lock);
#endif

  PYSCDC_LIBSCDC_PY_END();

  return SCDC_SUCCESS;
}

#undef MAX_SYS_WRITE_SIZE

#endif /* PYSCDC_LOGFILE */


#if PY_MAJOR_VERSION >= 3

static struct PyModuleDef scdc_module = {
   PyModuleDef_HEAD_INIT,
   MODULE_NAMESTR, /* name of module */
   NULL,           /* module documentation, may be NULL */
   -1,             /* size of per-interpreter state of the module, or -1 if the module keeps state in global variables. */
   scdc_methods
};

#endif

#if PY_MAJOR_VERSION >= 3
PyObject *Z_CONCAT(PyInit_, MODULE_NAME)(void)
#else
PyMODINIT_FUNC Z_CONCAT(init, MODULE_NAME)(void)
#endif
{
  PyObject *module;
  scdcint_t i;

  PYSCDC_LIBSCDC_DEF()


#if PY_MAJOR_VERSION >= 3
  module = PyModule_Create(&scdc_module);
#else
  module = Py_InitModule(MODULE_NAMESTR, scdc_methods);
#endif

  if (module == NULL) fprintf(stderr, "error: python module initialization fails\n");

  for (i = 0; i < PYSCDC_OBJ_LAST; ++i) pyscdc_objs[i] = NULL;

  pyscdc_objs[PYSCDC_OBJ_SCDC] = PyImport_AddModule("scdc");
  pyscdc_objs[PYSCDC_OBJ_SCDCMOD] = module;

/*  PYSCDC_TRACE_OBJECT(pyscdc_objs[PYSCDC_OBJ_SCDC], "scdc: ");
  PYSCDC_TRACE_OBJECT(pyscdc_objs[PYSCDC_OBJ_SCDCMOD], "scdcmod: ");*/

  if (PyObject_HasAttrString(pyscdc_objs[PYSCDC_OBJ_SCDC], "SCDCMOD_THREADING"))
    pyscdc_threading = (PyObject_GetAttrString(pyscdc_objs[PYSCDC_OBJ_SCDC], "SCDCMOD_THREADING") == Py_True);

  PYSCDC_TRACE("modinit: pyscdc_threading: '%d'", pyscdc_threading);

  PYSCDC_THREADING(PyEval_InitThreads(););

  PYSCDC_LIBSCDC_BEGIN();
  scdc_main_log_init(
#ifdef PYSCDC_LOGFILE
    "log_filepath", PYSCDC_LOGFILE
#else
    "log_handler", pyscdc_log_handler_sys_writestdout, NULL, pyscdc_log_handler_sys_writestderr, NULL
    );
#endif
  PYSCDC_LIBSCDC_END();

#if PY_MAJOR_VERSION >= 3
  return module;
#endif
}


void pyscdc_pyerr_log_cout_print(void)
{
  PYSCDC_LIBSCDC_DEF()


  /* FIXME: error message goes to stderr */
  PyErr_Print();

  PYSCDC_LIBSCDC_BEGIN();
  scdc_log_cout_printf("\n");
  PYSCDC_LIBSCDC_END();

/*  PyObject *ptype, *pvalue, *ptraceback;

  PyErr_Fetch(&ptype, &pvalue, &ptraceback);

PyObject *exc_type = NULL, *exc_value = NULL, *exc_tb = NULL;
PyErr_Fetch(&exc_type, &exc_value, &exc_tb);
PyObject* str_exc_type = PyObject_Repr(exc_type); //Now a unicode
object
PyObject* pyStr = PyUnicode_AsEncodedString(str_exc_type, "utf-8",
"Error ~");
const char *strExcType =  PyBytes_AS_STRING(pyStr);
Py_XDECREF(str_exc_type);
Py_XDECREF(pyStr);

Py_XDECREF(exc_type);
Py_XDECREF(exc_value);
Py_XDECREF(exc_tb);

  PyObject_Print(ptype, stdout, 0);
  PyObject_Print(pvalue, stdout, 0);
  PyObject_Print(ptraceback, stdout, 0);*/
}


void pyscdc_dataset_inout_log_cout_print(PyObject *d, int str)
{
  const char *s;
  PyObject *q, *r;

  PYSCDC_LIBSCDC_DEF()


  q = (str)?PyObject_Str(d):PyObject_Repr(d);
/*  PYSCDC_TRACE_CALL(q, "pyscdc_dataset_inout_log_cout_print: PyObject_%s: ", ((str)?"Str":"Repr"));*/

  if (!q) return;

  r = PyObject_CallMethod(q, "replace", "cs", 0, "\\x00");
/*  PYSCDC_TRACE_CALL(q, "pyscdc_dataset_inout_log_cout_print: replace: ");*/

  if (!r) return;

  pyscdc_parseret = PyArg_Parse(r, "s", &s);
/*  PYSCDC_TRACE_PARSE("pyscdc_dataset_inout_log_cout_print: ");*/

  PYSCDC_LIBSCDC_BEGIN();
  scdc_log_cout_printf(s);
  PYSCDC_LIBSCDC_END();
}


void pyscdc_object_log_cout_print(PyObject *o, int str)
{
  const char *s;
  PyObject *r;

  PYSCDC_LIBSCDC_DEF()


  r = (str)?PyObject_Str(o):PyObject_Repr(o);
/*  PYSCDC_TRACE_CALL(r, "pyscdc_object_log_cout_print: PyObject_%s: ", ((str)?"Str":"Repr"));*/

  if (!r) PyErr_Print();

  pyscdc_parseret = PyArg_Parse(r, "s", &s);
/*  PYSCDC_TRACE_PARSE("pyscdc_object_log_cout_print: ");*/

  PYSCDC_LIBSCDC_BEGIN();
  scdc_log_cout_printf(s);
  PYSCDC_LIBSCDC_END();
}


static PyObject *pyscdc_log_cout_write(PyObject *self, PyObject *pyargs)
{
  const char *s;
  scdcint_t n;

  PYSCDC_LIBSCDC_DEF()


  PyArg_ParseTuple(pyargs, "s", &s);

  PYSCDC_LIBSCDC_BEGIN();
  n = scdc_log_cout_printf(s);
  PYSCDC_LIBSCDC_END();

  if (n < 0) Py_RETURN_FALSE;

  Py_RETURN_NONE;  
}


static PyObject *pyscdc_log_cerr_write(PyObject *self, PyObject *pyargs)
{
  const char *s;
  scdcint_t n;

  PYSCDC_LIBSCDC_DEF()


  PyArg_ParseTuple(pyargs, "s", &s);

  PYSCDC_LIBSCDC_BEGIN();
  n = scdc_log_cerr_printf(s);
  PYSCDC_LIBSCDC_END();

  if (n < 0) Py_RETURN_FALSE;

  Py_RETURN_NONE;
}


static PyObject *pyscdc_CONFIG(PyObject *self, PyObject *pyargs)
{
  return Py_BuildValue("(OOOOOO)",
#if HAVE_PYSCDC_INFO
    Py_True
#else
    Py_False
#endif
    ,
#if HAVE_PYSCDC_TRACE
    Py_True
#else
    Py_False
#endif
    ,
#if HAVE_PYSCDC_FAIL
    Py_True
#else
    Py_False
#endif
    ,
#if HAVE_PYSCDC_ERROR
    Py_True
#else
    Py_False
#endif
    ,
#if HAVE_PYSCDC_ASSERT
    Py_True
#else
    Py_False
#endif
    ,
#if HAVE_PYSCDC_DEBUG
    Py_True
#else
    Py_False
#endif
    );
}


static PyObject *pyscdc_CONSTANTS(PyObject *self, PyObject *pyargs)
{
  return Py_BuildValue("O" "cccc" "iiiiii",
    Py_None,
    SCDC_DATASET_INOUT_TOTAL_SIZE_GIVEN_EXACT,
    SCDC_DATASET_INOUT_TOTAL_SIZE_GIVEN_AT_LEAST,
    SCDC_DATASET_INOUT_TOTAL_SIZE_GIVEN_AT_MOST,
    SCDC_DATASET_INOUT_TOTAL_SIZE_GIVEN_NONE,
    SCDC_NODEPORT_START_NONE,
    SCDC_NODEPORT_START_LOOP_UNTIL_CANCEL,
    SCDC_NODEPORT_START_LOOP_UNTIL_IDLE,
    SCDC_NODEPORT_START_ASYNC_UNTIL_CANCEL,
    SCDC_NODEPORT_START_ASYNC_UNTIL_IDLE,
    PYSCDC_PYBUF_DEFAULT_SIZE
  );
}


static PyObject *pyscdc_cptr2py(PyObject *self, PyObject *pyargs)
{
  const char *type;
  char f[16];
  Py_ssize_t size;
  pyscdc_cptr_t cptr = PYSCDC_CPTR_NULL;


  PYSCDC_TRACE_OBJECT(pyargs, "pyscdc_cptr2str: pyargs: ");

  if (PyTuple_Size(pyargs) <= 2)
  {
    pyscdc_parseret = PyArg_ParseTuple(pyargs, PYSCDC_CPTR_FMT "z", &cptr, &type);
    PYSCDC_TRACE_PARSE("pyscdc_cptr2py: ");

    sprintf(f, "%s", type);
    return Py_BuildValue(f, (char *) PYSCDC_CPTR_TO_C(cptr));

  } else
  {
    pyscdc_parseret = PyArg_ParseTuple(pyargs, PYSCDC_CPTR_FMT "zn", &cptr, &type, &size);
    PYSCDC_TRACE_PARSE("pyscdc_cptr2py: ");

    sprintf(f, "%s#", type);
    return Py_BuildValue(f, (char *) PYSCDC_CPTR_TO_C(cptr), size);
  }
}


static PyObject *pyscdc_py2cptr(PyObject *self, PyObject *pyargs)
{
  const char *type;
  void *ptr;


  PYSCDC_TRACE_OBJECT(pyargs, "pyscdc_py2cptr: pyargs: ");

  pyscdc_parseret = PyArg_Parse(PyTuple_GetItem(pyargs, 1), "z", &type);
  PYSCDC_TRACE_PARSE("pyscdc_py2cptr: ");

  pyscdc_parseret = PyArg_Parse(PyTuple_GetItem(pyargs, 0), type, &ptr);
  PYSCDC_TRACE_PARSE("pyscdc_py2cptr: ");

  return Py_BuildValue(PYSCDC_CPTR_FMT, PYSCDC_CPTR_TO_PY(ptr));
}


static PyObject *pyscdc_pybuf(PyObject *self, PyObject *pyargs)
{
  void *buf = 0;
  scdcint_t size = PYSCDC_PYBUF_DEFAULT_SIZE;
  PyObject *pybuf;


  PYSCDC_TRACE_OBJECT(pyargs, "pyscdc_pybuf: pyargs: ");

  if (PyTuple_Size(pyargs) > 0)
  {
    pyscdc_parseret = PyArg_Parse(PyTuple_GetItem(pyargs, 0), PYSCDCINT_FMT, &size);
    PYSCDC_TRACE_PARSE("pyscdc_pybuf: ");
  }

  PYSCDC_TRACE("pyscdc_pybuf: size: %" scdcint_fmt, size);

#if PY_MAJOR_VERSION >= 3
  buf = PyBytes_AsString(PyUnicode_FromStringAndSize(NULL, size));
#else
  buf = PyString_AsString(PyString_FromStringAndSize(NULL, size));
#endif

  pybuf = PyObject_CallMethod(pyscdc_objs[PYSCDC_OBJ_SCDC], "cbuf", PYSCDC_CPTR_FMT, PYSCDC_CPTR_TO_PY(buf));
  PYSCDC_TRACE_CALL(pybuf, "pyscdc_pybuf: pybuf: ");

  return pybuf;
}


static PyObject *pyscdc_dataset_input_struct2class(scdc_dataset_input_t *input, PyObject *pyinput, PyObject *next)
{
  PyObject *pyinput_buf, *pyinput_next, *pyinput_data;


  PYSCDC_TRACE_DATASET_INPUT(input, "pyscdc_dataset_input_struct2class: input: '%p', ", input);
  PYSCDC_TRACE_OBJECT(next, "pyscdc_dataset_input_struct2class: pyinput: %p, next: ", pyinput);

  if (input == NULL) Py_RETURN_NONE;

  if (!pyinput)
  {
    pyinput = PyObject_CallMethod(pyscdc_objs[PYSCDC_OBJ_SCDC], "dataset_input", "");
    PYSCDC_TRACE_CALL(pyinput, "pyscdc_dataset_input_struct2class: dataset_input(): ");
  }

  pyinput_buf = PyObject_CallMethod(pyscdc_objs[PYSCDC_OBJ_SCDC], "cbuf", PYSCDC_CPTR_FMT, PYSCDC_CPTR_TO_PY(SCDC_DATASET_INOUT_BUF_PTR(input)));
  PYSCDC_TRACE_CALL(pyinput, "pyscdc_dataset_input_struct2class: cbuf(): ");

  pyinput_next = pyinput_data = Py_None;

  if (!PYSCDC_NEXT_IS_IGNORE(next))
  {
    if (PYSCDC_NEXT_IS_PACK(next))
    {
      /* pack -> unpack of pyscdc_dataset_input_class2struct */
      pyinput_next = (input->next != NULL)?next:Py_None;
      pyinput_data = Py_BuildValue("(OO)",
        PyObject_CallMethod(pyscdc_objs[PYSCDC_OBJ_SCDC], "cptr", PYSCDC_CPTR_FMT, PYSCDC_CPTR_TO_PY(input->next)),
        PyObject_CallMethod(pyscdc_objs[PYSCDC_OBJ_SCDC], "cptr", PYSCDC_CPTR_FMT, PYSCDC_CPTR_TO_PY(input->data)));

    } else
    {
      /* unpack -> pack of pyscdc_dataset_input_class2struct */
      /* keep next and data of given pyinput */
      pyinput_next = PyObject_GetAttrString(pyinput, "next");
      pyinput_data = PyObject_GetAttrString(pyinput, "data");
    }
  }

  pyscdc_ret = PyObject_CallMethod(pyinput, "set_all", "((zO" PYSCDCINT_FMT PYSCDCINT_FMT "c" PYSCDCINT_FMT "OO" PYSCDC_CPTR_FMT "))",
    input->format, pyinput_buf, SCDC_DATASET_INOUT_BUF_SIZE(input), input->total_size, input->total_size_given, SCDC_DATASET_INOUT_BUF_CURRENT(input), pyinput_next, pyinput_data, PYSCDC_CPTR_TO_PY(input->intern));
  PYSCDC_TRACE_CALL(pyscdc_ret, "pyscdc_dataset_input_struct2class: set_all: ");

  PYSCDC_TRACE_DATASET_PYINPUT(pyinput, "pyscdc_dataset_input_struct2class: pyinput: ");

  return pyinput;
}


static scdc_dataset_input_t *pyscdc_dataset_input_class2struct(PyObject *pyinput, scdc_dataset_input_t *input, scdc_dataset_inout_next_f *next, scdcint_t parse_buf)
{
  const char *format;
  PyObject *pyinput_all, *pyinput_buf, *pyinput_next, *pyinput_data, *pyret;
  Py_ssize_t buf_len;
  pyscdc_cptr_t pyinput_intern, cptr_next, cptr_data, cptr_buf;


  PYSCDC_TRACE_DATASET_PYINPUT(pyinput, "pyscdc_dataset_input_class2struct: pyinput: '%p', ", pyinput);
  PYSCDC_TRACE("pyscdc_dataset_input_class2struct: input: %p, next: %p, parse_buf: %" scdcint_fmt, input, next, parse_buf);

  if (pyinput == Py_None) return NULL;

  pyinput_all = PyObject_CallMethod(pyinput, "get_all", "");
  PYSCDC_TRACE_CALL(pyinput_all, "pyscdc_dataset_input_class2struct: get_all: ");

  pyscdc_parseret = PyArg_ParseTuple(pyinput_all, "zO" PYSCDCINT_FMT PYSCDCINT_FMT "c" PYSCDCINT_FMT "OO" PYSCDC_CPTR_FMT,
    &format, &pyinput_buf, &SCDC_DATASET_INOUT_BUF_SIZE(input), &input->total_size, &input->total_size_given, &SCDC_DATASET_INOUT_BUF_CURRENT(input), &pyinput_next, &pyinput_data, &pyinput_intern);
  PYSCDC_TRACE_PARSE("pyscdc_dataset_input_class2struct: pyinput_all: ");

  strncpy(input->format, (format)?format:"", sizeof(input->format));

  if (PyObject_IsInstance(pyinput_buf, pyscdc_objs[PYSCDC_OBJ_CBUF]))
  {
    PYSCDC_TRACE("pyscdc_dataset_input_class2struct: cbuf buf");
    pyscdc_parseret = PyArg_Parse(PyObject_CallMethod(pyinput_buf, "get_c", NULL), PYSCDC_CPTR_FMT, &cptr_buf);
    PYSCDC_TRACE_PARSE("pyscdc_dataset_input_class2struct: ");

    SCDC_DATASET_INOUT_BUF_PTR(input) = (void *) PYSCDC_CPTR_TO_C(cptr_buf);

  } else if (parse_buf)
  {
    PYSCDC_TRACE("pyscdc_dataset_input_class2struct: string buf");
    pyscdc_parseret = PyArg_ParseTuple(PyTuple_Pack(1, pyinput_buf), "z#", &SCDC_DATASET_INOUT_BUF_PTR(input), &buf_len);
    PYSCDC_TRACE_PARSE("pyscdc_dataset_input_class2struct: ");

    SCDC_DATASET_INOUT_BUF_SIZE(input) = buf_len;

  } else
  {
    SCDC_DATASET_INOUT_BUF_PTR(input) = NULL;
    SCDC_DATASET_INOUT_BUF_SIZE(input) = 0;
  }

  PYSCDC_ASSERT(SCDC_DATASET_INOUT_BUF_SIZE(input) >= SCDC_DATASET_INOUT_BUF_CURRENT(input));

  input->next = input->data = NULL;

  if (!PYSCDC_NEXT_IS_IGNORE(next))
  {
    if (PYSCDC_NEXT_IS_PACK(next))
    {
      /* pack -> unpack of pyscdc_dataset_input_struct2class */
      input->next = (pyinput_next != Py_None)?next:NULL;
      input->data = pyinput;

    } else if (pyinput_data != Py_None)
    {
      /* unpack -> pack of pyscdc_dataset_input_struct2class */
      pyret = PyObject_CallMethod(PyTuple_GetItem(pyinput_data, 0), "get_c", NULL);
      PYSCDC_TRACE_CALL("pyscdc_dataset_input_class2struct: get_c: ");

      pyscdc_parseret = PyArg_Parse(pyret, PYSCDC_CPTR_FMT, &cptr_next);
      PYSCDC_TRACE_PARSE("pyscdc_dataset_input_class2struct: ");

      pyret = PyObject_CallMethod(PyTuple_GetItem(pyinput_data, 1), "get_c", NULL);
      PYSCDC_TRACE_CALL("pyscdc_dataset_input_class2struct: get_c: ");

      pyscdc_parseret = PyArg_Parse(pyret, PYSCDC_CPTR_FMT, &cptr_data);
      PYSCDC_TRACE_PARSE("pyscdc_dataset_input_class2struct: ");

      input->next = (scdc_dataset_inout_next_f *) PYSCDC_CPTR_TO_C(cptr_next);
      input->data = (void *) PYSCDC_CPTR_TO_C(cptr_data);
    }
  }

  input->intern = (void *) PYSCDC_CPTR_TO_C(pyinput_intern);

  return input;
}


static PyObject *pyscdc_dataset_output_struct2class(scdc_dataset_output_t *output, PyObject *pyoutput, PyObject *next)
{
  PyObject *pyoutput_buf, *pyoutput_next, *pyoutput_data;


  PYSCDC_TRACE_DATASET_OUTPUT(output, "pyscdc_dataset_output_struct2class: output: '%p', ", output);
  PYSCDC_TRACE_OBJECT(next, "pyscdc_dataset_output_struct2class: pyoutput: %p, next: ", pyoutput);

  if (output == NULL) Py_RETURN_NONE;

  if (!pyoutput)
  {
    pyoutput = PyObject_CallMethod(pyscdc_objs[PYSCDC_OBJ_SCDC], "dataset_output", "");
    PYSCDC_TRACE_CALL(pyoutput, "pyscdc_dataset_output_struct2class: dataset_output(): ");
  }

  pyoutput_buf = PyObject_CallMethod(pyscdc_objs[PYSCDC_OBJ_SCDC], "cbuf", PYSCDC_CPTR_FMT, PYSCDC_CPTR_TO_PY(SCDC_DATASET_INOUT_BUF_PTR(output)));
  PYSCDC_TRACE_CALL(pyoutput, "pyscdc_dataset_output_struct2class: cbuf(): ");

  pyoutput_next = pyoutput_data = Py_None;

  if (!PYSCDC_NEXT_IS_IGNORE(next))
  {
    if (PYSCDC_NEXT_IS_PACK(next))
    {
      /* pack -> unpack of pyscdc_dataset_output_class2struct */
      pyoutput_next = (output->next != NULL)?next:Py_None;
      pyoutput_data = Py_BuildValue("(OO)",
        PyObject_CallMethod(pyscdc_objs[PYSCDC_OBJ_SCDC], "cptr", PYSCDC_CPTR_FMT, PYSCDC_CPTR_TO_PY(output->next)),
        PyObject_CallMethod(pyscdc_objs[PYSCDC_OBJ_SCDC], "cptr", PYSCDC_CPTR_FMT, PYSCDC_CPTR_TO_PY(output->data)));

    } else
    {
      /* unpack -> pack of pyscdc_dataset_output_class2struct */
      /* keep next and data of given pyoutput */
      pyoutput_next = PyObject_GetAttrString(pyoutput, "next");
      pyoutput_data = PyObject_GetAttrString(pyoutput, "data");
    }
  }

  pyscdc_ret = PyObject_CallMethod(pyoutput, "set_all", "((zO" PYSCDCINT_FMT PYSCDCINT_FMT "c" PYSCDCINT_FMT "OO" PYSCDC_CPTR_FMT "))",
    output->format, pyoutput_buf, SCDC_DATASET_INOUT_BUF_SIZE(output), output->total_size, output->total_size_given, SCDC_DATASET_INOUT_BUF_CURRENT(output), pyoutput_next, pyoutput_data, PYSCDC_CPTR_TO_PY(output->intern));
  PYSCDC_TRACE_CALL(pyscdc_ret, "pyscdc_dataset_output_struct2class: set_all: ");

  PYSCDC_TRACE_DATASET_PYOUTPUT(pyoutput, "pyscdc_dataset_output_struct2class: pyoutput: ");

  return pyoutput;
}


static scdc_dataset_output_t *pyscdc_dataset_output_class2struct(PyObject *pyoutput, scdc_dataset_output_t *output, scdc_dataset_inout_next_f *next, scdcint_t parse_buf)
{
  const char *format;
  PyObject *pyoutput_all, *pyoutput_buf, *pyoutput_next, *pyoutput_data, *pyret;
  Py_ssize_t buf_len;
  pyscdc_cptr_t pyoutput_intern, cptr_next, cptr_data, cptr_buf;


  PYSCDC_TRACE_DATASET_PYOUTPUT(pyoutput, "pyscdc_dataset_output_class2struct: pyoutput: '%p', ", pyoutput);
  PYSCDC_TRACE("pyscdc_dataset_output_class2struct: output: %p, next: %p, parse_buf: %" scdcint_fmt, output, next, parse_buf);

  if (pyoutput == Py_None) return NULL;

  pyoutput_all = PyObject_CallMethod(pyoutput, "get_all", "");
  PYSCDC_TRACE_CALL(pyoutput_all, "pyscdc_dataset_output_class2struct: get_all: ");

  pyscdc_parseret = PyArg_ParseTuple(pyoutput_all, "zO" PYSCDCINT_FMT PYSCDCINT_FMT "c" PYSCDCINT_FMT "OO" PYSCDC_CPTR_FMT,
    &format, &pyoutput_buf, &SCDC_DATASET_INOUT_BUF_SIZE(output), &output->total_size, &output->total_size_given, &SCDC_DATASET_INOUT_BUF_CURRENT(output), &pyoutput_next, &pyoutput_data, &pyoutput_intern);
  PYSCDC_TRACE_PARSE("pyscdc_dataset_output_class2struct: pyoutput_all: ");

  strncpy(output->format, (format)?format:"", sizeof(output->format));

  if (PyObject_IsInstance(pyoutput_buf, pyscdc_objs[PYSCDC_OBJ_CBUF]))
  {
    PYSCDC_TRACE("pyscdc_dataset_output_class2struct: cbuf buf");
    pyscdc_parseret = PyArg_Parse(PyObject_CallMethod(pyoutput_buf, "get_c", NULL), PYSCDC_CPTR_FMT, &cptr_buf);
    PYSCDC_TRACE_PARSE("pyscdc_dataset_output_class2struct: ");

    SCDC_DATASET_INOUT_BUF_PTR(output) = (void *) PYSCDC_CPTR_TO_C(cptr_buf);

  } else if (parse_buf)
  {
    PYSCDC_TRACE("pyscdc_dataset_output_class2struct: string buf");
    pyscdc_parseret = PyArg_ParseTuple(PyTuple_Pack(1, pyoutput_buf), "z#", &SCDC_DATASET_INOUT_BUF_PTR(output), &buf_len);
    PYSCDC_TRACE_PARSE("pyscdc_dataset_output_class2struct: ");

    SCDC_DATASET_INOUT_BUF_SIZE(output) = buf_len;

  } else
  {
    SCDC_DATASET_INOUT_BUF_PTR(output) = NULL;
    SCDC_DATASET_INOUT_BUF_SIZE(output) = 0;
  }

  PYSCDC_ASSERT(SCDC_DATASET_INOUT_BUF_SIZE(output) >= SCDC_DATASET_INOUT_BUF_CURRENT(output));

  output->next = output->data = NULL;

  if (!PYSCDC_NEXT_IS_IGNORE(next))
  {
    if (PYSCDC_NEXT_IS_PACK(next))
    {
      /* pack -> unpack of pyscdc_dataset_output_struct2class */
      output->next = (pyoutput_next != Py_None)?next:NULL;
      output->data = pyoutput;

    } else if (pyoutput_data != Py_None)
    {
      /* unpack -> pack of pyscdc_dataset_output_struct2class */
      pyret = PyObject_CallMethod(PyTuple_GetItem(pyoutput_data, 0), "get_c", NULL);
      PYSCDC_TRACE_CALL("pyscdc_dataset_output_class2struct: get_c: ");

      pyscdc_parseret = PyArg_Parse(pyret, PYSCDC_CPTR_FMT, &cptr_next);
      PYSCDC_TRACE_PARSE("pyscdc_dataset_output_class2struct: ");

      pyret = PyObject_CallMethod(PyTuple_GetItem(pyoutput_data, 1), "get_c", NULL);
      PYSCDC_TRACE_CALL("pyscdc_dataset_output_class2struct: get_c: ");

      pyscdc_parseret = PyArg_Parse(pyret, PYSCDC_CPTR_FMT, &cptr_data);
      PYSCDC_TRACE_PARSE("pyscdc_dataset_output_class2struct: ");

      output->next = (scdc_dataset_inout_next_f *) PYSCDC_CPTR_TO_C(cptr_next);
      output->data = (void *) PYSCDC_CPTR_TO_C(cptr_data);
    }
  }

  output->intern = (void *) PYSCDC_CPTR_TO_C(pyoutput_intern);

  return output;
}


static scdcint_t pyscdc_dataset_input_next_struct(scdc_dataset_input_t *input)
{
  PyObject *pyinput, *pynext, *pyres, *pyret;

  PYSCDC_LIBSCDC_PY_DEF()


  PYSCDC_LIBSCDC_PY_BEGIN();

  PYSCDC_TRACE_DATASET_INPUT(input, "pyscdc_dataset_input_next_struct: IN input: ");

  pyinput = input->data;

  pyinput = pyscdc_dataset_input_struct2class(input, pyinput, PYSCDC_NEXT_UNPACK);
  PYSCDC_TRACE_DATASET_PYINPUT(pyinput, "pyscdc_dataset_input_next_struct: IN pyinput: ");

  pynext = PyObject_GetAttrString(pyinput, "next");

  if (pynext != Py_None)
  {
    pyres = PyObject_CallFunction(pynext, "(O)", pyinput);
    PYSCDC_TRACE_CALL(pyres, "pyscdc_dataset_input_next_struct: ");

    pyscdc_parseret = PyArg_Parse(pyres, "O", &pyret);
    PYSCDC_TRACE_PARSE("pyscdc_dataset_input_next_struct: ");

  } else pyret = Py_False;

  PYSCDC_TRACE_OBJECT(pyret, "pyscdc_dataset_input_next_struct: next return: ");
  PYSCDC_TRACE_DATASET_PYINPUT(pyinput, "pyscdc_dataset_input_next_struct: OUT pyinput: ");

  input = pyscdc_dataset_input_class2struct(pyinput, input, PYSCDC_NEXT_PACK(pyscdc_dataset_input_next_struct), 1);
  PYSCDC_TRACE_DATASET_INPUT(input, "pyscdc_dataset_input_next_struct: OUT input: ");

  PYSCDC_LIBSCDC_PY_END();

  return (pyret == Py_True)?SCDC_SUCCESS:SCDC_FAILURE;
}


static scdcint_t pyscdc_dataset_output_next_struct(scdc_dataset_output_t *output)
{
  PyObject *pyoutput, *pynext, *pyres, *pyret;

  PYSCDC_LIBSCDC_PY_DEF()


  PYSCDC_LIBSCDC_PY_BEGIN();

  PYSCDC_TRACE_DATASET_OUTPUT(output, "pyscdc_dataset_output_next_struct: IN output: ");

  pyoutput = output->data;

  pyoutput = pyscdc_dataset_output_struct2class(output, pyoutput, PYSCDC_NEXT_UNPACK);
  PYSCDC_TRACE_DATASET_PYOUTPUT(pyoutput, "pyscdc_dataset_output_next_struct: IN pyoutput: ");

  pynext = PyObject_GetAttrString(pyoutput, "next");

  if (pynext != Py_None)
  {
    pyres = PyObject_CallFunction(pynext, "(O)", pyoutput);
    PYSCDC_TRACE_CALL(pyres, "pyscdc_dataset_output_next_struct: ");

    pyscdc_parseret = PyArg_Parse(pyres, "O", &pyret);
    PYSCDC_TRACE_PARSE("pyscdc_dataset_output_next_struct: ");

  } else pyret = Py_False;

  PYSCDC_TRACE_OBJECT(pyret, "pyscdc_dataset_output_next_struct: next return: ");
  PYSCDC_TRACE_DATASET_PYOUTPUT(pyoutput, "pyscdc_dataset_output_next_struct: OUT pyoutput: ");

  output = pyscdc_dataset_output_class2struct(pyoutput, output, PYSCDC_NEXT_PACK(pyscdc_dataset_output_next_struct), 1);
  PYSCDC_TRACE_DATASET_OUTPUT(output, "pyscdc_dataset_output_next_struct: OUT output: ");

  PYSCDC_LIBSCDC_PY_END();

  return (pyret == Py_True)?SCDC_SUCCESS:SCDC_FAILURE;
}


static PyObject *pyscdc_dataset_input_next_class(PyObject *self, PyObject *pyargs)
{
  PyObject *pyinput, *next;
  scdc_dataset_input_t _input, *input = &_input;
  scdcint_t ret;

  PYSCDC_LIBSCDC_DEF()


  pyinput = PyTuple_GetItem(pyargs, 0);

  PYSCDC_TRACE_DATASET_PYINPUT(pyinput, "pyscdc_dataset_input_next_class: IN pyinput: ");

  input = pyscdc_dataset_input_class2struct(pyinput, input, PYSCDC_NEXT_UNPACK, 1);
  PYSCDC_TRACE_DATASET_INPUT(input, "pyscdc_dataset_input_next_class: IN input: ");

  PYSCDC_LIBSCDC_BEGIN();
  ret = input->next(input);
  PYSCDC_LIBSCDC_END();

  PYSCDC_TRACE("pyscdc_dataset_input_next_class: next: %" scdcint_fmt, ret);

  PYSCDC_TRACE_DATASET_INPUT(input, "pyscdc_dataset_input_next_class: OUT input: ");

  next = PyObject_GetAttrString(pyscdc_objs[PYSCDC_OBJ_SCDCMOD], "dataset_input_next_class");

  pyinput = pyscdc_dataset_input_struct2class(input, pyinput, PYSCDC_NEXT_PACK(next));
  PYSCDC_TRACE_DATASET_PYINPUT(pyinput, "pyscdc_dataset_input_next_struct: OUT pyinput: ");

  PYSCDC_RETURN_BOOL(ret);
}


static PyObject *pyscdc_dataset_output_next_class(PyObject *self, PyObject *pyargs)
{
  PyObject *pyoutput, *next;
  scdc_dataset_output_t _output, *output = &_output;
  scdcint_t ret;

  PYSCDC_LIBSCDC_DEF()


  pyoutput = PyTuple_GetItem(pyargs, 0);

  PYSCDC_TRACE_DATASET_PYOUTPUT(pyoutput, "pyscdc_dataset_output_next_class: IN pyoutput: ");

  output = pyscdc_dataset_output_class2struct(pyoutput, output, PYSCDC_NEXT_UNPACK, 1);
  PYSCDC_TRACE_DATASET_OUTPUT(output, "pyscdc_dataset_output_next_class: IN output: ");

  PYSCDC_LIBSCDC_BEGIN();
  ret = output->next(output);
  PYSCDC_LIBSCDC_END();

  PYSCDC_TRACE("pyscdc_dataset_output_next_class: next: %" scdcint_fmt, ret);

  PYSCDC_TRACE_DATASET_OUTPUT(output, "pyscdc_dataset_output_next_class: OUT output: ");

  next = PyObject_GetAttrString(pyscdc_objs[PYSCDC_OBJ_SCDCMOD], "dataset_output_next_class");

  pyoutput = pyscdc_dataset_output_struct2class(output, pyoutput, PYSCDC_NEXT_PACK(next));
  PYSCDC_TRACE_DATASET_PYOUTPUT(pyoutput, "pyscdc_dataset_output_next_struct: OUT pyoutput: ");

  PYSCDC_RETURN_BOOL(ret);
}


typedef struct _pyscdc_dataprov_hook_t
{
  PyObject *dataprov;
  PyObject *open, *close, *config, *dataset_open, *dataset_close, *dataset_open_read_state, *dataset_close_write_state, *dataset_cmd;

} pyscdc_dataprov_hook_t;


static pyscdc_dataprov_hook_t *pyscdc_dataprov_hook_init(PyObject *hook_class)
{
  pyscdc_dataprov_hook_t *pyscdc_dataprov_hook = malloc(sizeof(pyscdc_dataprov_hook_t));

  pyscdc_dataprov_hook->open = PyObject_GetAttrString(hook_class, "open");
  pyscdc_dataprov_hook->close = PyObject_GetAttrString(hook_class, "close");
  pyscdc_dataprov_hook->config = PyObject_GetAttrString(hook_class, "config");

  pyscdc_dataprov_hook->dataset_open = PyObject_GetAttrString(hook_class, "dataset_open");
  pyscdc_dataprov_hook->dataset_close = PyObject_GetAttrString(hook_class, "dataset_close");

  pyscdc_dataprov_hook->dataset_open_read_state = PyObject_GetAttrString(hook_class, "dataset_open_read_state");
  pyscdc_dataprov_hook->dataset_close_write_state = PyObject_GetAttrString(hook_class, "dataset_close_write_state");

  pyscdc_dataprov_hook->dataset_cmd = PyObject_GetAttrString(hook_class, "dataset_cmd");

  return pyscdc_dataprov_hook;
}


static void pyscdc_dataprov_hook_release(pyscdc_dataprov_hook_t *pyscdc_dataprov_hook)
{
  Py_XDECREF(pyscdc_dataprov_hook->open);
  Py_XDECREF(pyscdc_dataprov_hook->close);
  Py_XDECREF(pyscdc_dataprov_hook->config);

  Py_XDECREF(pyscdc_dataprov_hook->dataset_open);
  Py_XDECREF(pyscdc_dataprov_hook->dataset_close);

  Py_XDECREF(pyscdc_dataprov_hook->dataset_open_read_state);
  Py_XDECREF(pyscdc_dataprov_hook->dataset_close_write_state);

  Py_XDECREF(pyscdc_dataprov_hook->dataset_cmd);

  free(pyscdc_dataprov_hook);
}


static void *pyscdc_dataprov_hook_open_intern(void *intern_data, const char *conf, scdc_args_t *args, scdcint_t *ret)
{
  PyObject *hook_class = intern_data;
  pyscdc_args_data_py_t *args_data_py = args->data;
  pyscdc_dataprov_hook_t *pyscdc_dataprov_hook = NULL;
  scdcint_t i;
  PyObject *open_args = NULL;

  PYSCDC_LIBSCDC_PY_DEF()


  PYSCDC_LIBSCDC_PY_BEGIN();

  PYSCDC_TRACE("pyscdc_dataprov_hook_open_data: intern_data: '%p', conf: '%s', args: %p", intern_data, conf, args);
  PYSCDC_TRACE_OBJECT(hook_class, "pyscdc_dataprov_hook_open_data: hook_class:");

  pyscdc_dataprov_hook = pyscdc_dataprov_hook_init(hook_class);

  pyscdc_dataprov_hook->dataprov = Py_None;

  *ret = SCDC_SUCCESS;

  if (pyscdc_dataprov_hook->open && pyscdc_dataprov_hook->open != Py_None)
  {
    PYSCDC_TRACE("pyscdc_dataprov_hook_open_data: nargs: %" scdcint_fmt, args_data_py->nargs - args_data_py->arg);
    open_args = PyTuple_New(1 + args_data_py->nargs - args_data_py->arg);
    PyTuple_SET_ITEM(open_args, 0, Py_BuildValue("s", conf));
    for (i = 0; i < args_data_py->nargs - args_data_py->arg; ++i) PyTuple_SET_ITEM(open_args, i + 1, args_data_py->args[args_data_py->arg + i]);

    pyscdc_dataprov_hook->dataprov = PyObject_CallObject(pyscdc_dataprov_hook->open, open_args);
    PYSCDC_TRACE_CALL(pyscdc_dataprov_hook->dataprov, "pyscdc_dataprov_hook_open: ");
    PYSCDC_TRACE_OBJECT(pyscdc_dataprov_hook->dataprov, "pyscdc_dataprov_hook_open: dataprov: ");

    if (pyscdc_dataprov_hook->dataprov == Py_False)
    {
      *ret = SCDC_FAILURE;

      pyscdc_dataprov_hook_release(pyscdc_dataprov_hook);
      pyscdc_dataprov_hook = NULL;
    }
  }

  PYSCDC_TRACE("pyscdc_dataprov_hook_open_data: return: %p", pyscdc_dataprov_hook);

  PYSCDC_LIBSCDC_PY_END();

  return pyscdc_dataprov_hook;
}


static scdcint_t pyscdc_dataprov_hook_close(void *dataprov)
{
  pyscdc_dataprov_hook_t *pyscdc_dataprov_hook = dataprov;
  PyObject *result = NULL;
  scdcint_t ret = SCDC_SUCCESS;

  PYSCDC_LIBSCDC_PY_DEF()


  PYSCDC_LIBSCDC_PY_BEGIN();

  PYSCDC_TRACE("pyscdc_dataprov_hook_close_data: dataprov: %p", dataprov);
  PYSCDC_TRACE_OBJECT(pyscdc_dataprov_hook->dataprov, "pyscdc_dataprov_hook_close: dataprov: ");

  if (pyscdc_dataprov_hook->close && pyscdc_dataprov_hook->close != Py_None)
  {
    result = PyObject_CallFunction(pyscdc_dataprov_hook->close, "O", pyscdc_dataprov_hook->dataprov);
    PYSCDC_TRACE_CALL(result, "pyscdc_dataprov_hook_close_data: ");

    ret = (result == Py_True)?SCDC_SUCCESS:SCDC_FAILURE;
  }

  pyscdc_dataprov_hook_release(pyscdc_dataprov_hook);

  PYSCDC_TRACE("pyscdc_dataprov_hook_close_data: return '%" scdcint_fmt "'", ret);

  PYSCDC_LIBSCDC_PY_END();

  return ret;
}


static scdcint_t pyscdc_dataprov_hook_config(void *dataprov, const char *cmd, const char *param, const char *val, scdcint_t val_size, char **result, scdcint_t *result_size)
{
  pyscdc_dataprov_hook_t *pyscdc_dataprov_hook = dataprov;
  PyObject *pyresult = NULL;
  Py_buffer pybuf;
  scdcint_t ret;

  PYSCDC_LIBSCDC_PY_DEF()


  PYSCDC_LIBSCDC_PY_BEGIN();

  PYSCDC_TRACE("pyscdc_dataprov_hook_config: dataprov: %p, cmd: '%s', param: '%s', val: '%*.s'", dataprov, cmd, param, (int) val_size, val);
  PYSCDC_TRACE_OBJECT(pyscdc_dataprov_hook->dataprov, "pyscdc_dataprov_hook_close: dataprov: ");

  pyresult = PyObject_CallFunction(pyscdc_dataprov_hook->config, "Ossz#", pyscdc_dataprov_hook->dataprov, cmd, param, val, (Py_ssize_t) val_size);
  PYSCDC_TRACE_CALL(result, "pyscdc_dataprov_hook_config: ");

  if (pyresult != Py_False)
  {
    if (result && result_size)
    {
      if (pyresult != Py_True)
      {
        pyscdc_parseret = PyArg_Parse(pyresult, "z*", &pybuf);
        PYSCDC_TRACE_PARSE("pyscdc_dataprov_hook_config: ");

        *result_size = z_min(pybuf.len, *result_size - 1);

        memcpy(*result, pybuf.buf, *result_size);

      } else *result_size = 0;

      (*result)[*result_size] = '\0';

      PYSCDC_TRACE("pyscdc_dataprov_hook_config: result: '%s'", *result);
    }

    ret = SCDC_SUCCESS;

  } else ret = SCDC_FAILURE;

  PYSCDC_TRACE("pyscdc_dataprov_hook_config: return '%" scdcint_fmt "'", ret);

  PYSCDC_LIBSCDC_PY_END();

  return ret;
}


static void *pyscdc_dataprov_hook_dataset_open(void *dataprov, const char *path)
{
  pyscdc_dataprov_hook_t *pyscdc_dataprov_hook = dataprov;
  PyObject *pyscdc_dataset = NULL;

  PYSCDC_LIBSCDC_PY_DEF()


  PYSCDC_LIBSCDC_PY_BEGIN();

  PYSCDC_TRACE("pyscdc_dataprov_hook_dataset_open: dataprov: %p, path: '%s'", dataprov, path);
  PYSCDC_TRACE_OBJECT(pyscdc_dataprov_hook->dataprov, "pyscdc_dataprov_hook_dataset_open: dataprov: ");

  pyscdc_dataset = PyObject_CallFunction(pyscdc_dataprov_hook->dataset_open, "Os", pyscdc_dataprov_hook->dataprov, path);
  PYSCDC_TRACE_CALL(pyscdc_dataset, "pyscdc_dataprov_hook_close: ");

  PYSCDC_TRACE("pyscdc_dataprov_hook_dataset_open: return: %p", pyscdc_dataset);

  PYSCDC_LIBSCDC_PY_END();

  return pyscdc_dataset;
}


static scdcint_t pyscdc_dataprov_hook_dataset_close(void *dataprov, void *dataset)
{
  pyscdc_dataprov_hook_t *pyscdc_dataprov_hook = dataprov;
  PyObject *pyscdc_dataset = dataset;
  PyObject *result;
  scdcint_t ret;

  PYSCDC_LIBSCDC_PY_DEF()


  PYSCDC_LIBSCDC_PY_BEGIN();

  PYSCDC_TRACE("pyscdc_dataprov_hook_dataset_close: dataprov: %p, dataset: %p", dataprov, dataset);
  PYSCDC_TRACE_OBJECT(pyscdc_dataprov_hook->dataprov, "pyscdc_dataprov_hook_dataset_close: dataprov: ");
  PYSCDC_TRACE_OBJECT(pyscdc_dataset, "pyscdc_dataprov_hook_dataset_close: dataset: ");

  result = PyObject_CallFunction(pyscdc_dataprov_hook->dataset_close, "OO", pyscdc_dataprov_hook->dataprov, pyscdc_dataset);
  PYSCDC_TRACE_CALL(result, "pyscdc_dataprov_hook_dataset_close: ");

  ret = (result == Py_True)?SCDC_SUCCESS:SCDC_FAILURE;

  PYSCDC_TRACE("pyscdc_dataprov_hook_dataset_close: return: '%" scdcint_fmt "'", ret);

  PYSCDC_LIBSCDC_PY_END();

  return ret;
}


static void *pyscdc_dataprov_hook_dataset_open_read_state(void *dataprov, const char *buf, scdcint_t buf_size)
{
  pyscdc_dataprov_hook_t *pyscdc_dataprov_hook = dataprov;
  PyObject *pyscdc_dataset = NULL;

  PYSCDC_LIBSCDC_PY_DEF()


  PYSCDC_LIBSCDC_PY_BEGIN();

  PYSCDC_TRACE("pyscdc_dataprov_hook_dataset_open_read_state: dataprov: %p, buf: %p, buf_size: '%" scdcint_fmt "'", dataprov, buf, buf_size);
  PYSCDC_TRACE_OBJECT(pyscdc_dataprov_hook->dataprov, "pyscdc_dataprov_hook_dataset_open_read_state: dataprov: ");

  PYSCDC_TRACE("pyscdc_dataprov_hook_dataset_open_read_state: buf: '%.*s'", (int) buf_size, buf);

  pyscdc_dataset = PyObject_CallFunction(pyscdc_dataprov_hook->dataset_open_read_state, "Oz#", pyscdc_dataprov_hook->dataprov, buf, (Py_ssize_t) buf_size);
  PYSCDC_TRACE_CALL(pyscdc_dataset, "pyscdc_dataprov_hook_dataset_open_read_state: ");

  PYSCDC_TRACE("pyscdc_dataprov_hook_dataset_open_read_state: return: %p", pyscdc_dataset);

  PYSCDC_LIBSCDC_PY_END();

  return pyscdc_dataset;
}


static scdcint_t pyscdc_dataprov_hook_dataset_close_write_state(void *dataprov, void *dataset, char *buf, scdcint_t buf_size)
{
  pyscdc_dataprov_hook_t *pyscdc_dataprov_hook = dataprov;
  PyObject *pyscdc_dataset = dataset;
  PyObject *result;
  Py_buffer pybuf;
  scdcint_t n;

  PYSCDC_LIBSCDC_PY_DEF()


  PYSCDC_LIBSCDC_PY_BEGIN();

  PYSCDC_TRACE("pyscdc_dataprov_hook_dataset_close_write_state: dataprov: %p, dataset: %p, buf: %p, buf_size: '%" scdcint_fmt "'", dataprov, dataset, buf, buf_size);
  PYSCDC_TRACE_OBJECT(pyscdc_dataprov_hook->dataprov, "pyscdc_dataprov_hook_dataset_close_write_state: dataprov: ");
  PYSCDC_TRACE_OBJECT(pyscdc_dataset, "pyscdc_dataprov_hook_dataset_close_write_state: dataset: ");

  result = PyObject_CallFunction(pyscdc_dataprov_hook->dataset_close_write_state, "OO" PYSCDCINT_FMT, pyscdc_dataprov_hook->dataprov, pyscdc_dataset, buf_size);
  PYSCDC_TRACE_CALL(result, "pyscdc_dataprov_hook_dataset_close_write_state: ");
  PYSCDC_TRACE_OBJECT(result, "pyscdc_dataprov_hook_dataset_close_write_state: result: ");

  pyscdc_parseret = PyArg_Parse(result, "z*", &pybuf);
  PYSCDC_TRACE_PARSE("pyscdc_dataprov_hook_dataset_close_write_state: ");

  n = (buf_size < pybuf.len)?buf_size:pybuf.len;

  memcpy(buf, pybuf.buf, n);

  PYSCDC_TRACE("pyscdc_dataprov_hook_dataset_close_write_state: return: '%" scdcint_fmt "'", n);

  PYSCDC_LIBSCDC_PY_END();

  return n;
}


static scdcint_t pyscdc_dataprov_hook_dataset_cmd(void *dataprov, void *dataset, const char *cmd, const char *params, scdc_dataset_input_t *input, scdc_dataset_output_t *output)
{
  pyscdc_dataprov_hook_t *pyscdc_dataprov_hook = dataprov;
  PyObject *pyscdc_dataset = (dataset)?dataset:Py_None;
  PyObject *pyinput, *pyinput_given, *pyinput_next, *pyoutput, *pyoutput_given, *pyoutput_next, *pyret;
  scdc_dataset_input_t _input_given, *input_given = &_input_given;
  scdc_dataset_output_t _output_given, *output_given = &_output_given;

  PYSCDC_LIBSCDC_PY_DEF()


  PYSCDC_LIBSCDC_PY_BEGIN();

  PYSCDC_TRACE("pyscdc_dataprov_hook_dataset_cmd: dataprov: %p, dataset: %p, cmd: '%s', params: '%s', input: %p, output: %p", dataprov, dataset, cmd, params, input, output);
  PYSCDC_TRACE_OBJECT(pyscdc_dataprov_hook->dataprov, "pyscdc_dataprov_hook_dataset_cmd: dataprov: ");
  PYSCDC_TRACE_OBJECT(pyscdc_dataset, "pyscdc_dataprov_hook_dataset_cmd: dataset: ");
  PYSCDC_TRACE_DATASET_INPUT(input, "pyscdc_dataprov_hook_dataset_cmd: IN input: ");
  PYSCDC_TRACE_DATASET_OUTPUT(output, "pyscdc_dataprov_hook_dataset_cmd: IN output: ");

  pyinput_next = PyObject_GetAttrString(pyscdc_objs[PYSCDC_OBJ_SCDCMOD], "dataset_input_next_class");

  pyinput = pyscdc_dataset_input_struct2class(input, NULL, PYSCDC_NEXT_PACK(pyinput_next));
  PYSCDC_TRACE_DATASET_PYINPUT(pyinput, "pyscdc_dataprov_hook_dataset_cmd: IN pyinput: ");

  if (pyinput != Py_None)
  {
    /* create a shallow copy because the hook function can modify some field that are later required for the unpack */
    pyinput_given = PyObject_CallMethod(pyinput, "get_copy", "");
    PYSCDC_TRACE_CALL(pyinput_given, "pyscdc_dataset_input_struct2class: get_copy: ");

  } else pyinput_given = Py_None;

  pyoutput_next = PyObject_GetAttrString(pyscdc_objs[PYSCDC_OBJ_SCDCMOD], "dataset_output_next_class");

  pyoutput = pyscdc_dataset_output_struct2class(output, NULL, PYSCDC_NEXT_PACK(pyoutput_next));
  PYSCDC_TRACE_DATASET_PYOUTPUT(pyoutput, "pyscdc_dataprov_hook_dataset_cmd: IN pyoutput: ");

  if (pyoutput != Py_None)
  {
    /* create a shallow copy because the hook function can modify some field that are later required for the unpack */
    pyoutput_given = PyObject_CallMethod(pyoutput, "get_copy", "");
    PYSCDC_TRACE_CALL(pyoutput_given, "pyscdc_dataset_output_struct2class: get_copy: ");

  } else pyoutput_given = Py_None;

  pyret = PyObject_CallFunction(pyscdc_dataprov_hook->dataset_cmd, "OOssOO", pyscdc_dataprov_hook->dataprov, pyscdc_dataset, cmd, params, pyinput, pyoutput);
  PYSCDC_TRACE_CALL(pyret, "pyscdc_dataprov_hook_dataset_cmd: ");

  PYSCDC_TRACE_OBJECT(pyret, "pyscdc_dataprov_hook_dataset_cmd: return: ");
  PYSCDC_TRACE_DATASET_PYINPUT(pyinput, "pyscdc_dataprov_hook_dataset_cmd: OUT pyinput: ");
  PYSCDC_TRACE_DATASET_PYOUTPUT(pyoutput, "pyscdc_dataprov_hook_dataset_cmd: OUT pyoutput: ");

  input_given = pyscdc_dataset_input_class2struct(pyinput_given, input_given, PYSCDC_NEXT_UNPACK, 0);
  PYSCDC_TRACE_DATASET_INPUT(input_given, "pyscdc_dataprov_hook_dataset_cmd: OUT input given: ");

  input = pyscdc_dataset_input_class2struct(pyinput, input, PYSCDC_NEXT_PACK(pyscdc_dataset_input_next_struct), 0);
  PYSCDC_TRACE_DATASET_INPUT(input, "pyscdc_dataprov_hook_dataset_cmd: OUT input return: ");

  output_given = pyscdc_dataset_output_class2struct(pyoutput_given, output_given, PYSCDC_NEXT_UNPACK, 0);
  PYSCDC_TRACE_DATASET_OUTPUT(output_given, "pyscdc_dataprov_hook_dataset_cmd: OUT output given: ");

  output = pyscdc_dataset_output_class2struct(pyoutput, output, PYSCDC_NEXT_PACK(pyscdc_dataset_output_next_struct), 1);
  PYSCDC_TRACE_DATASET_OUTPUT(output, "pyscdc_dataprov_hook_dataset_cmd: OUT output return: ");

  PYSCDC_LIBSCDC_PY_END();

  return (pyret == Py_True)?SCDC_SUCCESS:SCDC_FAILURE;
}


static void pyscdc_dataprov_hook_get_args(scdc_dataprov_hook_t *hook, PyObject *hook_class)
{
  PyObject *o;

  hook->open = NULL;
  hook->close = pyscdc_dataprov_hook_close;

  o = PyObject_GetAttrString(hook_class, "config");
  hook->config = (o && o != Py_None)?pyscdc_dataprov_hook_config:NULL;
  Py_XDECREF(o);

  o = PyObject_GetAttrString(hook_class, "dataset_open");
  hook->dataset_open = (o && o != Py_None)?pyscdc_dataprov_hook_dataset_open:NULL;
  Py_XDECREF(o);

  o = PyObject_GetAttrString(hook_class, "dataset_close");
  hook->dataset_close = (o && o != Py_None)?pyscdc_dataprov_hook_dataset_close:NULL;
  Py_XDECREF(o);

  o = PyObject_GetAttrString(hook_class, "dataset_open_read_state");
  hook->dataset_open_read_state = (o && o != Py_None)?pyscdc_dataprov_hook_dataset_open_read_state:NULL;
  Py_XDECREF(o);

  o = PyObject_GetAttrString(hook_class, "dataset_close_write_state");
  hook->dataset_close_write_state = (o && o != Py_None)?pyscdc_dataprov_hook_dataset_close_write_state:NULL;
  Py_XDECREF(o);

  hook->dataset_cmd = pyscdc_dataprov_hook_dataset_cmd;
}


static scdcint_t pyscdc_dataprov_jobrun_handler(void *data, const char *jobid, const char *cmd, const char *params, scdc_dataset_input_t *input, scdc_dataset_output_t *output)
{
  PyObject *pyinput, *pyinput_given, *pyinput_next, *pyoutput, *pyoutput_given, *pyoutput_next, *pyret;
  scdc_dataset_input_t _input_given, *input_given = &_input_given;
  scdc_dataset_output_t _output_given, *output_given = &_output_given;

  PYSCDC_LIBSCDC_PY_DEF()


  PYSCDC_LIBSCDC_PY_BEGIN();

  PYSCDC_TRACE("pyscdc_dataprov_jobrun_handler: data: '%p', jobid: '%s', cmd: '%s', params: '%s', input: %p, output: %p", data, jobid, cmd, params, input, output);
  PYSCDC_TRACE_DATASET_INPUT(input, "pyscdc_dataprov_hook_dataset_cmd: IN input: ");
  PYSCDC_TRACE_DATASET_OUTPUT(output, "pyscdc_dataprov_hook_dataset_cmd: IN output: ");

  pyinput_next = PyObject_GetAttrString(pyscdc_objs[PYSCDC_OBJ_SCDCMOD], "dataset_input_next_class");

  pyinput = pyscdc_dataset_input_struct2class(input, NULL, PYSCDC_NEXT_PACK(pyinput_next));
  PYSCDC_TRACE_DATASET_PYINPUT(pyinput, "pyscdc_dataprov_jobrun_handler: IN pyinput: ");

  if (pyinput != Py_None)
  {
    /* create a shallow copy because the hook function can modify some field that are later required for the unpack */
    pyinput_given = PyObject_CallMethod(pyinput, "get_copy", "");
    PYSCDC_TRACE_CALL(pyinput_given, "pyscdc_dataprov_jobrun_handler: input get_copy: ");

  } else pyinput_given = Py_None;

  pyoutput_next = PyObject_GetAttrString(pyscdc_objs[PYSCDC_OBJ_SCDCMOD], "dataset_output_next_class");

  pyoutput = pyscdc_dataset_output_struct2class(output, NULL, PYSCDC_NEXT_PACK(pyoutput_next));
  PYSCDC_TRACE_DATASET_PYOUTPUT(pyoutput, "pyscdc_dataprov_jobrun_handler: IN pyoutput: ");

  if (pyoutput != Py_None)
  {
    /* create a shallow copy because the hook function can modify some field that are later required for the unpack */
    pyoutput_given = PyObject_CallMethod(pyoutput, "get_copy", "");
    PYSCDC_TRACE_CALL(pyoutput_given, "pyscdc_dataprov_jobrun_handler: output get_copy: ");

  } else pyoutput_given = Py_None;

  pyret = PyObject_CallFunction(PyTuple_GetItem(data, 0), "OsssOO", PyTuple_GetItem(data, 1), jobid, cmd, params, pyinput, pyoutput);
  PYSCDC_TRACE_CALL(pyret, "pyscdc_dataprov_jobrun_handler: ");

  PYSCDC_TRACE_OBJECT(pyret, "pyscdc_dataprov_jobrun_handler: return: ");
  PYSCDC_TRACE_DATASET_PYINPUT(pyinput, "pyscdc_dataprov_jobrun_handler: OUT pyinput: ");
  PYSCDC_TRACE_DATASET_PYOUTPUT(pyoutput, "pyscdc_dataprov_jobrun_handler: OUT pyoutput: ");

  input_given = pyscdc_dataset_input_class2struct(pyinput_given, input_given, PYSCDC_NEXT_UNPACK, 0);
  PYSCDC_TRACE_DATASET_INPUT(input_given, "pyscdc_dataprov_jobrun_handler: OUT input given: ");

  input = pyscdc_dataset_input_class2struct(pyinput, input, PYSCDC_NEXT_PACK(pyscdc_dataset_input_next_struct), 0);
  PYSCDC_TRACE_DATASET_INPUT(input, "pyscdc_dataprov_jobrun_handler: OUT input return: ");

  output_given = pyscdc_dataset_output_class2struct(pyoutput_given, output_given, PYSCDC_NEXT_UNPACK, 0);
  PYSCDC_TRACE_DATASET_OUTPUT(output_given, "pyscdc_dataprov_jobrun_handler: OUT output given: ");

  output = pyscdc_dataset_output_class2struct(pyoutput, output, PYSCDC_NEXT_PACK(pyscdc_dataset_output_next_struct), 1);
  PYSCDC_TRACE_DATASET_OUTPUT(output, "pyscdc_dataprov_jobrun_handler: OUT output return: ");

  PYSCDC_LIBSCDC_PY_END();

  return (pyret == Py_True)?SCDC_SUCCESS:SCDC_FAILURE;
}


static scdcint_t pyscdc_nodeport_cmd_handler(void *data, const char *cmd, const char *params, scdcint_t params_size)
{
  PyObject *result;
  scdcint_t ret;

  PYSCDC_LIBSCDC_PY_DEF()


  PYSCDC_LIBSCDC_PY_BEGIN();

  PYSCDC_TRACE("pyscdc_nodeport_cmd_handler: data: %p, cmd: '%s', params: '%.*s'", data, cmd, (int) params_size, params);

  result = PyObject_CallFunction(PyTuple_GetItem(data, 0), "Osz#", PyTuple_GetItem(data, 1), cmd, params, (Py_ssize_t) params_size);

  ret = (result == Py_True)?SCDC_SUCCESS:SCDC_FAILURE;

  PYSCDC_TRACE("pyscdc_nodeport_cmd_handler: return: %" scdcint_fmt, ret);

  PYSCDC_LIBSCDC_PY_END();

  return ret;
}


static scdcint_t pyscdc_nodeport_timer_handler(void *data)
{
  PyObject *result;
  scdcint_t ret;

  PYSCDC_LIBSCDC_PY_DEF()


  PYSCDC_LIBSCDC_PY_BEGIN();

  PYSCDC_TRACE("pyscdc_nodeport_timer_handler: data: %p", data);

  result = PyObject_CallFunction(PyTuple_GetItem(data, 0), "O", PyTuple_GetItem(data, 1));

  ret = (result == Py_True)?SCDC_SUCCESS:SCDC_FAILURE;

  PYSCDC_TRACE("pyscdc_nodeport_timer_handler: return: %" scdcint_fmt, ret);

  PYSCDC_LIBSCDC_PY_END();

  return ret;
}


static scdcint_t pyscdc_nodeport_loop_handler(void *data, scdcint_t l)
{
  PyObject *result;
  scdcint_t ret;

  PYSCDC_LIBSCDC_PY_DEF()


  PYSCDC_LIBSCDC_PY_BEGIN();

  PYSCDC_TRACE("pyscdc_nodeport_loop_handler: data: %p, l: %" scdcint_fmt, data, l);

  result = PyObject_CallFunction(PyTuple_GetItem(data, 0), "O" PYSCDCINT_FMT, PyTuple_GetItem(data, 1), l);

  ret = (result == Py_True)?SCDC_SUCCESS:SCDC_FAILURE;

  PYSCDC_TRACE("pyscdc_nodeport_loop_handler: return: %" scdcint_fmt, ret);

  PYSCDC_LIBSCDC_PY_END();

  return ret;
}


static scdc_arg_ref_t pyscdc_args_get_py(void *data, scdcint_t type, void *v)
{
  pyscdc_args_data_py_t *args_data_py = data;
  char *buf;
  Py_ssize_t buf_len;
  scdc_arg_ref_t ref = SCDC_ARG_REF_NONE;
  PyObject *o;

  PYSCDC_LIBSCDC_PY_DEF()


  PYSCDC_LIBSCDC_PY_BEGIN();

  PYSCDC_TRACE("pyscdc_args_get_py: data: '%p', type: '%" scdcint_fmt "', v: '%p'", data, type, v);

  switch (type)
  {
    case SCDC_ARGS_TYPE_INT:
      PYSCDC_ASSERT(args_data_py->nargs - args_data_py->arg >= 1);
      pyscdc_parseret = PyArg_Parse(args_data_py->args[args_data_py->arg], "i", v);
      PYSCDC_TRACE_PARSE("pyscdc_args_get_py: INT: ");
      args_data_py->arg += 1;
      break;
    case SCDC_ARGS_TYPE_SCDCINT:
      PYSCDC_ASSERT(args_data_py->nargs - args_data_py->arg >= 1);
      pyscdc_parseret = PyArg_Parse(args_data_py->args[args_data_py->arg], PYSCDCINT_FMT, v);
      PYSCDC_TRACE_PARSE("pyscdc_args_get_py: SCDCINT: ");
      args_data_py->arg += 1;
      break;
    case SCDC_ARGS_TYPE_DOUBLE:
      PYSCDC_ASSERT(args_data_py->nargs - args_data_py->arg >= 1);
      pyscdc_parseret = PyArg_Parse(args_data_py->args[args_data_py->arg], "d", v);
      PYSCDC_TRACE_PARSE("pyscdc_args_get_py: DOUBLE: ");
      args_data_py->arg += 1;
      break;
    case SCDC_ARGS_TYPE_CSTR:
      PYSCDC_ASSERT(args_data_py->nargs - args_data_py->arg >= 1);
      pyscdc_parseret = PyArg_Parse(args_data_py->args[args_data_py->arg], "s", v);
      PYSCDC_TRACE_PARSE("pyscdc_args_get_py: CSTR: ");
      args_data_py->arg += 1;
      break;
/*    case SCDC_ARGS_TYPE_PTR:
      return 0;*/
    case SCDC_ARGS_TYPE_IN_STREAM:
    case SCDC_ARGS_TYPE_OUT_STREAM:
      PYSCDC_ASSERT(args_data_py->nargs - args_data_py->arg >= 1);
#if PY_MAJOR_VERSION >= 3
      *((FILE **) v) = NULL;
      ref = SCDC_ARG_REF_NULL;
#else
      *((FILE **) v) = PyFile_AsFile(args_data_py->args[args_data_py->arg]);
#endif
      args_data_py->arg += 1;
      break;
    case SCDC_ARGS_TYPE_BUF:
      PYSCDC_ASSERT(args_data_py->nargs - args_data_py->arg >= 1);
      PYSCDC_TRACE_OBJECT(args_data_py->args[args_data_py->arg], "pyscdc_args_get_py: arg: ");
      pyscdc_parseret = PyArg_Parse(args_data_py->args[args_data_py->arg], "z#", &buf, &buf_len);
      PYSCDC_TRACE_PARSE("pyscdc_args_get_py: BUF: ");
      ((scdc_args_buf_t *) v)->buf = buf;
      ((scdc_args_buf_t *) v)->buf_size = buf_len;
      args_data_py->arg += 1;
      break;
    case SCDC_ARGS_TYPE_DATASET_INPUT:
      PYSCDC_ASSERT(args_data_py->nargs - args_data_py->arg >= 1);
      pyscdc_dataset_input_class2struct(args_data_py->args[args_data_py->arg], (scdc_dataset_input_t *) v, PYSCDC_NEXT_IGNORE, 0);
      args_data_py->arg += 1;
      ref = (scdc_arg_ref_t) args_data_py->args[args_data_py->arg - 1];
      break;
    case SCDC_ARGS_TYPE_DATASET_OUTPUT:
      PYSCDC_ASSERT(args_data_py->nargs - args_data_py->arg >= 1);
      pyscdc_dataset_output_class2struct(args_data_py->args[args_data_py->arg], (scdc_dataset_output_t *) v, PYSCDC_NEXT_IGNORE, 0);
      args_data_py->arg += 1;
      ref = (scdc_arg_ref_t) args_data_py->args[args_data_py->arg - 1];
      break;
/*    case SCDC_ARGS_TYPE_DATASET_INPUT_PTR:*/
/*    case SCDC_ARGS_TYPE_DATASET_OUTPUT_PTR:*/
    case SCDC_ARGS_TYPE_DATAPROV_HOOK:
      PYSCDC_ASSERT(args_data_py->nargs - args_data_py->arg >= 1);
      pyscdc_dataprov_hook_get_args(&((scdc_args_dataprov_hook_t *) v)->hook, args_data_py->args[args_data_py->arg]);
      ((scdc_args_dataprov_hook_t *) v)->hook_open_intern = pyscdc_dataprov_hook_open_intern;
      ((scdc_args_dataprov_hook_t *) v)->intern_data = args_data_py->args[args_data_py->arg];
      args_data_py->arg += 1;
      break;
    case SCDC_ARGS_TYPE_DATAPROV_JOBRUN_HANDLER:
      PYSCDC_ASSERT(args_data_py->nargs - args_data_py->arg >= 2);
      PYSCDC_TRACE_OBJECT(args_data_py->args[args_data_py->arg], "pyscdc_args_get_py: handler: ");
      PYSCDC_TRACE_OBJECT(args_data_py->args[args_data_py->arg + 1], "pyscdc_args_get_py: data: ");
      ((scdc_dataprov_jobrun_handler_args_t *) v)->handler = pyscdc_dataprov_jobrun_handler;
      ((scdc_dataprov_jobrun_handler_args_t *) v)->data = PyTuple_Pack(2, args_data_py->args[args_data_py->arg], args_data_py->args[args_data_py->arg + 1]);
      PYSCDC_TRACE_OBJECT(((scdc_dataprov_jobrun_handler_args_t *) v)->data, "pyscdc_args_get_py: new ref: ");
      args_data_py->arg += 2;
      break;
    case SCDC_ARGS_TYPE_NODEPORT_CMD_HANDLER:
      PYSCDC_ASSERT(args_data_py->nargs - args_data_py->arg >= 2);
      PYSCDC_TRACE_OBJECT(args_data_py->args[args_data_py->arg], "pyscdc_args_get_py: handler: ");
      PYSCDC_TRACE_OBJECT(args_data_py->args[args_data_py->arg + 1], "pyscdc_args_get_py: data: ");
      ((scdc_nodeport_cmd_handler_args_t *) v)->handler = pyscdc_nodeport_cmd_handler;
      ((scdc_nodeport_cmd_handler_args_t *) v)->data = PyTuple_Pack(2, args_data_py->args[args_data_py->arg], args_data_py->args[args_data_py->arg + 1]);
      PYSCDC_TRACE_OBJECT(((scdc_dataprov_jobrun_handler_args_t *) v)->data, "pyscdc_args_get_py: new ref: ");
      args_data_py->arg += 2;
      break;
    case SCDC_ARGS_TYPE_NODEPORT_TIMER_HANDLER:
      PYSCDC_ASSERT(args_data_py->nargs - args_data_py->arg >= 2);
      PYSCDC_TRACE_OBJECT(args_data_py->args[args_data_py->arg], "pyscdc_args_get_py: handler: ");
      PYSCDC_TRACE_OBJECT(args_data_py->args[args_data_py->arg + 1], "pyscdc_args_get_py: data: ");
      ((scdc_nodeport_timer_handler_args_t *) v)->handler = pyscdc_nodeport_timer_handler;
      ((scdc_nodeport_timer_handler_args_t *) v)->data = PyTuple_Pack(2, args_data_py->args[args_data_py->arg], args_data_py->args[args_data_py->arg + 1]);
      PYSCDC_TRACE_OBJECT(((scdc_dataprov_jobrun_handler_args_t *) v)->data, "pyscdc_args_get_py: new ref: ");
      args_data_py->arg += 2;
      break;
    case SCDC_ARGS_TYPE_NODEPORT_LOOP_HANDLER:
      PYSCDC_ASSERT(args_data_py->nargs - args_data_py->arg >= 2);
      PYSCDC_TRACE_OBJECT(args_data_py->args[args_data_py->arg], "pyscdc_args_get_py: handler: ");
      PYSCDC_TRACE_OBJECT(args_data_py->args[args_data_py->arg + 1], "pyscdc_args_get_py: data: ");
      ((scdc_nodeport_loop_handler_args_t *) v)->handler = pyscdc_nodeport_loop_handler;
      ((scdc_nodeport_loop_handler_args_t *) v)->data = PyTuple_Pack(2, args_data_py->args[args_data_py->arg], args_data_py->args[args_data_py->arg + 1]);
      PYSCDC_TRACE_OBJECT(((scdc_dataprov_jobrun_handler_args_t *) v)->data, "pyscdc_args_get_py: new ref: ");
      args_data_py->arg += 2;
      break;
    case SCDC_ARGS_TYPE_NODEPORT_LOOP_HANDLER_DUMMY:
      ((scdc_nodeport_loop_handler_args_t *) v)->handler = pyscdc_nodeport_loop_handler;
      o = PyObject_GetAttrString(pyscdc_objs[PYSCDC_OBJ_SCDC], "loop_handler_dummy");
      ((scdc_nodeport_loop_handler_args_t *) v)->data = PyTuple_Pack(2, o, Py_None);
      PYSCDC_TRACE_OBJECT(((scdc_dataprov_jobrun_handler_args_t *) v)->data, "pyscdc_args_get_py: new ref: ");
      Py_DECREF(o); /* PyObject_GetAttrString has returned a new reference */
      break;
    default:
      PYSCDC_ERROR("pyscdc_args_get_py: error: unsupported type '%" scdcint_fmt "'", type);
      ref = SCDC_ARG_REF_NULL;
  }

  PYSCDC_LIBSCDC_PY_END();

  return ref;
}


static scdc_arg_ref_t pyscdc_args_set_py(void *data, scdcint_t type, void *v, scdc_arg_ref_t ref)
{
  PYSCDC_LIBSCDC_PY_DEF()


  PYSCDC_LIBSCDC_PY_BEGIN();

  PYSCDC_TRACE("pyscdc_args_set_py: '%p', type: '%" scdcint_fmt "', v: '%p', ref: '%" scdc_arg_ref_fmt "'", data, type, v, ref);

  if (ref == SCDC_ARG_REF_NULL);
  else if (ref == SCDC_ARG_REF_NONE) ref = SCDC_ARG_REF_NULL;
  else
  switch (type)
  {
    case SCDC_ARGS_TYPE_DATASET_OUTPUT:
      pyscdc_dataset_output_struct2class((scdc_dataset_output_t *) v, ref, PYSCDC_NEXT_IGNORE);
      break;
    default:
      PYSCDC_ERROR("pyscdc_args_set_py: error: unsupported type '%" scdcint_fmt "'", type);
      ref = SCDC_ARG_REF_NULL;
  }

  PYSCDC_LIBSCDC_PY_END();

  return ref;
}


void pyscdc_args_free_py(void *data, scdcint_t type, void *v, scdc_arg_ref_t ref)
{
  PYSCDC_LIBSCDC_PY_DEF()


  PYSCDC_LIBSCDC_PY_BEGIN();

  PYSCDC_TRACE("pyscdc_args_free_py: '%p', type: '%" scdcint_fmt "', v: '%p', ref: '%" scdc_arg_ref_fmt "'", data, type, v, ref);

  switch (type)
  {
    case SCDC_ARGS_TYPE_DATAPROV_JOBRUN_HANDLER:
      PYSCDC_TRACE_OBJECT(((scdc_dataprov_jobrun_handler_args_t *) v)->data, "pyscdc_args_free_py: dec ref: ");
      Py_DECREF((PyObject *) ((scdc_dataprov_jobrun_handler_args_t *) v)->data);
      break;
    case SCDC_ARGS_TYPE_NODEPORT_LOOP_HANDLER_DUMMY:
      PYSCDC_TRACE_OBJECT(((scdc_nodeport_loop_handler_args_t *) v)->data, "pyscdc_args_free_py: dec ref: ");
      Py_DECREF(((scdc_nodeport_loop_handler_args_t *) v)->data);
      break;
  }

  PYSCDC_LIBSCDC_PY_END();
}


static void pyscdc_args_py_init(scdc_args_t *args, PyObject *pyargs, scdcint_t nskip)
{
  pyscdc_args_data_py_t *args_data_py;
  scdcint_t i;


  args_data_py = malloc(sizeof(pyscdc_args_data_py_t));

  args_data_py->nargs = 0;
  args_data_py->arg = 0;
  for (i = 0; i < PYSCDC_ARGS_MAX; ++i) args_data_py->args[i] = NULL;

  args_data_py->nargs = PyTuple_Size(pyargs) - nskip;
  if (args_data_py->nargs > PYSCDC_ARGS_MAX) args_data_py->nargs = PYSCDC_ARGS_MAX;
  for (i = 0; i < args_data_py->nargs; ++i) args_data_py->args[i] = PyTuple_GetItem(pyargs, nskip + i);

  args->data = args_data_py;
  args->get = pyscdc_args_get_py;
  args->set = pyscdc_args_set_py;
  args->free = pyscdc_args_free_py;
}


static void pyscdc_args_py_release(scdc_args_t *args)
{
  pyscdc_args_data_py_t *args_data_py = args->data;

  free(args_data_py);
}


static PyObject *pyscdc_init(PyObject *self, PyObject *pyargs)
{
  const char *conf = NULL;
  scdc_args_t args;
  scdcint_t ret;

  PYSCDC_LIBSCDC_DEF()


  PYSCDC_TRACE_OBJECT(pyargs, "pyscdc_init: pyargs: ");

  pyscdc_objs[PYSCDC_OBJ_DATAPROV]       = PyObject_GetAttrString(pyscdc_objs[PYSCDC_OBJ_SCDC], "dataprov");
  pyscdc_objs[PYSCDC_OBJ_NODEPORT]       = PyObject_GetAttrString(pyscdc_objs[PYSCDC_OBJ_SCDC], "nodeport");
  pyscdc_objs[PYSCDC_OBJ_DATASET]        = PyObject_GetAttrString(pyscdc_objs[PYSCDC_OBJ_SCDC], "dataset");
  pyscdc_objs[PYSCDC_OBJ_CBUF]           = PyObject_GetAttrString(pyscdc_objs[PYSCDC_OBJ_SCDC], "cbuf");
  pyscdc_objs[PYSCDC_OBJ_DATASET_INPUT]  = PyObject_GetAttrString(pyscdc_objs[PYSCDC_OBJ_SCDC], "dataset_input");
  pyscdc_objs[PYSCDC_OBJ_DATASET_OUTPUT] = PyObject_GetAttrString(pyscdc_objs[PYSCDC_OBJ_SCDC], "dataset_output");

  pyscdc_parseret = PyArg_ParseTuple(PyTuple_GetSlice(pyargs, 0, 1), "z", &conf);
  PYSCDC_TRACE_PARSE("pyscdc_init: ");

  if (!conf) conf = SCDC_INIT_DEFAULT;

  pyscdc_args_py_init(&args, pyargs, 1);

  PYSCDC_TRACE("pyscdc_init: conf: '%s', nargs: %" scdcint_fmt, conf, ((pyscdc_args_data_py_t *) args.data)->nargs);

  PYSCDC_LIBSCDC_BEGIN();
  ret = scdc_init_intern(conf, &args);
  PYSCDC_LIBSCDC_END();

  pyscdc_args_py_release(&args);

  PYSCDC_TRACE("pyscdc_init: return '%" scdcint_fmt "'", ret);

  PYSCDC_RETURN_BOOL(ret);
}


static PyObject *pyscdc_release(PyObject *self, PyObject *pyargs)
{
  PYSCDC_LIBSCDC_DEF()


  PYSCDC_TRACE("pyscdc_release:");

  PYSCDC_LIBSCDC_BEGIN();
  scdc_release_intern();
  PYSCDC_LIBSCDC_END();

  PYSCDC_TRACE("pyscdc_release: return");

  PYSCDC_LIBSCDC_BEGIN();
  scdc_log_release();
  PYSCDC_LIBSCDC_END();

  Py_RETURN_NONE;
}


static PyObject *pyscdc_log_init(PyObject *self, PyObject *pyargs)
{
  const char *conf = NULL;
  scdc_args_t args;
  scdcint_t ret;

  PYSCDC_LIBSCDC_DEF()


  PYSCDC_TRACE_OBJECT(pyargs, "pyscdc_log_init: pyargs: ");

  pyscdc_parseret = PyArg_ParseTuple(PyTuple_GetSlice(pyargs, 0, 1), "s", &conf);
  PYSCDC_TRACE_PARSE("pyscdc_log_init: ");

  pyscdc_args_py_init(&args, pyargs, 1);

  PYSCDC_TRACE("pyscdc_log_init: conf: '%s', nargs: %" scdcint_fmt, conf, ((pyscdc_args_data_py_t *) args.data)->nargs);

  PYSCDC_LIBSCDC_BEGIN();
  ret = scdc_log_init_intern(conf, &args);
  PYSCDC_LIBSCDC_END();

  pyscdc_args_py_release(&args);

  PYSCDC_TRACE("pyscdc_log_init: return '%" scdcint_fmt "'", ret);

  PYSCDC_RETURN_BOOL(ret);
}


static PyObject *pyscdc_log_release(PyObject *self, PyObject *pyargs)
{
  PYSCDC_LIBSCDC_DEF()


  PYSCDC_TRACE("pyscdc_log_release:");

  PYSCDC_LIBSCDC_BEGIN();
  scdc_log_release_intern();
  PYSCDC_LIBSCDC_END();

  PYSCDC_TRACE("pyscdc_log_release: return");

  Py_RETURN_NONE;
}


static PyObject *pyscdc_dataprov_open(PyObject *self, PyObject *pyargs)
{
  scdc_dataprov_t dataprov = SCDC_DATAPROV_NULL;
  const char *base_path = NULL, *conf = NULL;
  scdc_args_t args;
  PyObject *pydataprov;

  PYSCDC_LIBSCDC_DEF()


  PYSCDC_TRACE_OBJECT(pyargs, "pyscdc_dataprov_open: pyargs: ");

  pyscdc_parseret = PyArg_ParseTuple(PyTuple_GetSlice(pyargs, 0, 2), "ss", &base_path, &conf);
  PYSCDC_TRACE_PARSE("pyscdc_dataprov_open: ");

  pyscdc_args_py_init(&args, pyargs, 2);

  PYSCDC_TRACE("pyscdc_dataprov_open: base_path: '%s', conf: '%s', nargs: %" scdcint_fmt, base_path, conf, ((pyscdc_args_data_py_t *) args.data)->nargs);

  PYSCDC_LIBSCDC_BEGIN();
  dataprov = scdc_dataprov_open_intern(base_path, conf, &args);
  PYSCDC_LIBSCDC_END();

  pyscdc_args_py_release(&args);

  PYSCDC_TRACE("pyscdc_dataprov_open: dataprov: '%" scdc_dataprov_fmt "'", dataprov);

  if (dataprov == SCDC_DATAPROV_NULL) pydataprov = Py_False;
  else pydataprov = PyObject_CallMethod(pyscdc_objs[PYSCDC_OBJ_SCDC], "dataprov", PYSCDC_DATAPROV_FMT, PYSCDC_DATAPROV_TO_PY(dataprov));

  PYSCDC_TRACE_OBJECT_STR(pydataprov, "pyscdc_dataprov_open: return: ");

  PYSCDC_RETURN_OBJECT(pydataprov);
}


static PyObject *pyscdc_dataprov_close(PyObject *self, PyObject *pyargs)
{
  PyObject *pydataprov = NULL;
  pyscdc_dataprov_t pydataprov_dataprov = PYSCDC_DATAPROV_NULL;
  scdc_dataprov_t dataprov = SCDC_DATAPROV_NULL;

  PYSCDC_LIBSCDC_DEF()


  PYSCDC_TRACE_OBJECT(pyargs, "pyscdc_dataprov_close: pyargs: ");

  pydataprov = PyTuple_GetItem(pyargs, 0);

  if (pydataprov == Py_None) Py_RETURN_NONE;

  if (!PyObject_IsInstance(pydataprov, pyscdc_objs[PYSCDC_OBJ_DATAPROV]))
  {
    PYSCDC_FAIL("pyscdc_dataprov_close: not a dataprov instance");
    Py_RETURN_FALSE;
  }

  pyscdc_parseret = PyArg_Parse(PyObject_CallMethod(pydataprov, "get_c", NULL), PYSCDC_DATAPROV_FMT, &pydataprov_dataprov);
  PYSCDC_TRACE_PARSE("pyscdc_dataprov_close: ");

  dataprov = PYSCDC_DATAPROV_TO_C(pydataprov_dataprov);

  PYSCDC_TRACE("pyscdc_dataprov_close: dataprov: '%" scdc_dataprov_fmt "'", dataprov);

  PYSCDC_LIBSCDC_BEGIN();
  scdc_dataprov_close(dataprov);
  PYSCDC_LIBSCDC_END();

  PYSCDC_TRACE("pyscdc_dataprov_close: return");

  Py_RETURN_NONE;
}


static PyObject *pyscdc_nodeport_open(PyObject *self, PyObject *pyargs)
{
  scdc_nodeport_t nodeport = SCDC_NODEPORT_NULL;
  const char *conf = NULL;
  scdc_args_t args;
  PyObject *pynodeport;

  PYSCDC_LIBSCDC_DEF()


  PYSCDC_TRACE_OBJECT(pyargs, "pyscdc_nodeport_open: pyargs: ");

  pyscdc_parseret = PyArg_ParseTuple(PyTuple_GetSlice(pyargs, 0, 1), "s", &conf);
  PYSCDC_TRACE_PARSE("pyscdc_nodeport_open: ");

  pyscdc_args_py_init(&args, pyargs, 1);

  PYSCDC_TRACE("pyscdc_nodeport_open: conf: '%s', args: %" scdcint_fmt, conf, ((pyscdc_args_data_py_t *) args.data)->nargs);

  PYSCDC_LIBSCDC_BEGIN();
  nodeport = scdc_nodeport_open_intern(conf, &args);
  PYSCDC_LIBSCDC_END();

  pyscdc_args_py_release(&args);

  PYSCDC_TRACE("pyscdc_nodeport_open: nodeport: '%" scdcint_fmt "'", nodeport);

  if (nodeport == SCDC_NODEPORT_NULL) pynodeport = Py_False;
  else pynodeport = PyObject_CallMethod(pyscdc_objs[PYSCDC_OBJ_SCDC], "nodeport", PYSCDC_NODEPORT_FMT, PYSCDC_NODEPORT_TO_PY(nodeport));

  PYSCDC_TRACE_OBJECT_STR(pynodeport, "pyscdc_nodeport_open: return: ");

  PYSCDC_RETURN_OBJECT(pynodeport);
}


static PyObject *pyscdc_nodeport_close(PyObject *self, PyObject *pyargs)
{
  PyObject *pynodeport = NULL;
  pyscdc_nodeport_t pynodeport_nodeport = PYSCDC_NODEPORT_NULL;
  scdc_nodeport_t nodeport = SCDC_NODEPORT_NULL;

  PYSCDC_LIBSCDC_DEF()


  PYSCDC_TRACE_OBJECT(pyargs, "pyscdc_nodeport_close: pyargs: ");

  pynodeport = PyTuple_GetItem(pyargs, 0);

  if (pynodeport == Py_None) Py_RETURN_NONE;

  if (!PyObject_IsInstance(pynodeport, pyscdc_objs[PYSCDC_OBJ_NODEPORT]))
  {
    PYSCDC_FAIL("pyscdc_nodeport_close: not a nodeport instance");
    Py_RETURN_FALSE;
  }

  pyscdc_parseret = PyArg_Parse(PyObject_CallMethod(pynodeport, "get_c", NULL), PYSCDC_NODEPORT_FMT, &pynodeport_nodeport);
  PYSCDC_TRACE_PARSE("pyscdc_nodeport_close: ");

  nodeport = PYSCDC_NODEPORT_TO_C(pynodeport_nodeport);

  PYSCDC_TRACE("pyscdc_nodeport_close: nodeport: '%" scdcint_fmt "'", nodeport);

  PYSCDC_LIBSCDC_BEGIN();
  scdc_nodeport_close(nodeport);
  PYSCDC_LIBSCDC_END();

  PYSCDC_TRACE("pyscdc_nodeport_close: return");

  Py_RETURN_NONE;
}


static PyObject *pyscdc_nodeport_start(PyObject *self, PyObject *pyargs)
{
  PyObject *pynodeport = NULL, *pymode = NULL;
  pyscdc_nodeport_t pynodeport_nodeport = PYSCDC_NODEPORT_NULL;
  scdc_nodeport_t nodeport = SCDC_NODEPORT_NULL;
  scdcint_t mode, ret;

  PYSCDC_LIBSCDC_DEF()


  PYSCDC_TRACE_OBJECT(pyargs, "pyscdc_nodeport_start: pyargs: ");

  pynodeport = PyTuple_GetItem(pyargs, 0);

  if (pynodeport == Py_None) Py_RETURN_NONE;

  if (!PyObject_IsInstance(pynodeport, pyscdc_objs[PYSCDC_OBJ_NODEPORT]))
  {
    PYSCDC_FAIL("pyscdc_nodeport_start: not a nodeport instance");
    Py_RETURN_FALSE;
  }

  pyscdc_parseret = PyArg_Parse(PyObject_CallMethod(pynodeport, "get_c", NULL), PYSCDC_NODEPORT_FMT, &pynodeport_nodeport);
  PYSCDC_TRACE_PARSE("pyscdc_nodeport_start: ");

  nodeport = PYSCDC_NODEPORT_TO_C(pynodeport_nodeport);

  pymode = PyTuple_GetItem(pyargs, 1);

  if (pymode != Py_None)
  {
    pyscdc_parseret = PyArg_Parse(pymode, PYSCDCINT_FMT, &mode);
    PYSCDC_TRACE_PARSE("pyscdc_nodeport_start: ");

  } else mode = SCDC_NODEPORT_START_NONE;

  PYSCDC_TRACE("pyscdc_nodeport_start: nodeport: '%" scdcint_fmt "', mode: %" scdcint_fmt, nodeport, mode);

  PYSCDC_LIBSCDC_BEGIN();
  ret = scdc_nodeport_start(nodeport, mode);
  PYSCDC_LIBSCDC_END();

  PYSCDC_TRACE("pyscdc_nodeport_start: return");

  PYSCDC_RETURN_BOOL(ret);
}


static PyObject *pyscdc_nodeport_stop(PyObject *self, PyObject *pyargs)
{
  PyObject *pynodeport = NULL;
  pyscdc_nodeport_t pynodeport_nodeport = PYSCDC_NODEPORT_NULL;
  scdc_nodeport_t nodeport = SCDC_NODEPORT_NULL;
  scdcint_t ret;

  PYSCDC_LIBSCDC_DEF()


  PYSCDC_TRACE_OBJECT(pyargs, "pyscdc_nodeport_stop: pyargs: ");

  pynodeport = PyTuple_GetItem(pyargs, 0);

  if (pynodeport == Py_None) Py_RETURN_NONE;

  if (!PyObject_IsInstance(pynodeport, pyscdc_objs[PYSCDC_OBJ_NODEPORT]))
  {
    PYSCDC_FAIL("pyscdc_nodeport_stop: not a nodeport instance");
    Py_RETURN_FALSE;
  }

  pyscdc_parseret = PyArg_Parse(PyObject_CallMethod(pynodeport, "get_c", NULL), PYSCDC_NODEPORT_FMT, &pynodeport_nodeport);
  PYSCDC_TRACE_PARSE("pyscdc_nodeport_stop: ");

  nodeport = PYSCDC_NODEPORT_TO_C(pynodeport_nodeport);

  PYSCDC_TRACE("pyscdc_nodeport_stop: nodeport: '%" scdcint_fmt "'", nodeport);

  PYSCDC_LIBSCDC_BEGIN();
  ret = scdc_nodeport_stop(nodeport);
  PYSCDC_LIBSCDC_END();

  PYSCDC_TRACE("pyscdc_nodeport_stop: return");

  PYSCDC_RETURN_BOOL(ret);
}


static PyObject *pyscdc_nodeport_cancel(PyObject *self, PyObject *pyargs)
{
  PyObject *pynodeport = NULL, *pyinterrupt = NULL;
  pyscdc_nodeport_t pynodeport_nodeport = PYSCDC_NODEPORT_NULL;
  scdc_nodeport_t nodeport = SCDC_NODEPORT_NULL;
  scdcint_t interrupt, ret;

  PYSCDC_LIBSCDC_DEF()


  PYSCDC_TRACE_OBJECT(pyargs, "pyscdc_nodeport_cancel: pyargs: ");

  pynodeport = PyTuple_GetItem(pyargs, 0);

  if (pynodeport == Py_None) Py_RETURN_NONE;

  if (!PyObject_IsInstance(pynodeport, pyscdc_objs[PYSCDC_OBJ_NODEPORT]))
  {
    PYSCDC_FAIL("pyscdc_nodeport_cancel: not a nodeport instance");
    Py_RETURN_FALSE;
  }

  pyscdc_parseret = PyArg_Parse(PyObject_CallMethod(pynodeport, "get_c", NULL), PYSCDC_NODEPORT_FMT, &pynodeport_nodeport);
  PYSCDC_TRACE_PARSE("pyscdc_nodeport_cancel: ");

  nodeport = PYSCDC_NODEPORT_TO_C(pynodeport_nodeport);

  pyinterrupt = PyTuple_GetItem(pyargs, 1);

  interrupt = (pyinterrupt == Py_True);

  PYSCDC_TRACE("pyscdc_nodeport_cancel: nodeport: '%" scdcint_fmt "', interrupt: '%" scdcint_fmt "'", nodeport, interrupt);

  PYSCDC_LIBSCDC_BEGIN();
  ret = scdc_nodeport_cancel(nodeport, interrupt);
  PYSCDC_LIBSCDC_END();

  PYSCDC_TRACE("pyscdc_nodeport_cancel: return");

  PYSCDC_RETURN_BOOL(ret);
}


static PyObject *pyscdc_nodeport_authority(PyObject *self, PyObject *pyargs)
{
  const char *conf = NULL;
  scdc_args_t args;
  const char *authority;
  PyObject *pyauthority;

  PYSCDC_LIBSCDC_DEF()


  PYSCDC_TRACE_OBJECT(pyargs, "pyscdc_nodeport_authority: pyargs: ");

  pyscdc_parseret = PyArg_ParseTuple(PyTuple_GetSlice(pyargs, 0, 1), "z", &conf);
  PYSCDC_TRACE_PARSE("pyscdc_nodeport_authority: ");

  pyscdc_args_py_init(&args, pyargs, 1);

  PYSCDC_TRACE("pyscdc_nodeport_authority: conf: '%s', nargs: %" scdcint_fmt, conf, ((pyscdc_args_data_py_t *) args.data)->nargs);

  PYSCDC_LIBSCDC_BEGIN();
  authority = scdc_nodeport_authority_intern(conf, &args);
  PYSCDC_LIBSCDC_END();

  pyscdc_args_py_release(&args);

  PYSCDC_TRACE("pyscdc_nodeport_authority: authority: '%s'", authority);

  if (!authority) pyauthority = Py_False;
  else pyauthority = Py_BuildValue("s", authority);

  PYSCDC_TRACE_OBJECT_STR(pyauthority, "pyscdc_nodeport_authority: return: ");

  PYSCDC_RETURN_OBJECT(pyauthority);
}


static PyObject *pyscdc_nodeport_supported(PyObject *self, PyObject *pyargs)
{
  const char *uri = NULL;
  scdc_args_t args;
  scdcint_t ret;

  PYSCDC_LIBSCDC_DEF()


  PYSCDC_TRACE_OBJECT(pyargs, "pyscdc_nodeport_supported: pyargs: ");

  pyscdc_parseret = PyArg_ParseTuple(PyTuple_GetSlice(pyargs, 0, 1), "z", &uri);
  PYSCDC_TRACE_PARSE("pyscdc_nodeport_supported: ");

  pyscdc_args_py_init(&args, pyargs, 1);

  PYSCDC_TRACE("pyscdc_nodeport_supported: uri: '%s', nargs: %" scdcint_fmt, uri, ((pyscdc_args_data_py_t *) args.data)->nargs);

  PYSCDC_LIBSCDC_BEGIN();
  ret = scdc_nodeport_supported_intern(uri, &args);
  PYSCDC_LIBSCDC_END();

  pyscdc_args_py_release(&args);

  PYSCDC_TRACE("pyscdc_nodeport_supported: return '%" scdcint_fmt "'", ret);

  PYSCDC_RETURN_BOOL(ret);
}


static PyObject *pyscdc_dataset_open(PyObject *self, PyObject *pyargs)
{
  const char *uri;
  scdc_args_t args;
  scdc_dataset_t dataset;
  PyObject *pydataset;

  PYSCDC_LIBSCDC_DEF()


  PYSCDC_TRACE_OBJECT(pyargs, "pyscdc_dataset_open: pyargs: ");

  pyscdc_parseret = PyArg_ParseTuple(PyTuple_GetSlice(pyargs, 0, 1), "z", &uri);
  PYSCDC_TRACE_PARSE("pyscdc_dataset_open: ");

  pyscdc_args_py_init(&args, pyargs, 1);

  PYSCDC_TRACE("pyscdc_dataset_open: uri: '%s', nargs: %" scdcint_fmt, uri, ((pyscdc_args_data_py_t *) args.data)->nargs);

  PYSCDC_LIBSCDC_BEGIN();
  dataset = scdc_dataset_open_intern(uri, &args);
  PYSCDC_LIBSCDC_END();

  pyscdc_args_py_release(&args);

  PYSCDC_TRACE("pyscdc_dataset_open: dataset: '%" scdc_dataset_fmt "'", dataset);

  if (dataset == SCDC_DATASET_NULL) pydataset = Py_False;
  else pydataset = PyObject_CallMethod(pyscdc_objs[PYSCDC_OBJ_SCDC], "dataset", PYSCDC_DATASET_FMT, PYSCDC_DATASET_TO_PY(dataset));

  PYSCDC_TRACE_OBJECT_STR(pydataset, "pyscdc_dataset_open: return: ");

  PYSCDC_RETURN_OBJECT(pydataset);
}


static PyObject *pyscdc_dataset_close(PyObject *self, PyObject *pyargs)
{
  PyObject *pydataset = NULL;
  pyscdc_dataset_t pydataset_dataset = PYSCDC_DATASET_NULL;
  scdc_dataset_t dataset = SCDC_DATASET_NULL;

  PYSCDC_LIBSCDC_DEF()


  PYSCDC_TRACE_OBJECT(pyargs, "pyscdc_dataset_close: pyargs: ");

  pydataset = PyTuple_GetItem(pyargs, 0);

  if (pydataset == Py_None) Py_RETURN_NONE;

  if (!PyObject_IsInstance(pydataset, pyscdc_objs[PYSCDC_OBJ_DATASET]))
  {
    PYSCDC_FAIL("pyscdc_dataset_close: not a dataset instance");
    Py_RETURN_FALSE;
  }

  pyscdc_parseret = PyArg_Parse(PyObject_CallMethod(pydataset, "get_c", NULL), PYSCDC_DATASET_FMT, &pydataset_dataset);
  PYSCDC_TRACE_PARSE("pyscdc_dataset_close: ");

  dataset = PYSCDC_DATASET_TO_C(pydataset_dataset);

  PYSCDC_TRACE("pyscdc_dataset_close: dataset: '%" scdc_dataset_fmt "'", dataset);

  PYSCDC_LIBSCDC_BEGIN();
  scdc_dataset_close(dataset);
  PYSCDC_LIBSCDC_END();

  PYSCDC_TRACE("pyscdc_dataset_close: return");

  Py_RETURN_NONE;
}


static PyObject *pyscdc_dataset_cmd(PyObject *self, PyObject *pyargs)
{
  PyObject *pydataset = NULL;
  scdc_dataset_t dataset = SCDC_DATASET_NULL;
  const char *cmd = NULL;
  scdc_args_t args;
  pyscdc_dataset_t pydataset_dataset = PYSCDC_DATASET_NULL;
  PyObject *pyinput, *pyinput_next, *pyoutput, *pyoutput_next;
#if PYSCDC_TRACE_DATASET
  PyObject *pyinput_given, *pyoutput_given;
#endif
  scdc_dataset_input_t _input, *input = &_input, _input_given, *input_given = &_input_given;
  scdc_dataset_output_t _output, *output = &_output, _output_given, *output_given = &_output_given;
  scdcint_t ret = 0;

  PYSCDC_LIBSCDC_DEF()


  PYSCDC_TRACE_OBJECT(pyargs, "pyscdc_dataset_cmd: pyargs: ");

  pyscdc_parseret = PyArg_ParseTuple(PyTuple_GetSlice(pyargs, 0, 4), "OsOO", &pydataset, &cmd, &pyinput, &pyoutput);
  PYSCDC_TRACE_PARSE("pyscdc_dataset_cmd: ");

  pyscdc_args_py_init(&args, pyargs, 4);

  if (pydataset != Py_None)
  {
    if (!PyObject_IsInstance(pydataset, pyscdc_objs[PYSCDC_OBJ_DATASET]))
    {
      PYSCDC_FAIL("pyscdc_dataset_cmd: not a dataset instance");
      Py_RETURN_FALSE;
    }

    pyscdc_parseret = PyArg_Parse(PyObject_CallMethod(pydataset, "get_c", NULL), PYSCDC_DATASET_FMT, &pydataset_dataset);
    PYSCDC_TRACE_PARSE("pyscdc_dataset_close: ");

    dataset = PYSCDC_DATASET_TO_C(pydataset_dataset);

  } else dataset = SCDC_DATASET_NULL;

  PYSCDC_TRACE_DATASET_PYINPUT(pyinput, "pyscdc_dataset_cmd: IN pyinput: ");
  PYSCDC_TRACE_DATASET_PYOUTPUT(pyoutput, "pyscdc_dataset_cmd: IN pyoutput: ");

  scdc_dataset_input_unset(input);

  input = pyscdc_dataset_input_class2struct(pyinput, input, PYSCDC_NEXT_PACK(pyscdc_dataset_input_next_struct), 1);
  PYSCDC_TRACE_DATASET_INPUT(input, "pyscdc_dataset_cmd: IN input: ");

  if (input) *input_given = *input;
  else input_given = NULL;

  scdc_dataset_output_unset(output);

  output = pyscdc_dataset_output_class2struct(pyoutput, output, PYSCDC_NEXT_PACK(pyscdc_dataset_output_next_struct), 1);
  PYSCDC_TRACE_DATASET_OUTPUT(output, "pyscdc_dataset_cmd: IN output: ");

  if (output) *output_given = *output;
  else output_given = NULL;

  PYSCDC_TRACE("pyscdc_dataset_cmd: dataset: '%" scdc_dataset_fmt "', cmd: '%s', input: %p, output: %p", dataset, cmd, input, output);

  PYSCDC_LIBSCDC_BEGIN();
  ret = scdc_dataset_cmd_intern(dataset, cmd, input, output, &args);
  PYSCDC_LIBSCDC_END();

  pyscdc_args_py_release(&args);

  PYSCDC_TRACE("pyscdc_dataset_cmd: return: '%" scdcint_fmt "'", ret);

  /* unpack given input */
#if PYSCDC_TRACE_DATASET
  pyinput_given =
#endif
    pyscdc_dataset_input_struct2class(input_given, NULL, PYSCDC_NEXT_UNPACK);
  PYSCDC_TRACE_DATASET_PYINPUT(pyinput_given, "pyscdc_dataset_cmd: OUT pyinput given: ");

  pyinput_next = PyObject_GetAttrString(pyscdc_objs[PYSCDC_OBJ_SCDCMOD], "dataset_input_next_class");

  /* pack return input */
  pyinput = pyscdc_dataset_input_struct2class(input, pyinput, PYSCDC_NEXT_PACK(pyinput_next));
  PYSCDC_TRACE_DATASET_PYINPUT(pyinput, "pyscdc_dataset_cmd: OUT pyinput return: ");

  /* unpack given output */
#if PYSCDC_TRACE_DATASET
  pyoutput_given =
#endif
    pyscdc_dataset_output_struct2class(output_given, NULL, PYSCDC_NEXT_UNPACK);
  PYSCDC_TRACE_DATASET_PYOUTPUT(pyoutput_given, "pyscdc_dataset_cmd: OUT pyoutput given: ");

  pyoutput_next = PyObject_GetAttrString(pyscdc_objs[PYSCDC_OBJ_SCDCMOD], "dataset_output_next_class");

  /* pack return output  */
  pyoutput = pyscdc_dataset_output_struct2class(output, pyoutput, PYSCDC_NEXT_PACK(pyoutput_next));
  PYSCDC_TRACE_DATASET_PYOUTPUT(pyoutput, "pyscdc_dataset_cmd: OUT pyoutput: ");

  PYSCDC_RETURN_BOOL(ret);
}


static PyObject *pyscdc_dataset_input_create(PyObject *self, PyObject *pyargs)
{
  const char *conf;
  scdc_args_t args;
  scdc_dataset_input_t _input, *input = &_input;
  PyObject *pyinput, *pyinput_next;

  PYSCDC_LIBSCDC_DEF()


  pyscdc_parseret = PyArg_ParseTuple(PyTuple_GetSlice(pyargs, 0, 1), "s", &conf);
  PYSCDC_TRACE_PARSE("pyscdc_dataset_input_create: ");

  pyscdc_args_py_init(&args, pyargs, 1);

  PYSCDC_TRACE("pyscdc_dataset_input_create: conf: '%s', args: %" scdcint_fmt, conf, ((pyscdc_args_data_py_t *) args.data)->nargs);

  scdc_dataset_input_unset(input);

  PYSCDC_LIBSCDC_BEGIN();
  input = scdc_dataset_input_create_intern(input, conf, &args);
  PYSCDC_LIBSCDC_END();

  PYSCDC_TRACE_DATASET_INPUT(input, "pyscdc_dataset_input_create: input: ");

  pyscdc_args_py_release(&args);

  pyinput_next = PyObject_GetAttrString(pyscdc_objs[PYSCDC_OBJ_SCDCMOD], "dataset_input_next_class");

  pyinput = pyscdc_dataset_input_struct2class(input, NULL, PYSCDC_NEXT_PACK(pyinput_next));
  PYSCDC_TRACE_DATASET_PYINPUT(pyinput, "pyscdc_dataset_input_create: pyinput: ");

  PYSCDC_RETURN_OBJECT(pyinput);
}


static PyObject *pyscdc_dataset_input_destroy(PyObject *self, PyObject *pyargs)
{
  scdc_dataset_input_t _input, *input = &_input;
  PyObject *pyinput;

  PYSCDC_LIBSCDC_DEF()


  pyinput = PyTuple_GetItem(pyargs, 0);
  PYSCDC_TRACE_DATASET_PYINPUT(pyinput, "pyscdc_dataset_input_destroy: pyinput: ");

  scdc_dataset_input_unset(input);

  input = pyscdc_dataset_input_class2struct(pyinput, input, PYSCDC_NEXT_UNPACK, 0);
  PYSCDC_TRACE_DATASET_INPUT(input, "pyscdc_dataset_input_destroy: input: ");

  PYSCDC_LIBSCDC_BEGIN();
  scdc_dataset_input_destroy(input);
  PYSCDC_LIBSCDC_END();

  Py_RETURN_NONE;
}


static PyObject *pyscdc_dataset_output_create(PyObject *self, PyObject *pyargs)
{
  const char *conf;
  scdc_args_t args;
  scdc_dataset_output_t _output, *output = &_output;
  PyObject *pyoutput, *pyoutput_next;

  PYSCDC_LIBSCDC_DEF()


  pyscdc_parseret = PyArg_ParseTuple(PyTuple_GetSlice(pyargs, 0, 1), "s", &conf);
  PYSCDC_TRACE_PARSE("pyscdc_dataset_output_create: ");

  pyscdc_args_py_init(&args, pyargs, 1);

  PYSCDC_TRACE("pyscdc_dataset_output_create: conf: '%s', , args: %" scdcint_fmt, conf, ((pyscdc_args_data_py_t *) args.data)->nargs);

  scdc_dataset_output_unset(output);

  PYSCDC_LIBSCDC_BEGIN();
  output = scdc_dataset_output_create_intern(output, conf, &args);
  PYSCDC_LIBSCDC_END();

  PYSCDC_TRACE_DATASET_OUTPUT(output, "pyscdc_dataset_output_create: output: ");

  pyscdc_args_py_release(&args);

  pyoutput_next = PyObject_GetAttrString(pyscdc_objs[PYSCDC_OBJ_SCDCMOD], "dataset_output_next_class");

  pyoutput = pyscdc_dataset_output_struct2class(output, NULL, PYSCDC_NEXT_PACK(pyoutput_next));
  PYSCDC_TRACE_DATASET_PYOUTPUT(pyoutput, "pyscdc_dataset_output_create: pyoutput: ");

  PYSCDC_RETURN_OBJECT(pyoutput);
}


static PyObject *pyscdc_dataset_output_destroy(PyObject *self, PyObject *pyargs)
{
  scdc_dataset_output_t _output, *output = &_output;
  PyObject *pyoutput = NULL;

  PYSCDC_LIBSCDC_DEF()


  pyoutput = PyTuple_GetItem(pyargs, 0);
  PYSCDC_TRACE_DATASET_PYOUTPUT(pyoutput, "pyscdc_dataset_output_destroy: pyoutput: ");

  output = pyscdc_dataset_output_class2struct(pyoutput, output, PYSCDC_NEXT_UNPACK, 0);
  PYSCDC_TRACE_DATASET_OUTPUT(output, "pyscdc_dataset_output_destroy: output: ");

  PYSCDC_LIBSCDC_BEGIN();
  scdc_dataset_output_destroy(output);
  PYSCDC_LIBSCDC_END();

  Py_RETURN_NONE;
}


static PyObject *pyscdc_dataset_input_redirect(PyObject *self, PyObject *pyargs)
{
  const char *conf;
  scdc_args_t args;
  scdc_dataset_input_t _input, *input = &_input;
  PyObject *pyinput, *pyinput_next;
  scdcint_t ret;

  PYSCDC_LIBSCDC_DEF()


  pyscdc_parseret = PyArg_ParseTuple(PyTuple_GetSlice(pyargs, 0, 2), "Os", &pyinput, &conf);
  PYSCDC_TRACE_PARSE("pyscdc_dataset_input_redirect: ");

  PYSCDC_TRACE_DATASET_PYINPUT(pyinput, "pyscdc_dataset_input_redirect: IN pyinput: ");

  pyscdc_args_py_init(&args, pyargs, 2);

  PYSCDC_TRACE("pyscdc_dataset_input_redirect: conf: '%s'", conf);

  scdc_dataset_input_unset(input);

  input = pyscdc_dataset_input_class2struct(pyinput, input, PYSCDC_NEXT_UNPACK, 1);
  PYSCDC_TRACE_DATASET_INPUT(input, "pyscdc_dataset_input_destroy: IN input: ");

  PYSCDC_LIBSCDC_BEGIN();
  ret = scdc_dataset_input_redirect_intern(input, conf, &args);
  PYSCDC_LIBSCDC_END();

  PYSCDC_TRACE_DATASET_INPUT(input, "pyscdc_dataset_input_redirect: OUT input: ");

  pyscdc_args_py_release(&args);

  pyinput_next = PyObject_GetAttrString(pyscdc_objs[PYSCDC_OBJ_SCDCMOD], "dataset_input_next_class");

  pyinput = pyscdc_dataset_input_struct2class(input, pyinput, PYSCDC_NEXT_PACK(pyinput_next));
  PYSCDC_TRACE_DATASET_PYINPUT(pyinput, "pyscdc_dataset_input_create: OUT pyinput: ");

  PYSCDC_RETURN_BOOL(ret);
}


static PyObject *pyscdc_dataset_output_redirect(PyObject *self, PyObject *pyargs)
{
  const char *conf;
  scdc_args_t args;
  scdc_dataset_output_t _output, *output = &_output;
  PyObject *pyoutput, *pyoutput_next;
  scdcint_t ret;

  PYSCDC_LIBSCDC_DEF()


  pyscdc_parseret = PyArg_ParseTuple(PyTuple_GetSlice(pyargs, 0, 2), "Os", &pyoutput, &conf);
  PYSCDC_TRACE_PARSE("pyscdc_dataset_output_redirect: ");

  PYSCDC_TRACE_DATASET_PYOUTPUT(pyoutput, "pyscdc_dataset_output_redirect: IN pyoutput: ");

  pyscdc_args_py_init(&args, pyargs, 2);

  PYSCDC_TRACE("pyscdc_dataset_output_redirect: conf: '%s'", conf);

  scdc_dataset_output_unset(output);

  output = pyscdc_dataset_output_class2struct(pyoutput, output, PYSCDC_NEXT_UNPACK, 1);
  PYSCDC_TRACE_DATASET_INPUT(output, "pyscdc_dataset_output_destroy: IN output: ");

  PYSCDC_LIBSCDC_BEGIN();
  ret = scdc_dataset_output_redirect_intern(output, conf, &args);
  PYSCDC_LIBSCDC_END();

  PYSCDC_TRACE_DATASET_INPUT(output, "pyscdc_dataset_output_redirect: OUT output: ");

  pyscdc_args_py_release(&args);

  pyoutput_next = PyObject_GetAttrString(pyscdc_objs[PYSCDC_OBJ_SCDCMOD], "dataset_output_next_class");

  pyoutput = pyscdc_dataset_output_struct2class(output, pyoutput, PYSCDC_NEXT_PACK(pyoutput_next));
  PYSCDC_TRACE_DATASET_PYOUTPUT(pyoutput, "pyscdc_dataset_output_create: OUT pyoutput: ");

  PYSCDC_RETURN_BOOL(ret);
}
