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


#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>

#include <cerrno>
#include <cstring>
#include <exception>

#define SCDC_TRACE_NOT  !SCDC_TRACE_TRANSPORT_TCP

#include "config.hh"
#include "log.hh"
#include "transport_tcp.hh"


using namespace std;


#define SCDC_LOG_PREFIX  "transport-tcp: "


struct scdc_transport_tcp_connection_t: public scdc_transport_connection_t
{
  int socket;

  scdc_transport_tcp_connection_t(scdc_transport *transport_)
    :scdc_transport_connection_t(transport_), socket(0) { };
};


struct scdc_transport_tcp_server_t
{
  struct sockaddr_in saddr;
  int server_socket;

  scdc_transport_tcp_server_t()
    :server_socket(0) { };
};


#if SCDC_TRANSPORT_TCP_USE_SHUTDOWN_EXCEPTION
class scdc_transport_tcp_shutdown_ex: public exception
{
};
#endif


scdc_transport_tcp::scdc_transport_tcp()
  :scdc_transport(), tcp_server(0)
{
}


scdc_transport_tcp::~scdc_transport_tcp()
{
}


bool scdc_transport_tcp::open(const char *address, int port)
{
  SCDC_TRACE("open: address: '" << ((address && address[0] != '\0')?address:"<ANY>") << "', port: " << port);

  tcp_server = new scdc_transport_tcp_server_t();

  tcp_server->server_socket = ::socket(AF_INET, SOCK_STREAM, 0);
  if (tcp_server->server_socket == -1)
  {
    SCDC_ERROR("open: creating server socket failed: '" << strerror(errno) << "'");
    return false;
  }

  struct hostent *h = 0;

  if (address && address[0] != '\0')
  {
    h = gethostbyname(address);

    if (!h)
    {
      SCDC_ERROR("open: gethostbyname failed: '" << strerror(h_errno) << "'");
      return false;
    }
  }

  memset(&tcp_server->saddr, 0, sizeof(tcp_server->saddr));

  tcp_server->saddr.sin_family = AF_INET;
  if (h) tcp_server->saddr.sin_addr = *((struct in_addr **) h->h_addr_list)[0];
  else tcp_server->saddr.sin_addr.s_addr = htonl(INADDR_ANY);
  tcp_server->saddr.sin_port = htons(port);

  if (::bind(tcp_server->server_socket, (struct sockaddr *) &tcp_server->saddr, sizeof(tcp_server->saddr)) == -1)
  {
    SCDC_ERROR("open: binding server socket failed: '" << strerror(errno) << "'");
    ::close(tcp_server->server_socket);
    return false;
  }

  if (::listen(tcp_server->server_socket, TRANSPORT_TCP_LISTEN_BACKLOG) == -1)
  {
    SCDC_ERROR("open: listen server socket failed: '" << strerror(errno) << "'");
    ::close(tcp_server->server_socket);
    return false;
  }

  return scdc_transport::open();
}


void scdc_transport_tcp::close()
{
  SCDC_TRACE("close:");

  ::close(tcp_server->server_socket);

  delete tcp_server;
  tcp_server = 0;
}


/*bool scdc_transport_tcp::start(scdcint_t mode)
{
  SCDC_TRACE("start: mode '" << mode << "');

  return scdc_transport::start(mode);
}*/


/*bool scdc_transport_tcp::stop()
{
  SCDC_TRACE("stop:");

  return scdc_transport::stop();
}*/


/*bool scdc_transport_tcp::cancel(bool interrupt)
{
  SCDC_TRACE("cancel: interrupt: '" << interrupt << "'");

  return scdc_transport::cancel(interrupt);
}*/


/*bool scdc_transport_tcp::resume()
{
  SCDC_TRACE("resume:");

  return scdc_transport::resume();
}*/


