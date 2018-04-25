// Wrapper of C-language FILE struct -*- C++ -*-

// Copyright (C) 2000-2017 Free Software Foundation, Inc.
//
// This file is part of the GNU ISO C++ Library.  This library is free
// software; you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the
// Free Software Foundation; either version 3, or (at your option)
// any later version.

// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// Under Section 7 of GPL version 3, you are granted additional
// permissions described in the GCC Runtime Library Exception, version
// 3.1, as published by the Free Software Foundation.

// You should have received a copy of the GNU General Public License and
// a copy of the GCC Runtime Library Exception along with this program;
// see the files COPYING3 and COPYING.RUNTIME respectively.  If not, see
// <http://www.gnu.org/licenses/>.

//
// ISO C++ 14882: 27.8  File-based streams
//

/** @file bits/basic_file.h
 *  This is an internal header file, included by other library headers.
 *  Do not attempt to use it directly. @headername{ios}
 */

#ifndef __BASIC_FILE_SCDC_HH__
#define __BASIC_FILE_SCDC_HH__

#pragma GCC system_header

#include <bits/c++config.h>
// #include <bits/c++io.h>  // for __c_lock and __c_file
#include <bits/move.h>   // for swap
#include <ios>

#include <cstring>

#include "scdc.h"

#define BASIC_FILE_SCDC_APPEND  0


#define __basic_file  __basic_file_scdc

namespace FSTREAM_SCDC_NAMESPACE _GLIBCXX_VISIBILITY(default)
{
  using namespace std;

  static const char *fstream_scdc_local_base = 0;
  static const char *fstream_scdc_local_path = 0;
  static scdc_dataprov_t fstream_scdc_dp_local = SCDC_DATAPROV_NULL;

#define __class__  "fstream_scdc_env"

  class fstream_scdc_env
  {
    protected:
      int initialized;

    public:
      fstream_scdc_env():initialized(-1)
      {
        TRACE(__class__, __func__, "");
      }

      ~fstream_scdc_env()
      {
        TRACE(__class__, __func__, "");
        initialized = -1;
        release();
      }

      void init()
      {
        TRACE(__class__, __func__ , "");
        if (initialized < 0)
        {
          TRACE(__class__, __func__, "first init");

          /* on first init */
          initialized = 0;
          scdc_init(SCDC_INIT_DEFAULT);

#ifdef USE_FSTREAM_SCDC_LOCAL
          const char *local_base = fstream_scdc_local_base;
          const char *local_path = fstream_scdc_local_path;
          if (!local_base || !local_path)
          {
            local_base = FSTREAM_SCDC_LOCAL_BASE;
            local_path = FSTREAM_SCDC_LOCAL_PATH;
          }

          TRACE(__class__, __func__, std::string("local data provider '") + local_base + "' with path '" + local_path + "'");

          fstream_scdc_dp_local = scdc_dataprov_open(local_base, "fs:access", local_path);
#endif /* USE_FSTREAM_SCDC_LOCAL */
        }

      if (initialized == 0)
      {
        TRACE(__class__, __func__, "every init");

        /* on every init */
      }

      ++initialized;
    }

    void release()
    {
      TRACE(__class__, __func__ , "");
      if (initialized < 0)
      {
        TRACE(__class__, __func__, "last release");

          /* on last release */
#ifdef USE_FSTREAM_SCDC_LOCAL
          scdc_dataprov_close(fstream_scdc_dp_local);
#endif /* USE_FSTREAM_SCDC_LOCAL */

          scdc_release();
      }

      if (initialized <= 0) return;

      --initialized;

      if (initialized == 0)
      {
        TRACE(__class__, __func__, "every release");

        /* on every release */
      }
    }
  };

#undef __class__

  static fstream_scdc_env the_fstream_scdc_env;

#define __class__  "basic_file_scdc"

  // Generic declaration.
  template<typename _CharT>
    class __basic_file;

  // Specialization.
  template<>
    class __basic_file<char>
    {
      // Underlying data source/sink.
      scdc_dataset_t _M_dataset;

      // True iff we opened _M_cfile, and thus must close it ourselves.
      bool 		_M_cfile_created;

      bool do_read, do_write, do_truncate, do_append;
      streamsize offset;
      bool eof;

    public:
      static bool is_valid(const char* __name)
      {
        TRACE(__class__, __func__, __name);
        return (scdc_nodeport_supported(__name) == SCDC_SUCCESS);
      }

