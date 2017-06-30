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


#define _GNU_SOURCE

#include <dlfcn.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "scdc.h"
#include "z_pack.h"
#include "common.h"
#include "fileio_scdc.h"


#define TRACE_PREFIX   "libfileio_scdc: "

#define LIBFILEIO_SCDC_PATH_MAX     256

#define LIBFILEIO_SCDC_TRACE            0
#define LIBFILEIO_SCDC_TRACE_INTRO      0
#define LIBFILEIO_SCDC_TRACE_OUTRO      0
#define LIBFILEIO_SCDC_TRACE_BODY       0
#define LIBFILEIO_SCDC_TRACE_ORIG       0
#define LIBFILEIO_SCDC_TRACE_ORIG_RD    0
#define LIBFILEIO_SCDC_TRACE_ORIG_WR    0
#define LIBFILEIO_SCDC_TRACE_SCDC       0
#define LIBFILEIO_SCDC_TRACE_SCDC_RD    0
#define LIBFILEIO_SCDC_TRACE_SCDC_WR    0

#define LIBFILEIO_SCDC_STATS            0
#define LIBFILEIO_SCDC_STATS_ORIG       1
#define LIBFILEIO_SCDC_STATS_SCDC       1

#define LIBFILEIO_SCDC_ENABLED          1

#define LIBFILEIO_SCDC_PREFIX           0
#define LIBFILEIO_SCDC_PREFIX_NAME      libfileio_scdc_

#define LIBFILEIO_SCDC_PROVIDE_FD     1
#define LIBFILEIO_SCDC_PROVIDE_FILE   1
#define LIBFILEIO_SCDC_PROVIDE_FILE_  1  /* provide getc and putc (which may be defined as macros) */
#define LIBFILEIO_SCDC_PROVIDE_FS     1  /* provide stat, fstat, lstat, remove, unlink, rename */
#define LIBFILEIO_SCDC_PROVIDE        (LIBFILEIO_SCDC_PROVIDE_FD || LIBFILEIO_SCDC_PROVIDE_FILE || LIBFILEIO_SCDC_PROVIDE_FS)

#ifdef _LARGEFILE64_SOURCE
# define LIBFILEIO_SCDC_PROVIDE_64      1
#else
# define LIBFILEIO_SCDC_PROVIDE_64      0
#endif

#define LIBFILEIO_SCDC_PROVIDE_ISOC99_FSCANF   1

#define LIBFILEIO_SCDC_FD_NULL  0

#define LIBFILEIO_SCDC_FGETS_CHUNK_SIZE         1024
#define LIBFILEIO_SCDC_FSCANF_BUF_SIZE          1024
#define LIBFILEIO_SCDC_FSCANF_SSCANF            0
#define LIBFILEIO_SCDC_FSCANF_FMEM_PERSISTENT   1
#define LIBFILEIO_SCDC_FPRINTF_BUF_SIZE         1024
#define LIBFILEIO_SCDC_FPRINTF_SPRINTF          0
#define LIBFILEIO_SCDC_FPRINTF_FMEM_PERSISTENT  1


#if LIBFILEIO_SCDC_PREFIX

# define LIBF_F(_f_)      Z_CONCAT(LIBFILEIO_SCDC_PREFIX_NAME, _f_)
# define LIBF_F_STR(_f_)  Z_STRINGIFY(Z_CONCAT(LIBFILEIO_SCDC_PREFIX_NAME, _f_))

# define ORIG_INIT()        Z_NOP()
# define ORIG_RELEASE()     Z_NOP()
# define ORIG_F_INIT(_f_)   Z_NOP()
# define ORIG_F(_f_)        _f_
# define ORIG_F_INIT_(_f_)  Z_NOP()
# define ORIG_F_(_f_)       _f_

#else /* LIBFILEIO_SCDC_PREFIX */

# define LIBF_F(_f_)      _f_
# define LIBF_F_STR(_f_)  Z_STRINGIFY(_f_)

# include "libfileio_scdc_origs.h"
# define ORIG_INIT()        LIBFILEIO_SCDC_ORIGS_INIT()
# define ORIG_RELEASE()     LIBFILEIO_SCDC_ORIGS_RELEASE()
# define ORIG_F_INIT(_f_)   LIBFILEIO_SCDC_ORIGS_F_INIT(_f_)
# define ORIG_F(_f_)        LIBFILEIO_SCDC_ORIGS_F(_f_)
# define ORIG_F_INIT_(_f_)  LIBFILEIO_SCDC_ORIGS_F_INIT_(_f_)
# define ORIG_F_(_f_)       LIBFILEIO_SCDC_ORIGS_F_(_f_)

#endif /* LIBFILEIO_SCDC_PREFIX */

#include "libfileio_scdc_trace.h"
#include "libfileio_scdc_stats.h"


int original_fprintf(FILE *stream, const char *format, ...)
{
  va_list ap;
  va_start(ap, format);

  ORIG_F_INIT(vfprintf);
  int ret = ORIG_F(vfprintf)(stream, format, ap);

  va_end(ap);

  return ret;
}


#if LIBFILEIO_SCDC_PROVIDE

#if LIBFILEIO_SCDC_ENABLED

typedef struct
{
  int mark, fd, err;

  fileio_scdc_t fio;

#if LIBFILEIO_SCDC_FSCANF_FMEM_PERSISTENT
  FILE *fscanf_file;
  scdcint_t fscanf_buf_size;
  void *fscanf_buf;
#endif /* LIBFILEIO_SCDC_FSCANF_FMEM_PERSISTENT */

#if LIBFILEIO_SCDC_FPRINTF_FMEM_PERSISTENT
  FILE *fprintf_file;
  scdcint_t fprintf_buf_size;
  void *fprintf_buf;
#endif /* LIBFILEIO_SCDC_FPRINTF_FMEM_PERSISTENT */

} libfileio_scdc_t;


#define LIBFILEIO_SCDC_MARK           0xdeadbeef
#define LIBFILEIO_SCDC_VALID(_lfio_)  (((libfileio_scdc_t *) (_lfio_))->mark == LIBFILEIO_SCDC_MARK)


static libfileio_scdc_t *libfileio_scdc_create()
{
  libfileio_scdc_t *lfio = malloc(sizeof(libfileio_scdc_t));

  lfio->mark = LIBFILEIO_SCDC_MARK;
  lfio->fd = -1;
  lfio->err = 0;

#if LIBFILEIO_SCDC_FSCANF_FMEM_PERSISTENT
  lfio->fscanf_file = NULL;
  lfio->fscanf_buf_size = 0;
  lfio->fscanf_buf = NULL;
#endif /* LIBFILEIO_SCDC_FSCANF_FMEM_PERSISTENT */

#if LIBFILEIO_SCDC_FPRINTF_FMEM_PERSISTENT
  lfio->fprintf_file = NULL;
  lfio->fprintf_buf_size = 0;
  lfio->fprintf_buf = NULL;
#endif /* LIBFILEIO_SCDC_FPRINTF_FMEM_PERSISTENT */

  return lfio;
}


static void libfileio_scdc_destroy(libfileio_scdc_t *lfio)
{
#if LIBFILEIO_SCDC_FSCANF_FMEM_PERSISTENT
  if (lfio->fscanf_file) fclose(lfio->fscanf_file);
  if (lfio->fscanf_buf) free(lfio->fscanf_buf);
#endif /* LIBFILEIO_SCDC_FSCANF_FMEM_PERSISTENT */

#if LIBFILEIO_SCDC_FPRINTF_FMEM_PERSISTENT
  if (lfio->fprintf_file) fclose(lfio->fprintf_file);
  if (lfio->fprintf_buf) free(lfio->fprintf_buf);
#endif /* LIBFILEIO_SCDC_FPRINTF_FMEM_PERSISTENT */

  free(lfio);
}


#define LIBFILEIO_SCDC_FD_OFFSET       1024
#define LIBFILEIO_SCDC_FD_SIZE         16
#define LIBFILEIO_SCDC_FD_VALID(_fd_)  ((_fd_) >= LIBFILEIO_SCDC_FD_OFFSET && (_fd_) < LIBFILEIO_SCDC_FD_OFFSET + LIBFILEIO_SCDC_FD_SIZE)

static struct
{
  int next_fd;
#if LIBFILEIO_SCDC_FD_NULL
  int null_fd, null_fd_ref;
#endif /* LIBFILEIO_SCDC_FD_NULL */
  libfileio_scdc_t *ios[LIBFILEIO_SCDC_FD_SIZE];
  
} libfileio_scdc_fd =
{
  LIBFILEIO_SCDC_FD_OFFSET,
#if LIBFILEIO_SCDC_FD_NULL
 -1, 0,
#endif /* LIBFILEIO_SCDC_FD_NULL */
};


#if LIBFILEIO_SCDC_FD_NULL

static int libfileio_scdc_fd_null_open(int fd, int flags)
{
  if (libfileio_scdc_fd.null_fd < 0)
  {
    ORIG_F_INIT(open);
    libfileio_scdc_fd.null_fd = ORIG_F(open)("/dev/null", flags);
    libfileio_scdc_fd.null_fd_ref = 0;
  }

  ++libfileio_scdc_fd.null_fd_ref;

  return dup2(libfileio_scdc_fd.null_fd, fd);
}


static void libfileio_scdc_fd_null_close(int fd)
{
  ORIG_F_INIT(close);
  ORIG_F(close)(fd);

  if (libfileio_scdc_fd.null_fd_ref <= 0) return;

  --libfileio_scdc_fd.null_fd_ref;

  if (libfileio_scdc_fd.null_fd_ref == 0)
  {
    ORIG_F_INIT(close);
    ORIG_F(close)(libfileio_scdc_fd.null_fd);
    libfileio_scdc_fd.null_fd = -1;
  }
}

#endif /* LIBFILEIO_SCDC_FD_NULL */


static int libfileio_scdc_fd_alloc(int flags)
{
  int i;

  for (i = libfileio_scdc_fd.next_fd - LIBFILEIO_SCDC_FD_OFFSET; i < LIBFILEIO_SCDC_FD_SIZE; ++i)
  {
    if (libfileio_scdc_fd.ios[i] == NULL) break;
  }

  if (i >= LIBFILEIO_SCDC_FD_SIZE) return -1;

  int fd = LIBFILEIO_SCDC_FD_OFFSET + i;

  libfileio_scdc_fd.next_fd = fd + 1;

#if LIBFILEIO_SCDC_FD_NULL
  fd = libfileio_scdc_fd_null_open(fd, flags);
#endif /* LIBFILEIO_SCDC_FD_NULL */

  return fd;
}


static void libfileio_scdc_fd_free(int fd)
{
#if LIBFILEIO_SCDC_FD_NULL
  libfileio_scdc_fd_null_close(fd);
#endif /* LIBFILEIO_SCDC_FD_NULL */

  libfileio_scdc_fd.ios[fd - LIBFILEIO_SCDC_FD_OFFSET] = NULL;

  libfileio_scdc_fd.next_fd = z_min(fd, libfileio_scdc_fd.next_fd);
}


static libfileio_scdc_t *libfileio_scdc_fd_get(int fd)
{
  return libfileio_scdc_fd.ios[fd - LIBFILEIO_SCDC_FD_OFFSET];
}


static void libfileio_scdc_fd_set(int fd, libfileio_scdc_t *lfio)
{
  lfio->fd = fd;
  libfileio_scdc_fd.ios[fd - LIBFILEIO_SCDC_FD_OFFSET] = lfio;
}


#if 0
static int libfileio_scdc_fd_find(fileio_scdc_t *io)
{
  return io->fd;
}
#endif


#endif /* LIBFILEIO_SCDC_ENABLED */


static int libfileio_scdc_initialized = -1;