scdc_transport_connection_t *scdc_transport_tcp::accept()
{
  SCDC_TRACE("accept:");

  scdc_transport_tcp_connection_t *tcp_conn = new scdc_transport_tcp_connection_t(this);

  socklen_t saddrlen = sizeof(tcp_server->saddr);

#if SCDC_TRANSPORT_TCP_USE_SHUTDOWN_EXCEPTION
  try
  {
#endif
    tcp_conn->socket = ::accept(tcp_server->server_socket, (struct sockaddr *) &tcp_server->saddr, &saddrlen);

#if SCDC_TRANSPORT_TCP_USE_SHUTDOWN_EXCEPTION
  } catch (const std::exception& ex)
  {
    SCDC_ERROR("connection " << tcp_conn << ": exception during accept");
    tcp_conn->socket = -1;
  }
#endif

  if (tcp_conn->socket != -1)
  {
    tcp_conn->open = true;

  } else
  {
    if (!shutdowned) SCDC_FAIL("accept: accepting connection failed: '" << strerror(errno) << "'");
    delete tcp_conn;
    tcp_conn = 0;
    return 0;
  }

  SCDC_TRACE("accept: '" << tcp_conn << "'");

  return scdc_transport::accept(tcp_conn);
}


void scdc_transport_tcp::shutdown(scdc_transport_connection_t *conn, bool interrupt)
{
  SCDC_TRACE("shutdown: interrupt: '" << interrupt << "'");

  scdc_transport::shutdown(conn, interrupt);

#if SCDC_TRANSPORT_TCP_USE_SHUTDOWN_EXCEPTION
  if (interrupt) throw scdc_transport_tcp_shutdown_ex();
#endif

  if (conn)
  {
    SCDC_TRACE("shutdown: connection socket shutdown");

    scdc_transport_tcp_connection_t *tcp_conn = static_cast<scdc_transport_tcp_connection_t *>(conn);

    ::shutdown(tcp_conn->socket, SHUT_RDWR);

  } else if (tcp_server)
  {
    SCDC_TRACE("shutdown: server socket shutdown");

    ::shutdown(tcp_server->server_socket, SHUT_RDWR);
  }
}


void scdc_transport_tcp::serve(scdc_transport_connection_t *conn)
{
#if SCDC_TRANSPORT_TCP_USE_SHUTDOWN_EXCEPTION
  /* catch our shutdown exceptions while serving (i.e., outside send or receive) */
  try
  {
#endif
    scdc_transport::serve(conn);

#if SCDC_TRANSPORT_TCP_USE_SHUTDOWN_EXCEPTION
  } catch (const scdc_transport_tcp_shutdown_ex &ex)
  {
    SCDC_INFO("serve: catch shutdown exception");
  }
#endif
}


void scdc_transport_tcp::close(scdc_transport_connection_t *conn)
{
  SCDC_TRACE("close: '" << conn << "'");

  scdc_transport_tcp_connection_t *tcp_conn = static_cast<scdc_transport_tcp_connection_t *>(conn);

  ::close(tcp_conn->socket);

  scdc_transport::close(conn);

  delete tcp_conn;
}


scdc_transport_connection_t *scdc_transport_tcp::connect(const char *host, int port)
{
  SCDC_TRACE("connect: host: '" << host << "', port: " << port);

  scdc_transport_tcp_connection_t *tcp_conn = new scdc_transport_tcp_connection_t(this);

  tcp_conn->socket = ::socket(AF_INET, SOCK_STREAM, 0);
  if (tcp_conn->socket == -1)
  {
    SCDC_ERROR("connect: creating client socket failed: '" << strerror(errno) << "'");
    return 0;
  }

  struct hostent *h = gethostbyname(host);
  if (!h)
  {
    SCDC_ERROR("connect: gethostbyname failed: '" << strerror(h_errno) << "'");
    return 0;
  }
  
  struct sockaddr_in saddr;
  memset(&saddr, 0, sizeof(saddr));

  saddr.sin_family = AF_INET;
  saddr.sin_addr = *((struct in_addr **) h->h_addr_list)[0];
  saddr.sin_port = htons(port);

  int r;
  bool except = false;

#if SCDC_TRANSPORT_TCP_USE_SHUTDOWN_EXCEPTION
  try
  {
#endif
    r = ::connect(tcp_conn->socket, (struct sockaddr *) &saddr, sizeof(saddr));

#if SCDC_TRANSPORT_TCP_USE_SHUTDOWN_EXCEPTION
  } catch (const scdc_transport_tcp_shutdown_ex &ex)
  {
    SCDC_INFO("connect: catch shutdown exception");
    r = -1;
    except = true;
  }
#endif

  if (r == -1)
  {
    if (!except) SCDC_FAIL("connect: connecting to server failed: '" << strerror(errno) << "'");
    ::close(tcp_conn->socket);
    delete tcp_conn;
    return 0;
  }

  return scdc_transport::connect(tcp_conn);
}


