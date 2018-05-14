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


#include <cstring>
#include <exception>

#define SCDC_TRACE_NOT  !SCDC_TRACE_TRANSPORT_STREAM

#include "config.hh"
#include "log.hh"
#include "transport_stream.hh"


using namespace std;


#define SCDC_LOG_PREFIX  "transport-stream: "


struct scdc_transport_stream_connection_t: public scdc_transport_connection_t
{
#if SCDC_TRANSPORT_STREAM_USE_IOSTREAM
  istream *is;
  ostream *os;
#endif
  FILE *ifile, *ofile;

  scdc_transport_stream_connection_t(scdc_transport *transport_)
    :scdc_transport_connection_t(transport_),
#if SCDC_TRANSPORT_STREAM_USE_IOSTREAM
    is(0), os(0),
#endif
    ifile(0), ofile(0) { }
};


struct scdc_transport_stream_server_t
{
  scdc_transport_stream_server_t(scdc_transport *transport_)
    :stream_conn(transport_) { }

  scdc_transport_stream_connection_t stream_conn;
};


#if SCDC_TRANSPORT_STREAM_USE_SHUTDOWN_EXCEPTION
class scdc_transport_stream_shutdown_ex: public exception
{
};
#endif


scdc_transport_stream::scdc_transport_stream()
  :scdc_transport()
{
  stream_server = new scdc_transport_stream_server_t(this);
}


scdc_transport_stream::~scdc_transport_stream()
{
  delete stream_server; stream_server = 0;
}


#if SCDC_TRANSPORT_STREAM_USE_IOSTREAM
bool scdc_transport_stream::start(scdcint_t mode, std::istream *is, std::ostream *os)
{
  SCDC_TRACE("start: mode '" << mode << "', is: '" << is << "', os: '" << os << "'");

  stream_server->stream_conn.is = (is)?is:&cin;
  stream_server->stream_conn.os = (os)?os:&cout;
  stream_server->stream_conn.ifile = 0;
  stream_server->stream_conn.ofile = 0;

  return scdc_transport::start(mode);
}
#endif


bool scdc_transport_stream::start(scdcint_t mode, FILE *ifile, FILE *ofile)
{
  SCDC_TRACE("start: mode '" << mode << "', ifile: '" << ifile << "', ofile: '" << ofile << "'");

#if SCDC_TRANSPORT_STREAM_USE_IOSTREAM
  stream_server->stream_conn.is = 0;
  stream_server->stream_conn.os = 0;
#endif
  stream_server->stream_conn.ifile = (ifile)?ifile:stdin;
  stream_server->stream_conn.ofile = (ofile)?ofile:stdout;

  return scdc_transport::start(mode);
}


bool scdc_transport_stream::stop()
{
  SCDC_TRACE("stop");

  if (!scdc_transport::stop())
  {
    SCDC_FAIL("stop: failed to stop");
    return false;
  }

#if SCDC_TRANSPORT_STREAM_USE_IOSTREAM
  stream_server->stream_conn.is = 0;
  stream_server->stream_conn.os = 0;
#endif
  stream_server->stream_conn.ifile = 0;
  stream_server->stream_conn.ofile = 0;

  return true;
}


/*bool scdc_transport_stream::cancel(bool interrupt)
{
  SCDC_TRACE("cancel: interrupt: '" << interrupt << "'");

  return scdc_transport::cancel(interrupt);
}*/


/*bool scdc_transport_stream::resume()
{
  SCDC_TRACE("resume:");

  return scdc_transport::resume();
}*/


scdc_transport_connection_t *scdc_transport_stream::accept()
{
  SCDC_TRACE("accept:");

  scdc_transport_stream_connection_t *stream_conn = 0;

  stream_server->stream_conn.open = 
#if SCDC_TRANSPORT_STREAM_USE_IOSTREAM
    (stream_server->stream_conn.is && !stream_server->stream_conn.is->eof() && stream_server->stream_conn.os && !stream_server->stream_conn.os->eof()) ||
#endif
    (stream_server->stream_conn.ifile && !feof(stream_server->stream_conn.ifile) && stream_server->stream_conn.ofile && !feof(stream_server->stream_conn.ofile));

  SCDC_TRACE("accept: I/O streams open: " << stream_server->stream_conn.open);

  if (stream_server->stream_conn.open)
  {
    stream_conn = &stream_server->stream_conn;

  } else
  {
    cancel(false);
    return 0;
  }

  SCDC_TRACE("accept: '" << stream_conn << "'");

  return scdc_transport::accept(stream_conn);
}