static void libfileio_scdc_init()
{
  if (libfileio_scdc_initialized < 0)
  {
    /* on first init */
    ORIG_INIT();

#if LIBFILEIO_SCDC_ENABLED
    libfileio_scdc_fd.next_fd = LIBFILEIO_SCDC_FD_OFFSET;
    int i;
    for (i = 0; i < LIBFILEIO_SCDC_FD_SIZE; ++i) libfileio_scdc_fd.ios[i] = NULL;
#endif

    fileio_scdc_local(getenv("LIBFILEIO_SCDC_LOCAL_BASE"), getenv("LIBFILEIO_SCDC_LOCAL_PATH"));

    libfileio_scdc_initialized = 0;
  }

  if (libfileio_scdc_initialized == 0)
  {
    /* on every init */
  }

  ++libfileio_scdc_initialized;
}


static void libfileio_scdc_release()
{
  if (libfileio_scdc_initialized <= 0) return;

  --libfileio_scdc_initialized;

  if (libfileio_scdc_initialized == 0)
  {
    /* on every release */
  }
}


#define LIBFILEIO_SCDC_INIT()     libfileio_scdc_init()
#define LIBFILEIO_SCDC_RELEASE()  libfileio_scdc_release()


#if LIBFILEIO_SCDC_ENABLED

static const char *libfileio_scdc_path_mod(const char *path, char *mod)
{
  /* FIXME */
#if HAVE_REGEX_H
  const char *path_regex = getenv("LIBFILEIO_SCDC_PATH_REGEX");
#endif
#if HAVE_PCRE_H
  const char *path_pcre = getenv("LIBFILEIO_SCDC_PATH_PCRE");
#endif

  const char *path_match = getenv("LIBFILEIO_SCDC_PATH_MATCH");
  const char *path_unprefix = getenv("LIBFILEIO_SCDC_PATH_UNPREFIX");
  const char *path_prefix = getenv("LIBFILEIO_SCDC_PATH_PREFIX");

  int n = LIBFILEIO_SCDC_PATH_MAX;
  char *m = mod;

  *m = '\0';

  size_t path_match_len = (path_match)?strlen(path_match):0;
  if (strncmp(path, path_match, path_match_len) == 0)
  {
    size_t path_unprefix_len = (path_unprefix)?strlen(path_unprefix):0;
    if (strncmp(path, path_unprefix, path_unprefix_len) == 0) path += path_unprefix_len;

    if (path_prefix)
    {
      int s = snprintf(m, n, path_prefix);
      n -= s;
      m += s;
    }
  }

  int s = snprintf(m, n, path);
  n -= s;

  return (s > 0)?mod:NULL;
}

#endif /* LIBFILEIO_SCDC_ENABLED */

#endif /* LIBFILEIO_SCDC_PROVIDE */


#if LIBFILEIO_SCDC_PROVIDE_FD

#define XXSTR(_xx_)  (((_xx_) == 64)?"64":"")

static int openXX(int XX, const char *pathname, int flags, mode_t mode)
{
  TRACE_F_CTRL_INTRO(LIBF_F_STR(open) "%s: pathname: '%s', flags: %d", XXSTR(XX), pathname, flags);

  LIBFILEIO_SCDC_INIT();

  int fd = -1;

#if LIBFILEIO_SCDC_ENABLED
  char path_mod[LIBFILEIO_SCDC_PATH_MAX];
  const char *p = libfileio_scdc_path_mod(pathname, path_mod);

  TRACE_F_CTRL_BODY(LIBF_F_STR(open) "%s: modified pathname: '%s'", XXSTR(XX), p);

  if (fileio_scdc_valid(p))
  {
    TRACE_F_CTRL_SCDC(LIBF_F_STR(open) "%s: pathname: '%s', flags: %d", XXSTR(XX), p, flags);

    libfileio_scdc_t *lfio = libfileio_scdc_create();

    TRACE_F_CTRL_SCDC(LIBF_F_STR(open) "%s: lfio: %" dptr_fmt, XXSTR(XX), DPTR(lfio));

    int err;

    if (fileio_scdc_open(&lfio->fio, p, flags, &err) == FILEIO_SCDC_SUCCESS)
    {
      fd = libfileio_scdc_fd_alloc(flags);

      if (fd >= 0)
      {
        libfileio_scdc_fd_set(fd, lfio);

      } else
      {
        err = -1;
        fileio_scdc_close(&lfio->fio);
      }
    }

    if (err)
    {
      libfileio_scdc_destroy(lfio);

      fd = -1;
      errno = err;
    }

    TRACE_F_CTRL_SCDC(LIBF_F_STR(open) "%s: return: %d", XXSTR(XX), fd);

    LIBFILEIO_SCDC_STATS_SCDC_CMD(LIBFILEIO_SCDC_STATS_ADD(fd, pathname););

  } else
#endif
  {
    TRACE_F_CTRL_ORIG(LIBF_F_STR(open) "%s: pathname: '%s', flags: %d", XXSTR(XX), pathname, flags);

    if (XX == 64)
    {
      ORIG_F_INIT(open64);
      fd = ORIG_F(open64)(pathname, flags, mode);

    } else
    {
      ORIG_F_INIT(open);
      fd = ORIG_F(open)(pathname, flags, mode);
    }

    TRACE_F_CTRL_ORIG(LIBF_F_STR(open) "%s: return: %d", XXSTR(XX), fd);

    LIBFILEIO_SCDC_STATS_ORIG_CMD(LIBFILEIO_SCDC_STATS_ADD(fd, pathname););
  }

  LIBFILEIO_SCDC_RELEASE();

  TRACE_F_CTRL_OUTRO(LIBF_F_STR(open) "%s: return: %d", XXSTR(XX), fd);

  return fd;
}


int LIBF_F(open)(const char *pathname, int flags, ...)
{
  va_list argp;
  va_start(argp, flags);
  mode_t mode = va_arg(argp, mode_t);
  va_end(argp);

  return openXX(0, pathname, flags, mode);
}


static int creatXX(int XX, const char *pathname, mode_t mode)
{
  TRACE_F_CTRL_INTRO(LIBF_F_STR(creat) "%s: pathname: '%s', mode: %" duint_fmt, XXSTR(XX), pathname, DUINT(mode));

  LIBFILEIO_SCDC_INIT();

  int fd = -1;

#if LIBFILEIO_SCDC_ENABLED
  char path_mod[LIBFILEIO_SCDC_PATH_MAX];
  const char *p = libfileio_scdc_path_mod(pathname, path_mod);

  TRACE_F_CTRL_BODY(LIBF_F_STR(open) "%s: modified pathname: '%s'", XXSTR(XX), p);

  if (fileio_scdc_valid(p))
  {
    TRACE_F_CTRL_SCDC(LIBF_F_STR(creat) "%s: pathname: '%s', mode: %" duint_fmt, XXSTR(XX), p, DUINT(mode));

    libfileio_scdc_t *lfio = libfileio_scdc_create();

    int flags = O_CREAT|O_WRONLY|O_TRUNC;
    int err = 0;

    TRACE_F_CTRL_SCDC(LIBF_F_STR(creat) "%s: lfio: %" dptr_fmt ", flags: %d", XXSTR(XX), DPTR(lfio), flags);

    if (fileio_scdc_open(&lfio->fio, p, flags, &err) == FILEIO_SCDC_SUCCESS)
    {
      fd = libfileio_scdc_fd_alloc(flags);

      if (fd >= 0)
      {
        libfileio_scdc_fd_set(fd, lfio);

      } else
      {
        err = -1;
        fileio_scdc_close(&lfio->fio);
      }
    }

    if (err)
    {
      libfileio_scdc_destroy(lfio);

      fd = err;
    }

    TRACE_F_CTRL_SCDC(LIBF_F_STR(creat) "%s: return: %d", XXSTR(XX), fd);

    LIBFILEIO_SCDC_STATS_SCDC_CMD(LIBFILEIO_SCDC_STATS_ADD(fd, pathname););

  } else
#endif
  {
    TRACE_F_CTRL_ORIG(LIBF_F_STR(creat) "%s: pathname: '%s', mode: %" duint_fmt, XXSTR(XX), pathname, DUINT(mode));

    if (XX == 64)
    {
      ORIG_F_INIT(creat64);
      fd = ORIG_F(creat64)(pathname, mode);

    } else
    {
      ORIG_F_INIT(creat);
      fd = ORIG_F(creat)(pathname, mode);
    }

    TRACE_F_CTRL_ORIG(LIBF_F_STR(creat) "%s: return: %d", XXSTR(XX), fd);

    LIBFILEIO_SCDC_STATS_ORIG_CMD(LIBFILEIO_SCDC_STATS_ADD(fd, pathname););
  }

  LIBFILEIO_SCDC_RELEASE();

  TRACE_F_CTRL_OUTRO(LIBF_F_STR(creat) "%s: return: %d", XXSTR(XX), fd);

  return fd;
}


int LIBF_F(creat)(const char *pathname, mode_t mode)
{
  return creatXX(0, pathname, mode);
}


int LIBF_F(close)(int fd)
{
  TRACE_F_CTRL_INTRO(LIBF_F_STR(close) ": fd: %d", fd);

  LIBFILEIO_SCDC_INIT();

  int ret = -1;

#if LIBFILEIO_SCDC_ENABLED
  if (LIBFILEIO_SCDC_FD_VALID(fd))
  {
    LIBFILEIO_SCDC_STATS_SCDC_CMD(
      LIBFILEIO_SCDC_STATS_INFO(fd);
      LIBFILEIO_SCDC_STATS_DEL(fd);
    );

    libfileio_scdc_t *lfio = libfileio_scdc_fd_get(fd);

    TRACE_F_CTRL_SCDC(LIBF_F_STR(close) ": lfio: %" dptr_fmt ", fd: %d", DPTR(lfio), fd);

    if (lfio)
    {
      libfileio_scdc_fd_free(lfio->fd);

      ret = fileio_scdc_close(&lfio->fio);

      libfileio_scdc_destroy(lfio);
    }

    TRACE_F_CTRL_SCDC(LIBF_F_STR(close) ": return: %d", ret);

  } else
#endif
  {
    LIBFILEIO_SCDC_STATS_ORIG_CMD(
      LIBFILEIO_SCDC_STATS_INFO(fd);
      LIBFILEIO_SCDC_STATS_DEL(fd);
    );

    TRACE_F_CTRL_ORIG(LIBF_F_STR(close) ": fd: %d", fd);

    ORIG_F_INIT(close);
    ret = ORIG_F(close)(fd);

    TRACE_F_CTRL_ORIG(LIBF_F_STR(close) ": return: %d", ret);
  }

  LIBFILEIO_SCDC_RELEASE();

  TRACE_F_CTRL_OUTRO(LIBF_F_STR(close) ": return: %d", ret);

  return ret;
}


ssize_t LIBF_F(read)(int fd, void *buf, size_t count)
{
  TRACE_F_RD_INTRO(LIBF_F_STR(read) ": fd: %d, buf: %" dptr_fmt ", count: %zu", fd, DPTR(buf), count);

  LIBFILEIO_SCDC_INIT();

  ssize_t ret = -1;

#if LIBFILEIO_SCDC_ENABLED
  if (LIBFILEIO_SCDC_FD_VALID(fd))
  {
    LIBFILEIO_SCDC_STATS_SCDC_CMD(LIBFILEIO_SCDC_STATS_TIME(tstat););

    libfileio_scdc_t *lfio = libfileio_scdc_fd_get(fd);

    TRACE_F_RD_SCDC(LIBF_F_STR(read) ": lfio: %" dptr_fmt ", fd: %d, buf: %" dptr_fmt ", count: %zu", DPTR(lfio), fd, DPTR(buf), count);

    if (lfio)
    {
      ret = fileio_scdc_read(&lfio->fio, buf, count, 0);

      if (ret < 0) lfio->err = 1;
    }

    TRACE_F_RD_SCDC(LIBF_F_STR(read) ": return: %zd", ret);

    LIBFILEIO_SCDC_STATS_SCDC_CMD(
      if (ret >= 0) LIBFILEIO_SCDC_STATS_NREAD(fd, ret, tstat);
    );

  } else
#endif
  {
    LIBFILEIO_SCDC_STATS_ORIG_CMD(LIBFILEIO_SCDC_STATS_TIME(tstat););

    TRACE_F_RD_ORIG(LIBF_F_STR(read) ": fd: %d, buf: %" dptr_fmt ", count: %zu", fd, DPTR(buf), count);

    ORIG_F_INIT(read);
    ret = ORIG_F(read)(fd, buf, count);

    TRACE_F_RD_ORIG(LIBF_F_STR(read) ": return: %zd", ret);

    LIBFILEIO_SCDC_STATS_ORIG_CMD(
      if (ret >= 0) LIBFILEIO_SCDC_STATS_NREAD(fd, ret, tstat);
    );
  }

  LIBFILEIO_SCDC_RELEASE();

  TRACE_F_RD_OUTRO(LIBF_F_STR(read) ": return: %zd", ret);

  return ret;
}


