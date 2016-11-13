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


#include <fstream>
#include <vector>
#include <cstdarg>

#include "config.hh"
#include "log.hh"
#include "args.hh"


using namespace std;


#define SCDC_LOG_PREFIX  "log: "


class ostreambuf_buf_write: public std::streambuf
{
  public:
    explicit ostreambuf_buf_write(std::size_t buf_size_ = 1024)
      :buf(buf_size_)
    {
      char *base = &buf.front();
      setp(base, base + buf.size());
    }

  private:
    int_type overflow(int_type ch)
    {
      if (ch == traits_type::eof()) return traits_type::eof();

      sync();

      *pptr() = ch;
      pbump(1);

      return ch;
    }

    int sync()
    {
      std::ptrdiff_t n = pptr() - pbase();
      pbump(-n);

      return (write(pbase(), n) == static_cast<std::size_t>(n) && flush() == 0)?0:1;
    }

    virtual std::size_t write(const char *b, std::size_t n) = 0;
    virtual int flush() = 0;

  private:
    std::vector<char> buf;
};


class ostreambuf_log_handler: public ostreambuf_buf_write
{
  public:
    explicit ostreambuf_log_handler(scdc_log_handler_args_t *log_handler_)
      :ostreambuf_buf_write(), log_handler(*log_handler_)
    { }

    explicit ostreambuf_log_handler(scdc_log_handler_args_t *log_handler_, std::size_t buf_size_)
      :ostreambuf_buf_write(buf_size_), log_handler(*log_handler_)
    { }

  private:
    std::size_t write(const char *b, std::size_t n)
    {
      if (n == 0) return 0;

      return log_handler.handler(log_handler.data, b, n);
    }

    int flush()
    {
      return static_cast<int>(log_handler.handler(log_handler.data, 0, 0));
    }

  private:
    scdc_log_handler_args_t log_handler;
};


class ostream_log_handler: private ostreambuf_log_handler, public std::ostream
{
  public:
    explicit ostream_log_handler(scdc_log_handler_args_t *log_handler)
      :ostreambuf_log_handler(log_handler), std::ostream(this)
    { }
};


class ostreambuf_FILE: public ostreambuf_buf_write
{
  public:
    explicit ostreambuf_FILE(FILE *ofile_)
      :ostreambuf_buf_write(), ofile(ofile_)
    { }

    explicit ostreambuf_FILE(FILE *ofile_, std::size_t buf_size_)
      :ostreambuf_buf_write(buf_size_), ofile(ofile_)
    { }

  private:
    std::size_t write(const char *b, std::size_t n)
    {
      if (n == 0) return 0;

      return fwrite(b, 1, n, ofile);
    }

    int flush()
    {
      return fflush(ofile);
    }

  private:
    FILE *ofile;
};


class ostream_FILE: private ostreambuf_FILE, public std::ostream
{
  public:
    explicit ostream_FILE(FILE *ofile)
      :ostreambuf_FILE(ofile), std::ostream(this)
    { }
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
    case SCDC_ARGS_TYPE_CSTR:
    case SCDC_ARGS_TYPE_PTR:
      *((void **) v) = va_arg(*data_va->ap, void *);
      return SCDC_ARG_REF_NONE;
    case SCDC_ARGS_TYPE_OUT_STREAM:
      *((FILE **) v) = va_arg(*data_va->ap, FILE *);
      return SCDC_ARG_REF_NONE;
    case SCDC_ARGS_TYPE_LOG_HANDLER:
      ((scdc_log_handler_args_t *) v)->handler = va_arg(*data_va->ap, scdc_log_handler_f *);
      ((scdc_log_handler_args_t *) v)->data = va_arg(*data_va->ap, void *);
      return SCDC_ARG_REF_NONE;
  }

  SCDC_TRACE("scdc_args_get_va: error: unknown type '" << type << "'");

  return SCDC_ARG_REF_NULL;
}


