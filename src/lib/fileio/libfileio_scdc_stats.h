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


#ifndef __LIBFILEIO_SCDC_STATS_H__
#define __LIBFILEIO_SCDC_STATS_H__


#if HAVE_UTHASH_H
# include <uthash.h>
#else
# undef LIBFILEIO_SCDC_STATS
#endif

#if LIBFILEIO_SCDC_STATS

#include "z_pack.h"
#include "common.h"

typedef struct
{
  int fd;
  char p[LIBFILEIO_SCDC_PATH_MAX];
  dsint_t nread, nwrite;
  double tinit, tread, twrite;
  UT_hash_handle hh;

} libfileio_scdc_stats_t;

libfileio_scdc_stats_t *libfileio_scdc_stats = NULL;

#define LIBFILEIO_SCDC_STATS_CMD(_c_)  Z_MOP(_c_)

#define LIBFILEIO_SCDC_STATS_INIT()  Z_NOP()

static inline void LIBFILEIO_SCDC_STATS_RELEASE()
{
  libfileio_scdc_stats_t *e, *t;

  HASH_ITER(hh, libfileio_scdc_stats, e, t)
  {
    HASH_DEL(libfileio_scdc_stats, e);
    free(e);
  }
}

static inline void LIBFILEIO_SCDC_STATS_ADD(int fd, const char *p)
{
  if (fd < 0) return;

  TRACE_F("stats: add: fd: %d, file: '%s'", fd, p);

  libfileio_scdc_stats_t *e = malloc(sizeof(libfileio_scdc_stats_t));

  e->fd = fd;

  strncpy(e->p, p, LIBFILEIO_SCDC_PATH_MAX - 1);
  e->p[LIBFILEIO_SCDC_PATH_MAX - 1] = '\0';

  e->nread = e->nwrite = 0;

  e->tinit = z_time_wtime();
  e->tread = e->twrite = 0;

  HASH_ADD_INT(libfileio_scdc_stats, fd, e);
}

static inline void LIBFILEIO_SCDC_STATS_DEL(int fd)
{
  if (fd < 0) return;

  libfileio_scdc_stats_t *e;

  HASH_FIND_INT(libfileio_scdc_stats, &fd, e);

  if (!e) return;

  TRACE_F("stats: del: fd: %d, file: '%s'", fd, e->p);

  HASH_DEL(libfileio_scdc_stats, e);
  free(e);
}

#define LIBFILEIO_SCDC_STATS_TIME(_t_)  double _t_ = z_time_wtime()

static inline void LIBFILEIO_SCDC_STATS_NREAD(int fd, dsint_t nread, double tstat)
{
  if (fd < 0) return;

  libfileio_scdc_stats_t *e;

  HASH_FIND_INT(libfileio_scdc_stats, &fd, e);

  if (!e) return;

  e->nread += nread;

  if (tstat >= 0) e->tread += z_time_wtime() - tstat;
}

static inline void LIBFILEIO_SCDC_STATS_NWRITE(int fd, dsint_t nwrite, double tstat)
{
  if (fd < 0) return;

  libfileio_scdc_stats_t *e;

  HASH_FIND_INT(libfileio_scdc_stats, &fd, e);

  if (!e) return;

  e->nwrite += nwrite;

  if (tstat >= 0) e->twrite += z_time_wtime() - tstat;
}

static inline void LIBFILEIO_SCDC_STATS_INFO(int fd)
{
  if (fd < 0) return;

  libfileio_scdc_stats_t *e;

  HASH_FIND_INT(libfileio_scdc_stats, &fd, e);

  if (e)
  {
    double tcur = z_time_wtime();
    double tsum = tcur - e->tinit;

    TRACE_F("stats: info: fd: %d, file: '%s', nread: %" dsint_fmt ", nwrite: %" dsint_fmt ", t: %f, read: %.2f, write: %.2f", fd, e->p, e->nread, e->nwrite, tsum, e->nread / tsum * 1e-6, e->nwrite / tsum * 1e-6);

  } else
  {
    TRACE_F("stats: info: fd: %d, file not found", fd);
  }
}

#else /* LIBFILEIO_SCDC_STATS */

#define LIBFILEIO_SCDC_STATS_CMD(...)      Z_NOP()
#define LIBFILEIO_SCDC_STATS_INIT(...)     Z_NOP()
#define LIBFILEIO_SCDC_STATS_RELEASE(...)  Z_NOP()
#define LIBFILEIO_SCDC_STATS_ADD(...)      Z_NOP()
#define LIBFILEIO_SCDC_STATS_DEL(...)      Z_NOP()
#define LIBFILEIO_SCDC_STATS_TIME(...)     
#define LIBFILEIO_SCDC_STATS_NREAD(...)    Z_NOP()
#define LIBFILEIO_SCDC_STATS_NWRITE(...)   Z_NOP()
#define LIBFILEIO_SCDC_STATS_INFO(...)     Z_NOP()

#undef LIBFILEIO_SCDC_STATS_ORIG
#undef LIBFILEIO_SCDC_STATS_SCDC

#endif /* LIBFILEIO_SCDC_STATS */


#if LIBFILEIO_SCDC_STATS_ORIG
# define LIBFILEIO_SCDC_STATS_ORIG_CMD(...)  __VA_ARGS__
#else
# define LIBFILEIO_SCDC_STATS_ORIG_CMD(...)  Z_NOP()
#endif

#if LIBFILEIO_SCDC_STATS_SCDC
# define LIBFILEIO_SCDC_STATS_SCDC_CMD(...)  __VA_ARGS__
#else
# define LIBFILEIO_SCDC_STATS_SCDC_CMD(...)  Z_NOP()
#endif


#endif /* __LIBFILEIO_SCDC_STATS_H__ */