ssize_t LIBF_F(write)(int fd, const void *buf, size_t count)
{
  TRACE_F_WR_INTRO(LIBF_F_STR(write) ": fd: %d, buf: %" dptr_fmt ", count: %zu", fd, DPTR(buf), count);

  LIBFILEIO_SCDC_INIT();

  ssize_t ret = -1;

#if LIBFILEIO_SCDC_ENABLED
  if (LIBFILEIO_SCDC_FD_VALID(fd))
  {
    LIBFILEIO_SCDC_STATS_SCDC_CMD(LIBFILEIO_SCDC_STATS_TIME(tstat););

    libfileio_scdc_t *lfio = libfileio_scdc_fd_get(fd);

    TRACE_F_WR_SCDC(LIBF_F_STR(write) ": lfio: %" dptr_fmt ", fd: %d, buf: %" dptr_fmt ", count: %zu", DPTR(lfio), fd, DPTR(buf), count);

    if (lfio)
    {
      ret = fileio_scdc_write(&lfio->fio, buf, count);

      if (ret < 0) lfio->err = 1;
    }

    TRACE_F_WR_SCDC(LIBF_F_STR(write) ": return: %zd", ret);

    LIBFILEIO_SCDC_STATS_SCDC_CMD(
      if (ret >= 0) LIBFILEIO_SCDC_STATS_NWRITE(fd, ret, tstat);
    );

  } else
#endif
  {
    LIBFILEIO_SCDC_STATS_ORIG_CMD(LIBFILEIO_SCDC_STATS_TIME(tstat););

    TRACE_F_WR_ORIG(LIBF_F_STR(write) ": fd: %d, buf: %" dptr_fmt ", count: %zu", fd, DPTR(buf), count);

    ORIG_F_INIT(write);
    ret = ORIG_F(write)(fd, buf, count);

    TRACE_F_WR_ORIG(LIBF_F_STR(write) ": return: %zd", ret);

    LIBFILEIO_SCDC_STATS_ORIG_CMD(
      if (ret >= 0) LIBFILEIO_SCDC_STATS_NWRITE(fd, ret, tstat);
    );
  }

  LIBFILEIO_SCDC_RELEASE();

  TRACE_F_WR_OUTRO(LIBF_F_STR(write) ": return: %zd", ret);

  return ret;
}


int LIBF_F(fsync)(int fd)
{
  TRACE_F_CTRL_INTRO(LIBF_F_STR(fsync) ": fd: %d", fd);

  LIBFILEIO_SCDC_INIT();

  int ret = -1;

#if LIBFILEIO_SCDC_ENABLED
  if (LIBFILEIO_SCDC_FD_VALID(fd))
  {
    libfileio_scdc_t *lfio = libfileio_scdc_fd_get(fd);

    TRACE_F_CTRL_SCDC(LIBF_F_STR(fsync) ": lfio: %" dptr_fmt ", fd: %d", DPTR(lfio), fd);

    if (lfio)
    {
      ret = fileio_scdc_sync(&lfio->fio);
    }

    TRACE_F_CTRL_SCDC(LIBF_F_STR(fsync) ": return: %d", ret);

  } else
#endif
  {
    TRACE_F_CTRL_ORIG(LIBF_F_STR(fsync) ": fd: %d", fd);

    ORIG_F_INIT(fsync);
    ret = ORIG_F(fsync)(fd);

    TRACE_F_CTRL_ORIG(LIBF_F_STR(fsync) ": return: %d", ret);
  }

  LIBFILEIO_SCDC_RELEASE();

  TRACE_F_CTRL_OUTRO(LIBF_F_STR(fsync) ": return: %d", ret);

  return ret;
}


static dsint_t lseekXX(int XX, int fd, dsint_t offset, int whence)
{
  TRACE_F_CTRL_INTRO(LIBF_F_STR(lseek) "%s: fd: %d, offset: %" dsint_fmt ", whence: %d", XXSTR(XX), fd, offset, whence);

  LIBFILEIO_SCDC_INIT();

  dsint_t ret = -1;

#if LIBFILEIO_SCDC_ENABLED
  if (LIBFILEIO_SCDC_FD_VALID(fd))
  {
    libfileio_scdc_t *lfio = libfileio_scdc_fd_get(fd);

    TRACE_F_CTRL_SCDC(LIBF_F_STR(lseek) "%s: lfio: %" dptr_fmt ", fd: %d, offset: %" dsint_fmt ", whence: %d", XXSTR(XX), DPTR(lfio), fd, offset, whence);

    if (lfio)
    {
      ret = fileio_scdc_seek(&lfio->fio, offset, whence, 0);
    }

    TRACE_F_CTRL_SCDC(LIBF_F_STR(lseek) "%s: return: %" dsint_fmt, XXSTR(XX), ret);

  } else
#endif
  {
    TRACE_F_CTRL_ORIG(LIBF_F_STR(lseek) "%s: fd: %d, offset: %" dsint_fmt ", whence: %d", XXSTR(XX), fd, offset, whence);

    if (XX == 64)
    {
      ORIG_F_INIT(lseek64);
      ret = ORIG_F(lseek64)(fd, offset, whence);

    } else
    {
      ORIG_F_INIT(lseek);
      ret = ORIG_F(lseek)(fd, offset, whence);
    }

    TRACE_F_CTRL_ORIG(LIBF_F_STR(lseek) "%s: return: %" dsint_fmt, XXSTR(XX), ret);
  }

  LIBFILEIO_SCDC_RELEASE();

  TRACE_F_CTRL_OUTRO(LIBF_F_STR(lseek) "%s: return: %" dsint_fmt, XXSTR(XX), ret);

  return ret;
}


off_t LIBF_F(lseek)(int fd, off_t offset, int whence)
{
  return lseekXX(0, fd, offset, whence);
}


static dsint_t fstatXX(int XX, int fd, void *buf)
{
  TRACE_F_CTRL_INTRO(LIBF_F_STR(fstat) "%s: fd: %d, buf: %" dptr_fmt, XXSTR(XX), fd, DPTR(buf));

  LIBFILEIO_SCDC_INIT();

  dsint_t ret = -1;

#if LIBFILEIO_SCDC_ENABLED
  if (LIBFILEIO_SCDC_FD_VALID(fd))
  {
    libfileio_scdc_t *lfio = libfileio_scdc_fd_get(fd);

    TRACE_F_CTRL_SCDC(LIBF_F_STR(fstat) "%s: lfio: %" dptr_fmt ", fd: %d, buf: %" dptr_fmt, XXSTR(XX), DPTR(lfio), fd, DPTR(buf));

    int err = 0;

    if (lfio)
    {
      fileio_scdc_stat_t fio_stat;

      if (fileio_scdc_stat(&lfio->fio, &fio_stat, &err) == FILEIO_SCDC_SUCCESS)
      {
        ret = 0;

        if (XX == 64)
        {
          struct stat64 *buf_stat = (struct stat64 *) buf;

          memset(buf_stat, 0, sizeof(struct stat));

          buf_stat->st_size = fio_stat.size;

        } else
        {
          struct stat *buf_stat = (struct stat *) buf;

          memset(buf_stat, 0, sizeof(struct stat));

          buf_stat->st_size = fio_stat.size;
        }

      } else
      {
        errno = err;
      }
    }

    TRACE_F_CTRL_SCDC(LIBF_F_STR(fstat) "%s: return: %" dsint_fmt, XXSTR(XX), ret);

  } else
#endif
  {
    TRACE_F_CTRL_ORIG(LIBF_F_STR(fstat) "%s: fd: %d, buf: %" dptr_fmt, XXSTR(XX), fd, DPTR(buf));

    if (XX == 64)
    {
      ORIG_F_INIT(fstat64);
      if (ORIG_F(fstat64)) ret = ORIG_F(fstat64)(fd, buf);
      else ret = __fxstat64(_STAT_VER, fd, buf);

    } else
    {
      ORIG_F_INIT(fstat);
      if (ORIG_F(fstat)) ret = ORIG_F(fstat)(fd, buf);
      else ret = __fxstat(_STAT_VER, fd, buf);
    }

    TRACE_F_CTRL_ORIG(LIBF_F_STR(fstat) "%s: return: %" dsint_fmt, XXSTR(XX), ret);
  }

  LIBFILEIO_SCDC_RELEASE();

  TRACE_F_CTRL_OUTRO(LIBF_F_STR(fstat) "%s: return: %" dsint_fmt, XXSTR(XX), ret);

  return ret;
}


int LIBF_F(fstat)(int fd, struct stat *buf)
{
  return fstatXX(0, fd, buf);
}


#ifdef LIBFILEIO_SCDC_PROVIDE_64

int LIBF_F(open64)(const char *pathname, int flags, ...)
{
  va_list argp;
  va_start(argp, flags);
  mode_t mode = va_arg(argp, mode_t);
  va_end(argp);

  return openXX(64, pathname, flags, mode);
}


int LIBF_F(creat64)(const char *pathname, mode_t mode)
{
  return creatXX(64, pathname, mode);
}


off64_t LIBF_F(lseek64)(int fd, off64_t offset, int whence)
{
  return lseekXX(64, fd, offset, whence);
}


int LIBF_F(fstat64)(int fd, struct stat64 *buf)
{
  return fstatXX(64, fd, buf);
}

#undef XXSTR

#endif /* LIBFILEIO_SCDC_PROVIDE_64 */

#endif /* LIBFILEIO_SCDC_PROVIDE_FILE */


#if LIBFILEIO_SCDC_PROVIDE_FILE

#if LIBFILEIO_SCDC_ENABLED

static int file_mode_to_flags(const char *mode)
{
  int flags = 0;
  int plus = 0;
/*  int binary = 0;*/

  int i;
  for (i = 1; i < strlen(mode); ++i)
  {
    if (mode[i] == '+') plus = 1;
/*    if (mode[i] == 'b') binary = 1;*/
  }

  switch (mode[0])
  {
    case 'r':
      flags |= (plus)?O_RDWR:O_RDONLY;
      break;
    case 'w':
      flags |= O_CREAT|O_TRUNC;
      flags |= (plus)?O_RDWR:O_WRONLY;
      break;
    case 'a':
      flags |= O_APPEND;
      flags |= (plus)?O_RDWR:O_WRONLY;
      break;
  }

  return flags;
}

#endif /* LIBFILEIO_SCDC_ENABLED */


#if LIBFILEIO_SCDC_STATS_ORIG || LIBFILEIO_SCDC_STATS_SCDC

static int file_to_fd(FILE *file)
{
  if (!file) return -1;

  int fd = -1;

#if LIBFILEIO_SCDC_ENABLED
  if (LIBFILEIO_SCDC_VALID(file))
  {
    libfileio_scdc_t *lfio = (libfileio_scdc_t *) file;

    fd = lfio->fd;

  } else
#endif
  {
    ORIG_F_INIT(fileno);
    fd = ORIG_F(fileno)(file);
  }

  return fd;
}

#endif /* LIBFILEIO_SCDC_STATS */