static scdc_arg_ref_t scdc_args_set_va(void *data, scdcint_t type, void *v, scdc_arg_ref_t arg_ref)
{
  SCDC_TRACE("scdc_args_set_va: '" << data << "', type: '" << type << "', v: '" << v << "', arg_ref: '" << arg_ref << "'");

  return SCDC_ARG_REF_NULL;
}


static void scdc_args_free_va(void *data, scdcint_t type, void *v, scdc_arg_ref_t arg_ref)
{
  SCDC_TRACE("scdc_args_free_va: '" << data << "', type: '" << type << "', v: '" << v << "', arg_ref: '" << arg_ref << "'");
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
  args->free = scdc_args_free_va;
}


static void scdc_args_va_release(scdc_args_t *args)
{
  scdc_args_data_va_t *data_va = static_cast<scdc_args_data_va_t *>(args->data);

  delete data_va;
}


scdc_log::scdc_log()
  :cout_default(0), cout(0), cerr_default(0), cerr(0)
{
#if SCDC_LOG_LOCK
  pthread_mutex_t static_mutex = PTHREAD_MUTEX_INITIALIZER;
  lock = static_mutex;
#endif
}


scdc_log::scdc_log(const char *conf, ...)
  :cout_default(0), cout(0), cerr_default(0), cerr(0)
{
#if SCDC_LOG_LOCK
  pthread_mutex_t static_mutex = PTHREAD_MUTEX_INITIALIZER;
  lock = static_mutex;
#endif

  va_list ap;
  va_start(ap, conf);

  default_init_intern(conf, ap);

  va_end(ap);
}


scdc_log::~scdc_log()
{
  release();

  cout = 0;
  cerr = 0;

  default_release();
}


bool scdc_log::default_init_intern(const char *conf, va_list aq)
{
  SCDC_TRACE("default_init_intern: conf: '" << conf << "'");

  scdc_args_t args;

  va_list ap;
  va_copy(ap, aq);

  scdc_args_va_init(&args, &ap);

  bool ret = init(conf, &args);

  scdc_args_va_release(&args);

  va_end(ap);

  if (cout_default) cout_default->flush();
  delete cout_default;
  cout_default = cout;

  if (cerr_default) cerr_default->flush();
  delete cerr_default;
  cerr_default = cerr;

  SCDC_TRACE("default_init_intern: return: " << ret << "'");

  return ret;
}


bool scdc_log::default_init(const char *conf, ...)
{
  SCDC_TRACE("default_init: conf: '" << conf << "'");

  va_list ap;
  va_start(ap, conf);

  bool ret = default_init_intern(conf, ap);

  va_end(ap);

  SCDC_TRACE("default_init: return: " << ret << "'");

  return ret;
}


void scdc_log::default_release()
{
  SCDC_TRACE("default_release:");

  delete cout_default;
  cout_default = 0;

  delete cerr_default;
  cerr_default =  0;

  SCDC_TRACE("default_release: return");
}


