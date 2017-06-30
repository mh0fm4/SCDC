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


#ifndef __SCDC_ARGS_HH__
#define __SCDC_ARGS_HH__


#include <list>

#include "scdc_defs.h"
#include "scdc_args.h"



static inline void scdc_args_t_ctor(scdc_args_t &args)
{
  args.data = 0;
  args.get = 0;
  args.set = 0;
  args.free = 0;
};


struct scdc_args_get_entry_t
{
  scdcint_t type;
  scdc_arg_ref_t arg_ref;
  scdcint_t n;
  void *v0, *v1;

  scdc_args_get_entry_t()
    :type(SCDC_ARGS_TYPE_NULL), arg_ref(SCDC_ARG_REF_NULL), n(0), v0(0), v1(0) { }
  scdc_args_get_entry_t(scdcint_t type_, scdc_arg_ref_t arg_ref_, scdcint_t n_, void *v0_, void *v1_)
    :type(type_), arg_ref(arg_ref_), n(n_), v0(v0_), v1(v1_) { }

  bool operator== (const scdc_args_get_entry_t &v) {
    return (type == v.type && arg_ref == v.arg_ref && n == v.n && v0 == v.v0 && v1 == v.v1);
  }
};


class scdc_args
{
  public:
    scdc_args() { scdc_args_t_ctor(args); }
    scdc_args(const scdc_args_t &args_) { args = args_; }
    scdc_args(const scdc_args_t *args_) { args = *args_; }
    scdc_args(const scdc_args *args_) { *this = *args_; }

    ~scdc_args() { free_all(); }

    template<typename T>
    scdc_arg_ref_t get(scdcint_t type, T *v, bool long_ref = false) {
      if (!args.get) return SCDC_ARG_REF_NULL;
      scdc_arg_ref_t r = args.get(args.data, type, static_cast<void *>(v));
      if (long_ref && r != SCDC_ARG_REF_NULL) add_get(type, r, 1, v, 0);
      return r;
    }

    template<typename T0, typename T1>
    scdc_arg_ref_t get(scdcint_t type, T0 *v0, T1 *v1, bool long_ref = false) {
      if (!args.get) return SCDC_ARG_REF_NULL;
      void *a[2] = { reinterpret_cast<void *>(v0), static_cast<void *>(v1) };
      scdc_arg_ref_t r = args.get(args.data, type, a);
      if (long_ref && r != SCDC_ARG_REF_NULL) add_get(type, r, 2, v0, v1);
      return r;
    }

    template<typename T>
    scdc_arg_ref_t set(scdcint_t type, T *v, scdc_arg_ref_t arg_ref) {
      if (!args.set) return SCDC_ARG_REF_NULL;
      return args.set(args.data, type, static_cast<void *>(v), arg_ref);
    }

    template<typename T>
    void free(scdcint_t type, T *v, scdc_arg_ref_t arg_ref) {
      if (!args.free || arg_ref == SCDC_ARG_REF_NULL) return;
      del_get(type, arg_ref, 1, v, 0);
      args.free(args.data, type, static_cast<void *>(v), arg_ref);
    }

    template<typename T0, typename T1>
    void free(scdcint_t type, T0 *v0, T1 *v1, scdc_arg_ref_t arg_ref) {
      if (!args.free || arg_ref == SCDC_ARG_REF_NULL) return;
      del_get(type, arg_ref, 2, v0, v1);
      void *a[2] = { static_cast<void *>(v0), static_cast<void *>(v1) };
      args.free(args.data, type, static_cast<void *>(a), arg_ref);
    }

    void free_all() {
      while (gets.size() > 0) {
        const scdc_args_get_entry_t &e = gets.front();
        if (e.n == 1) free<void>(e.type, e.v0, e.arg_ref);
        else if (e.n == 2) free<void, void>(e.type, e.v0, e.v1, e.arg_ref);
      }
    }

    /* FIXME: get_nargs_left? */

    scdc_args_t *get_args() { return &args; }
    void clear_args_data() { args.data = 0; }

  private:
    scdc_args_t args;
    typedef std::list<scdc_args_get_entry_t> gets_t;
    gets_t gets;

    void add_get(scdcint_t type, scdc_arg_ref_t arg_ref, scdcint_t n, void *v0, void *v1) {
      gets.push_back(scdc_args_get_entry_t(type, arg_ref, n, v0, v1));
    }
      
    void del_get(scdcint_t type, scdc_arg_ref_t arg_ref, scdcint_t n, void *v0, void *v1) {
      const scdc_args_get_entry_t v(type, arg_ref, n, v0, v1);
      for (gets_t::iterator i = gets.begin(); i != gets.end(); ++i) if (*i == v) { gets.erase(i); break; }
    }
};


class scdc_args_string
{
  public:
    scdc_args_string(std::string &s_): s(s_) {
    }

    template<typename T>
    scdc_arg_ref_t get(scdcint_t type, T *v, bool long_ref = false) {
      scdc_arg_ref_t r = get(s, type, v);
/*      if (long_ref && r != SCDC_ARG_REF_NULL) add_get(type, r, 1, v, 0);*/
      return r;
    }

    template<typename T>
    static scdc_arg_ref_t get(std::string &s_, scdcint_t type, T *v) {
      scdc_arg_ref_t r = SCDC_ARG_REF_NULL;
      if (type == SCDC_ARGS_TYPE_SCDCINT)
      {
        if (sscanf(s_.c_str(), "%" scdcint_fmt, v) == 1) r = SCDC_ARG_REF_NONE;
      }
      return r;
    }

  private:
    std::string &s;
};


#endif /* __SCDC_ARGS_HH__ */