FILE *LIBF_F(fopen)(const char *path, const char *mode)
{
  TRACE_F_CTRL_INTRO(LIBF_F_STR(fopen) ": path: '%s', mode: '%s'", path, mode);

  LIBFILEIO_SCDC_INIT();

  FILE *file = NULL;

#if LIBFILEIO_SCDC_ENABLED
  char path_mod[LIBFILEIO_SCDC_PATH_MAX];
  const char *p = libfileio_scdc_path_mod(path, path_mod);

  TRACE_F_CTRL_BODY(LIBF_F_STR(fopen) ": modified pathname: '%s'", p);

  if (fileio_scdc_valid(p))
  {
    TRACE_F_CTRL_SCDC(LIBF_F_STR(fopen) ": path: '%s', mode: '%s'", p, mode);

    libfileio_scdc_t *lfio = libfileio_scdc_create();

    int flags = file_mode_to_flags(mode);
    int err = 0;

    TRACE_F_CTRL_SCDC(LIBF_F_STR(fopen) ": lfio: %" dptr_fmt ", flags: %d", DPTR(lfio), flags);

    if (fileio_scdc_open(&lfio->fio, p, flags, &err) == FILEIO_SCDC_SUCCESS)
    {
      int fd = libfileio_scdc_fd_alloc(flags);

      if (fd >= 0)
      {
        libfileio_scdc_fd_set(fd, lfio);
        file = (FILE *) lfio;

      } else
      {
        err = -1;
        fileio_scdc_close(&lfio->fio);
      }
    }

    if (err)
    {
      libfileio_scdc_destroy(lfio);
    }

    TRACE_F_CTRL_SCDC(LIBF_F_STR(fopen) ": return: %" dptr_fmt, DPTR(file));

    LIBFILEIO_SCDC_STATS_SCDC_CMD(LIBFILEIO_SCDC_STATS_ADD(file_to_fd(file), path););

  } else
#endif
  {
    TRACE_F_CTRL_ORIG(LIBF_F_STR(fopen) ": path: '%s', mode: '%s'", path, mode);

    ORIG_F_INIT(fopen);
    file = ORIG_F(fopen)(path, mode);

    if (!file) TRACE_F_CTRL_ORIG_FAIL(LIBF_F_STR(fopen) ": '%s'", strerror(errno));

    TRACE_F_CTRL_ORIG(LIBF_F_STR(fopen) ": return: %" dptr_fmt, DPTR(file));

    LIBFILEIO_SCDC_STATS_ORIG_CMD(LIBFILEIO_SCDC_STATS_ADD(file_to_fd(file), path););
  }

  LIBFILEIO_SCDC_RELEASE();

  TRACE_F_CTRL_OUTRO(LIBF_F_STR(fopen) ": return: %" dptr_fmt, DPTR(file));

  return file;
}


FILE *LIBF_F(fdopen)(int fd, const char *mode)
{
  TRACE_F_CTRL_INTRO(LIBF_F_STR(fdopen) ": fd: %d, mode: '%s'", fd, mode);

  LIBFILEIO_SCDC_INIT();

  FILE *file = NULL;

#if LIBFILEIO_SCDC_ENABLED
  if (LIBFILEIO_SCDC_FD_VALID(fd))
  {
    TRACE_F_CTRL_SCDC(LIBF_F_STR(fdopen) ": fd: %d, mode: '%s'", fd, mode);

    file = (FILE *) libfileio_scdc_fd_get(fd);

    TRACE_F_CTRL_SCDC(LIBF_F_STR(fdopen) ": return: %" dptr_fmt, DPTR(file));

  } else
#endif
  {
    TRACE_F_CTRL_ORIG(LIBF_F_STR(fdopen) ": fd: %d, mode: '%s'", fd, mode);

    ORIG_F_INIT(fdopen);
    file = ORIG_F(fdopen)(fd, mode);

    if (!file) TRACE_F_CTRL_ORIG_FAIL(LIBF_F_STR(fdopen) ": '%s'", strerror(errno));

    TRACE_F_CTRL_ORIG(LIBF_F_STR(fdopen) ": return: %" dptr_fmt, DPTR(file));
  }

  LIBFILEIO_SCDC_RELEASE();

  TRACE_F_CTRL_OUTRO(LIBF_F_STR(fdopen) ": return: %" dptr_fmt, DPTR(file));

  return file;
}


FILE *LIBF_F(freopen)(const char *path, const char *mode, FILE *stream)
{
  TRACE_F_CTRL_INTRO(LIBF_F_STR(freopen) ": path: '%s', mode: '%s', stream: %" dptr_fmt, path, mode, DPTR(stream));

  LIBFILEIO_SCDC_INIT();

  /* only supported if (scdc-path and not scdc-stream) or (not scdc-path and scdc-stream) */

  FILE *file = NULL;

#if LIBFILEIO_SCDC_ENABLED
  if (LIBFILEIO_SCDC_VALID(stream))
  {
    LIBFILEIO_SCDC_STATS_SCDC_CMD(
      LIBFILEIO_SCDC_STATS_INFO(file_to_fd(stream));
      LIBFILEIO_SCDC_STATS_DEL(file_to_fd(stream));
    );

    char path_mod[LIBFILEIO_SCDC_PATH_MAX];
    const char *p = libfileio_scdc_path_mod(path, path_mod);

    TRACE_F_CTRL_SCDC(LIBF_F_STR(freopen) ": modified pathname: '%s'", p);

    libfileio_scdc_t *lfio = (libfileio_scdc_t *) stream;

    TRACE_F_CTRL_SCDC(LIBF_F_STR(freopen) ": lfio: %" dptr_fmt, DPTR(lfio));

    if (fileio_scdc_valid(p))
    {
      TRACE_F_CTRL_SCDC(LIBF_F_STR(freopen) ": path: '%s', mode: '%s'", p, mode);

      int flags = file_mode_to_flags(mode);
      int fd = lfio->fd;
      int err = 0;

      TRACE_F_CTRL_SCDC(LIBF_F_STR(freopen) ": flags: %d, fd: %d", flags, fd);

      if (fileio_scdc_reopen(&lfio->fio, p, flags, &err) == FILEIO_SCDC_SUCCESS)
      {
        file = (FILE *) lfio;

      } else
      {
        TRACE_F_CTRL_SCDC_FAIL(LIBF_F_STR(freopen) ": fileio_scdc_reopen failed with err: %d", err);

        libfileio_scdc_fd_free(fd);
        libfileio_scdc_destroy(lfio);
      }

    } else
    {
      TRACE_F_CTRL_SCDC_FAIL(LIBF_F_STR(freopen) ": scdc stream and non-scdc path not supported");
    }

    TRACE_F_CTRL_OUTRO(LIBF_F_STR(freopen) ": return: %" dptr_fmt, DPTR(file));

    LIBFILEIO_SCDC_STATS_SCDC_CMD(LIBFILEIO_SCDC_STATS_ADD(file_to_fd(file), path););

  } else
#endif
  {
    LIBFILEIO_SCDC_STATS_ORIG_CMD(
      LIBFILEIO_SCDC_STATS_INFO(file_to_fd(stream));
      LIBFILEIO_SCDC_STATS_DEL(file_to_fd(stream));
    );

    TRACE_F_CTRL_ORIG(LIBF_F_STR(freopen) ": path: '%s', mode: '%s', stream: %" dptr_fmt, path, mode, DPTR(stream));

    if (!fileio_scdc_valid(path))
    {
      ORIG_F_INIT(freopen);
      file = ORIG_F(freopen)(path, mode, stream);

      if (!file) TRACE_F_CTRL_ORIG_FAIL(LIBF_F_STR(freopen) ": '%s'", strerror(errno));

    } else
    {
      TRACE_F_CTRL_ORIG_FAIL(LIBF_F_STR(freopen) ": file stream and scdc path not supported");
    }

    TRACE_F_CTRL_ORIG(LIBF_F_STR(freopen) ": return: %" dptr_fmt, DPTR(file));

    LIBFILEIO_SCDC_STATS_ORIG_CMD(LIBFILEIO_SCDC_STATS_ADD(file_to_fd(file), path););
  }

  LIBFILEIO_SCDC_RELEASE();

  TRACE_F_CTRL_OUTRO(LIBF_F_STR(freopen) ": return: %" dptr_fmt, DPTR(file));

  return file;
}


int LIBF_F(fclose)(FILE *stream)
{
  TRACE_F_CTRL_INTRO(LIBF_F_STR(fclose) ": stream: %" dptr_fmt, DPTR(stream));

  LIBFILEIO_SCDC_INIT();

  int ret = -1;

#if LIBFILEIO_SCDC_ENABLED
  if (LIBFILEIO_SCDC_VALID(stream))
  {
    LIBFILEIO_SCDC_STATS_SCDC_CMD(
      LIBFILEIO_SCDC_STATS_INFO(file_to_fd(stream));
      LIBFILEIO_SCDC_STATS_DEL(file_to_fd(stream));
    );

    libfileio_scdc_t *lfio = (libfileio_scdc_t *) stream;

    TRACE_F_CTRL_SCDC(LIBF_F_STR(fclose) ": lfio: %" dptr_fmt, DPTR(lfio));

    libfileio_scdc_fd_free(lfio->fd);

    ret = fileio_scdc_close(&lfio->fio);

    libfileio_scdc_destroy(lfio);

    TRACE_F_CTRL_SCDC(LIBF_F_STR(fclose) ": return: %d", ret);

  } else
#endif
  {
    LIBFILEIO_SCDC_STATS_ORIG_CMD(
      LIBFILEIO_SCDC_STATS_INFO(file_to_fd(stream));
      LIBFILEIO_SCDC_STATS_DEL(file_to_fd(stream));
    );

    TRACE_F_CTRL_ORIG(LIBF_F_STR(fclose) ": stream: %" dptr_fmt, DPTR(stream));

    ORIG_F_INIT(fclose);
    ret = ORIG_F(fclose)(stream);

    TRACE_F_CTRL_ORIG(LIBF_F_STR(fclose) ": return: %d", ret);
  }

  LIBFILEIO_SCDC_RELEASE();

  TRACE_F_CTRL_OUTRO(LIBF_F_STR(fclose) ": return: %d", ret);

  return ret;
}


size_t LIBF_F(fread)(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
  TRACE_F_RD_INTRO(LIBF_F_STR(fread) ": ptr: %" dptr_fmt ", size: %zu, nmemb: %zu, stream: %" dptr_fmt, DPTR(ptr), size, nmemb, DPTR(stream));

  LIBFILEIO_SCDC_INIT();

  size_t ret = 0;

#if LIBFILEIO_SCDC_ENABLED
  if (LIBFILEIO_SCDC_VALID(stream))
  {
    LIBFILEIO_SCDC_STATS_SCDC_CMD(LIBFILEIO_SCDC_STATS_TIME(tstat););

    libfileio_scdc_t *lfio = (libfileio_scdc_t *) stream;

    TRACE_F_RD_SCDC(LIBF_F_STR(fread) ": lfio: %" dptr_fmt ", ptr: %" dptr_fmt ", size: %zu, nmemb: %zu", DPTR(lfio), DPTR(ptr), size, nmemb);

    scdcint_t r = fileio_scdc_read(&lfio->fio, ptr, size * nmemb, 0);

    if (r < 0) lfio->err = 1;
    else
    {
      ret = r / size;
      ssize_t seek = ret * size - r;
      if (seek != 0) fileio_scdc_seek(&lfio->fio, seek, SEEK_CUR, 0);
    }

    TRACE_F_RD_SCDC(LIBF_F_STR(fread) ": return: %zu", ret);

    LIBFILEIO_SCDC_STATS_SCDC_CMD(LIBFILEIO_SCDC_STATS_NREAD(file_to_fd(stream), size * ret, tstat););

  } else
#endif
  {
    LIBFILEIO_SCDC_STATS_ORIG_CMD(LIBFILEIO_SCDC_STATS_TIME(tstat););

    TRACE_F_RD_ORIG(LIBF_F_STR(fread) ": ptr: %" dptr_fmt ", size: %zu, nmemb: %zu, stream: %" dptr_fmt, DPTR(ptr), size, nmemb, DPTR(stream));

    ORIG_F_INIT(fread);
    ret = ORIG_F(fread)(ptr, size, nmemb, stream);

    TRACE_F_RD_ORIG(LIBF_F_STR(fread) ": return: %zu", ret);

    LIBFILEIO_SCDC_STATS_ORIG_CMD(LIBFILEIO_SCDC_STATS_NREAD(file_to_fd(stream), size * ret, tstat););
  }

  LIBFILEIO_SCDC_RELEASE();
 
  TRACE_F_RD_OUTRO(LIBF_F_STR(fread) ": return: %zu", ret);

  return ret;
}


