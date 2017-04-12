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


#include <cstdio>
#include <cstdlib>
#include <string>
#include <algorithm>
#include <dirent.h>
#include <unistd.h>

#include "z_pack.h"

#define SCDC_TRACE_NOT  !SCDC_TRACE_DATASET_INOUT

#include "config.hh"
#include "common.hh"
#include "log.hh"
#include "args.hh"
#include "dataset_inout.h"


using namespace std;


#define SCDC_LOG_PREFIX  "dataset_inout: "


#define INPUT_CREATE_NEXT           1

#define TAR_SHELL  "/bin/sh"
#define TAR_QUIET  0

#define MAX_PATHNAME  256
#define FOLLOW_LINKS  1

#define SCDC_DATASET_INOUT_TYPE_NULL         -1
#define SCDC_DATASET_INOUT_TYPE_NONE         0
#define SCDC_DATASET_INOUT_TYPE_COPY         1
#define SCDC_DATASET_INOUT_TYPE_ALLOC        2
#define SCDC_DATASET_INOUT_TYPE_BUFFER       3
#define SCDC_DATASET_INOUT_TYPE_FD           4
#define SCDC_DATASET_INOUT_TYPE_FDDUP        5
#define SCDC_DATASET_INOUT_TYPE_STREAM       6
#define SCDC_DATASET_INOUT_TYPE_STREAMDUP    7
#define SCDC_DATASET_INOUT_TYPE_FILE         8
#define SCDC_DATASET_INOUT_TYPE_FS           9
#define SCDC_DATASET_INOUT_TYPE_FSLIST_DIR   10
#define SCDC_DATASET_INOUT_TYPE_FSLIST_FILE  11
#define SCDC_DATASET_INOUT_TYPE_FSLIST_LINK  12
#define SCDC_DATASET_INOUT_TYPE_PRODUCE      13
#define SCDC_DATASET_INOUT_TYPE_CONSUME      14


const scdc_buf_t scdc_buf_none = { 0, 0, 0 };


const scdc_dataset_inout_intern_t scdc_dataset_inout_intern_null =
{
  0, /* alloc_size */
  SCDC_DATASET_INOUT_TYPE_NULL, /* type */
  0, 0, /* buf, data */
  0, /* destroy */
};


#define INOUT_INTERN_BACKUP(_io_)   Z_MOP(if ((_io_)->intern) { (_io_)->intern->buf = SCDC_DATASET_INOUT_BUF_PTR(_io_); (_io_)->intern->data = (_io_)->data; })
#define INOUT_INTERN_RESTORE(_io_)  Z_MOP(if ((_io_)->intern) { SCDC_DATASET_INOUT_BUF_PTR(_io_) = (_io_)->intern->buf; (_io_)->data = (_io_)->intern->data; })


const scdc_dataset_inout_t scdc_dataset_inout_none =
{
  "", /* format */
#if !SCDC_DEPRECATED
  { 0, 0, 0 }, /* buf { ptr, size, current } */
#else /* !SCDC_DEPRECATED */
  0, 0, 0, /* buf, buf_size, current size */
#endif /* !SCDC_DEPRECATED */
  0, SCDC_DATASET_INOUT_TOTAL_SIZE_GIVEN_NONE, /* total_size, total_size_given */
  0, /* next */
  0, /* data */
  0, 0, /* intern, intern_data */
};


static scdcint_t scdc_dataset_inout_endl_next(scdc_dataset_inout_t *inout)
{
  if (inout) inout->next = scdc_dataset_inout_endl_next;

  return SCDC_SUCCESS;
}


const scdc_dataset_inout_t scdc_dataset_inout_endl =
{
  "", /* format */
#if !SCDC_DEPRECATED
  { 0, 0, 0 }, /* buf { ptr, size, current } */
#else /* !SCDC_DEPRECATED */
  0, 0, 0, /* buf, buf_size, current size */
#endif /* !SCDC_DEPRECATED */
  0, SCDC_DATASET_INOUT_TOTAL_SIZE_GIVEN_NONE, /* total_size, total_size_given */
  scdc_dataset_inout_endl_next, /* next */
  0, /* data */
  0, 0, /* intern, intern_data */
};


typedef struct _scdc_args_data_va_t
{
  scdcint_t arg;
  va_list *ap;

} scdc_args_data_va_t;


static scdc_arg_ref_t scdc_args_get_va(void *data, scdcint_t type, void *v)
{
  scdc_args_data_va_t *data_va = static_cast<scdc_args_data_va_t *>(data);

  SCDC_TRACE("scdc_args_get_va: data: '" << data_va << "' (arg: '" << data_va->arg << "', ap: '" << data_va->ap << "'), type: '" << type << "', v: " << v);

  switch (type)
  {
    case SCDC_ARGS_TYPE_INT:
      *((int *) v) = va_arg(*data_va->ap, int);
      data_va->arg += 1;
      return SCDC_ARG_REF_NONE;
    case SCDC_ARGS_TYPE_SCDCINT:
      *((scdcint_t *) v) = va_arg(*data_va->ap, scdcint_t);
      data_va->arg += 1;
      return SCDC_ARG_REF_NONE;
    case SCDC_ARGS_TYPE_CSTR:
    case SCDC_ARGS_TYPE_PTR:
    case SCDC_ARGS_TYPE_IN_STREAM:
    case SCDC_ARGS_TYPE_OUT_STREAM:
      *((void **) v) = va_arg(*data_va->ap, void *);
      data_va->arg += 1;
      return SCDC_ARG_REF_NONE;
    case SCDC_ARGS_TYPE_BUF:
      ((scdc_args_buf_t *) v)->buf = va_arg(*data_va->ap, void *);
      ((scdc_args_buf_t *) v)->buf_size = va_arg(*data_va->ap, scdcint_t);
      data_va->arg += 2;
      return SCDC_ARG_REF_NONE;
    case SCDC_ARGS_TYPE_DATASET_INPUT:
    {
      scdc_dataset_input_t *input = va_arg(*data_va->ap, scdc_dataset_input_t *);
      *((scdc_dataset_input_t *) v) = *input;
      data_va->arg += 1;
      return static_cast<scdc_arg_ref_t>(input);
    }
    case SCDC_ARGS_TYPE_DATASET_OUTPUT:
    {
      scdc_dataset_output_t *output = va_arg(*data_va->ap, scdc_dataset_output_t *);
      *((scdc_dataset_output_t *) v) = *output;
      data_va->arg += 1;
      return static_cast<scdc_arg_ref_t>(output);
    }
  }

  SCDC_TRACE("scdc_args_get_va: error: unknown type '" << type << "'");

  return SCDC_ARG_REF_NULL;
}


static scdc_arg_ref_t scdc_args_set_va(void *data, scdcint_t type, void *v, scdc_arg_ref_t arg_ref)
{
  SCDC_TRACE("scdc_args_set_va: data: '" << data << "', type: '" << type << "', v: '" << v << "', arg_ref: '" << arg_ref << "'");

  return SCDC_ARG_REF_NULL;
}


static void scdc_args_va_init(scdc_args_t *args, va_list *ap)
{
  scdc_args_data_va_t *data_va;


  data_va = new scdc_args_data_va_t;

  data_va->arg = 0;
  data_va->ap = ap;

  args->data = data_va;
  args->get = scdc_args_get_va;
  args->set = scdc_args_set_va;
}


static void scdc_args_va_release(scdc_args_t *args)
{
  scdc_args_data_va_t *data_va = static_cast<scdc_args_data_va_t *>(args->data);

  delete data_va;
}


void scdc_dataset_output_printf(int append, scdc_dataset_output_t *output, const char *fmt, ...)
{
  if (!output || !SCDC_DATASET_INOUT_BUF_PTR(output)) return;

  strncpy(output->format, "text", SCDC_FORMAT_MAX_SIZE);

  if (!append)
  {
    SCDC_DATASET_INOUT_BUF_CURRENT(output) = 0;
    output->total_size = 0;
  }

  char *buf = static_cast<char *>(SCDC_DATASET_INOUT_BUF_PTR(output));
  buf += SCDC_DATASET_INOUT_BUF_CURRENT(output);

  scdcint_t buf_size = SCDC_DATASET_INOUT_BUF_SIZE(output) - SCDC_DATASET_INOUT_BUF_CURRENT(output);

  va_list argptr;
  va_start(argptr, fmt);
  scdcint_t n = vsnprintf(buf, buf_size, fmt, argptr);
  va_end(argptr);

  n = min(buf_size, n);

  SCDC_DATASET_INOUT_BUF_CURRENT(output) += n;
  output->total_size += n;

  output->total_size_given = SCDC_DATASET_INOUT_TOTAL_SIZE_GIVEN_EXACT;

  output->next = 0;
  output->data = 0;
}


typedef int printf_t(const char *format, ...);

static void scdc_dataset_inout_prn(scdc_dataset_inout_t *inout, const char *prefix, printf_t *prn)
{
  prn("%s<", prefix);

  if (inout)
  {
    prn("format: '%s', buf: %" scdcint_fmt " at %p, total_size: %" scdcint_fmt ", total_size_given: %c, current_size: %" scdcint_fmt ", next: %p, data: %p, intern: %p",
      inout->format, SCDC_DATASET_INOUT_BUF_SIZE(inout), SCDC_DATASET_INOUT_BUF_PTR(inout), inout->total_size, inout->total_size_given, SCDC_DATASET_INOUT_BUF_CURRENT(inout), inout->next, inout->data, inout->intern);

    if (strcmp(inout->format, "text") == 0) prn(", content: '%.*s'", (int) SCDC_DATASET_INOUT_BUF_CURRENT(inout), (char *) SCDC_DATASET_INOUT_BUF_PTR(inout));
  }

  prn(">");
}


void scdc_dataset_input_log_cout_print(scdc_dataset_input_t *input)
{
  scdc_dataset_inout_prn(input, "input", scdc_log_cout_printf);
}


void scdc_dataset_output_log_cout_print(scdc_dataset_output_t *output)
{
  scdc_dataset_inout_prn(output, "output", scdc_log_cout_printf);
}


void scdc_dataset_input_print(scdc_dataset_input_t *input)
{
  scdc_dataset_inout_prn(input, "input", printf);
}


