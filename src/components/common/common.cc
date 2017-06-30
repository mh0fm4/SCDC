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


#include <sys/time.h>

#include <cstdio>
#include <cstring>
#include <algorithm>
#include <cmath>
#include <vector>

#define SCDC_TRACE_NOT  !SCDC_TRACE_COMMON

#include "config.hh"
#include "log.hh"
#include "common.hh"


using namespace std;


#define SCDC_LOG_PREFIX  "common: "


bool has_prefix_suffix(const char *s, const char *prefix, const char *suffix)
{
  size_t s_len = strlen(s);
  size_t prefix_len = (prefix)?strlen(prefix):0;
  size_t suffix_len = (suffix)?strlen(suffix):0;

/*  return (!prefix || strncmp(s, prefix, prefix_len) == 0) && (!suffix || (s_len >= suffix_len && strncmp(s + s_len - suffix_len, suffix, suffix_len) == 0));*/

  return (strncmp(s, prefix, prefix_len) == 0 && s_len >= suffix_len && strncmp(s + s_len - suffix_len, suffix, suffix_len) == 0);
}


bool has_prefix(const char *s, const char *prefix)
{
  return has_prefix_suffix(s, prefix, 0);
}


bool has_suffix(const char *s, const char *suffix)
{
  return has_prefix_suffix(s, 0, suffix);
}


void split_cmdline(const char *cmdline, scdcint_t cmdline_size, string *cmd, string *uri, string *params)
{
  const char *s = cmdline;
  scdcint_t ss = (cmdline_size < 0)?strlen(s):cmdline_size;

  SCDC_TRACE("split_cmdline: '" << string(s, ss) << "'");

  if (cmd) *cmd = "";
  if (uri) *uri = "";
  if (params) *params = "";

  if (!s || ss <= 0) goto exit;

  if (s[0] != ':' && strncmp(s, "scdc:", min(static_cast<scdcint_t>(5), ss)) != 0 && strncmp(s, "scdc+", min(static_cast<scdcint_t>(5), ss)) != 0) goto split_cmd;

  const char *n;

  n = memchr(s, ' ', ss);
  if (uri) *uri = (n)?string(s, min(ss, static_cast<scdcint_t>(n - s))):string(s, ss);
  if (n) { n++; ss -= n - s; } else { ss = 0; } 
  s = n;

  if (!s || ss <= 0) goto exit;

  SCDC_TRACE("split_cmdline: s: '" << string(s, ss) << "'");

split_cmd:

  n = memchr(s, ' ', ss);
  if (cmd) *cmd = (n)?string(s, min(ss, static_cast<scdcint_t>(n - s))):string(s, ss);
  if (n) { n++; ss -= n - s; } else { ss = 0; } 
  s = n;

  if (!s || ss <= 0) goto exit;

  SCDC_TRACE("split_cmdline: s: '" << string(s, ss) << "'");

  if (params) *params = string(s, ss);

exit:
  SCDC_TRACE("split_cmdline: cmd: '" << ((cmd)?*cmd:"") << "', uri: '" << ((uri)?*uri:"") << "', params: '" << ((params)?*params:"") << "'");
}


void join_cmdline(const char *cmd, const char *uri, const char *params, string &cmdline)
{
  SCDC_TRACE("join_cmdline: cmd: '" << ((cmd)?cmd:"") << "', uri: '" << ((uri)?uri:"") << "', params: '" << ((params)?params:"") << "'");

  cmdline = "";

  if (uri && strlen(uri) > 0)
  {
    if (cmdline.size() > 0) cmdline += " ";
    cmdline += uri;
  }

  if (cmd && strlen(cmd) > 0)
  {
    if (cmdline.size() > 0) cmdline += " ";
    cmdline += cmd;
  }

  if (params && strlen(params) > 0)
  {
    if (cmdline.size() > 0) cmdline += " ";
    cmdline += params;
  }

  SCDC_TRACE("join_cmdline: '" << cmdline << "'");
}