bool scdc_transport_tcp::disconnect(scdc_transport_connection_t *conn)
{
  SCDC_TRACE("disconnect: conn: '" << conn << "'");

  if (!scdc_transport::disconnect(conn)) return false;

  scdc_transport_tcp_connection_t *tcp_conn = static_cast<scdc_transport_tcp_connection_t *>(conn);

  ::close(tcp_conn->socket);

  delete tcp_conn;

  return true;
}


scdcint_t scdc_transport_tcp::send(scdc_transport_connection_t *conn, const void *buf, scdcint_t buf_size)
{
  SCDC_TRACE("send: conn: '" << conn << "', buf: '" << buf << "', buf_size: " << buf_size);

  scdc_transport_tcp_connection_t *tcp_conn = static_cast<scdc_transport_tcp_connection_t *>(conn);

  scdcint_t n = 0;

  bool except = false;

#if SCDC_TRANSPORT_TCP_USE_SHUTDOWN_EXCEPTION
  try
  {
#endif
do_send:
    n = ::send(tcp_conn->socket, buf, buf_size, 0);

    if (n < 0 && errno == EINTR)
    {
      SCDC_TRACE("send: continuing after EINTR");
      goto do_send;
    }

#if SCDC_TRANSPORT_TCP_USE_SHUTDOWN_EXCEPTION
  } catch (const scdc_transport_tcp_shutdown_ex &ex)
  {
    SCDC_INFO("connection: " << tcp_conn << ": send: catch shutdown exception");
    n = -1;
    except = false;
  }
#endif

  if (n < 0)
  {
    if (!except) SCDC_ERROR("connection: " << tcp_conn << ": send: send failed: '" << strerror(errno) << "'");
    return -1;
  }

  SCDC_TRACE("connection: " << tcp_conn << ": sent " << n << " byte(s)");
  
  return n;
}


scdcint_t scdc_transport_tcp::receive(scdc_transport_connection_t *conn, void *buf, scdcint_t max_buf_size)
{
  SCDC_TRACE("receive: conn: '" << conn << "', buf: '" << buf << "', max_buf_size: " << max_buf_size);

  scdc_transport_tcp_connection_t *tcp_conn = static_cast<scdc_transport_tcp_connection_t *>(conn);

  if (max_buf_size <= 0)
  {
    SCDC_TRACE("connection: " << tcp_conn << ": receive: nothing to receive");
    return 0;
  }

  scdcint_t n = 0;

  bool except = false;

#if SCDC_TRANSPORT_TCP_USE_SHUTDOWN_EXCEPTION
  try
  {
#endif

do_recv:
    n = ::recv(tcp_conn->socket, buf, max_buf_size, 0);

    if (n < 0 && errno == EINTR)
    {
      SCDC_TRACE("receive: continuing after EINTR");
      goto do_recv;
    }

#if SCDC_TRANSPORT_TCP_USE_SHUTDOWN_EXCEPTION
  } catch (const scdc_transport_tcp_shutdown_ex &ex)
  {
    SCDC_INFO("connection " << tcp_conn << ": receive: catch shutdown receive");
    n = -1;
    except = false;
  }
#endif

  if (n < 0)
  {
    if (!except) SCDC_ERROR("connection: " << tcp_conn << ": receive: receive failed: '" << strerror(errno) << "'");
    return -1;
  }

  SCDC_TRACE("connection " << tcp_conn << ": receive: received " << n << " byte(s)");

  /* receiving 0 bytes signals that the socket was closed */
  if (n == 0) n = -1;

  return n;
}


#undef SCDC_LOG_PREFIX