void scdc_dataset_output_print(scdc_dataset_output_t *output)
{
  scdc_dataset_inout_prn(output, "output", printf);
}


void scdc_dataset_inout_unset(scdc_dataset_inout_t *inout)
{
  if (!inout) return;

  inout->format[0] = '\0';

  SCDC_DATASET_INOUT_BUF_PTR(inout) = 0;
  SCDC_DATASET_INOUT_BUF_SIZE(inout) = 0;
  SCDC_DATASET_INOUT_BUF_CURRENT(inout) = 0;

  inout->total_size = 0;
  inout->total_size_given = SCDC_DATASET_INOUT_TOTAL_SIZE_GIVEN_NONE;

  inout->data = 0;
  inout->next = 0;

  inout->intern = 0;
  inout->intern_data = 0;
}


static bool scdc_dataset_inout_alloc(scdc_dataset_inout_t *inout, scdcint_t buf_size, scdcint_t data_size)
{
  if (!inout) return false;

  if (inout->intern) return true;

  scdcint_t alloc_size = sizeof(scdc_dataset_inout_intern_t) + data_size + buf_size;

#if USE_DEBUG
  char *b = static_cast<char *>(calloc(alloc_size, 1));
#else
  char *b = static_cast<char *>(malloc(alloc_size));
#endif

  if (!b) return false;

  inout->intern = (scdc_dataset_inout_intern_t *) b;
  b += sizeof(scdc_dataset_inout_intern_t);

  inout->intern->alloc_size = alloc_size;

  if (data_size > 0)
  {
    inout->data = b;
    b += data_size;
  }

  if (buf_size > 0)
  {
    SCDC_DATASET_INOUT_BUF_PTR(inout) = b;
    SCDC_DATASET_INOUT_BUF_SIZE(inout) = buf_size;
  }

  return true;
}


static void scdc_dataset_inout_free(scdc_dataset_inout_t *inout)
{
  free(inout->intern);
  
  inout->intern = 0;
  
  SCDC_DATASET_INOUT_BUF_PTR(inout) = 0;

  inout->data = 0;
}


void scdc_dataset_input_unset(scdc_dataset_input_t *input)
{
  scdc_dataset_inout_unset(input);
}


void scdc_dataset_output_unset(scdc_dataset_output_t *output)
{
  scdc_dataset_inout_unset(output);
}


void scdc_dataset_output_copy(scdc_dataset_output_t *src, scdc_dataset_output_t *dst)
{
  *dst = *src;
}


/* type: output copy */
/*static void scdc_dataset_output_copy_destroy(scdc_dataset_output_t *output);*/

static scdc_dataset_output_t *scdc_dataset_output_copy_create(scdc_dataset_output_t *output, const char *conf, scdc_args_t *args)
{
  if (!output) return 0;

  scdc_args xargs(args);

  xargs.get<scdc_dataset_output_t>(SCDC_ARGS_TYPE_DATASET_OUTPUT, output);

  if (output->intern)
  {
    output->intern->type = SCDC_DATASET_INOUT_TYPE_COPY;
    output->intern->destroy = 0;
  }

  return output;
}


/*static void scdc_dataset_output_copy_destroy(scdc_dataset_output_t *output)
{
}*/


/* type: inout alloc */
static void scdc_dataset_inout_alloc_destroy(scdc_dataset_inout_t *inout);

static scdc_dataset_inout_t *scdc_dataset_inout_alloc_create(scdc_dataset_inout_t *inout, const char *conf, scdc_args_t *args)
{
  scdcint_t size = DEFAULT_DATASET_INOUT_BUF_SIZE;

  stringlist confs(':', conf);
  scdc_args xargs(args);

  if (confs.front() == "size")
  {
    xargs.get<scdcint_t>(SCDC_ARGS_TYPE_SCDCINT, &size);
    confs.front_pop();
  }

  if (!scdc_dataset_inout_alloc(inout, size, 0)) return 0;

  inout->intern->type = SCDC_DATASET_INOUT_TYPE_ALLOC;
  inout->intern->destroy = scdc_dataset_inout_alloc_destroy;

  return inout;
}


static void scdc_dataset_inout_alloc_destroy(scdc_dataset_inout_t *inout)
{
  if (!inout) return;

  scdc_dataset_inout_free(inout);
}


/* type: input alloc */
static void scdc_dataset_input_alloc_destroy(scdc_dataset_input_t *input);

static scdc_dataset_input_t *scdc_dataset_input_alloc_create(scdc_dataset_input_t *input, const char *conf, scdc_args_t *args)
{
  input = scdc_dataset_inout_alloc_create(input, conf, args);

  if (input) input->intern->destroy = scdc_dataset_input_alloc_destroy; 

  return input;
}


static void scdc_dataset_input_alloc_destroy(scdc_dataset_input_t *input)
{
  if (!input) return;

  scdc_dataset_inout_alloc_destroy(input);
}


/* type: output alloc */
static void scdc_dataset_output_alloc_destroy(scdc_dataset_output_t *output);

static scdc_dataset_output_t *scdc_dataset_output_alloc_create(scdc_dataset_output_t *output, const char *conf, scdc_args_t *args)
{
  output = scdc_dataset_inout_alloc_create(output, conf, args);

  if (!output) return 0;

  output->intern->destroy = scdc_dataset_output_alloc_destroy;

  return output;
}


static void scdc_dataset_output_alloc_destroy(scdc_dataset_output_t *output)
{
  if (!output) return;

  scdc_dataset_inout_alloc_destroy(output);
}


/* type: input buffer */
static void scdc_dataset_input_buffer_destroy(scdc_dataset_input_t *input);

static scdc_dataset_input_t *scdc_dataset_input_buffer_create(scdc_dataset_input_t *input, const char *conf, scdc_args_t *args)
{
  scdc_args xargs(args);

  scdc_args_buf_t buffer;
  xargs.get<scdc_args_buf_t>(SCDC_ARGS_TYPE_BUF, &buffer);

  if (!scdc_dataset_inout_alloc(input, 0, 0)) return 0;

  input->intern->type = SCDC_DATASET_INOUT_TYPE_BUFFER;
  input->intern->destroy = scdc_dataset_input_buffer_destroy;

  SCDC_DATASET_INOUT_BUF_PTR(input) = buffer.buf;
  SCDC_DATASET_INOUT_BUF_SIZE(input) = buffer.buf_size;

  strcpy(input->format, "data");

  SCDC_DATASET_INOUT_BUF_CURRENT(input) = SCDC_DATASET_INOUT_BUF_SIZE(input);
  input->total_size = SCDC_DATASET_INOUT_BUF_SIZE(input);
  input->total_size_given = SCDC_DATASET_INOUT_TOTAL_SIZE_GIVEN_EXACT;

  return input;
}


static void scdc_dataset_input_buffer_destroy(scdc_dataset_input_t *input)
{
  if (!input) return;

  scdc_dataset_inout_free(input);
}


/* type: output buffer */
static void scdc_dataset_output_buffer_destroy(scdc_dataset_output_t *output);

static scdc_dataset_output_t *scdc_dataset_output_buffer_create(scdc_dataset_output_t *output, const char *conf, scdc_args_t *args)
{
  scdc_args xargs(args);

  scdc_args_buf_t v;
  xargs.get<scdc_args_buf_t>(SCDC_ARGS_TYPE_BUF, &v);

  if (!scdc_dataset_inout_alloc(output, 0, 0)) return 0;

  output->intern->type = SCDC_DATASET_INOUT_TYPE_BUFFER;
  output->intern->destroy = scdc_dataset_output_buffer_destroy;

  SCDC_DATASET_INOUT_BUF_PTR(output) = v.buf;
  SCDC_DATASET_INOUT_BUF_SIZE(output) = v.buf_size;

  strcpy(output->format, "data");

  return output;
}


static void scdc_dataset_output_buffer_destroy(scdc_dataset_output_t *output)
{
  if (!output) return;

  scdc_dataset_inout_free(output);
}


template<typename T>
struct inout_FX_data_t
{
  T fx;
  scdcint_t chunk_size;
  scdcint_t size_left;

  scdcint_t read(void *b, scdcint_t n);
  scdcint_t write(void *b, scdcint_t n);

  scdcint_t seek(scdcint_t offset, int whence);
  scdcint_t tell();

  scdcint_t eof(scdcint_t last_read);
  scdcint_t sync();
  scdcint_t truncate(scdcint_t n);

  scdcint_t get_size_left();
};


template<>
scdcint_t inout_FX_data_t<int>::read(void *b, scdcint_t n)
{
  return ::read(fx, b, n);
}


template<>
scdcint_t inout_FX_data_t<int>::write(void *b, scdcint_t n)
{
  return ::write(fx, b, n);
}


template<>
scdcint_t inout_FX_data_t<int>::seek(scdcint_t offset, int whence)
{
  return ::lseek(fx, offset, whence);
}


template<>
scdcint_t inout_FX_data_t<int>::tell()
{
  return ::lseek(fx, 0, SEEK_CUR);
}


template<>
scdcint_t inout_FX_data_t<int>::eof(scdcint_t last_read)
{
  return (last_read == 0);
}


template<>
scdcint_t inout_FX_data_t<int>::sync()
{
  return ::fsync(fx);
}


template<>
scdcint_t inout_FX_data_t<int>::truncate(scdcint_t n)
{
  return ::ftruncate(fx, n);
}


template<>
scdcint_t inout_FX_data_t<int>::get_size_left()
{
  scdcint_t pos = lseek(fx, 0, SEEK_CUR);

  if (pos < 0) return -1;

  scdcint_t size = lseek(fx, 0, SEEK_END);

  if (size < 0) return -1;

  size -= pos;

  lseek(fx, pos, SEEK_SET);

  return size;
}


template<>
scdcint_t inout_FX_data_t<FILE *>::read(void *b, scdcint_t n)
{
  return ::fread(b, 1, n, fx);
}


template<>
scdcint_t inout_FX_data_t<FILE *>::write(void *b, scdcint_t n)
{
  return ::fwrite(b, 1, n, fx);
}


template<>
scdcint_t inout_FX_data_t<FILE *>::seek(scdcint_t offset, int whence)
{
  return ::fseeko(fx, offset, whence);
}


