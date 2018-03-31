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


#ifndef __COMMON_HH__
#define __COMMON_HH__


#include <cstring>
#include <string>
#include <list>


inline static const char *memchr(const char *s, int c, size_t n)  { return static_cast<const char *>(memchr(static_cast<const void *>(s), c, n)); }

static inline std::string &ltrim(std::string &s, const char *t = 0)
{
  s.erase(0, s.find_first_not_of(t?t:" \t"));

  return s;
}

static inline std::string &rtrim(std::string &s, const char *t = 0)
{
  size_t p = s.find_last_not_of(t?t:" \t");
  if (std::string::npos != p) s.erase(p + 1);
  else s.clear();

  return s;
}

static inline std::string &trim(std::string &s, const char *t = 0)
{
  return ltrim(rtrim(s, t), t);
}

static inline std::string ltrim(const std::string &s, const char *t = 0)
{
  std::string str(s);
  return ltrim(str, t);
}

static inline std::string rtrim(const std::string &s, const char *t = 0)
{
  std::string str(s);
  return rtrim(str, t);
}

static inline std::string trim(const std::string &s, const char *t = 0)
{
  std::string str(s);
  return trim(str, t);
}

static inline std::string ltrim(const char *s, const char *t = 0)
{
  return ltrim(std::string(s), t);
}

static inline std::string rtrim(const char *s, const char *t = 0)
{
  return rtrim(std::string(s), t);
}

static inline std::string trim(const char *s, const char *t = 0)
{
  return trim(std::string(s), t);
}


class stringlist
{
  public:
    stringlist(const std::string s):sep(':') {  parse(s.c_str(), s.size()); }
    stringlist(int sep_, const std::string s):sep(sep_) {  parse(s.c_str(), s.size());  }

    stringlist(const char *s, ssize_t len = -1):sep(':') {  parse(s, len); }
    stringlist(int sep_, const char *s, ssize_t len = -1):sep(sep_) {  parse(s, len);  }

    void parse(const char *s, ssize_t len = -1)
    {
      parse(sep, s, len);
    }

    void parse(int sep_, const char *s, ssize_t len = -1)
    {
      if (!s || len == 0) return;

      const char *s_ = s;
      length = 0;

      while (s[0] != '\0' && len != 0)
      {
        size_t l;

        const char *n = (len >= 0)?memchr(s, sep_, len):strchr(s, sep_);

        if (n) l = n - s;
        else l = (len >= 0)?len:strlen(s);

        lst.push_back(entry_t(std::string(s, l), s - s_));

        if (n) ++l;

        s += l;
        len -= l;

        length += l;
      }
    }

    std::string conflate()
    {
      return conflate(sep);
    }

    std::string conflate(int sep_)
    {
      std::string s;

      for (list_t::iterator i = lst.begin(); i != lst.end(); ++i) s += i->first + static_cast<char>(sep_);

      if (s.size() > 0) s.resize(s.size() - 1);

      return s;
    }

    void conflate(std::string &s)
    {
      s = conflate();
    }

    void conflate(int sep_, std::string &s)
    {
      s = conflate(sep_);
    }

    std::string front()
    {
      std::string s;

      front(s);

      return s;
    }

    bool front(std::string &s)
    {
      if (lst.size() <= 0) return false;

      s = lst.front().first;

      return true;
    }

    std::string front_pop()
    {
      std::string s;

      front_pop(s);

      return s;
    }

    bool front_pop(std::string &s)
    {
      if (lst.size() <= 0) return false;

      s = front();

      lst.pop_front();

      return true;
    }

    std::string back()
    {
      std::string s;

      back(s);

      return s;
    }

    bool back(std::string &s)
    {
      if (lst.size() <= 0) return false;

      s = lst.back().first;

      return true;
    }

    std::string back_pop()
    {
      std::string s;

      back_pop(s);

      return s;
    }

    bool back_pop(std::string &s)
    {
      if (lst.size() <= 0) return false;

      s = back();

      lst.pop_back();

      return true;
    }

    size_t size()
    {
      return lst.size();
    }

    size_t offset()
    {
      return (size() > 0)?lst.front().second:length;
    }

  private:
    typedef std::pair<std::string, size_t> entry_t;
    typedef std::list<entry_t> list_t;
    list_t lst;
    int sep;
    size_t length;
};


bool has_prefix_suffix(const char *s, const char *prefix, const char *suffix);
bool has_prefix(const char *s, const char *prefix);
bool has_suffix(const char *s, const char *suffix);

static inline std::string trim_prefix_suffix(const char *s, const char *prefix, const char *suffix)
{
  std::string str(s);
  size_t prefix_len = (prefix)?strlen(prefix):0;
  size_t suffix_len = (suffix)?strlen(suffix):0;
  return str.substr(prefix_len, str.size() - prefix_len - suffix_len);
}

void split_cmdline(const char *cmdline, scdcint_t cmdline_size, std::string *cmd, std::string *uri, std::string *params);
void join_cmdline(const char *cmd, const char *uri, const char *params, std::string &cmdline);

void split_uri(const char *uri, scdcint_t uri_size, std::string *scheme, std::string *authority, std::string *path);
void join_uri(const char *scheme, const char *authority, const char *path, std::string &uri);

void normalize_path(const char *path, std::string &normalized);

void abstimeout(struct timespec *abstime, double timeout = 0, double z_time_wtime_base = -1);

void mem2str(const void *mem, scdcint_t mem_size, std::string &str);
void str2mem(const std::string str, void *mem, scdcint_t *mem_size);


#endif /* __COMMON_HH__ */