size_t LIBF_F(fwrite)(const void *ptr, size_t size, size_t nmemb, FILE *stream)
{
  TRACE_F_WR_INTRO(LIBF_F_STR(fwrite) ": ptr: %" dptr_fmt ", size: %zu, nmemb: %zu, stream: %" dptr_fmt, DPTR(ptr), size, nmemb, DPTR(stream));

  LIBFILEIO_SCDC_INIT();

  size_t ret = 0;

#if LIBFILEIO_SCDC_ENABLED
  if (LIBFILEIO_SCDC_VALID(stream))
  {
    LIBFILEIO_SCDC_STATS_SCDC_CMD(LIBFILEIO_SCDC_STATS_TIME(tstat););

    libfileio_scdc_t *lfio = (libfileio_scdc_t *) stream;

    TRACE_F_WR_SCDC(LIBF_F_STR(fwrite) ": lfio: %" dptr_fmt ", ptr: %" dptr_fmt ", size: %zu, nmemb: %zu", DPTR(lfio), DPTR(ptr), size, nmemb);

    scdcint_t r = fileio_scdc_write(&lfio->fio, ptr, size * nmemb);

    if (r < 0) lfio->err = 1;
    else
    {
      ret = r / size;
      ssize_t seek = ret * size - r;
      if (seek != 0) fileio_scdc_seek(&lfio->fio, seek, SEEK_CUR, 1);
    }

    TRACE_F_WR_SCDC(LIBF_F_STR(fwrite) ": return: %zu", ret);

    LIBFILEIO_SCDC_STATS_SCDC_CMD(LIBFILEIO_SCDC_STATS_NWRITE(file_to_fd(stream), size * ret, tstat););

  } else
#endif
  {
    LIBFILEIO_SCDC_STATS_ORIG_CMD(LIBFILEIO_SCDC_STATS_TIME(tstat););

    TRACE_F_WR_ORIG(LIBF_F_STR(fwrite) ": ptr: %" dptr_fmt ", size: %zu, nmemb: %zu, stream: %" dptr_fmt, DPTR(ptr), size, nmemb, DPTR(stream));

    ORIG_F_INIT(fwrite);
    ret = ORIG_F(fwrite)(ptr, size, nmemb, stream);

    TRACE_F_WR_ORIG(LIBF_F_STR(fwrite) ": return: %zu", ret);

    LIBFILEIO_SCDC_STATS_ORIG_CMD(LIBFILEIO_SCDC_STATS_NWRITE(file_to_fd(stream), size * ret, tstat););
  }

  LIBFILEIO_SCDC_RELEASE();

  TRACE_F_WR_OUTRO(LIBF_F_STR(fwrite) ": return: %zu", ret);

  return ret;
}


int LIBF_F(fflush)(FILE *stream)
{
  TRACE_F_CTRL_INTRO(LIBF_F_STR(fflush) ": stream: %" dptr_fmt, DPTR(stream));

  LIBFILEIO_SCDC_INIT();

  int ret = -1;

#if LIBFILEIO_SCDC_ENABLED
  if (stream && LIBFILEIO_SCDC_VALID(stream))
  {
    libfileio_scdc_t *lfio = (libfileio_scdc_t *) stream;

    TRACE_F_CTRL_SCDC(LIBF_F_STR(fflush) ": lfio: %" dptr_fmt, DPTR(lfio));

    ret = fileio_scdc_sync(&lfio->fio);

    TRACE_F_CTRL_SCDC(LIBF_F_STR(fflush) ": return: %d", ret);

  } else
#endif
  {
    TRACE_F_CTRL_ORIG(LIBF_F_STR(fflush) ": stream: %" dptr_fmt, DPTR(stream));

    ORIG_F_INIT(fflush);
    ret = ORIG_F(fflush)(stream);

    TRACE_F_CTRL_ORIG(LIBF_F_STR(fflush) ": return: %d", ret);
  }

  LIBFILEIO_SCDC_RELEASE();

  TRACE_F_CTRL_OUTRO(LIBF_F_STR(fflush) ": return: %d", ret);

  return ret;
}


#define XXSTR(_xx_)  (((_xx_) == 0)?"o":(((_xx_) == 64)?"o64":""))

static int fseekXX(int XX, FILE *stream, dsint_t offset, int whence)
{
  TRACE_F_CTRL_INTRO(LIBF_F_STR(fseek) "%s: stream: %" dptr_fmt ", offset: %" dsint_fmt ", whence: %d", XXSTR(XX), DPTR(stream), offset, whence);

  LIBFILEIO_SCDC_INIT();

  int ret = -1;

#if LIBFILEIO_SCDC_ENABLED
  if (LIBFILEIO_SCDC_VALID(stream))
  {
    libfileio_scdc_t *lfio = (libfileio_scdc_t *) stream;

    TRACE_F_CTRL_SCDC(LIBF_F_STR(fseek) "%s: lfio: %" dptr_fmt ", offset: %" dsint_fmt ", whence: %d", XXSTR(XX), DPTR(lfio), offset, whence);

    if (fileio_scdc_seek(&lfio->fio, offset, whence, 0) != FILEIO_SCDC_FAILURE)
    {
      ret = 0;
    }

    TRACE_F_CTRL_SCDC(LIBF_F_STR(fseek) "%s: return: %d", XXSTR(XX), ret);

  } else
#endif
  {
    TRACE_F_CTRL_ORIG(LIBF_F_STR(fseek) "%s: stream: %" dptr_fmt ", offset: %" dsint_fmt ", whence: %d", XXSTR(XX), DPTR(stream), offset, whence);

    if (XX == 0)
    {
      ORIG_F_INIT(fseeko);
      ret = ORIG_F(fseeko)(stream, offset, whence);

    } else if (XX == 64)
    {
      ORIG_F_INIT(fseeko64);
      ret = ORIG_F(fseeko64)(stream, offset, whence);

    } else
    {
      ORIG_F_INIT(fseek);
      ret = ORIG_F(fseek)(stream, offset, whence);
    }

    TRACE_F_CTRL_ORIG(LIBF_F_STR(fseek) "%s: return: %d", XXSTR(XX), ret);
  }

  LIBFILEIO_SCDC_RELEASE();

  TRACE_F_CTRL_OUTRO(LIBF_F_STR(fseek) "%s: return: %d", XXSTR(XX), ret);

  return ret;
}


int LIBF_F(fseek)(FILE *stream, long offset, int whence)
{
  return fseekXX(-1, stream, offset, whence);
}


static dsint_t ftellXX(int XX, FILE *stream)
{
  TRACE_F_CTRL_INTRO(LIBF_F_STR(ftell) "%s: stream: %" dptr_fmt, XXSTR(XX), DPTR(stream));

  LIBFILEIO_SCDC_INIT();

  dsint_t ret = -1;

#if LIBFILEIO_SCDC_ENABLED
  if (LIBFILEIO_SCDC_VALID(stream))
  {
    libfileio_scdc_t *lfio = (libfileio_scdc_t *) stream;

    TRACE_F_CTRL_SCDC(LIBF_F_STR(ftell) "%s: lfio: %" dptr_fmt, XXSTR(XX), DPTR(lfio));

    ret = fileio_scdc_seek(&lfio->fio, 0, SEEK_CUR, 0);

    TRACE_F_CTRL_SCDC(LIBF_F_STR(ftell) "%s: return: %" dsint_fmt, XXSTR(XX), ret);

  } else
#endif
  {
    TRACE_F_CTRL_ORIG(LIBF_F_STR(ftell) "%s: stream: %" dptr_fmt, XXSTR(XX), DPTR(stream));

    if (XX == 0)
    {
      ORIG_F_INIT(ftello);
      ret = ORIG_F(ftello)(stream);

    } else if (XX == 64)
    {
      ORIG_F_INIT(ftello64);
      ret = ORIG_F(ftello64)(stream);

    } else
    {
      ORIG_F_INIT(ftell);
      ret = ORIG_F(ftell)(stream);
    }

    TRACE_F_CTRL_ORIG(LIBF_F_STR(ftell) "%s: return: %" dsint_fmt, XXSTR(XX), ret);
  }

  LIBFILEIO_SCDC_RELEASE();

  TRACE_F_CTRL_OUTRO(LIBF_F_STR(ftell) "%s: return: %" dsint_fmt, XXSTR(XX), ret);

  return ret;
}


long LIBF_F(ftell)(FILE *stream)
{
  return ftellXX(-1, stream);
}


int LIBF_F(fseeko)(FILE *stream, off_t offset, int whence)
{
  return fseekXX(0, stream, offset, whence);
}


off_t LIBF_F(ftello)(FILE *stream)
{
  return ftellXX(0, stream);
}

#undef XXSTR


void LIBF_F(rewind)(FILE *stream)
{
  TRACE_F_CTRL_INTRO(LIBF_F_STR(rewind) ": stream: %" dptr_fmt, DPTR(stream));

  LIBFILEIO_SCDC_INIT();

#if LIBFILEIO_SCDC_ENABLED
  if (LIBFILEIO_SCDC_VALID(stream))
  {
    libfileio_scdc_t *lfio = (libfileio_scdc_t *) stream;

    TRACE_F_CTRL_SCDC(LIBF_F_STR(rewind) ": lfio: %" dptr_fmt, DPTR(lfio));

    fileio_scdc_seek(&lfio->fio, 0, SEEK_SET, 0);

    TRACE_F_CTRL_SCDC(LIBF_F_STR(rewind) ": return");

  } else
#endif
  {
    TRACE_F_CTRL_ORIG(LIBF_F_STR(rewind) ": stream: %" dptr_fmt, DPTR(stream));

    ORIG_F_INIT(rewind);
    ORIG_F(rewind)(stream);

    TRACE_F_CTRL_ORIG(LIBF_F_STR(rewind) ": return");
  }

  LIBFILEIO_SCDC_RELEASE();

  TRACE_F_CTRL_OUTRO(LIBF_F_STR(rewind) ": return");
}


void LIBF_F(clearerr)(FILE *stream)
{
  TRACE_F_CTRL_INTRO(LIBF_F_STR(clearerr) ": stream: %" dptr_fmt, DPTR(stream));

  LIBFILEIO_SCDC_INIT();

#if LIBFILEIO_SCDC_ENABLED
  if (LIBFILEIO_SCDC_VALID(stream))
  {
    libfileio_scdc_t *lfio = (libfileio_scdc_t *) stream;

    TRACE_F_CTRL_SCDC(LIBF_F_STR(clearerr) ": lfio: %" dptr_fmt, DPTR(lfio));

    lfio->err = 0;

    TRACE_F_CTRL_SCDC(LIBF_F_STR(clearerr) ": return");

  } else
#endif
  {
    TRACE_F_CTRL_ORIG(LIBF_F_STR(clearerr) ": stream: %" dptr_fmt, DPTR(stream));

    ORIG_F_INIT(clearerr);
    ORIG_F(clearerr)(stream);

    TRACE_F_CTRL_ORIG(LIBF_F_STR(clearerr) ": return");
  }

  LIBFILEIO_SCDC_RELEASE();

  TRACE_F_CTRL_OUTRO(LIBF_F_STR(clearerr) ": return");
}