template<>
scdcint_t inout_FX_data_t<FILE *>::tell()
{
  return ::ftello(fx);
}


template<>
scdcint_t inout_FX_data_t<FILE *>::eof(scdcint_t last_read)
{
  return feof(fx);
}


template<>
scdcint_t inout_FX_data_t<FILE *>::sync()
{
  return ::fsync(fileno(fx));
}


template<>
scdcint_t inout_FX_data_t<FILE *>::truncate(scdcint_t n)
{
  return ::ftruncate(fileno(fx), n);
}


template<>
scdcint_t inout_FX_data_t<FILE *>::get_size_left()
{
  scdcint_t pos = ftello(fx);

  if (pos < 0 || fseeko(fx, 0, SEEK_END) < 0) return -1;

  scdcint_t size = ftello(fx);

  if (size < 0) return -1;

  size -= pos;

  fseeko(fx, pos, SEEK_SET);

  return size;
}


/* type: input FILE */
template<typename T>
static scdcint_t input_FX_next(scdc_dataset_input_t *input)
{
  inout_FX_data_t<T> *data = static_cast<inout_FX_data_t<T> *>(input->data);

  size_t n = SCDC_DATASET_INOUT_BUF_SIZE(input);

  if (data->chunk_size >= 0) n = z_min(static_cast<scdcint_t>(n), data->chunk_size);

  if (data->size_left >= 0) n = z_min(static_cast<scdcint_t>(n), data->size_left);

  n = data->read(SCDC_DATASET_INOUT_BUF_PTR(input), n);

  data->size_left -= n;

  /* set to the number of bytes read in buf */
  SCDC_DATASET_INOUT_BUF_CURRENT(input) = n;

  if (input->total_size_given == SCDC_DATASET_INOUT_TOTAL_SIZE_GIVEN_AT_LEAST) input->total_size += n;

  if (data->eof(n) || data->size_left == 0)
  {
    input->total_size_given = SCDC_DATASET_INOUT_TOTAL_SIZE_GIVEN_EXACT;
    input->next = 0;
  }

  return SCDC_SUCCESS;
}


template<typename T>
static void scdc_dataset_input_FX_destroy(scdc_dataset_input_t *input, T *fx = 0);

template<typename T>
static scdc_dataset_inout_t *scdc_dataset_input_FX_create(scdc_dataset_input_t *input, const char *conf, scdc_args_t *args, T fx)
{
  bool offset_set = false;
  bool offset_front = true;
  scdcint_t offset = 0;
  scdcint_t size = -1;
  scdcint_t chunk_size = -1;
  scdc_args_buf_t buffer = { 0, DEFAULT_DATASET_INOUT_BUF_SIZE };

  stringlist confs(':', conf);
  scdc_args xargs(args);

  scdc_dataset_inout_t *inout_redirect = 0;
  if (xargs.get<scdc_dataset_inout_t *>(SCDC_ARGS_TYPE_DATASET_INOUT_REDIRECT_PTR, &inout_redirect) != SCDC_ARG_REF_NULL && inout_redirect)
  {
    buffer.buf = SCDC_DATASET_INOUT_BUF_PTR(inout_redirect);
    buffer.buf_size = SCDC_DATASET_INOUT_BUF_SIZE(inout_redirect);
  }

  string c;
  scdcint_t i = 0;
  while (confs.front_pop(c))
  {
    if (c == "offset")
    {
      if (xargs.get<scdcint_t>(SCDC_ARGS_TYPE_SCDCINT, &offset) == SCDC_ARG_REF_NULL)
      {
        SCDC_ERROR("scdc_dataset_input_FXX_create: getting file offset");
        return 0;
      }

      offset_set = true;

    } else if (c == "size")
    {
      if (xargs.get<scdcint_t>(SCDC_ARGS_TYPE_SCDCINT, &size) == SCDC_ARG_REF_NULL)
      {
        SCDC_ERROR("scdc_dataset_input_FXX_create: getting file size");
        return 0;
      }

    } else if (c == "chunks")
    {
      if (xargs.get<scdcint_t>(SCDC_ARGS_TYPE_SCDCINT, &chunk_size) == SCDC_ARG_REF_NULL)
      {
        SCDC_ERROR("scdc_dataset_input_FXX_create: getting chunk size");
        return 0;
      }

    } else if (c == "buffer")
    {
      if (xargs.get<scdc_args_buf_t>(SCDC_ARGS_TYPE_BUF, &buffer) == SCDC_ARG_REF_NULL)
      {
        SCDC_ERROR("scdc_dataset_input_FXX_create: getting buffer");
        return 0;
      }

    } else
    {
      char r;

      if (!c.empty())
      switch (i)
      {
        case 0:
          r = toupper(c[c.size() - 1]);
          if (r == 'F' || r == 'B') { offset_front = (r == 'F'); c.resize(c.size() - 1); }
          scdc_args_string(c).get<scdcint_t>(c, SCDC_ARGS_TYPE_SCDCINT, &offset);
          offset_set = true;
          break;
        case 1: scdc_args_string(c).get<scdcint_t>(c, SCDC_ARGS_TYPE_SCDCINT, &size); break;
        case 2: scdc_args_string(c).get<scdcint_t>(c, SCDC_ARGS_TYPE_SCDCINT, &chunk_size); break;
        default: SCDC_FAIL("scdc_dataset_input_FXX_create: ignoring parameter '" << c << "'"); break;
      }

      ++i;
    }
  }

  SCDC_TRACE("scdc_dataset_input_FXX_create: offset: " << offset << ", size: " << size << ", chunks: " << chunk_size <<
             ", buffer: " << buffer.buf << " / " << buffer.buf_size);

  if (!input) return 0;

  if (!scdc_dataset_inout_alloc(input, (buffer.buf?0:buffer.buf_size), sizeof(inout_FX_data_t<T>))) return 0;

  if (buffer.buf)
  {
    SCDC_DATASET_INOUT_BUF_PTR(input) = buffer.buf;
    SCDC_DATASET_INOUT_BUF_SIZE(input) = buffer.buf_size;
  }

  input->intern->type = SCDC_DATASET_INOUT_TYPE_NONE;
  input->intern->destroy = NULL;

  strcpy(input->format, "data");

  inout_FX_data_t<T> *data = static_cast<inout_FX_data_t<T> *>(input->data);

  data->fx = fx;
  data->chunk_size = chunk_size;
  data->size_left = size;

  if (offset_set && data->seek(offset, (offset_front)?SEEK_SET:SEEK_END) < 0)
  {
    SCDC_ERROR("scdc_dataset_input_FXX_create: seek to offset " << offset << " from " << ((offset_front)?"front":"back"));
    return 0;
  }

  SCDC_TRACE("scdc_dataset_input_FXX_create: offset: " << data->tell());

#if DETERMINE_EXACT_TOTAL_SIZE
  scdcint_t size_left = (fd != -1)?get_FD_size_left(fd):((file != NULL)?get_FILE_size_left(file):-1);

  if (size_left >= 0)
  {
    input->total_size = size_left;
    if (size >= 0) input->total_size = z_min(input->total_size, size);
    input->total_size_given = SCDC_DATASET_INOUT_TOTAL_SIZE_GIVEN_EXACT;

  } else
#endif
  {
    if (size >= 0)
    {
      input->total_size = size;
      input->total_size_given = SCDC_DATASET_INOUT_TOTAL_SIZE_GIVEN_AT_MOST;

    } else
    {
      input->total_size = 0;
      input->total_size_given = SCDC_DATASET_INOUT_TOTAL_SIZE_GIVEN_AT_LEAST;
    }
  }

  SCDC_DATASET_INOUT_BUF_CURRENT(input) = 0;

  input->next = input_FX_next<T>;

  return input;
}

#undef DETERMINE_EXACT_TOTAL_SIZE


template<typename T>
static void scdc_dataset_input_FX_destroy(scdc_dataset_input_t *input, T *fx)
{
  if (!input) return;

  if (fx) *fx = static_cast<inout_FX_data_t<T> *>(input->data)->fx;

  scdc_dataset_inout_free(input);
}


/* type: output FILE */

#define SYNC_AFTER_WRITE  0

template<typename T>
static scdcint_t output_FX_next(scdc_dataset_output_t *output)
{
  inout_FX_data_t<T> *data = static_cast<inout_FX_data_t<T> *>(output->data);

  size_t n = SCDC_DATASET_INOUT_BUF_CURRENT(output);

  if (data->chunk_size >= 0) n = z_min(static_cast<scdcint_t>(n), data->chunk_size);

  if (data->size_left >= 0) n = z_min(static_cast<scdcint_t>(n), data->size_left);

  n = data->write(SCDC_DATASET_INOUT_BUF_PTR(output), n);

#if SYNC_AFTER_WRITE
  data->sync();
#endif

  data->size_left -= n;

  /* set to the nuber of bytes that have been written */
  SCDC_DATASET_INOUT_BUF_CURRENT(output) = n;

  if (data->size_left == 0)
  {
    output->next = 0;
  }

  return SCDC_SUCCESS;
}

#undef SYNC_AFTER_WRITE


template<typename T>
static void scdc_dataset_output_FX_destroy(scdc_dataset_output_t *output, T *fx = 0);