    public:
      __basic_file() throw ()
      : _M_dataset(SCDC_DATASET_NULL), _M_cfile_created(false),
      do_read(false), do_write(false), do_truncate(false), do_append(false),
      offset(0), eof(false)
      {}

#if __cplusplus >= 201103L
      __basic_file(__basic_file&& __rv) noexcept
      : _M_dataset(__rv._M_dataset), _M_cfile_created(__rv._M_cfile_created),
      do_read(__rv.do_read), do_write(__rv.do_write), do_truncate(__rv.do_truncate), do_append(__rv.do_append),
      offset(__rv.offset), eof(__rv.eof)
      {
	__rv._M_dataset = SCDC_DATASET_NULL;
	__rv._M_cfile_created = false;
	__rv.do_read = false;
	__rv.do_write = false;
	__rv.do_truncate = false;
	__rv.do_append = false;
	__rv.offset = 0;
	__rv.eof = false;
      }

      __basic_file& operator=(const __basic_file&) = delete;
      __basic_file& operator=(__basic_file&&) = delete;

      void
      swap(__basic_file& __f) noexcept
      {
	std::swap(_M_dataset, __f._M_dataset);
	std::swap(_M_cfile_created, __f._M_cfile_created);
	std::swap(do_read, __f.do_read);
	std::swap(do_write, __f.do_write);
	std::swap(do_truncate, __f.do_truncate);
	std::swap(do_append, __f.do_append);
	std::swap(offset, __f.offset);
	std::swap(eof, __f.eof);
      }
#endif

      __basic_file* 
      open(const char* __name, ios_base::openmode __mode, int __prot = 0664)
      {
        TRACE(__class__, __func__, __name);

        __basic_file* ret = 0;

        do_read = __mode & std::ios_base::in;
        do_write = __mode & std::ios_base::out;
        do_truncate = __mode & std::ios_base::trunc;
#if BASIC_FILE_SCDC_APPEND
        do_append = __mode & std::ios_base::app;
#endif
        offset = 0;
        eof = false;

        char p[256];
        const char *f = strrchr(__name, '/');

        if (f)
        {
          int pn = f - __name + 1;
          strncpy(p, __name, pn);
          p[pn] = '\0';
          ++f;
        }

        the_fstream_scdc_env.init();

        // open path without file and fail if it fails (i.e., does not exist)
        _M_dataset = scdc_dataset_open(p);
        if (_M_dataset == SCDC_DATASET_NULL) goto do_return;

        char cmd[256];

        // if trunc and out then 'rm file'
        if (do_truncate && do_write)
        {
          sprintf(cmd, "rm %s", f);
          scdc_dataset_cmd(_M_dataset, cmd, NULL, NULL);
        }

        // if out then 'put file' without input -> create empty file if it does not exist, yet
        if (do_write)
        {
          sprintf(cmd, "put %s", f);
          scdc_dataset_cmd(_M_dataset, cmd, NULL, NULL);
        }

        //* 'cd file' and fail if it does not exist */
        sprintf(cmd, "cd %s", f);
        if (scdc_dataset_cmd(_M_dataset, cmd, NULL, NULL) != SCDC_SUCCESS) goto do_return;

        ret = this;

do_return:
        if (!ret)
        {
          scdc_dataset_close(_M_dataset);

          _M_dataset = SCDC_DATASET_NULL;
        }

        if (_M_dataset == SCDC_DATASET_NULL) the_fstream_scdc_env.release();

        return ret;
      }

      // __basic_file*
      // sys_open(__c_file* __file, ios_base::openmode);

      // __basic_file*
      // sys_open(int __fd, ios_base::openmode __mode) throw ();

      __basic_file* 
      close()
      {
        TRACE(__class__, __func__, "");

        __basic_file* ret = this;

        scdc_dataset_close(_M_dataset);
        _M_dataset = SCDC_DATASET_NULL;

        the_fstream_scdc_env.release();

        return ret;
      }

      _GLIBCXX_PURE bool 
      is_open() const throw ()
      {
        TRACE(__class__, __func__, "");
        return (_M_dataset != SCDC_DATASET_NULL);
      }

      // _GLIBCXX_PURE int 
      // fd() throw ();

      // _GLIBCXX_PURE __c_file*
      // file() throw ();

      ~__basic_file()
      {
        TRACE(__class__, __func__, "");
        if (_M_dataset != SCDC_DATASET_NULL) close();
      }