void scdc_transport_stream::shutdown(scdc_transport_connection_t *conn, bool interrupt)
{
  SCDC_TRACE("shutdown: conn: '" << conn << "', interrupt: '" << interrupt << "'");

  scdc_transport::shutdown(conn, interrupt);

#if SCDC_TRANSPORT_STREAM_USE_SHUTDOWN_EXCEPTION
  if (interrupt) throw scdc_transport_stream_shutdown_ex();
#endif

  if (conn)
  {
    SCDC_TRACE("shutdown: connection stream shutdown");

    scdc_transport_stream_connection_t *stream_conn = static_cast<scdc_transport_stream_connection_t *>(conn);

    /* FIXME: make closing streams optional */
    if (stream_conn->ifile) fclose(stream_conn->ifile);
/*    if (stream_conn->ofile) fclose(stream_conn->ofile);*/
  }
}


void scdc_transport_stream::serve(scdc_transport_connection_t *conn)
{
#if SCDC_TRANSPORT_STREAM_USE_SHUTDOWN_EXCEPTION
  /* catch our shutdown exceptions while serving (i.e., outside send or receive) */
  try
  {
#endif
    scdc_transport::serve(conn);

#if SCDC_TRANSPORT_STREAM_USE_SHUTDOWN_EXCEPTION
  } catch (const scdc_transport_stream_shutdown_ex &ex)
  {
    SCDC_INFO("serve: catch shutdown exception");
  }
#endif
}


void scdc_transport_stream::close(scdc_transport_connection_t *conn)
{
  SCDC_TRACE("close: '" << conn << "'");

  scdc_transport::close(conn);
}


#if SCDC_TRANSPORT_STREAM_USE_IOSTREAM
bool scdc_transport_stream::connect(std::istream *is, std::ostream *os)
{
  SCDC_TRACE("connect: is: '" << is << "', os: '" << os << "'");

  if (connection)
  {
    SCDC_ERROR("connect: already connected");
    return false;
  }

  scdc_transport_stream_connection_t *stream_conn = new scdc_transport_stream_connection_t();

  stream_conn->is = (is)?is:&cin;
  stream_conn->os = (os)?os:&cout;
  stream_conn->ifile = 0;
  stream_conn->ofile = 0;

  connection = stream_conn;

  return scdc_transport::connect();
}
#endif


scdc_transport_connection_t *scdc_transport_stream::connect(FILE *ifile, FILE *ofile)
{
  SCDC_TRACE("start: ifile: '" << ifile << "', ofile: '" << ofile << "'");

  scdc_transport_stream_connection_t *stream_conn = new scdc_transport_stream_connection_t(this);

#if SCDC_TRANSPORT_STREAM_USE_IOSTREAM
  stream_conn->is = 0;
  stream_conn->os = 0;
#endif
  stream_conn->ifile = (ifile)?ifile:stdin;
  stream_conn->ofile = (ofile)?ofile:stdout;

  return scdc_transport::connect(stream_conn);
}


bool scdc_transport_stream::disconnect(scdc_transport_connection_t *conn)
{
  SCDC_TRACE("disconnect: conn: '" << conn << "'");

  if (!scdc_transport::disconnect(conn)) return false;

  scdc_transport_stream_connection_t *stream_conn = static_cast<scdc_transport_stream_connection_t *>(conn);

  delete stream_conn;

  return true;
}