template<typename T>
static scdc_dataset_output_t *scdc_dataset_output_FX_create(scdc_dataset_output_t *output, const char *conf, scdc_args_t *args, T fx)
{
  bool offset_set = false;
  bool offset_front = true;
  scdcint_t offset = 0;
  scdcint_t size = -1;
  scdcint_t chunk_size = -1;
  scdc_args_buf_t buffer = { 0, DEFAULT_DATASET_INOUT_BUF_SIZE };
  bool trunc = false;

  stringlist confs(':', conf);
  scdc_args xargs(args);

  scdc_dataset_inout_t *inout_redirect = 0;
  if (xargs.get<scdc_dataset_inout_t *>(SCDC_ARGS_TYPE_DATASET_INOUT_REDIRECT_PTR, &inout_redirect) != SCDC_ARG_REF_NULL && inout_redirect)
  {
    buffer.buf = SCDC_DATASET_INOUT_BUF_PTR(inout_redirect);
    buffer.buf_size = SCDC_DATASET_INOUT_BUF_SIZE(inout_redirect);
  }

  string c;
  scdcint_t i = 0;
  while (confs.front_pop(c))
  {
    if (c == "offset")
    {
      if (xargs.get<scdcint_t>(SCDC_ARGS_TYPE_SCDCINT, &offset) == SCDC_ARG_REF_NULL)
      {
        SCDC_ERROR("scdc_dataset_output_FX_create: getting file offset");
        return 0;
      }

      offset_set = true;

    } else if (c == "size")
    {
      if (xargs.get<scdcint_t>(SCDC_ARGS_TYPE_SCDCINT, &size) == SCDC_ARG_REF_NULL)
      {
        SCDC_ERROR("scdc_dataset_output_FX_create: getting file size");
        return 0;
      }

    } else if (c == "chunks")
    {
      if (xargs.get<scdcint_t>(SCDC_ARGS_TYPE_SCDCINT, &chunk_size) == SCDC_ARG_REF_NULL)
      {
        SCDC_ERROR("scdc_dataset_output_FX_create: getting chunk size");
        return 0;
      }

    } else if (c == "buffer")
    {
      if (xargs.get<scdc_args_buf_t>(SCDC_ARGS_TYPE_BUF, &buffer) == SCDC_ARG_REF_NULL)
      {
        SCDC_ERROR("scdc_dataset_output_FX_create: getting buffer");
        return 0;
      }

    } else if (c == "trunc")
    {
      trunc = true;

    } else
    {
      char r;

      if (!c.empty())
      switch (i)
      {
        case 0:
          r = toupper(c[c.size() - 1]);
          if (r == 'F' || r == 'B') { offset_front = (r == 'F'); c.resize(c.size() - 1); }
          scdc_args_string(c).get<scdcint_t>(c, SCDC_ARGS_TYPE_SCDCINT, &offset);
          offset_set = true;
          break;
        case 1: scdc_args_string(c).get<scdcint_t>(c, SCDC_ARGS_TYPE_SCDCINT, &size); break;
        case 2: scdc_args_string(c).get<scdcint_t>(c, SCDC_ARGS_TYPE_SCDCINT, &chunk_size); break;
        default: SCDC_FAIL("scdc_dataset_output_FX_create: ignoring parameter '" << c << "'"); break;
      }

      ++i;
    }
  }

  SCDC_TRACE("scdc_dataset_output_FX_create: offset: " << offset << ", size: " << size << ", chunks: " << chunk_size <<
             ", buffer: " << buffer.buf << " / " << buffer.buf_size << ", trunc: " << trunc);

  if (!output) return 0;

  if (!scdc_dataset_inout_alloc(output, (buffer.buf?0:buffer.buf_size), sizeof(inout_FX_data_t<T>))) return 0;

  if (buffer.buf)
  {
    SCDC_DATASET_INOUT_BUF_PTR(output) = buffer.buf;
    SCDC_DATASET_INOUT_BUF_SIZE(output) = buffer.buf_size;
  }

  output->intern->type = SCDC_DATASET_INOUT_TYPE_NONE;
  output->intern->destroy = NULL;

  strcpy(output->format, "data");

  inout_FX_data_t<T> *data = static_cast<inout_FX_data_t<T> *>(output->data);

  data->fx = fx;
  data->chunk_size = chunk_size;
  data->size_left = size;

  if (trunc && data->truncate(0) != 0)
  {
    SCDC_ERROR("scdc_dataset_output_FX_create: truncate file");
    return 0;
  }

  if (offset_set && data->seek(offset, (offset_front)?SEEK_SET:SEEK_END) < 0)
  {
    SCDC_ERROR("scdc_dataset_output_FX_create: seek to offset " << offset << " from " << ((offset_front)?"front":"back"));
    return 0;
  }

  SCDC_TRACE("scdc_dataset_output_FX_create: offset: " << data->tell());

  output->next = output_FX_next<T>;

  return output;
}


template<typename T>
static void scdc_dataset_output_FX_destroy(scdc_dataset_output_t *output, T *fx)
{
  if (!output) return;

  if (fx) *fx = static_cast<inout_FX_data_t<T> *>(output->data)->fx;

  scdc_dataset_inout_free(output);
}


/* type: input fd */
static void scdc_dataset_input_fd_destroy(scdc_dataset_input_t *input);

static scdc_dataset_inout_t *scdc_dataset_input_fd_create(scdc_dataset_input_t *input, const char *conf, scdc_args_t *args)
{
  scdc_args xargs(args);

  int fd;
  xargs.get<int>(SCDC_ARGS_TYPE_INT, &fd);

  if (!input) return 0;

  input = scdc_dataset_input_FX_create<int>(input, conf, args, fd);

  if (!input) return 0;

  input->intern->type = SCDC_DATASET_INOUT_TYPE_FD;
  input->intern->destroy = scdc_dataset_input_fd_destroy;

#if INPUT_CREATE_NEXT
  input->next(input);
#endif

  return input;
}


static void scdc_dataset_input_fd_destroy(scdc_dataset_input_t *input)
{
  if (!input) return;

  scdc_dataset_input_FX_destroy<int>(input);
}


/* type: output fd */
static void scdc_dataset_output_fd_destroy(scdc_dataset_output_t *output);

static scdc_dataset_output_t *scdc_dataset_output_fd_create(scdc_dataset_output_t *output, const char *conf, scdc_args_t *args)
{
  scdc_args xargs(args);

  int fd;
  xargs.get<int>(SCDC_ARGS_TYPE_INT, &fd);

  if (!output) return 0;

  output = scdc_dataset_output_FX_create<int>(output, conf, args, fd);

  if (!output) return 0;

  output->intern->type = SCDC_DATASET_INOUT_TYPE_STREAM;
  output->intern->destroy = scdc_dataset_output_fd_destroy;

  return output;
}


static void scdc_dataset_output_fd_destroy(scdc_dataset_output_t *output)
{
  if (!output) return;

  scdc_dataset_output_FX_destroy<int>(output);
}


/* type: input fddup */
static void scdc_dataset_input_fddup_destroy(scdc_dataset_input_t *input);

static scdc_dataset_inout_t *scdc_dataset_input_fddup_create(scdc_dataset_input_t *input, const char *conf, scdc_args_t *args)
{
  scdc_args xargs(args);

  int fd;
  xargs.get<int>(SCDC_ARGS_TYPE_INT, &fd);

  if (!input) return 0;

  int fddup = dup(fd);

  input = scdc_dataset_input_FX_create<int>(input, conf, args, fddup);

  if (!input) return 0;

  input->intern->type = SCDC_DATASET_INOUT_TYPE_FDDUP;
  input->intern->destroy = scdc_dataset_input_fddup_destroy;

#if INPUT_CREATE_NEXT
  input->next(input);
#endif

  return input;
}


static void scdc_dataset_input_fddup_destroy(scdc_dataset_input_t *input)
{
  if (!input) return;

  int fddup;

  scdc_dataset_input_FX_destroy<int>(input, &fddup);

  close(fddup);
}


/* type: output fddup */
static void scdc_dataset_output_fddup_destroy(scdc_dataset_output_t *output);

static scdc_dataset_output_t *scdc_dataset_output_fddup_create(scdc_dataset_output_t *output, const char *conf, scdc_args_t *args)
{
  scdc_args xargs(args);

  int fd;
  xargs.get<int>(SCDC_ARGS_TYPE_INT, &fd);

  if (!output) return 0;

  int fddup = dup(fd);

  output = scdc_dataset_output_FX_create<int>(output, conf, args, fddup);

  if (!output) return 0;

  output->intern->type = SCDC_DATASET_INOUT_TYPE_STREAM;
  output->intern->destroy = scdc_dataset_output_fddup_destroy;

  return output;
}


static void scdc_dataset_output_fddup_destroy(scdc_dataset_output_t *output)
{
  if (!output) return;

  int fddup;

  scdc_dataset_output_FX_destroy<int>(output, &fddup);

  close(fddup);
}


/* type: input stream */
static void scdc_dataset_input_stream_destroy(scdc_dataset_input_t *input);

static scdc_dataset_inout_t *scdc_dataset_input_stream_create(scdc_dataset_input_t *input, const char *conf, scdc_args_t *args)
{
  scdc_args xargs(args);

  FILE *stream = 0;
  xargs.get<FILE *>(SCDC_ARGS_TYPE_IN_STREAM, &stream);

  if (!stream) stream = stdin;

  if (!input) return 0;

  input = scdc_dataset_input_FX_create<FILE *>(input, conf, args, stream);

  if (!input) return 0;

  input->intern->type = SCDC_DATASET_INOUT_TYPE_STREAM;
  input->intern->destroy = scdc_dataset_input_stream_destroy;

#if INPUT_CREATE_NEXT
  input->next(input);
#endif

  return input;
}


static void scdc_dataset_input_stream_destroy(scdc_dataset_input_t *input)
{
  if (!input) return;

  scdc_dataset_input_FX_destroy<FILE *>(input);
}


/* type: output stream */
static void scdc_dataset_output_stream_destroy(scdc_dataset_output_t *output);

static scdc_dataset_output_t *scdc_dataset_output_stream_create(scdc_dataset_output_t *output, const char *conf, scdc_args_t *args)
{
  scdc_args xargs(args);

  FILE *stream = 0;
  xargs.get<FILE *>(SCDC_ARGS_TYPE_OUT_STREAM, &stream);

  if (!stream) stream = stdout;

  if (!output) return 0;

  output = scdc_dataset_output_FX_create<FILE *>(output, conf, args, stream);

  if (!output) return 0;

  output->intern->type = SCDC_DATASET_INOUT_TYPE_STREAM;
  output->intern->destroy = scdc_dataset_output_stream_destroy;

  return output;
}


static void scdc_dataset_output_stream_destroy(scdc_dataset_output_t *output)
{
  if (!output) return;

  scdc_dataset_output_FX_destroy<FILE *>(output);
}


/* type: input streamdup */
static void scdc_dataset_input_streamdup_destroy(scdc_dataset_input_t *input);