bool scdc_log::init(const char *conf, scdc_args_t *args)
{
  SCDC_TRACE("init: conf: '" << conf << "'");

  ostream *scdc_cout = 0;
  ostream *scdc_cerr = 0;

  if (!conf)
  {
    SCDC_FAIL("init: no config given");
    return false;
  }

  string c(conf);
  scdc_args xargs(*args);

  if (c == "log_handler")
  {
    scdc_log_handler_args_t outhandler, errhandler;

    if (!xargs.get<scdc_log_handler_args_t>(SCDC_ARGS_TYPE_LOG_HANDLER, &outhandler))
    {
      SCDC_FAIL("init: no output log handler given");
      return false;
    }

    if (!xargs.get<scdc_log_handler_args_t>(SCDC_ARGS_TYPE_LOG_HANDLER, &errhandler))
    {
      SCDC_FAIL("init: no error log handler given");
      return false;
    }

    scdc_cout = new ostream_log_handler(&outhandler);
    scdc_cerr = new ostream_log_handler(&errhandler);

  } else if (c == "log_FILE")
  {
    FILE *outfile = 0, *errfile = 0;

    if (!xargs.get<FILE *>(SCDC_ARGS_TYPE_OUT_STREAM, &outfile))
    {
      SCDC_FAIL("init: no output FILE given");
      return false;
    }

    if (!xargs.get<FILE *>(SCDC_ARGS_TYPE_OUT_STREAM, &errfile))
    {
      SCDC_FAIL("init: no error FILE given");
      return false;
    }

    scdc_cout = new ostream_FILE(outfile);
    scdc_cerr = new ostream_FILE(errfile);

  } else if (c == "log_filepath")
  {
    const char *filepath;

    if (!xargs.get<const char *>(SCDC_ARGS_TYPE_CSTR, &filepath))
    {
      SCDC_FAIL("init: no filepath given");
      return false;
    }

    scdc_cout = new ofstream((string(filepath) + ".out").c_str());
    scdc_cerr = new ofstream((string(filepath) + ".err").c_str());

  } else
  {
    SCDC_FAIL("init: unknown log type '" << c << "'");
    return false;
  }

  set_cout(scdc_cout);
  set_cerr(scdc_cerr);

  SCDC_TRACE("init: return");

  return true;
}


void scdc_log::release()
{
  SCDC_TRACE("release:");

  set_cout(cout_default);

  set_cerr(cerr_default);

  SCDC_TRACE("release: return");
}


void scdc_log::set_cout(std::ostream *cout_)
{
  if (cout) cout->flush();

  if (cout != cout_default) delete cout;

  cout = cout_;
}


void scdc_log::set_cerr(std::ostream *cerr_)
{
  if (cerr) cerr->flush();

  if (cerr != cerr_default) delete cerr;

  cerr = cerr_;
}


#ifdef __cplusplus
extern "C" {
#endif


#define OSTREAM_PRINTF_DEFAULT_SIZE  1024

static int ostream_printf(ostream *os, const char *format, va_list ap, bool nl)
{
  int n;
  char ss[OSTREAM_PRINTF_DEFAULT_SIZE], *s = ss;


  if (!os) return -1;

  va_list ap1;
  va_copy(ap1, ap);
  n = vsnprintf(s, OSTREAM_PRINTF_DEFAULT_SIZE, format, ap1);
  va_end(ap1);

  if (n >= OSTREAM_PRINTF_DEFAULT_SIZE)
  {
    char *s = new char[n + 1];

    va_list ap2;
    va_copy(ap2, ap);
    n = vsnprintf(s, OSTREAM_PRINTF_DEFAULT_SIZE, format, ap2);
    va_end(ap2);
  }

  *os << s;

  if (nl) *os << endl;

  if (s != ss) delete[] s;

  return n;
}

#undef OSTREAM_PRINTF_DEFAULT_SIZE


int scdc_log_cout_printf(const char *format, ...)
{
  va_list ap;
  va_start(ap, format);
  int r = ostream_printf(SCDC_MAIN_LOG()->get_cout(), format, ap, false);
  va_end(ap);

  return r;
}


int scdc_log_cout_printf_nl(const char *format, ...)
{
  va_list ap;
  va_start(ap, format);
  int r = ostream_printf(SCDC_MAIN_LOG()->get_cout(), format, ap, true);
  va_end(ap);

  return r;
}


int scdc_log_cerr_printf(const char *format, ...)
{
  va_list ap;
  va_start(ap, format);
  int r = ostream_printf(SCDC_MAIN_LOG()->get_cerr(), format, ap, false);
  va_end(ap);

  return r;
}


int scdc_log_cerr_printf_nl(const char *format, ...)
{
  va_list ap;
  va_start(ap, format);
  int r = ostream_printf(SCDC_MAIN_LOG()->get_cerr(), format, ap, true);
  va_end(ap);

  return r;
}


#ifdef __cplusplus
}
#endif


#undef SCDC_LOG_PREFIX