int LIBF_F(feof)(FILE *stream)
{
  TRACE_F_CTRL_INTRO(LIBF_F_STR(feof) ": stream: %" dptr_fmt, DPTR(stream));

  LIBFILEIO_SCDC_INIT();

  int ret = -1;

#if LIBFILEIO_SCDC_ENABLED
  if (LIBFILEIO_SCDC_VALID(stream))
  {
    libfileio_scdc_t *lfio = (libfileio_scdc_t *) stream;

    TRACE_F_CTRL_SCDC(LIBF_F_STR(feof) ": lfio: %" dptr_fmt, DPTR(lfio));

    ret = lfio->fio.eof;

    TRACE_F_CTRL_SCDC(LIBF_F_STR(feof) ": return: %d", ret);

  } else
#endif
  {
    TRACE_F_CTRL_ORIG(LIBF_F_STR(feof) ": stream: %" dptr_fmt, DPTR(stream));

    ORIG_F_INIT(feof);
    ret = ORIG_F(feof)(stream);

    TRACE_F_CTRL_ORIG(LIBF_F_STR(feof) ": return: %d", ret);
  }

  LIBFILEIO_SCDC_RELEASE();

  TRACE_F_CTRL_OUTRO(LIBF_F_STR(feof) ": return: %d", ret);

  return ret;
}


int LIBF_F(ferror)(FILE *stream)
{
  TRACE_F_CTRL_INTRO(LIBF_F_STR(ferror) ": stream: %" dptr_fmt, DPTR(stream));

  LIBFILEIO_SCDC_INIT();

  int ret = -1;

#if LIBFILEIO_SCDC_ENABLED
  if (LIBFILEIO_SCDC_VALID(stream))
  {
    libfileio_scdc_t *lfio = (libfileio_scdc_t *) stream;

    TRACE_F_CTRL_SCDC(LIBF_F_STR(ferror) ": lfio: %" dptr_fmt, DPTR(lfio));

    ret = lfio->err;

    TRACE_F_CTRL_SCDC(LIBF_F_STR(ferror) ": return: %d", ret);

  } else
#endif
  {
    TRACE_F_CTRL_ORIG(LIBF_F_STR(ferror) ": stream: %" dptr_fmt, DPTR(stream));

    ORIG_F_INIT(ferror);
    ret = ORIG_F(ferror)(stream);

    TRACE_F_CTRL_ORIG(LIBF_F_STR(ferror) ": return: %d", ret);
  }

  LIBFILEIO_SCDC_RELEASE();

  TRACE_F_CTRL_OUTRO(LIBF_F_STR(ferror) ": return: %d", ret);

  return ret;
}


int LIBF_F(fileno)(FILE *stream)
{
  TRACE_F_CTRL_INTRO(LIBF_F_STR(fileno) ": stream: %" dptr_fmt, DPTR(stream));

  LIBFILEIO_SCDC_INIT();

  int ret = -1;

#if LIBFILEIO_SCDC_ENABLED
  if (LIBFILEIO_SCDC_VALID(stream))
  {
    libfileio_scdc_t *lfio = (libfileio_scdc_t *) stream;

    TRACE_F_CTRL_SCDC(LIBF_F_STR(fileno) ": lfio: %" dptr_fmt, DPTR(lfio));

    ret = lfio->fd;

    TRACE_F_CTRL_SCDC(LIBF_F_STR(fileno) ": return: %d", ret);

  } else
#endif
  {
    TRACE_F_CTRL_ORIG(LIBF_F_STR(fileno) ": stream: %" dptr_fmt, DPTR(stream));

    ORIG_F_INIT(fileno);
    ret = ORIG_F(fileno)(stream);

    TRACE_F_CTRL_ORIG(LIBF_F_STR(fileno) ": return: %d", ret);
  }

  LIBFILEIO_SCDC_RELEASE();

  TRACE_F_CTRL_OUTRO(LIBF_F_STR(fileno) ": return: %d", ret);

  return ret;
}


#define XXSTR(_xx_)  ((XX == 1)?"f":"")

static int fgetcXX(int XX, FILE *stream)
{
  TRACE_F_RD_INTRO("%s" LIBF_F_STR(getc) ": stream: %" dptr_fmt, XXSTR(XX), DPTR(stream));

  LIBFILEIO_SCDC_INIT();

  int ret = EOF;

#if LIBFILEIO_SCDC_ENABLED
  if (LIBFILEIO_SCDC_VALID(stream))
  {
    LIBFILEIO_SCDC_STATS_SCDC_CMD(LIBFILEIO_SCDC_STATS_TIME(tstat););

    libfileio_scdc_t *lfio = (libfileio_scdc_t *) stream;

    TRACE_F_RD_SCDC("%s" LIBF_F_STR(getc) ": lfio: %" dptr_fmt, XXSTR(XX), DPTR(lfio));

    unsigned char c;

    int n = fileio_scdc_read(&lfio->fio, &c, 1, 0);

    if (n < 0) lfio->err = 1;

    if (n > 0) ret = c;

    TRACE_F_RD_SCDC("%s" LIBF_F_STR(getc) ": return: %d", XXSTR(XX), ret);

    LIBFILEIO_SCDC_STATS_SCDC_CMD(
      if (ret != EOF) LIBFILEIO_SCDC_STATS_NREAD(file_to_fd(stream), 1, tstat);
    );

  } else
#endif
  {
    LIBFILEIO_SCDC_STATS_ORIG_CMD(LIBFILEIO_SCDC_STATS_TIME(tstat););

    TRACE_F_RD_ORIG("%s" LIBF_F_STR(getc) ": stream: %" dptr_fmt, XXSTR(XX), DPTR(stream));

    if (XX == 1)
    {
      ORIG_F_INIT(fgetc);
      ret = ORIG_F(fgetc)(stream);

    }
#if LIBFILEIO_SCDC_PROVIDE_FILE_
      else
    {
      ORIG_F_INIT_(getc);
      ret = ORIG_F_(getc)(stream);
    }
#endif

    TRACE_F_RD_ORIG("%s" LIBF_F_STR(getc) ": return: %d", XXSTR(XX), ret);

    LIBFILEIO_SCDC_STATS_ORIG_CMD(
      if (ret != EOF) LIBFILEIO_SCDC_STATS_NREAD(file_to_fd(stream), 1, tstat);
    );
  }

  LIBFILEIO_SCDC_RELEASE();

  TRACE_F_RD_OUTRO("%s" LIBF_F_STR(getc) ": return: %d", XXSTR(XX), ret);

  return ret;
}


int LIBF_F(fgetc)(FILE *stream)
{
  return fgetcXX(1, stream);
}


#if LIBFILEIO_SCDC_PROVIDE_FILE_

int LIBF_F(getc)(FILE *stream)
{
  return fgetcXX(0, stream);
}

#endif


char *LIBF_F(fgets)(char *s, int size, FILE *stream)
{
  TRACE_F_RD_INTRO(LIBF_F_STR(fgets) ": s: %" dptr_fmt ", size: %d, stream: %" dptr_fmt, DPTR(s), size, DPTR(stream));

  LIBFILEIO_SCDC_INIT();

  char *ret = NULL;

#if LIBFILEIO_SCDC_ENABLED
  if (LIBFILEIO_SCDC_VALID(stream))
  {
    LIBFILEIO_SCDC_STATS_SCDC_CMD(LIBFILEIO_SCDC_STATS_TIME(tstat););

    libfileio_scdc_t *lfio = (libfileio_scdc_t *) stream;

    TRACE_F_RD_SCDC(LIBF_F_STR(fgets) ": lfio: %" dptr_fmt ", s: %" dptr_fmt ", size: %d", DPTR(lfio), DPTR(s), size);

    int nn = 0;
    int ii = 0;

    while (size - 1 - nn > 0 && ii >= nn && !lfio->fio.eof)
    {
      int n = fileio_scdc_read(&lfio->fio, s + ii,
#if LIBFILEIO_SCDC_FGETS_CHUNK_SIZE
        z_min(LIBFILEIO_SCDC_FGETS_CHUNK_SIZE, size - 1 - nn),
#else
        size - 1 - nn,
#endif
        1);

      if (n < 0)
      {
        nn = -1;
        break;
      }

      nn += n;

      for (; ii < nn; ++ii)
      if (s[ii] == '\n') break;
    }

    if (nn < 0) lfio->err = 1;
    else
    {
      if (ii < size)
      {
        ++ii;
        s[ii] = '\0';
        ret = s;
      }

      if (ii < nn) fileio_scdc_seek(&lfio->fio, ii - nn, SEEK_CUR, 0);
    }

    TRACE_F_RD_SCDC(LIBF_F_STR(fgets) ": return: %" dptr_fmt, DPTR(ret));

    LIBFILEIO_SCDC_STATS_SCDC_CMD(
      if (ret) LIBFILEIO_SCDC_STATS_NREAD(file_to_fd(stream), ii, tstat);
    );

  } else
#endif
  {
    LIBFILEIO_SCDC_STATS_ORIG_CMD(
      dsint_t nread = 0;
      LIBFILEIO_SCDC_STATS_TIME(tstat);
    );

    TRACE_F_RD_ORIG(LIBF_F_STR(fgets) ": s: %" dptr_fmt ", size: %d, stream: %" dptr_fmt, DPTR(s), size, DPTR(stream));

    LIBFILEIO_SCDC_STATS_ORIG_CMD(
      ORIG_F_INIT(ftell);
      nread = ORIG_F(ftell)(stream);
    );

    ORIG_F_INIT(fgets);
    ret = ORIG_F(fgets)(s, size, stream);

    LIBFILEIO_SCDC_STATS_ORIG_CMD(
      if (ret) nread = ORIG_F(ftell)(stream) - nread;
    );

    TRACE_F_RD_ORIG(LIBF_F_STR(fgets) ": return: %" dptr_fmt, DPTR(ret));

    LIBFILEIO_SCDC_STATS_ORIG_CMD(
      if (ret) LIBFILEIO_SCDC_STATS_NREAD(file_to_fd(stream), nread, tstat);
    );
  }

  LIBFILEIO_SCDC_RELEASE();

  TRACE_F_RD_OUTRO(LIBF_F_STR(fgets) ": return: %" dptr_fmt, DPTR(ret));

  return ret;
}


static int fputcXX(int XX, int c, FILE *stream)
{
  TRACE_F_WR_INTRO("%s" LIBF_F_STR(putc) ": c: %d, stream: %" dptr_fmt, XXSTR(XX), c, DPTR(stream));

  LIBFILEIO_SCDC_INIT();

  int ret = EOF;

#if LIBFILEIO_SCDC_ENABLED
  if (LIBFILEIO_SCDC_VALID(stream))
  {
    LIBFILEIO_SCDC_STATS_SCDC_CMD(LIBFILEIO_SCDC_STATS_TIME(tstat););

    libfileio_scdc_t *lfio = (libfileio_scdc_t *) stream;

    TRACE_F_WR_SCDC("%s" LIBF_F_STR(putc) ": lfio: %" dptr_fmt ", c: %d", XXSTR(XX), DPTR(lfio), c);

    int n = fileio_scdc_write(&lfio->fio, &c, 1);

    if (n < 0) lfio->err = 1;

    if (n > 0) ret = c;

    TRACE_F_WR_SCDC("%s" LIBF_F_STR(putc) ": return: %d", XXSTR(XX), ret);

    LIBFILEIO_SCDC_STATS_SCDC_CMD(
      if (ret != EOF) LIBFILEIO_SCDC_STATS_NWRITE(file_to_fd(stream), 1, tstat);
    );

  } else
#endif
  {
    LIBFILEIO_SCDC_STATS_ORIG_CMD(LIBFILEIO_SCDC_STATS_TIME(tstat););

    TRACE_F_WR_ORIG("%s" LIBF_F_STR(putc) ": c: %d, stream: %" dptr_fmt, XXSTR(XX), c, DPTR(stream));

    if (XX == 1)
    {
      ORIG_F_INIT(fputc);
      ret = ORIG_F(fputc)(c, stream);

    }
#if LIBFILEIO_SCDC_PROVIDE_FILE_
      else
    {
      ORIG_F_INIT_(putc);
      ret = ORIG_F_(putc)(c, stream);
    }
#endif

    TRACE_F_WR_ORIG("%s" LIBF_F_STR(putc) ": return: %d", XXSTR(XX), ret);

    LIBFILEIO_SCDC_STATS_ORIG_CMD(
      if (ret != EOF) LIBFILEIO_SCDC_STATS_NWRITE(file_to_fd(stream), 1, tstat);
    );
  }

  LIBFILEIO_SCDC_RELEASE();

  TRACE_F_WR_OUTRO("%s" LIBF_F_STR(fputc) ": return: %d", XXSTR(XX), ret);

  return ret;
}