static scdc_dataset_inout_t *scdc_dataset_input_streamdup_create(scdc_dataset_input_t *input, const char *conf, scdc_args_t *args)
{
  scdc_args xargs(args);

  FILE *stream = 0;
  xargs.get<FILE *>(SCDC_ARGS_TYPE_IN_STREAM, &stream);

  if (!stream) stream = stdin;

  if (!input) return 0;

  scdcint_t pos = ftello(stream);

  /* we use a duplicated fd, because using only a newly opened stream (with fdopen) leads to empty reads if the original stream is closed */
  int fddup = dup(fileno(stream));
  stream = fdopen(fddup, "r");

  if (!stream) return 0;

  if (pos >= 0) fseeko(stream, pos, SEEK_SET);

  input = scdc_dataset_input_FX_create<FILE *>(input, conf, args, stream);

  if (!input) return 0;

  input->intern->type = SCDC_DATASET_INOUT_TYPE_STREAMDUP;
  input->intern->destroy = scdc_dataset_input_streamdup_destroy;

#if INPUT_CREATE_NEXT
  input->next(input);
#endif

  return input;
}


static void scdc_dataset_input_streamdup_destroy(scdc_dataset_input_t *input)
{
  if (!input) return;

  FILE *stream;

  scdc_dataset_input_FX_destroy<FILE *>(input, &stream);

  int fddup = fileno(stream);
  fclose(stream);
  close(fddup);
}


/* type: output streamdup */
static void scdc_dataset_output_streamdup_destroy(scdc_dataset_output_t *output);

static scdc_dataset_output_t *scdc_dataset_output_streamdup_create(scdc_dataset_output_t *output, const char *conf, scdc_args_t *args)
{
  scdc_args xargs(args);

  FILE *stream = 0;
  xargs.get<FILE *>(SCDC_ARGS_TYPE_OUT_STREAM, &stream);

  if (!stream) stream = stdout;

  if (!output) return 0;

  scdcint_t pos = ftello(stream);

  /* we use a duplicated fd, because using only a newly opened stream (with fdopen) leads to empty reads if the original stream is closed */
  int fddup = dup(fileno(stream));
  stream = fdopen(fddup, "r+");  /* try opening an existing file */
  if (!stream) stream = fdopen(fddup, "w");  /* create a new file is necessary */

  if (!stream) return 0;

  if (pos >= 0) fseeko(stream, pos, SEEK_SET);

  output = scdc_dataset_output_FX_create<FILE *>(output, conf, args, stream);

  if (!output) return 0;

  output->intern->type = SCDC_DATASET_INOUT_TYPE_STREAM;
  output->intern->destroy = scdc_dataset_output_streamdup_destroy;

  return output;
}


static void scdc_dataset_output_streamdup_destroy(scdc_dataset_output_t *output)
{
  if (!output) return;

  FILE *stream;

  scdc_dataset_output_FX_destroy<FILE *>(output, &stream);

  int fddup = fileno(stream);
  fclose(stream);
  close(fddup);
}


/* type: input file */
static void scdc_dataset_input_file_destroy(scdc_dataset_input_t *input);

static scdc_dataset_inout_t *scdc_dataset_input_file_create(scdc_dataset_input_t *input, const char *conf, scdc_args_t *args)
{
  scdc_args xargs(args);

  const char *filename;
  xargs.get<const char *>(SCDC_ARGS_TYPE_CSTR, &filename);

  SCDC_TRACE("scdc_dataset_input_file_create: filename: '" << filename << "'");

  if (!input) return 0;

  FILE *file = fopen(filename, "r");

  if (!file)
  {
    SCDC_FAIL("scdc_dataset_input_file_create: opening file '" << filename << "' failed!");
    return 0;
  }

  input = scdc_dataset_input_FX_create<FILE *>(input, conf, args, file);

  if (!input)
  {
    SCDC_FAIL("scdc_dataset_input_file_create: creating input from FILE failed!");
    fclose(file);
    return 0;
  }

  input->intern->type = SCDC_DATASET_INOUT_TYPE_FILE;
  input->intern->destroy = scdc_dataset_input_file_destroy;

#if INPUT_CREATE_NEXT
  input->next(input);
#endif

  return input;
}


static void scdc_dataset_input_file_destroy(scdc_dataset_input_t *input)
{
  if (!input) return;

  FILE *file;

  scdc_dataset_input_FX_destroy<FILE *>(input, &file);

  fclose(file);
}


/* type: output file */
static void scdc_dataset_output_file_destroy(scdc_dataset_output_t *output);

static scdc_dataset_output_t *scdc_dataset_output_file_create(scdc_dataset_output_t *output, const char *conf, scdc_args_t *args)
{
  scdc_args xargs(args);

  const char *filename;
  xargs.get<const char *>(SCDC_ARGS_TYPE_CSTR, &filename);

  SCDC_TRACE("scdc_dataset_output_file_create: filename: '" << filename << "'");

  if (!output) return 0;

  FILE *file = fopen(filename, (z_fs_exists(filename))?"r+":"w");

  if (!file)
  {
    SCDC_TRACE("scdc_dataset_output_file_create: file '" << filename << "' failed!");
    return 0;
  }

  output = scdc_dataset_output_FX_create<FILE *>(output, conf, args, file);

  if (!output)
  {
    fclose(file);
    return 0;
  }

  output->intern->type = SCDC_DATASET_INOUT_TYPE_FILE;
  output->intern->destroy = scdc_dataset_output_file_destroy;

  return output;
}


static void scdc_dataset_output_file_destroy(scdc_dataset_output_t *output)
{
  if (!output) return;

  FILE *file;

  scdc_dataset_output_FX_destroy<FILE *>(output, &file);

  fclose(file);
}


/* input: type fs */
static void scdc_dataset_input_fs_destroy(scdc_dataset_input_t *input);

static scdc_dataset_inout_t *scdc_dataset_input_fs_create(scdc_dataset_input_t *input, const char *conf, scdc_args_t *args)
{
  scdc_args xargs(args);

  const char *dirname;
  xargs.get<const char *>(SCDC_ARGS_TYPE_CSTR, &dirname);

  SCDC_TRACE("scdc_dataset_input_fs_create: dirname: '" << dirname << "', trimed: '" << trim(dirname) << "'");

  if (trim(dirname).size() <= 0) dirname = ".";

  if (!(z_fs_is_directory(dirname)))
  {
    SCDC_FAIL("scdc_dataset_input_fs_create: directory '" << dirname << "' does not exist!");
    return 0;
  }

  const char *pathname;
  xargs.get<const char *>(SCDC_ARGS_TYPE_CSTR, &pathname);

  if (trim(pathname).size() <= 0) pathname = "-T /dev/null";

  SCDC_TRACE("scdc_dataset_input_fs_create: path: '" << pathname << "'");

  char tarcmd[MAX_PATHNAME];
  sprintf(tarcmd,
#ifdef TAR_SHELL
    TAR_SHELL " -c \""
#endif
    "cd %s && tar --ignore-failed-read -c %s"
#if TAR_QUIET
    " 2>/dev/null"
#endif
#ifdef TAR_SHELL
    "\""
#endif
    , dirname, pathname);

  SCDC_TRACE("scdc_dataset_input_fs_create: tarcmd: '" << tarcmd << "'");

  if (!input) return 0;

  FILE *ptar = popen(tarcmd, "r");

  if (!ptar)
  {
    SCDC_FAIL("scdc_dataset_input_fs_create: cmd '" << tarcmd << "' failed!");
    return 0;
  }

  input = scdc_dataset_input_FX_create<FILE *>(input, conf, args, ptar);

  if (!input)
  {
    SCDC_FAIL("scdc_dataset_input_fs_create: creating input from FILE failed!");
    pclose(ptar);
    return 0;
  }

  input->intern->type = SCDC_DATASET_INOUT_TYPE_FS;
  input->intern->destroy = scdc_dataset_input_fs_destroy;

  strcpy(input->format, "fstar");

#if INPUT_CREATE_NEXT
  input->next(input);
#endif

  return input;
}


static void scdc_dataset_input_fs_destroy(scdc_dataset_input_t *input)
{
  if (!input) return;

  FILE *ptar;

  scdc_dataset_input_FX_destroy<FILE *>(input, &ptar);

  pclose(ptar);
}


/* type: output fs */
static void scdc_dataset_output_fs_destroy(scdc_dataset_output_t *output);

static scdc_dataset_output_t *scdc_dataset_output_fs_create(scdc_dataset_output_t *output, const char *conf, scdc_args_t *args)
{
  scdc_args xargs(args);

  const char *dirname;
  xargs.get<const char *>(SCDC_ARGS_TYPE_CSTR, &dirname);

  SCDC_TRACE("scdc_dataset_output_fs_create: dirname: '" << dirname << "'");

  char tarcmd[MAX_PATHNAME];
  sprintf(tarcmd,
#ifdef TAR_SHELL
    TAR_SHELL " -c \""
#endif
    "tar -xpf - -C %s"
#if TAR_QUIET
    " 2>/dev/null"
#endif
#ifdef TAR_SHELL
    "\""
#endif
    , dirname);

  SCDC_TRACE("scdc_dataset_output_fs_create: tarcmd: '" << tarcmd << "'");

  if (!output) return 0;

  FILE *ptar = popen(tarcmd, "w");

  if (!ptar)
  {
    SCDC_TRACE("scdc_dataset_output_fs_create: cmd '" << tarcmd << "' failed!");
    return 0;
  }

  output = scdc_dataset_output_FX_create<FILE *>(output, conf, args, ptar);

  if (!output)
  {
    SCDC_FAIL("scdc_dataset_output_fs_create: creating output from FILE failed!");
    pclose(ptar);
    return 0;
  }

  output->intern->type = SCDC_DATASET_INOUT_TYPE_FS;
  output->intern->destroy = scdc_dataset_output_fs_destroy;

  strcpy(output->format, "fstar");

  return output;
}


static void scdc_dataset_output_fs_destroy(scdc_dataset_output_t *output)
{
  if (!output) return;

  FILE *ptar;

  scdc_dataset_output_FX_destroy<FILE *>(output, &ptar);

  pclose(ptar);
}


/* type: input fslist */
typedef struct _input_fslist_dir_data_t
{
  char pathname[MAX_PATHNAME];
  bool follow_links;
  DIR *dir;
  struct dirent *result;

} input_fslist_dir_data_t;


