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


#ifndef __SCDC_LOG_HH__
#define __SCDC_LOG_HH__


#define SCDC_LOG_LOCK  1


#if SCDC_LOG_LOCK
# include <pthread.h>
#endif

#include <cstdarg>
#include <iostream>

#include "args.hh"
#include "log.h"


#if SCDC_LOG_LOCK
extern pthread_mutex_t scdc_log_lock;
#endif

class scdc_log
{
  public:
    scdc_log();
    scdc_log(const char *conf, ...);
    ~scdc_log();

    bool default_init_intern(const char *conf, va_list aq);
    bool default_init(const char *conf, ...);
    void default_release();

    bool init(const char *conf, scdc_args_t *args);
    void release();

    void set_cout(std::ostream *cout_);
    std::ostream *get_cout() { return cout; }
    void cout_begin() {
#if SCDC_LOG_LOCK
      pthread_mutex_lock(&lock);
#endif
    }
    void cout_end() {
#if SCDC_LOG_LOCK
      pthread_mutex_unlock(&lock);
#endif
    }

    void set_cerr(std::ostream *cerr_);
    std::ostream *get_cerr() { return cerr; }
    void cerr_begin() {
#if SCDC_LOG_LOCK
      pthread_mutex_lock(&lock);
#endif
    }
    void cerr_end() {
#if SCDC_LOG_LOCK
      pthread_mutex_unlock(&lock);
#endif
    }

  private:
    std::ostream *cout_default, *cout, *cerr_default, *cerr;
    pthread_mutex_t lock;
};


extern scdc_log *scdc_main_context_log;
#define SCDC_MAIN_LOG()  scdc_main_context_log

#if 0
extern scdc_log scdc_main_log;
#define SCDC_MAIN_LOG()  (&scdc_main_log)
#endif

#define SCDC_LOG_COUT(_x_)  Z_MOP(if (SCDC_MAIN_LOG()->get_cout()) { SCDC_MAIN_LOG()->cout_begin(); *SCDC_MAIN_LOG()->get_cout() << _x_; SCDC_MAIN_LOG()->cout_end(); })
#define SCDC_LOG_CERR(_x_)  Z_MOP(if (SCDC_MAIN_LOG()->get_cerr()) { SCDC_MAIN_LOG()->cerr_begin(); *SCDC_MAIN_LOG()->get_cerr() << _x_; SCDC_MAIN_LOG()->cerr_end(); })

static inline std::string scdc_log_timestamp()
{
  time_t now = time(0);
  struct tm *timeinfo = localtime(&now);
  char s[256];
  sprintf(s, "%.2d.%.2d.%.4d %.2d:%.2d:%.2d: ", timeinfo->tm_mday, timeinfo->tm_mon + 1, timeinfo->tm_year + 1900, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
  return s;
}

#define SCDC_LOG_TIMESTAMP  scdc_log_timestamp()


#if HAVE_SCDC_INFO
# define SCDC_INFO_PREFIX  "SCDC-INFO: "
# define SCDC_INFO(_x_)    SCDC_LOG_COUT(SCDC_INFO_PREFIX << SCDC_LOG_TIMESTAMP << SCDC_LOG_PREFIX << _x_ << std::endl)
#else
# define SCDC_INFO(_x_)    Z_NOP()
#endif

#if HAVE_SCDC_TRACE && !SCDC_TRACE_NOT
# define SCDC_TRACE_DECL(_x_)                    _x_
# define SCDC_TRACE_CMD(_x_)                     Z_MOP(_x_)
# define SCDC_TRACE_PREFIX                       "SCDC-TRACE: "
# define SCDC_TRACE(_x_)                         SCDC_LOG_COUT(SCDC_TRACE_PREFIX << SCDC_LOG_PREFIX << _x_ << std::endl)
# define SCDC_TRACE_N(_x_)                       SCDC_LOG_COUT(SCDC_TRACE_PREFIX << SCDC_LOG_PREFIX << _x_)
# define SCDC_TRACE_C(_x_)                       SCDC_LOG_COUT(_x_ << std::endl)
# define SCDC_TRACE_C_N(_x_)                     SCDC_LOG_COUT(_x_)
# define SCDC_TRACE_DATASET_INPUT(_d_, _x_...)   Z_MOP(SCDC_TRACE_N(_x_); scdc_dataset_input_log_cout_print(_d_); SCDC_TRACE_C("");)
# define SCDC_TRACE_DATASET_OUTPUT(_d_, _x_...)  Z_MOP(SCDC_TRACE_N(_x_); scdc_dataset_output_log_cout_print(_d_); SCDC_TRACE_C("");)
#else
# define SCDC_TRACE_DECL(_x_)
# define SCDC_TRACE_CMD(_x_)                     Z_NOP()
# define SCDC_TRACE_N(_x_)                       Z_NOP()
# define SCDC_TRACE(_x_)                         Z_NOP()
# define SCDC_TRACE_C_N(_x_)                     Z_NOP()
# define SCDC_TRACE_C(_x_)                       Z_NOP()
# define SCDC_TRACE_INOUT(_d_, _x_)              Z_NOP()
# define SCDC_TRACE_INPUT(_d_, _x_)              Z_NOP()
# define SCDC_TRACE_OUTPUT(_d_, _x_)             Z_NOP()
# define SCDC_TRACE_DATASET_INPUT(_d_, _x_...)   Z_NOP()
# define SCDC_TRACE_DATASET_OUTPUT(_d_, _x_...)  Z_NOP()
#endif

#if HAVE_SCDC_FAIL
# define SCDC_FAIL_PREFIX  "SCDC-FAIL: "
# define SCDC_FAIL(_x_)    SCDC_LOG_COUT(SCDC_FAIL_PREFIX << SCDC_LOG_PREFIX << _x_ << std::endl)
#else
# define SCDC_FAIL(_x_)    Z_NOP()
#endif

#if HAVE_SCDC_ERROR
# define SCDC_ERROR_PREFIX  "SCDC-ERROR: "
# define SCDC_ERROR(_x_)    SCDC_LOG_CERR(SCDC_ERROR_PREFIX << SCDC_LOG_PREFIX << _x_ << std::endl)
#else
# define SCDC_ERROR(_x_)    Z_NOP()
#endif

#if HAVE_SCDC_ASSERT
# define SCDC_ASSERT_PREFIX  "SCDC-ASSERT: "
# define SCDC_ASSERT(_t_)    Z_MOP(if (!(_t_)) SCDC_LOG_CERR(SCDC_ASSERT_PREFIX << __FILE__ << ":" << __LINE__ << ": '" #_t_ << "' failed" << std::endl);)
#else
# define SCDC_ASSERT(_x_)    Z_NOP()
#endif

#if HAVE_SCDC_FATAL
# define SCDC_FATAL_PREFIX  "SCDC-FATAL: "
# define SCDC_FATAL(_x_)    SCDC_LOG_CERR(SCDC_FATAL_PREFIX << SCDC_LOG_PREFIX << _x_ << std::endl)
#else
# define SCDC_FATAL(_x_)    Z_NOP()
#endif


#endif /* __SCDC_LOG_HH__ */