int LIBF_F(fputc)(int c, FILE *stream)
{
  return fputcXX(1, c, stream);
}


#if LIBFILEIO_SCDC_PROVIDE_FILE_

int LIBF_F(putc)(int c, FILE *stream)
{
  return fputcXX(0, c, stream);
}

#endif

#undef XXSTR


int LIBF_F(fputs)(const char *s, FILE *stream)
{
  TRACE_F_WR_INTRO(LIBF_F_STR(fputs) ": s: %" dptr_fmt ", stream: %" dptr_fmt, DPTR(s), DPTR(stream));

  LIBFILEIO_SCDC_INIT();

  int ret = EOF;

#if LIBFILEIO_SCDC_ENABLED
  if (LIBFILEIO_SCDC_VALID(stream))
  {
    LIBFILEIO_SCDC_STATS_SCDC_CMD(LIBFILEIO_SCDC_STATS_TIME(tstat););

    libfileio_scdc_t *lfio = (libfileio_scdc_t *) stream;

    TRACE_F_WR_SCDC(LIBF_F_STR(fputs) ": lfio: %" dptr_fmt ", s: %" dptr_fmt, DPTR(lfio), DPTR(s));

    int n = fileio_scdc_write(&lfio->fio, s, strlen(s));

    if (n < 0) lfio->err = 1;

    if (n > 0) ret = n;

    TRACE_F_WR_SCDC(LIBF_F_STR(fputs) ": return: %d", ret);

    LIBFILEIO_SCDC_STATS_SCDC_CMD(
      if (ret != EOF) LIBFILEIO_SCDC_STATS_NWRITE(file_to_fd(stream), n, tstat);
    );

  } else
#endif
  {
    LIBFILEIO_SCDC_STATS_ORIG_CMD(
      dsint_t nwrite = 0;
      LIBFILEIO_SCDC_STATS_TIME(tstat);
    );

    TRACE_F_WR_ORIG(LIBF_F_STR(fputs) ": s: %" dptr_fmt ", stream: %" dptr_fmt, DPTR(s), DPTR(stream));

    LIBFILEIO_SCDC_STATS_ORIG_CMD(
      ORIG_F_INIT(ftell);
      nwrite = ORIG_F(ftell)(stream);
    );

    ORIG_F_INIT(fputs);
    ret = ORIG_F(fputs)(s, stream);

    LIBFILEIO_SCDC_STATS_ORIG_CMD(
      if (ret != EOF) nwrite = ORIG_F(ftell)(stream) - nwrite;
    );

    TRACE_F_WR_ORIG(LIBF_F_STR(fputs) ": return: %d", ret);

    LIBFILEIO_SCDC_STATS_ORIG_CMD(
      if (ret != EOF) LIBFILEIO_SCDC_STATS_NWRITE(file_to_fd(stream), nwrite, tstat);
    );
  }

  LIBFILEIO_SCDC_RELEASE();

  TRACE_F_WR_OUTRO(LIBF_F_STR(fputs) ": return: %d", ret);

  return ret;
}


#define XXSTR(_xx_)  ((XX == 1)?"v":"")

static int vfscanfXX(int XX, FILE *stream, const char *format, va_list ap)
{
  TRACE_F_RD_INTRO("%s" LIBF_F_STR(fscanf) ": stream: %" dptr_fmt ", format: %" dptr_fmt, XXSTR(XX), DPTR(stream), DPTR(format));

  LIBFILEIO_SCDC_INIT();

  int ret = EOF;

#if LIBFILEIO_SCDC_ENABLED
  if (LIBFILEIO_SCDC_VALID(stream))
  {
    LIBFILEIO_SCDC_STATS_SCDC_CMD(LIBFILEIO_SCDC_STATS_TIME(tstat););

    libfileio_scdc_t *lfio = (libfileio_scdc_t *) stream;

    TRACE_F_RD_SCDC("%s" LIBF_F_STR(fscanf) ": lfio: %" dptr_fmt ", format: %" dptr_fmt, XXSTR(XX), DPTR(lfio), DPTR(format));

    void *buf;
#if !LIBFILEIO_SCDC_FSCANF_SSCANF && LIBFILEIO_SCDC_FSCANF_FMEM_PERSISTENT
    if (!lfio->fscanf_buf)
    {
      lfio->fscanf_buf_size = LIBFILEIO_SCDC_FSCANF_BUF_SIZE;
      lfio->fscanf_buf = malloc(lfio->fscanf_buf_size);
      lfio->fscanf_file = fmemopen(lfio->fscanf_buf, lfio->fscanf_buf_size, "r");
    }
    buf = lfio->fscanf_buf;
#else
    char fscanf_buf[LIBFILEIO_SCDC_FSCANF_BUF_SIZE];
    buf = fscanf_buf;
#endif

    scdcint_t n = fileio_scdc_read(&lfio->fio, buf, LIBFILEIO_SCDC_FSCANF_BUF_SIZE, 1);

    long i = 0;

    if (n < 0) lfio->err = 1;
    else
    {
#if LIBFILEIO_SCDC_FSCANF_SSCANF
# error ""
#else /* LIBFILEIO_SCDC_FSCANF_SSCANF */
      FILE *f;
# if LIBFILEIO_SCDC_FSCANF_FMEM_PERSISTENT
      ORIG_F_INIT(fseek);
      ORIG_F(fseek)(lfio->fscanf_file, 0, SEEK_SET);

      ORIG_F_INIT(fflush);
      ORIG_F(fflush)(lfio->fscanf_file);

      f = lfio->fscanf_file;
# else /* !LIBFILEIO_SCDC_FSCANF_FMEM_PERSISTENT */
      f = fmemopen(buf, n, "r");
# endif /* !LIBFILEIO_SCDC_FSCANF_FMEM_PERSISTENT */
      
      ORIG_F_INIT(vfscanf);
      ret = ORIG_F(vfscanf)(f, format, ap);

      ORIG_F_INIT(ftell);
      i = ORIG_F(ftell)(f);

# if !LIBFILEIO_SCDC_FSCANF_FMEM_PERSISTENT
      ORIG_F_INIT(fclose);
      ORIG_F(fclose)(f);
# endif /* !LIBFILEIO_SCDC_FSCANF_FMEM_PERSISTENT */
#endif /* LIBFILEIO_SCDC_FSCANF_SSCANF */

      if (ret != EOF && n > i) fileio_scdc_seek(&lfio->fio, i - n, SEEK_CUR, 0);
    }

    TRACE_F_RD_SCDC("%s" LIBF_F_STR(fscanf) ": return: %d", XXSTR(XX), ret);

    LIBFILEIO_SCDC_STATS_SCDC_CMD(
      if (ret != EOF) LIBFILEIO_SCDC_STATS_NREAD(file_to_fd(stream), i, tstat);
    );

  } else
#endif
  {
    LIBFILEIO_SCDC_STATS_ORIG_CMD(
      dsint_t nread = 0;
      LIBFILEIO_SCDC_STATS_TIME(tstat);
    );

    TRACE_F_RD_ORIG("%s" LIBF_F_STR(fscanf) ": stream: %" dptr_fmt ", format: %" dptr_fmt, XXSTR(XX), DPTR(stream), DPTR(format));

    LIBFILEIO_SCDC_STATS_ORIG_CMD(
      ORIG_F_INIT(ftell);
      nread = ORIG_F(ftell)(stream);
    );

    ORIG_F_INIT(vfscanf);
    ret = ORIG_F(vfscanf)(stream, format, ap);

    LIBFILEIO_SCDC_STATS_ORIG_CMD(
      if (ret != EOF) nread = ORIG_F(ftell)(stream) - nread;
    );

    TRACE_F_RD_ORIG("%s" LIBF_F_STR(fscanf) ": return: %d", XXSTR(XX), ret);

    LIBFILEIO_SCDC_STATS_ORIG_CMD(
      if (ret != EOF) LIBFILEIO_SCDC_STATS_NREAD(file_to_fd(stream), nread, tstat);
    );
  }

  LIBFILEIO_SCDC_RELEASE();

  TRACE_F_RD_OUTRO("%s" LIBF_F_STR(fscanf) ": return: %d", XXSTR(XX), ret);

  return ret;
}


int LIBF_F(fscanf)(FILE *stream, const char *format, ...)
{
  va_list ap;
  va_start(ap, format);

  int ret = vfscanfXX(-1, stream, format, ap);

  va_end(ap);

  return ret;
}


int LIBF_F(vfscanf)(FILE *stream, const char *format, va_list ap)
{
  return vfscanfXX(0, stream, format, ap);
}


#if LIBFILEIO_SCDC_PROVIDE_ISOC99_FSCANF

int LIBF_F(__isoc99_fscanf)(FILE *stream, const char *format, ...)
{
  va_list ap;
  va_start(ap, format);

  int ret = vfscanfXX(-1, stream, format, ap);

  va_end(ap);

  return ret;
}


int LIBF_F(__isoc99_vfscanf)(FILE *stream, const char *format, va_list ap)
{
  return vfscanfXX(0, stream, format, ap);
}

#endif