static scdcint_t input_fslist_dir_next(scdc_dataset_input_t *input)
{
  input_fslist_dir_data_t *data = static_cast<input_fslist_dir_data_t *>(input->data);

  bool first = true;

  char *buf = static_cast<char *>(SCDC_DATASET_INOUT_BUF_PTR(input));
  scdcint_t buf_size = SCDC_DATASET_INOUT_BUF_SIZE(input);

  SCDC_DATASET_INOUT_BUF_CURRENT(input) = 0;

  do {

    if (data->result && strcmp(data->result->d_name, ".") != 0 && strcmp(data->result->d_name, "..") != 0)
    {
      if (static_cast<scdcint_t>(strlen(data->result->d_name) + 16) < buf_size || first)
      {
        string pathname = string(data->pathname) + data->result->d_name;

        scdcint_t n;

        if (!data->follow_links && z_fs_is_link(pathname.c_str()))
          n = snprintf(buf, buf_size, "%s:l|", data->result->d_name);
        else if (z_fs_is_directory(pathname.c_str()))
          n = snprintf(buf, buf_size, "%s:d|", data->result->d_name);
        else if (z_fs_is_file(pathname.c_str()))
          n = snprintf(buf, buf_size, "%s:f:%" scdcint_fmt "|", data->result->d_name, z_fs_get_file_size(pathname.c_str()));
        else
          n = snprintf(buf, buf_size, "%s:?" scdcint_fmt "|", data->result->d_name);

        n = min(n, buf_size);

        buf += n;
        buf_size -= n;

        SCDC_DATASET_INOUT_BUF_CURRENT(input) += n;
        input->total_size += n;

        first = false;

      } else break;
    }

    data->result = readdir(data->dir);

  } while (data->result);

  if (!data->result) input->next = 0;

  return SCDC_SUCCESS;
}


static void scdc_dataset_input_fslist_dir_destroy(scdc_dataset_input_t *input);

static scdc_dataset_inout_t *scdc_dataset_input_fslist_dir_create(scdc_dataset_input_t *input, const char *conf, scdc_args_t *args, const char *pathname)
{
  DIR *dir = opendir(pathname);

  if (!dir) return 0;

  scdcint_t size = DEFAULT_DATASET_INOUT_BUF_SIZE;

  stringlist confs(':', conf);
  scdc_args xargs(args);

  if (confs.front() == "size")
  {
    xargs.get<scdcint_t>(SCDC_ARGS_TYPE_SCDCINT, &size);
    confs.front_pop();
  }

  if (!scdc_dataset_inout_alloc(input, size, sizeof(input_fslist_dir_data_t)))
  {
    closedir(dir);
    return 0;
  }

  input->next = input_fslist_dir_next;

  snprintf(static_cast<input_fslist_dir_data_t *>(input->data)->pathname, MAX_PATHNAME, "%s%s", pathname, ((pathname[strlen(pathname) - 1] != '/')?"/":""));
  static_cast<input_fslist_dir_data_t *>(input->data)->follow_links = FOLLOW_LINKS;
  static_cast<input_fslist_dir_data_t *>(input->data)->dir = dir;
  static_cast<input_fslist_dir_data_t *>(input->data)->result = 0;

  input->intern->type = SCDC_DATASET_INOUT_TYPE_FSLIST_DIR;

  SCDC_DATASET_INOUT_BUF_CURRENT(input) = 0;
  input->total_size = 0;
  input->total_size_given = SCDC_DATASET_INOUT_TOTAL_SIZE_GIVEN_AT_LEAST;

  return input;
}


static void scdc_dataset_input_fslist_dir_destroy(scdc_dataset_input_t *input)
{
  if (!input) return;

  input_fslist_dir_data_t *data = static_cast<input_fslist_dir_data_t *>(input->data);

  closedir(data->dir);
  
  scdc_dataset_inout_free(input);
}


static void scdc_dataset_input_fslist_file_destroy(scdc_dataset_input_t *input);

static scdc_dataset_inout_t *scdc_dataset_input_fslist_file_create(scdc_dataset_input_t *input, const char *conf, scdc_args_t *args, const char *pathname)
{
  scdcint_t size = DEFAULT_DATASET_INOUT_BUF_SIZE;

  stringlist confs(':', conf);
  scdc_args xargs(args);

  if (confs.front() == "size")
  {
    xargs.get<scdcint_t>(SCDC_ARGS_TYPE_SCDCINT, &size);
    confs.front_pop();
  }

  if (!scdc_dataset_inout_alloc(input, size, 0)) return 0;

  input->intern->type = SCDC_DATASET_INOUT_TYPE_FSLIST_FILE;

  scdcint_t n = snprintf(static_cast<char *>(SCDC_DATASET_INOUT_BUF_PTR(input)), SCDC_DATASET_INOUT_BUF_SIZE(input), "f:%" scdcint_fmt "|", z_fs_get_file_size(pathname));

  SCDC_DATASET_INOUT_BUF_CURRENT(input) = n;
  input->total_size = n;
  input->total_size_given = SCDC_DATASET_INOUT_TOTAL_SIZE_GIVEN_EXACT;

  return input;
}


static void scdc_dataset_input_fslist_file_destroy(scdc_dataset_input_t *input)
{
  if (!input) return;
  
  scdc_dataset_inout_free(input);
}


static void scdc_dataset_input_fslist_link_destroy(scdc_dataset_input_t *input);

static scdc_dataset_inout_t *scdc_dataset_input_fslist_link_create(scdc_dataset_input_t *input, const char *conf, scdc_args_t *args, const char *pathname)
{
  scdcint_t size = DEFAULT_DATASET_INOUT_BUF_SIZE;

  stringlist confs(':', conf);
  scdc_args xargs(args);

  if (confs.front() == "size")
  {
    xargs.get<scdcint_t>(SCDC_ARGS_TYPE_SCDCINT, &size);
    confs.front_pop();
  }

  if (!scdc_dataset_inout_alloc(input, size, 0)) return 0;

  input->intern->type = SCDC_DATASET_INOUT_TYPE_FSLIST_FILE;

  scdcint_t n = 0;

  n += snprintf(static_cast<char *>(SCDC_DATASET_INOUT_BUF_PTR(input)) + n, SCDC_DATASET_INOUT_BUF_SIZE(input) - n, "l:");

  n += z_fs_get_link_target(pathname, static_cast<char *>(SCDC_DATASET_INOUT_BUF_PTR(input)) + n, SCDC_DATASET_INOUT_BUF_SIZE(input) - n - 1);

  n += snprintf(static_cast<char *>(SCDC_DATASET_INOUT_BUF_PTR(input)) + n, SCDC_DATASET_INOUT_BUF_SIZE(input) - n, "|");

  SCDC_DATASET_INOUT_BUF_CURRENT(input) = n;
  input->total_size = n;
  input->total_size_given = SCDC_DATASET_INOUT_TOTAL_SIZE_GIVEN_EXACT;

  return input;
}


static void scdc_dataset_input_fslist_link_destroy(scdc_dataset_input_t *input)
{
  if (!input) return;
  
  scdc_dataset_inout_free(input);
}


static void scdc_dataset_input_fslist_destroy(scdc_dataset_input_t *input);

static scdc_dataset_inout_t *scdc_dataset_input_fslist_create(scdc_dataset_input_t *input, const char *conf, scdc_args_t *args)
{
  scdc_args xargs(args);

  const char *pathname;
  xargs.get<const char *>(SCDC_ARGS_TYPE_CSTR, &pathname);

  bool follow_links = FOLLOW_LINKS;

  string format = "fslist";

  if (!follow_links && z_fs_is_link(pathname))
  {
    input = scdc_dataset_input_fslist_link_create(input, conf, args, pathname);
    format += ":link";

  } else if (z_fs_is_directory(pathname))
  {
    input = scdc_dataset_input_fslist_dir_create(input, conf, args, pathname);
    format += ":dir";

  } else if (z_fs_is_file(pathname))
  {
    input = scdc_dataset_input_fslist_file_create(input, conf, args, pathname);
    format += ":file";

  } else input = 0;

  if (!input)
  {
    SCDC_TRACE("scdc_dataset_input_fslist_create: pathname '" << pathname << "' is not a directory or file!");
    return 0;
  }

  input->intern->destroy = scdc_dataset_input_fslist_destroy;

  strcpy(input->format, format.c_str());

#if INPUT_CREATE_NEXT
  if (input->next) input->next(input);
#endif

  return input;
}


static void scdc_dataset_input_fslist_destroy(scdc_dataset_input_t *input)
{
  if (!input) return;

  if (input->intern->type == SCDC_DATASET_INOUT_TYPE_FSLIST_DIR)
    scdc_dataset_input_fslist_dir_destroy(input);
  else if (input->intern->type == SCDC_DATASET_INOUT_TYPE_FSLIST_FILE)
    scdc_dataset_input_fslist_file_destroy(input);
  else if (input->intern->type == SCDC_DATASET_INOUT_TYPE_FSLIST_LINK)
    scdc_dataset_input_fslist_link_destroy(input);
}


/* type: input produce */
typedef struct _input_produce_data_t
{
  scdcint_t total_size_left;

} input_produce_data_t;


static scdcint_t input_produce_next(scdc_dataset_input_t *input)
{
  input_produce_data_t *data = static_cast<input_produce_data_t *>(input->data);

  SCDC_DATASET_INOUT_BUF_CURRENT(input) = min(data->total_size_left, SCDC_DATASET_INOUT_BUF_SIZE(input));

  memset(SCDC_DATASET_INOUT_BUF_PTR(input), 0, SCDC_DATASET_INOUT_BUF_CURRENT(input));

  data->total_size_left -= SCDC_DATASET_INOUT_BUF_CURRENT(input);

  if (data->total_size_left <= 0) input->next = 0;

  return SCDC_SUCCESS;
}


static void scdc_dataset_input_produce_destroy(scdc_dataset_input_t *input);