void split_uri(const char *uri, scdcint_t uri_size, string *scheme, string *authority, string *path)
{
  string s = (uri_size < 0)?string(uri):string(uri, uri_size);

  SCDC_TRACE("split_uri: '" << s << "'");

  if (scheme) *scheme = "";
  if (authority) *authority = "";
  if (path) *path = "";

  size_t offset = 0, p;

  /* if there is a (first) ':', then ... */
  p = s.find(":", offset);
  if (p != string::npos)
  {
    /* ... all up to the ':' is the scheme */
    if (scheme) *scheme = s.substr(offset, p - offset);
    offset = p + 1;
  }

  /* if there is a '//' following, then ... */
  if (s.substr(offset, 2) == "//")
  {
    offset += 2;

    /* ... if there is a '/', then ... */
    p = s.find("/", offset);
    if (p != string::npos)
    {
      /* ... all up to the '/' is the authority */
      if (authority) *authority = s.substr(offset, p - offset);
      offset = p + 1;

    } else
    {
      /* ... all remaining is the authority */
      if (authority) *authority = s.substr(offset);
      offset = string::npos;
    }

  } else
  {
    /* ... if there is an immediate '/', then skip it (because it marks the beginning of the path) */
    p = s.find("/", offset);
    if (p == offset) ++offset;
  }

  /* if there is something left, then this is the path */
  if (offset != string::npos)
  {
    if (path) *path = s.substr(offset);
  }

  SCDC_TRACE("split_uri: scheme: '" << ((scheme)?*scheme:"") << "', authority: '" << ((authority)?*authority:"") << "', path: '" << ((path)?*path:"") << "'");
}


void join_uri(const char *scheme, const char *authority, const char *path, string &uri)
{
  SCDC_TRACE("join_uri: scheme: '" << ((scheme)?scheme:"") << "', authority: '" << ((authority)?authority:"") << "', path: '" << ((path)?path:"") << "'");

  uri = "";

  if (scheme && strlen(scheme) > 0)
  {
    uri += scheme;
    uri += ":";
  }

  if (authority && strlen(authority) > 0)
  {
    if (uri.size() > 0) uri += "//";
    uri += authority;
  }

  if (path && strlen(path) > 0)
  {
    if (uri.size() > 0) uri += "/";
    uri += path;
  }

  SCDC_TRACE("join_uri: '" << uri << "'");
}


void normalize_path(const char *path, string &normalized)
{
  SCDC_TRACE("normalize_path: path: '" << ((path)?path:"") << "'");

  normalized = "";

  if (!path) return;

  vector<string> parts;

  const char *p, *s = path;

  while (1)
  {
    p = strchr(s, '/');
    if (!p) p = s + strlen(s);

    string t(s, p - s);

    if (t == ".");
    else if (t == ".." && !parts.empty() && parts.back() != "..") parts.pop_back();
    else if (t.size() > 0) parts.push_back(t);
    
    if (p[0] == '\0') break;

    s = p + 1;
  }

  for (vector<string>::iterator i = parts.begin(); i != parts.end(); ++i)
  {
    if (path[0] == '/' || i != parts.begin()) normalized += "/";

    normalized += *i;
  }

  SCDC_TRACE("normalize_path: normalized: '" << normalized << "'");
}


static double z_time_wtime_offset = -1;

void abstimeout(struct timespec *abstime, double timeout, double z_time_wtime_base)
{
  if (z_time_wtime_offset < 0)
  {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    z_time_wtime_offset = (double) tv.tv_sec + (tv.tv_usec * 1e-6) - z_time_wtime();
  }

  double t = z_time_wtime_base;

  if (t < 0) t = z_time_wtime();

  t += timeout + z_time_wtime_offset;

  abstime->tv_sec = fabs(t);
  abstime->tv_nsec = (t - abstime->tv_sec) * 1e9;
}


void mem2str(const void *mem, scdcint_t mem_size, std::string &str)
{
/*  char v[sizeof(int) * 2 + 1];
  const char *m = mem;

  str = "";
  for (; mem_size >= sizeof(int); mem_size -= sizeof(int))
  {
    sprintf(v, "%.*X", static_cast<unsigned int *>(m));
    str += v;
  }

  for (; mem_size > 0; --mem_size)
  {
  }*/
}


void str2mem(const std::string str, void *mem, scdcint_t *mem_size)
{
}