static int vfprintfXX(int XX, FILE *stream, const char *format, va_list ap)
{
  TRACE_F_RD_INTRO("%s" LIBF_F_STR(fprintf) ": stream: %" dptr_fmt ", format: %" dptr_fmt, XXSTR(XX), DPTR(stream), DPTR(format));

  LIBFILEIO_SCDC_INIT();

  int ret = -1;

#if LIBFILEIO_SCDC_ENABLED
  if (LIBFILEIO_SCDC_VALID(stream))
  {
    LIBFILEIO_SCDC_STATS_SCDC_CMD(LIBFILEIO_SCDC_STATS_TIME(tstat););

    libfileio_scdc_t *lfio = (libfileio_scdc_t *) stream;

    TRACE_F_RD_SCDC("%s" LIBF_F_STR(fprintf) ": lfio: %" dptr_fmt ", format: %" dptr_fmt, XXSTR(XX), DPTR(lfio), DPTR(format));

    void *buf;
#if !LIBFILEIO_SCDC_FPRINTF_SPRINTF && LIBFILEIO_SCDC_FPRINTF_FMEM_PERSISTENT
    if (!lfio->fprintf_buf)
    {
      lfio->fprintf_buf_size = LIBFILEIO_SCDC_FPRINTF_BUF_SIZE;
      lfio->fprintf_buf = malloc(lfio->fprintf_buf_size);
      lfio->fprintf_file = fmemopen(lfio->fprintf_buf, lfio->fprintf_buf_size, "w");
    }
    buf = lfio->fprintf_buf;
#else
    char fprintf_buf[LIBFILEIO_SCDC_FPRINTF_BUF_SIZE];
    buf = fprintf_buf;
#endif

    long i = 0;

#if LIBFILEIO_SCDC_FPRINTF_SPRINTF
    ret = vsnprintf(buf, LIBFILEIO_SCDC_FPRINTF_BUF_SIZE, format, ap);

    i = ret;
#else /* LIBFILEIO_SCDC_FPRINTF_SPRINTF */
    FILE *f;
# if LIBFILEIO_SCDC_FPRINTF_FMEM_PERSISTENT
    ORIG_F_INIT(fseek);
    ORIG_F(fseek)(lfio->fprintf_file, 0, SEEK_SET);
    f = lfio->fprintf_file;
# else /* !LIBFILEIO_SCDC_FPRINTF_FMEM_PERSISTENT */
    f = fmemopen(buf, LIBFILEIO_SCDC_FPRINTF_BUF_SIZE, "w");
# endif /* !LIBFILEIO_SCDC_FPRINTF_FMEM_PERSISTENT */

    ORIG_F_INIT(vfprintf);
    ret = ORIG_F(vfprintf)(f, format, ap);

    ORIG_F_INIT(ftell);
    i = ORIG_F(ftell)(f);

# if LIBFILEIO_SCDC_FPRINTF_FMEM_PERSISTENT
    ORIG_F_INIT(fflush);
    ORIG_F(fflush)(f);
# else /* !LIBFILEIO_SCDC_FPRINTF_FMEM_PERSISTENT */
    ORIG_F_INIT(fclose);
    ORIG_F(fclose)(f);
# endif /* !LIBFILEIO_SCDC_FPRINTF_FMEM_PERSISTENT */
#endif /* LIBFILEIO_SCDC_FPRINTF_SPRINTF */

    if (ret >= 0 && i > 0) i = fileio_scdc_write(&lfio->fio, buf, i);

    if (ret < 0) lfio->err = 1;

    TRACE_F_WR_SCDC("%s" LIBF_F_STR(fprintf) ": return: %d", XXSTR(XX), ret);

    LIBFILEIO_SCDC_STATS_SCDC_CMD(
      if (ret >= 0) LIBFILEIO_SCDC_STATS_NWRITE(file_to_fd(stream), i, tstat);
    );

  } else
#endif
  {
    LIBFILEIO_SCDC_STATS_ORIG_CMD(
      dsint_t nwrite = 0;
      LIBFILEIO_SCDC_STATS_TIME(tstat);
    );

    TRACE_F_RD_ORIG("%s" LIBF_F_STR(fprintf) ": stream: %" dptr_fmt ", format: %" dptr_fmt, XXSTR(XX), DPTR(stream), DPTR(format));

    LIBFILEIO_SCDC_STATS_ORIG_CMD(
      ORIG_F_INIT(ftell);
      nwrite = ORIG_F(ftell)(stream);
    );

    ORIG_F_INIT(vfprintf);
    ret = ORIG_F(vfprintf)(stream, format, ap);

    LIBFILEIO_SCDC_STATS_ORIG_CMD(
      if (ret >= 0) nwrite = ORIG_F(ftell)(stream) - nwrite;
    );

    TRACE_F_WR_ORIG("%s" LIBF_F_STR(fprintf) ": return: %d", XXSTR(XX), ret);

    LIBFILEIO_SCDC_STATS_ORIG_CMD(
      if (ret >= 0) LIBFILEIO_SCDC_STATS_NWRITE(file_to_fd(stream), nwrite, tstat);
    );
  }

  LIBFILEIO_SCDC_RELEASE();

  TRACE_F_WR_OUTRO("%s" LIBF_F_STR(fprintf) ": return: %d", XXSTR(XX), ret);

  return ret;
}

#undef XXSTR


int LIBF_F(fprintf)(FILE *stream, const char *format, ...)
{
  va_list ap;
  va_start(ap, format);

  int ret = vfprintfXX(-1, stream, format, ap);

  va_end(ap);

  return ret;
}


int LIBF_F(vfprintf)(FILE *stream, const char *format, va_list ap)
{
  return vfprintfXX(0, stream, format, ap);
}


#ifdef LIBFILEIO_SCDC_PROVIDE_64

int LIBF_F(fseeko64)(FILE *stream, off64_t offset, int whence)
{
  return fseekXX(64, stream, offset, whence);
}


off64_t LIBF_F(ftello64)(FILE *stream)
{
  return ftellXX(64, stream);
}

#endif /* LIBFILEIO_SCDC_PROVIDE_64 */

#endif /* LIBFILEIO_SCDC_PROVIDE_FILE */


#if LIBFILEIO_SCDC_PROVIDE_FS

#define XXSTR(_xx_)  (((_xx_) == 64)?"64":"")

static dsint_t statXX(int XX, int l, const char *pathname, void *buf)
{
  TRACE_F_CTRL_INTRO("%s" LIBF_F_STR(stat) "%s: pathname: '%s', buf: %" dptr_fmt, ((l == 0)?"":"l"), XXSTR(XX), pathname, DPTR(buf));

  LIBFILEIO_SCDC_INIT();

  dsint_t ret = -1;

#if LIBFILEIO_SCDC_ENABLED
  char path_mod[LIBFILEIO_SCDC_PATH_MAX];
  const char *p = libfileio_scdc_path_mod(pathname, path_mod);

  TRACE_F_CTRL_BODY(LIBF_F_STR() "%s: modified pathname: '%s'", XXSTR(XX), p);

  if (fileio_scdc_valid(p))
  {
    TRACE_F_CTRL_SCDC("%s" LIBF_F_STR(stat) "%s: pathname: '%s', buf: %" dptr_fmt, ((l == 0)?"":"l"), XXSTR(XX), pathname, DPTR(buf));

    int err = 0;

    fileio_scdc_stat_t fio_stat;

    if (fileio_scdc_stat_path(p, &fio_stat, &err) == FILEIO_SCDC_SUCCESS)
    {
      ret = 0;

      if (XX == 64)
      {
        struct stat64 *buf_stat = (struct stat64 *) buf;

        memset(buf_stat, 0, sizeof(struct stat));

        buf_stat->st_size = fio_stat.size;

      } else
      {
        struct stat *buf_stat = (struct stat *) buf;

        memset(buf_stat, 0, sizeof(struct stat));

        buf_stat->st_size = fio_stat.size;
      }

    } else
    {
      errno = err;
    }

    TRACE_F_CTRL_SCDC("%s" LIBF_F_STR(stat) "%s: return: %" dsint_fmt, ((l == 0)?"":"l"), XXSTR(XX), ret);

  } else
#endif
  {
    TRACE_F_CTRL_ORIG("%s" LIBF_F_STR(stat) "%s: pathname: '%s', buf: %" dptr_fmt, ((l == 0)?"":"l"), XXSTR(XX), pathname, DPTR(buf));

    if (XX == 64)
    {
      if (l)
      {
        ORIG_F_INIT(lstat64);
        if (ORIG_F(lstat64)) ret = ORIG_F(lstat64)(pathname, buf);
        else ret = __lxstat64(_STAT_VER, pathname, buf);

      } else
      {
        ORIG_F_INIT(stat64);
        if (ORIG_F(stat64)) ret = ORIG_F(stat64)(pathname, buf);
        else ret = __xstat64(_STAT_VER, pathname, buf);
      }

    } else
    {
      if (l)
      {
        ORIG_F_INIT(lstat);
        if (ORIG_F(lstat)) ret = ORIG_F(lstat)(pathname, buf);
        else ret = __lxstat(_STAT_VER, pathname, buf);

      } else
      {
        ORIG_F_INIT(stat);
        if (ORIG_F(stat)) ret = ORIG_F(stat)(pathname, buf);
        else ret = __xstat(_STAT_VER, pathname, buf);
      }
    }

    TRACE_F_CTRL_ORIG("%s" LIBF_F_STR(stat) "%s: return: %" dsint_fmt, ((l == 0)?"":"l"), XXSTR(XX), ret);
  }

  LIBFILEIO_SCDC_RELEASE();

  TRACE_F_CTRL_OUTRO("%s" LIBF_F_STR(stat) "%s: return: %" dsint_fmt, ((l == 0)?"":"l"), XXSTR(XX), ret);

  return ret;
}

#undef XXSTR


int LIBF_F(stat)(const char *pathname, struct stat *buf)
{
  return statXX(0, 0, pathname, buf);
}


int LIBF_F(lstat)(const char *pathname, struct stat *buf)
{
  return statXX(0, 1, pathname, buf);
}


#ifdef LIBFILEIO_SCDC_PROVIDE_64

int LIBF_F(stat64)(const char *pathname, struct stat64 *buf)
{
  return statXX(64, 0, pathname, buf);
}


int LIBF_F(lstat64)(const char *pathname, struct stat64 *buf)
{
  return statXX(64, 1, pathname, buf);
}

#endif /* LIBFILEIO_SCDC_PROVIDE_64 */


#define XXSTR(_xx_)  ((XX == 0)?"unlink":"remove")

static int removeXX(int XX, const char *pathname)
{
  TRACE_F_CTRL_INTRO(LIBF_F_STR() "%s: pathname: '%s'", XXSTR(XX), pathname);

  LIBFILEIO_SCDC_INIT();

  int ret = -1;

#if LIBFILEIO_SCDC_ENABLED
  char path_mod[LIBFILEIO_SCDC_PATH_MAX];
  const char *p = libfileio_scdc_path_mod(pathname, path_mod);

  TRACE_F_CTRL_BODY(LIBF_F_STR() "%s: modified pathname: '%s'", XXSTR(XX), p);

  if (fileio_scdc_valid(p))
  {
    TRACE_F_CTRL_SCDC(LIBF_F_STR() "%s: pathname: '%s'", XXSTR(XX), p);

    int err = 0;

    if (fileio_scdc_remove(p, &err) == FILEIO_SCDC_SUCCESS)
    {
      ret = 0;

    } else
    {
      ret = err;
    }

    TRACE_F_CTRL_SCDC(LIBF_F_STR() "%s: return: %d", XXSTR(XX), ret);

  } else
#endif
  {
    TRACE_F_CTRL_ORIG(LIBF_F_STR() "%s: pathname: '%s'", XXSTR(XX), pathname);

    if (XX == 0)
    {
      ORIG_F_INIT(unlink);
      ret = ORIG_F(unlink)(pathname);

    } else
    {
      ORIG_F_INIT(remove);
      ret = ORIG_F(remove)(pathname);
    }

    TRACE_F_CTRL_ORIG(LIBF_F_STR() "%s: return: %d", XXSTR(XX), ret);
  }

  LIBFILEIO_SCDC_RELEASE();

  TRACE_F_CTRL_OUTRO(LIBF_F_STR() "%s: return: %d", XXSTR(XX), ret);

  return ret;
}


int LIBF_F(remove)(const char *pathname)
{
  return removeXX(1, pathname);
}


int LIBF_F(unlink)(const char *pathname)
{
  return removeXX(0, pathname);
}



int LIBF_F(rename)(const char *oldpath, const char *newpath)
{
  TRACE_F_CTRL_INTRO(LIBF_F_STR(rename) ": oldpath: '%s', newpath: '%s'", oldpath, newpath);

  LIBFILEIO_SCDC_INIT();

  int ret = -1;

#if LIBFILEIO_SCDC_ENABLED
  char oldpath_mod[LIBFILEIO_SCDC_PATH_MAX], newpath_mod[LIBFILEIO_SCDC_PATH_MAX];
  const char *oldp = libfileio_scdc_path_mod(oldpath, oldpath_mod);
  const char *newp = libfileio_scdc_path_mod(newpath, newpath_mod);

  TRACE_F_CTRL_BODY(LIBF_F_STR(rename) ": modified oldpath: '%s', modified newpath: '%s'", oldp, newp);

  if (fileio_scdc_valid(oldp) || fileio_scdc_valid(newp))
  {
    TRACE_F_CTRL_SCDC(LIBF_F_STR(rename) ": oldpath: '%s', newpath: '%s'", oldp, newp);

    int err = 0;

    if (fileio_scdc_move(oldp, newp, &err) == FILEIO_SCDC_SUCCESS)
    {
      ret = 0;

    } else
    {
      ret = err;
    }

    TRACE_F_CTRL_SCDC(LIBF_F_STR(rename) ": return: %d", ret);

  } else
#endif
  {
    TRACE_F_CTRL_ORIG(LIBF_F_STR(rename) ": oldpath: '%s', newpath: '%s'", oldpath, newpath);

    ORIG_F_INIT(rename);
    ret = ORIG_F(rename)(oldpath, newpath);

    TRACE_F_CTRL_ORIG(LIBF_F_STR(rename) ": return: %d", ret);
  }

  LIBFILEIO_SCDC_RELEASE();

  TRACE_F_CTRL_OUTRO(LIBF_F_STR(rename) ": return: %d", ret);

  return ret;
}

#endif /* LIBFILEIO_SCDC_PROVIDE_FS */