static scdc_dataset_inout_t *scdc_dataset_input_produce_create(scdc_dataset_input_t *input, const char *conf, scdc_args_t *args)
{
  scdc_args xargs(args);

  scdcint_t total_size = 0;

  xargs.get<scdcint_t>(SCDC_ARGS_TYPE_SCDCINT, &total_size);

  scdcint_t size = DEFAULT_DATASET_INOUT_BUF_SIZE;

  stringlist confs(':', conf);

  if (confs.front() == "size")
  {
    xargs.get<scdcint_t>(SCDC_ARGS_TYPE_SCDCINT, &size);
    confs.front_pop();
  }

  if (!input) return 0;

  if (!scdc_dataset_inout_alloc(input, size, sizeof(input_produce_data_t))) return 0;

  input->intern->type = SCDC_DATASET_INOUT_TYPE_PRODUCE;
  input->intern->destroy = scdc_dataset_input_produce_destroy;

  strcpy(input->format, "data");

  SCDC_DATASET_INOUT_BUF_CURRENT(input) = 0;
  input->total_size = total_size;
  input->total_size_given = SCDC_DATASET_INOUT_TOTAL_SIZE_GIVEN_EXACT;

  static_cast<input_produce_data_t *>(input->data)->total_size_left = total_size;

  input->next = input_produce_next;

#if INPUT_CREATE_NEXT
  input->next(input);
#endif

  return input;
}


static void scdc_dataset_input_produce_destroy(scdc_dataset_input_t *input)
{
  if (!input) return;

  scdc_dataset_inout_free(input);
}


/* type: output consume */
static scdcint_t output_consume_next(scdc_dataset_output_t *output)
{
  SCDC_TRACE("consuming output of size = " << SCDC_DATASET_INOUT_BUF_CURRENT(output));

  return SCDC_SUCCESS;
}


static void scdc_dataset_output_consume_destroy(scdc_dataset_output_t *output);

static scdc_dataset_output_t *scdc_dataset_output_consume_create(scdc_dataset_output_t *output, const char *conf, scdc_args_t *args)
{
  output = scdc_dataset_inout_alloc_create(output, conf, args);

  if (!output) return 0;

  output->intern->type = SCDC_DATASET_INOUT_TYPE_CONSUME;
  output->intern->destroy = scdc_dataset_output_consume_destroy;

  output->next = output_consume_next;

  return output;
}


static void scdc_dataset_output_consume_destroy(scdc_dataset_output_t *output)
{
  if (!output) return;

  scdc_dataset_inout_alloc_destroy(output);
}


/* autodestroy wrapper */
typedef struct _autodestroy_data_t
{
  scdc_dataset_inout_next_f *next;
  void *data;

} autodestroy_data_t;


static void autodestroy_un_pack(scdc_dataset_inout_t *inout, scdc_dataset_inout_next_f *next)
{
  SCDC_TRACE("autodestroy_un_pack: " << (next?"pack":"unpack") << ", IN next: " << reinterpret_cast<void *>(inout->next) << ", data: " << inout->data);

  if (next)
  {
    autodestroy_data_t *data = new autodestroy_data_t();

    data->next = inout->next;
    data->data = inout->data;

    inout->next = next;
    inout->data = data;

  } else
  {
    autodestroy_data_t *data = static_cast<autodestroy_data_t *>(inout->data);

    inout->next = data->next;
    inout->data = data->data;

    delete data;
  }

  SCDC_TRACE("autodestroy_un_pack: " << (next?"pack":"unpack") << ", OUT next: " << reinterpret_cast<void *>(inout->next) << ", data: " << inout->data);
}


static scdcint_t autodestroy_next(scdc_dataset_input_t *inout)
{
  autodestroy_un_pack(inout, 0);

  scdcint_t ret;

  if (inout->next)
  {
    ret = inout->next(inout);
    autodestroy_un_pack(inout, autodestroy_next);

  } else
  {
    if (inout->intern && inout->intern->destroy) inout->intern->destroy(inout);
    SCDC_DATASET_INOUT_BUF_CURRENT(inout) = 0;
    ret = SCDC_SUCCESS;
  }

  return ret;
}


static void scdc_dataset_input_autodestroy(scdc_dataset_input_t *input)
{
  autodestroy_un_pack(input, autodestroy_next);
}


scdc_dataset_input_t *scdc_dataset_input_create_intern(scdc_dataset_input_t *input, const char *conf, scdc_args_t *args)
{
  SCDC_TRACE("scdc_dataset_input_create_intern: input: " << static_cast<void *>(input) << ", conf: '" << conf << "'");

  scdc_dataset_input_t input_given, *input_ret = input;

  /* backup given input struct */
  if (input) input_given = *input;

  stringlist confs(':', conf);
  string ip_type = confs.front_pop();

  bool autodestroy = false;

  if (confs.front() == "autodestroy")
  {
    autodestroy = true;
    confs.front_pop();
  }

  conf += confs.offset();

  scdc_dataset_input_unset(input_ret);

  if (ip_type == "null")
    input_ret = 0;
  else if (ip_type == "none")
    { if (input_ret) *input_ret = *SCDC_DATASET_INPUT_NONE; }
  else if (ip_type == "endl" || ip_type == "endless")
    { if (input_ret) *input_ret = *SCDC_DATASET_INPUT_ENDL; }
  else if (ip_type == "alloc")
    input_ret = scdc_dataset_input_alloc_create(input_ret, conf, args);
  else if (ip_type == "buffer")
    input_ret = scdc_dataset_input_buffer_create(input_ret, conf, args);
  else if (ip_type == "fd")
    input_ret = scdc_dataset_input_fd_create(input_ret, conf, args);
  else if (ip_type == "fddup")
    input_ret = scdc_dataset_input_fddup_create(input_ret, conf, args);
  else if (ip_type == "stream")
    input_ret = scdc_dataset_input_stream_create(input_ret, conf, args);
  else if (ip_type == "streamdup")
    input_ret = scdc_dataset_input_streamdup_create(input_ret, conf, args);
  else if (ip_type == "file")
    input_ret = scdc_dataset_input_file_create(input_ret, conf, args);
  else if (ip_type == "fs")
    input_ret = scdc_dataset_input_fs_create(input_ret, conf, args);
  else if (ip_type == "fslist")
    input_ret = scdc_dataset_input_fslist_create(input_ret, conf, args);
  else if (ip_type == "produce" || ip_type == "zero")
    input_ret = scdc_dataset_input_produce_create(input_ret, conf, args);
  else
  {
    SCDC_ERROR("unknown input type '" << ip_type << "'");
    input_ret = 0;
  }

  if (!input_ret)
  {
    /* restore given input struct */
    if (input) *input = input_given;
    return 0;
  }

  INOUT_INTERN_BACKUP(input_ret);

  if (autodestroy) scdc_dataset_input_autodestroy(input_ret);

  SCDC_TRACE_DATASET_INPUT(input_ret, "scdc_dataset_input_create_intern: ");

  return input_ret;
}


scdc_dataset_input_t *scdc_dataset_input_create(scdc_dataset_input_t *input, const char *conf, ...)
{
  va_list ap;
  va_start(ap, conf);

  scdc_args_t args;

  scdc_args_va_init(&args, &ap);

  input = scdc_dataset_input_create_intern(input, conf, &args);

  scdc_args_va_release(&args);
  va_end(ap);

  return input;
}


void scdc_dataset_input_destroy(scdc_dataset_input_t *input)
{
  SCDC_TRACE_DATASET_INPUT(input, "scdc_dataset_input_destroy: input: ");

  if (!input)
  {
    SCDC_FAIL("scdc_dataset_input_destroy: no input given");
    return;
  }

  if (!input->intern) return;

  INOUT_INTERN_RESTORE(input);

  if (input->intern->destroy) input->intern->destroy(input);
}


static void scdc_dataset_output_autodestroy(scdc_dataset_output_t *output)
{
  autodestroy_un_pack(output, autodestroy_next);
}


scdc_dataset_output_t *scdc_dataset_output_create_intern(scdc_dataset_output_t *output, const char *conf, scdc_args_t *args)
{
  SCDC_TRACE("scdc_dataset_output_create_intern: output: " << static_cast<void *>(output) << ", conf: '" << conf << "'");

  scdc_dataset_output_t output_given, *output_ret = output;

  /* backup given output struct */
  if (output) output_given = *output;

  stringlist confs(':', conf);
  string op_type = confs.front_pop();

  bool autodestroy = false;

  if (confs.front() == "autodestroy")
  {
    autodestroy = true;
    confs.front_pop();
  }

  conf += confs.offset();

  scdc_dataset_output_unset(output_ret);

  if (op_type == "null")
    output_ret = 0;
  else if (op_type == "none")
    { if (output_ret) *output_ret = *SCDC_DATASET_OUTPUT_NONE; }
  else if (op_type == "endl" || op_type == "endless")
    { if (output_ret) *output_ret = *SCDC_DATASET_OUTPUT_ENDL; }
  else if (op_type == "copy" || op_type == "output" || op_type == "outputsink")
    output_ret = scdc_dataset_output_copy_create(output_ret, conf, args);
  else if (op_type == "alloc")
    output_ret = scdc_dataset_output_alloc_create(output_ret, conf, args);
  else if (op_type == "buffer")
    output_ret = scdc_dataset_output_buffer_create(output_ret, conf, args);
  else if (op_type == "fd")
    output_ret = scdc_dataset_output_fd_create(output_ret, conf, args);
  else if (op_type == "fddup")
    output_ret = scdc_dataset_output_fddup_create(output_ret, conf, args);
  else if (op_type == "stream")
    output_ret = scdc_dataset_output_stream_create(output_ret, conf, args);
  else if (op_type == "streamdup")
    output_ret = scdc_dataset_output_streamdup_create(output_ret, conf, args);
  else if (op_type == "file")
    output_ret = scdc_dataset_output_file_create(output_ret, conf, args);
  else if (op_type == "fs")
    output_ret = scdc_dataset_output_fs_create(output_ret, conf, args);
  else if (op_type == "consume")
    output_ret = scdc_dataset_output_consume_create(output_ret, conf, args);
  else
  {
    SCDC_ERROR("unknown output type '" << op_type << "'");
    output_ret = 0;
  }

  if (!output_ret)
  {
    /* restore given output struct */
    if (output) *output = output_given;
    return 0;
  }

  INOUT_INTERN_BACKUP(output_ret);

  if (autodestroy) scdc_dataset_output_autodestroy(output_ret);

  SCDC_TRACE_DATASET_OUTPUT(output, "scdc_dataset_output_create_intern: ");

  return output_ret;
}