scdcint_t scdc_transport_stream::send(scdc_transport_connection_t *conn, const void *buf, scdcint_t buf_size)
{
  SCDC_TRACE("send: conn: '" << conn << "', buf: '" << buf << "', buf_size: " << buf_size);

  scdc_transport_stream_connection_t *stream_conn = static_cast<scdc_transport_stream_connection_t *>(conn);

  scdcint_t n;

#if SCDC_TRANSPORT_STREAM_USE_SHUTDOWN_EXCEPTION
  try
  {
#endif
#if SCDC_TRANSPORT_STREAM_USE_IOSTREAM
    if (stream_conn->os)
    {
      if (stream_conn->os->eof())
      {
        SCDC_TRACE("connection: " << stream_conn << ": send: end of output stream");
        n = -1;

      } else
      {
        stream_conn->os->write(static_cast<const char *>(buf), buf_size);
        n = buf_size;
      }

    } else
#endif
    if (stream_conn->ofile)
    {
      if (feof(stream_conn->ofile))
      {
        SCDC_TRACE("connection: " << stream_conn << ": send: end of output stream");
        n = -1;

      } else
      {
        n = fwrite(buf, 1, buf_size, stream_conn->ofile);
      }

    } else
    {
      SCDC_FATAL("connection: " << stream_conn << ": send: no output stream available");
      n = -1;
    }

#if SCDC_TRANSPORT_STREAM_USE_SHUTDOWN_EXCEPTION
  } catch (const scdc_transport_stream_shutdown_ex &ex)
  {
    SCDC_INFO("connection: " << stream_conn << ": send: catch shutdown exception");
    n = -1;
  }
#endif

  SCDC_TRACE("connection: " << stream_conn << ": send: sent " << n << " byte(s)");

  return n;
}


scdcint_t scdc_transport_stream::receive(scdc_transport_connection_t *conn, void *buf, scdcint_t max_buf_size)
{
  SCDC_TRACE("receive: conn: '" << conn << "', buf: '" << buf << "', max_buf_size: " << max_buf_size);

  scdc_transport_stream_connection_t *stream_conn = static_cast<scdc_transport_stream_connection_t *>(conn);

  if (max_buf_size <= 0)
  {
    SCDC_TRACE("connection: " << stream_conn << ": receive: nothing to receive");
    return 0;
  }

  scdcint_t n;

#if SCDC_TRANSPORT_STREAM_USE_SHUTDOWN_EXCEPTION
  try
  {
#endif
#if SCDC_TRANSPORT_STREAM_USE_IOSTREAM
    if (stream_conn->is)
    {
      if (stream_conn->is->eof())
      {
        SCDC_TRACE("connection: " << stream_conn << ": receive: end of input stream");
        n = -1;

      } else
      {
        stream_conn->is->getline(static_cast<char *>(buf), max_buf_size);
        n = stream_conn->is->gcount() - 1;
      }

    } else
#endif
    if (stream_conn->ifile)
    {
      if (feof(stream_conn->ifile))
      {
        SCDC_TRACE("connection: " << stream_conn << ": receive: end of input stream");
        n = -1;

      } else
      {
#if 0
        n = fread(buf, 1, max_buf_size, stream_conn->ifile);
        SCDC_TRACE("connection: " << stream_conn << ": receive: fread done: n = " << n);
#else
        char *s = fgets(static_cast<char *>(buf), max_buf_size, stream_conn->ifile);
        if (s) n = strlen(s);
        else n = -1;
        if (n > 0 && s[n - 1] == '\n')
        {
          s[n - 1] = '\0';
          --n;
        }
        SCDC_TRACE("connection: " << stream_conn << ": receive: fgets done: n = " << n);
#endif
      }

    } else
    {
      SCDC_FATAL("connection: " << stream_conn << ": receive: no output stream available");
      n = -1;
    }

#if SCDC_TRANSPORT_STREAM_USE_SHUTDOWN_EXCEPTION
  } catch (const scdc_transport_stream_shutdown_ex &ex)
  {
    SCDC_INFO("connection: " << stream_conn << ": receive: catch shutdown exception");
    n = -1;
  }
#endif

  SCDC_TRACE("connection: " << stream_conn << ": receive: received " << n << " byte(s)");

  return n;
}


#undef SCDC_LOG_PREFIX