      streamsize 
      xsputn(const char* __s, streamsize __n)
      {
        TRACE(__class__, __func__, "n: %lld", (long long) __n);

        streamsize ret = -1;
        streamsize noffset = offset;

        if (!do_write) goto do_return;

#if BASIC_FILE_SCDC_APPEND
        if (do_append) noffset = 0;
#endif

        scdc_dataset_input_t input;

        scdc_dataset_input_unset(&input);
        SCDC_DATASET_INOUT_BUF_PTR(&input) = const_cast<char *>(__s);
        SCDC_DATASET_INOUT_BUF_SIZE(&input) = __n;
        SCDC_DATASET_INOUT_BUF_CURRENT(&input) = __n;
        input.total_size = __n;
        input.total_size_given = SCDC_DATASET_INOUT_TOTAL_SIZE_GIVEN_EXACT;

        char cmd[256];
        sprintf(cmd, "put %" scdcint_fmt "%c:%" scdcint_fmt,
#if BASIC_FILE_SCDC_APPEND
        (do_append)?0:
#endif
        noffset,
#if BASIC_FILE_SCDC_APPEND
        (do_append)?'B':
#endif
        'F', __n);

        if (scdc_dataset_cmd(_M_dataset, cmd, &input, NULL) != SCDC_SUCCESS) goto do_return;

        ret = __n;
        noffset += __n;

do_return:
        if (ret >= 0)
        {
#if BASIC_FILE_SCDC_APPEND
          if (!do_append)
#endif
          {
            offset = noffset;
          }
        }

        TRACE(__class__, __func__, "return: %lld, offset: %lld", (long long) ret, (long long) offset);

        return ret;
      }

      streamsize 
      xsputn_2(const char* __s1, streamsize __n1, const char* __s2, streamsize __n2)
      {
        if (xsputn(__s1, __n1) < 0 || xsputn(__s2, __n2) < 0) return -1;
        return __n1 + __n2;
      }

      streamsize 
      xsgetn(char* __s, streamsize __n)
      {
        TRACE(__class__, __func__, "n: %lld", (long long) __n);

        streamsize ret = -1;
        streamsize noffset = offset;

        if (!do_read) goto do_return;

        if (eof)
        {
          ret = 0;
          goto do_return;
        }

        char cmd[256];
        sprintf(cmd, "get %" scdcint_fmt "F:%" scdcint_fmt, noffset, __n);

        scdc_dataset_output_t output;

        scdc_dataset_output_unset(&output);
        SCDC_DATASET_INOUT_BUF_PTR(&output) = __s;
        SCDC_DATASET_INOUT_BUF_SIZE(&output) = __n;

        if (scdc_dataset_cmd(_M_dataset, cmd, NULL, &output) != SCDC_SUCCESS) goto do_return;

        {
          scdcint_t size_left = __n;
          char *buf = __s;

          do
          {
            scdcint_t n = std::min(SCDC_DATASET_INOUT_BUF_CURRENT(&output), size_left);

            if (SCDC_DATASET_INOUT_BUF_PTR(&output) != buf) memcpy(buf, SCDC_DATASET_INOUT_BUF_PTR(&output), n);

            /* inc. dest. */
            buf += n;
            size_left -= n;

            if (!output.next || size_left <= 0) break;

            SCDC_DATASET_INOUT_BUF_PTR(&output) = buf;
            SCDC_DATASET_INOUT_BUF_SIZE(&output) = size_left;

            if (output.next(&output) == SCDC_FAILURE) goto do_return;

          } while (1);

          while (output.next) output.next(&output);

          ret = __n - size_left;
          noffset += __n - size_left;
        }

do_return:
        if (ret >= 0)
        {
          if (ret < __n) eof = true;

          offset = noffset;
        }

        TRACE(__class__, __func__, "return: %lld, offset: %lld, eof: %d", (long long) ret, (long long) offset, (int) eof);

        return ret;
      }

      streamoff
      seekoff(streamoff __off, ios_base::seekdir __way) throw ()
      {
        TRACE(__class__, __func__, "");

        streamoff ret = -1;

        if (__way == ios_base::end)
        {
          char buf[32];
          scdc_dataset_output_t output;

          scdc_dataset_output_unset(&output);
          SCDC_DATASET_INOUT_BUF_PTR(&output) = buf;
          SCDC_DATASET_INOUT_BUF_SIZE(&output) = sizeof(buf) - 1;

          if (scdc_dataset_cmd(_M_dataset, "ls", NULL, &output) != SCDC_SUCCESS) goto do_return;

          long long o;
          std::string s(static_cast<const char *>(SCDC_DATASET_INOUT_BUF_PTR(&output)), SCDC_DATASET_INOUT_BUF_CURRENT(&output));
          sscanf(s.c_str(), "f:%lld|", &o);
          offset = o;
          __way = ios_base::cur;

          while (output.next) output.next(&output);
        }

        switch (__way)
        {
          case ios_base::beg:
            offset = std::max(static_cast<streamoff>(0), __off);
            ret = offset;
            break;
          case ios_base::cur:
            offset += __off;
            ret = offset;
            break;
          default:
            ret = -1;
            break;
        }

        if (ret >= 0) eof = false;

do_return:
        return ret;
      }

      // int 
      // sync();

      streamsize
      showmanyc()
      { return 0; }
  };

#undef __class__

} // namespace FSTREAM_SCDC_NAMESPACE

#undef __basic_file

#endif /* __BASIC_FILE_SCDC_HH__ */