scdc_dataset_output_t *scdc_dataset_output_create(scdc_dataset_output_t *output, const char *conf, ...)
{
  va_list ap;
  va_start(ap, conf);

  scdc_args_t args;

  scdc_args_va_init(&args, &ap);

  output = scdc_dataset_output_create_intern(output, conf, &args);

  scdc_args_va_release(&args);
  va_end(ap);

  return output;
}


void scdc_dataset_output_destroy(scdc_dataset_output_t *output)
{
  SCDC_TRACE_DATASET_OUTPUT(output, "scdc_dataset_output_destroy: ");

  if (!output)
  {
    SCDC_FAIL("scdc_dataset_output_destroy: no output given");
    return;
  }

  if (!output->intern) return;

  INOUT_INTERN_RESTORE(output);

  if (output->intern->destroy) output->intern->destroy(output);
}


typedef struct _scdc_args_data_ior_t
{
  scdc_dataset_inout_t *inout;
  scdc_args_t *args;

} scdc_args_data_ior_t;


static scdc_arg_ref_t scdc_args_get_ior(void *data, scdcint_t type, void *v)
{
  scdc_args_data_ior_t *data_ior = static_cast<scdc_args_data_ior_t *>(data);

  SCDC_TRACE("scdc_args_get_ior: data: " << data << " (inout: <...>, args: " << data_ior->args << "), type: " << type << ", v: " << v);

  if (type == SCDC_ARGS_TYPE_DATASET_INOUT_REDIRECT_PTR)
  {
    *((scdc_dataset_inout_t **) v) = data_ior->inout;
    return SCDC_ARG_REF_NONE;
  }

  return data_ior->args->get(data_ior->args->data, type, v);
}

scdc_arg_ref_t scdc_args_set_ior(void *data, scdcint_t type, void *v, scdc_arg_ref_t ref)
{
  scdc_args_data_ior_t *data_ior = static_cast<scdc_args_data_ior_t *>(data);

  return data_ior->args->set(data_ior->args->data, type, v, ref);
}

void scdc_args_free_ior(void *data, scdcint_t type, void *v, scdc_arg_ref_t ref)
{
  scdc_args_data_ior_t *data_ior = static_cast<scdc_args_data_ior_t *>(data);

  data_ior->args->free(data_ior->args->data, type, v, ref);
}


static void inout2inout(scdc_dataset_inout_t *src, scdc_dataset_inout_t *dst, bool copy)
{
  strncpy(dst->format, src->format, SCDC_FORMAT_MAX_SIZE);

  scdcint_t size = SCDC_DATASET_INOUT_BUF_CURRENT(src);

  if (copy)
  {
    size = min(size, SCDC_DATASET_INOUT_BUF_SIZE(dst));
    memcpy(SCDC_DATASET_INOUT_BUF_PTR(dst), SCDC_DATASET_INOUT_BUF_PTR(src), size);

  } else
  {
    SCDC_DATASET_INOUT_BUF_PTR(dst) = SCDC_DATASET_INOUT_BUF_PTR(src);
    SCDC_DATASET_INOUT_BUF_SIZE(dst) = SCDC_DATASET_INOUT_BUF_SIZE(src);
  }

  dst->total_size = src->total_size;
  dst->total_size_given = src->total_size_given;

  SCDC_DATASET_INOUT_BUF_CURRENT(dst) = size;
}


static scdcint_t scdc_dataset_inout_redirect_to(scdc_dataset_inout_t *inout, const char *conf, scdc_args_t *args)
{
  if (stringlist(':', conf).front() == "output")
  {
    scdc_args xargs(args);

    scdc_dataset_output_t output;
    scdc_arg_ref_t output_arg_ref = xargs.get<scdc_dataset_output_t>(SCDC_ARGS_TYPE_DATASET_OUTPUT, &output);

    if (output_arg_ref == SCDC_ARG_REF_NULL)
    {
      SCDC_FAIL("scdc_dataset_inout_redirect_to: getting output parameter failed!");
      return SCDC_FAILURE;
    }

    /* assign inout to output */
    inout2inout(inout, &output, false);

    output.next = inout->next; inout->next = 0;
    output.data = inout->data; inout->data = 0;

    output.intern = inout->intern;

    xargs.set<scdc_dataset_output_t>(SCDC_ARGS_TYPE_DATASET_OUTPUT, &output, output_arg_ref);

    return SCDC_SUCCESS;
  }

  scdc_dataset_output_t _output, *output = &_output;

  scdc_args_data_ior_t args_ior_data = { inout, args };
  scdc_args_t args_ior = { &args_ior_data, scdc_args_get_ior, args->set, args->free };

  output = scdc_dataset_output_create_intern(output, conf, &args_ior);

  if (!output)
  {
    SCDC_FAIL("scdc_dataset_inout_redirect_to: creating output for redirection failed!");
    return SCDC_FAILURE;
  }

  scdcint_t ret = SCDC_SUCCESS;

  if (output->next)
  {
    /* feed inout to the output */
    while (1)
    {
      /* update target output */
      inout2inout(inout, output, false);

      /* process output (just continue if output has no more next) */
      if (output->next && !output->next(output))
      {
        ret = SCDC_FAILURE;
        break;
      }

#if 0
      scdcint_t size = SCDC_DATASET_INOUT_BUF_CURRENT(inout) - SCDC_DATASET_INOUT_BUF_CURRENT(output);
#endif

      /* update given inout */
      inout2inout(output, inout, false);

#if 0
      SCDC_DATASET_INOUT_BUF_CURRENT(inout) = size;
#endif

      if (!inout->next) break;

      /* get next inout */
      if (!inout->next(inout))
      {
        ret = SCDC_FAILURE;
        break;
      }
    }

  } else {

    /* copy inout to the output */
    inout2inout(inout, output, true);

    /* set current_size of source to the size of the processed data */
    SCDC_DATASET_INOUT_BUF_CURRENT(inout) = SCDC_DATASET_INOUT_BUF_CURRENT(output);
  }

  scdc_dataset_output_destroy(output);

  return ret;
}


static scdcint_t scdc_dataset_inout_redirect_from(scdc_dataset_inout_t *inout, const char *conf, scdc_args_t *args)
{
  if (stringlist(':', conf).front() == "input")
  {
    scdc_args xargs(args);
  
    scdc_dataset_input_t input;
    scdc_arg_ref_t input_arg_ref = xargs.get<scdc_dataset_input_t>(SCDC_ARGS_TYPE_DATASET_INPUT, &input);

    if (input_arg_ref == SCDC_ARG_REF_NULL)
    {
      SCDC_FAIL("scdc_dataset_inout_redirect_from: getting output parameter failed!");
      return SCDC_FAILURE;
    }

    /* assign input to inout */
    inout2inout(&input, inout, false);

    inout->next = input.next; input.next = 0;
    inout->data = input.data; input.data = 0;

    inout->intern = input.intern;

    xargs.set<scdc_dataset_input_t>(SCDC_ARGS_TYPE_DATASET_OUTPUT, &input, input_arg_ref); /* FIXME: ..._OUTPUT?*/

    return SCDC_SUCCESS;
  }

  scdc_dataset_input_t _input, *input = &_input;

  scdc_args_data_ior_t args_ior_data = { inout, args };
  scdc_args_t args_ior = { &args_ior_data, scdc_args_get_ior, args->set, args->free };

  input = scdc_dataset_input_create_intern(input, conf, &args_ior);

  if (!input)
  {
    SCDC_FAIL("scdc_dataset_inout_redirect_from: creating input for redirection failed!");
    return SCDC_FAILURE;
  }

  scdc_dataset_input_autodestroy(input);

  /* FIXME: if inout->next, then loop similar to ..._redirect_to? */
  *inout = *input;

  return SCDC_SUCCESS;
}


static scdcint_t scdc_dataset_inout_redirect_intern(scdc_dataset_inout_t *inout, const char *conf, scdc_args_t *args)
{
  stringlist confs(':', conf);
  if (confs.front() == "from")
  {
    confs.front_pop();
    conf += confs.offset();
    
    return scdc_dataset_inout_redirect_from(inout, conf, args);
  }

  if (confs.front() == "to")
  {
    confs.front_pop();
    conf += confs.offset();
  }

  return scdc_dataset_inout_redirect_to(inout, conf, args);
}


scdcint_t scdc_dataset_input_redirect_intern(scdc_dataset_input_t *input, const char *conf, scdc_args_t *args)
{
  SCDC_TRACE("scdc_dataset_input_redirect_intern: conf: '" << conf << "'");

  if (!input)
  {
    SCDC_FAIL("scdc_dataset_input_redirect_intern: no input given");
    return SCDC_FAILURE;
  }

  return scdc_dataset_inout_redirect_intern(input, conf, args);
}


scdcint_t scdc_dataset_input_redirect(scdc_dataset_input_t *input, const char *conf, ...)
{
  va_list ap;
  va_start(ap, conf);

  scdc_args_t args;

  scdc_args_va_init(&args, &ap);

  scdcint_t ret = scdc_dataset_input_redirect_intern(input, conf, &args);

  scdc_args_va_release(&args);
  va_end(ap);

  return ret;
}


scdcint_t scdc_dataset_output_redirect_intern(scdc_dataset_output_t *output, const char *conf, scdc_args_t *args)
{
  SCDC_TRACE("scdc_dataset_output_redirect_intern: conf: '" << conf << "'");

  if (!output)
  {
    SCDC_FAIL("scdc_dataset_output_redirect_intern: no output given");
    return SCDC_FAILURE;
  }

  return scdc_dataset_inout_redirect_intern(output, conf, args);
}


scdcint_t scdc_dataset_output_redirect(scdc_dataset_output_t *output, const char *conf, ...)
{
  va_list ap;
  va_start(ap, conf);

  scdc_args_t args;

  scdc_args_va_init(&args, &ap);

  scdcint_t ret = scdc_dataset_output_redirect_intern(output, conf, &args);

  scdc_args_va_release(&args);
  va_end(ap);

  return ret;
}
