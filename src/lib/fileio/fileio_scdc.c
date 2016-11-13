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


#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "scdc.h"
#include "z_pack.h"
#include "common.h"
#include "fileio_scdc.h"


#define TRACE_PREFIX  "fileio_scdc: "

#define FILEIO_SCDC_TRACE           0
#define FILEIO_SCDC_TRACE_INTRO     0
#define FILEIO_SCDC_TRACE_OUTRO     0
#define FILEIO_SCDC_TRACE_BODY      0
#define FILEIO_SCDC_TRACE_PERF      0
#define FILEIO_SCDC_TRACE_SCDC      1


#define FILEIO_SCDC_LOCAL       1
#define FILEIO_SCDC_LOCAL_BASE  "store"
#define FILEIO_SCDC_LOCAL_PATH  "store"

const char *fileio_scdc_local_base = 0;
const char *fileio_scdc_local_path = 0;
scdc_dataprov_t dp_local = SCDC_DATAPROV_NULL;

#include "fileio_scdc_trace.h"


static int fileio_scdc_initialized = -1;


static void fileio_scdc_init()
{
  if (fileio_scdc_initialized < 0)
  {
    TRACE_F_CTRL_BODY("fileio_scdc_init: first init");

    fileio_scdc_initialized = 0;

    /* on first init */
    scdc_init(SCDC_INIT_DEFAULT);

#ifdef FILEIO_SCDC_LOCAL
    const char *local_base = fileio_scdc_local_base;
    const char *local_path = fileio_scdc_local_path;
    if (!local_base || !local_path)
    {
      local_base = FILEIO_SCDC_LOCAL_BASE;
      local_path = FILEIO_SCDC_LOCAL_PATH;
    }

    TRACE_F_CTRL_BODY("fileio_scdc_init: local data provider '%s' with path '%s'", local_base, local_path);

    dp_local = scdc_dataprov_open(local_base, "fs:access", local_path);
      
#endif
  }

  if (fileio_scdc_initialized == 0)
  {
    TRACE_F_CTRL_BODY("fileio_scdc_init: every init");

    /* on every init */
  }

  ++fileio_scdc_initialized;
}


static void fileio_scdc_release()
{
  if (fileio_scdc_initialized <= 0) return;

  --fileio_scdc_initialized;

  if (fileio_scdc_initialized == 0)
  {
    TRACE_F_CTRL_BODY("fileio_scdc_release: every release");

    /* on every release */
  }
}


scdcint_t fileio_scdc_local(const char *base, const char *path)
{
  fileio_scdc_local_base = base;
  fileio_scdc_local_path = path;

  return FILEIO_SCDC_SUCCESS;
}


scdcint_t fileio_scdc_valid(const char *path)
{
  return (scdc_nodeport_supported(path) == SCDC_SUCCESS);
}


#if FILEIO_SCDC_BUF

static void fileio_scdc_buf_unset(fileio_scdc_buf_t *b)
{
  b->max_size = b->max_read = b->max_write = 0;
  b->ptr = NULL;

  b->offset = -1;
  b->size = 0;
}

#endif /* FILEIO_SCDC_BUF */


#if !FILEIO_SCDC_SYNC

static void fileio_scdc_buf_init(fileio_scdc_buf_t *b)
{
  if (b->ptr) return;

  b->max_size = FILEIO_SCDC_BUF;
  b->max_read = FILEIO_SCDC_BUF_READ_CHUNK;
  b->max_write = FILEIO_SCDC_BUF_WRITE_CHUNK;

#if FILEIO_SCDC_BUF_ENV
  const char *s = 0;
  if ((s = getenv("LIBFILEIO_SCDC_BUFFER_SIZE"))) sscanf(s, "%" scdcint_fmt, &b->max_size);
  if ((s = getenv("LIBFILEIO_SCDC_BUFFER_SIZE_READ"))) sscanf(s, "%" scdcint_fmt, &b->max_read);
  if ((s = getenv("LIBFILEIO_SCDC_BUFFER_SIZE_WRITE"))) sscanf(s, "%" scdcint_fmt, &b->max_write);
#endif

  b->max_read = z_min(b->max_read, b->max_size);
  b->max_write = z_min(b->max_write, b->max_size);

  b->ptr = malloc(FILEIO_SCDC_BUF);

  b->size = 0;
#if !FILEIO_SCDC_SYNC_WRITE
  b->mod_begin = b->mod_end = -1;
#endif
}


static void fileio_scdc_buf_release(fileio_scdc_buf_t *b)
{
  if (b->ptr) free(b->ptr);

  fileio_scdc_buf_unset(b);
}


#if !FILEIO_SCDC_SYNC_WRITE

static void fileio_scdc_buf_sync(fileio_scdc_buf_t *b)
{
  b->mod_begin = b->mod_end = -1;
}

#endif


static void fileio_scdc_buf_flush(fileio_scdc_buf_t *b)
{
  b->size = 0;

#if !FILEIO_SCDC_SYNC_WRITE
  fileio_scdc_buf_sync(b);
#endif
}


static void fileio_scdc_buf_compact(fileio_scdc_buf_t *b, scdcint_t offset)
{
  if (offset < b->size)
  {
    memmove(b->ptr, b->ptr + offset, b->size - offset);
    b->size -= offset;

  } else b->size = 0;

#if !FILEIO_SCDC_SYNC_WRITE
  fileio_scdc_buf_sync(b);
#endif
}


#if !FILEIO_SCDC_SYNC_WRITE

/* write buffer to target */
scdcint_t fileio_scdc_sync_buf(fileio_scdc_t *fio)
{
  TRACE_F_CTRL_INTRO("fileio_scdc_sync_buf: fio: %" dptr_fmt, DPTR(fio));

  scdcint_t ret = FILEIO_SCDC_SUCCESS;

  if (fio->buf.offset < 0) goto do_return;

  scdcint_t n = fio->buf.mod_end - fio->buf.mod_begin;

  /* nothing to sync */
  if (n <= 0) goto do_return;

  TRACE_F_CTRL_BODY("fileio_scdc_sync_buf: put %" scdcint_fmt " to scdc", n);


  /* sync modified buffer */
  scdc_dataset_input_t input;

  scdc_dataset_input_unset(&input);
  input.buf_size = n;
  input.buf = fio->buf.ptr + fio->buf.mod_begin;
  input.current_size = n;
  input.total_size = n;
  input.total_size_given = SCDC_DATASET_INOUT_TOTAL_SIZE_GIVEN_EXACT;

  char cmd[256];
  sprintf(cmd, "put %" scdcint_fmt "F:%" scdcint_fmt, fio->buf.offset + fio->buf.mod_begin, n);

#if FILEIO_SCDC_TRACE_SCDC_CTRL
  double cmd_t = z_time_wtime();
  scdcint_t cmd_size = input.current_size;
#endif

  if (scdc_dataset_cmd(fio->dataset, cmd, &input, NULL) != SCDC_SUCCESS)
  {
    ret = FILEIO_SCDC_FAILURE;
  }

#if FILEIO_SCDC_TRACE_SCDC_CTRL
  cmd_t = z_time_wtime() - cmd_t;
  TRACE_F_CTRL_SCDC("fileio_scdc_sync_buf: cmd time: %f, size: %" scdcint_fmt ", rate: %f", cmd_t, cmd_size, cmd_size / cmd_t);
#endif

  /* set buffer to synced */
  fileio_scdc_buf_sync(&fio->buf);

do_return:

  TRACE_F_CTRL_OUTRO("fileio_scdc_sync_buf: return: %" scdcint_fmt, ret);

  return ret;
}

#endif /* !FILEIO_SCDC_SYNC_WRITE */


/* empty the buffer */
scdcint_t fileio_scdc_flush_buf(fileio_scdc_t *fio)
{
  TRACE_F_CTRL_INTRO("fileio_scdc_flush_buf: fio: %" dptr_fmt, DPTR(fio));

  scdcint_t ret = FILEIO_SCDC_SUCCESS;

  if (fio->buf.offset < 0) goto do_return;

#if !FILEIO_SCDC_SYNC_WRITE
  /* write buffer to target */
  fileio_scdc_sync_buf(fio);
#endif /* !FILEIO_SCDC_SYNC_WRITE */

  /* set buffer to empty */
  fileio_scdc_buf_flush(&fio->buf);

  fio->buf.offset = -1;
  
do_return:

  TRACE_F_CTRL_OUTRO("fileio_scdc_flush_buf: return: %" scdcint_fmt, ret);

  return ret;
}


/* compact the buffer */
scdcint_t fileio_scdc_compact_buf(fileio_scdc_t *fio, scdcint_t offset)
{
  TRACE_F_CTRL_INTRO("fileio_scdc_compact_buf: fio: %" dptr_fmt ", offset: %" scdcint_fmt, DPTR(fio), offset);

  scdcint_t ret = FILEIO_SCDC_SUCCESS;

  if (fio->buf.offset < 0) goto do_return;

#if !FILEIO_SCDC_SYNC_WRITE
  /* write buffer to target */
  fileio_scdc_sync_buf(fio);
#endif /* !FILEIO_SCDC_SYNC_WRITE */

  /* compact buffer */
  fileio_scdc_buf_compact(&fio->buf, offset - fio->buf.offset);

  if (fio->buf.size > 0) fio->buf.offset = offset;
  else fio->buf.offset = -1;

do_return:

  TRACE_F_CTRL_OUTRO("fileio_scdc_compact_buf: return: %" scdcint_fmt, ret);

  return ret;
}

#endif /* !FILEIO_SCDC_SYNC */


/* supported flags:
   - O_RDONLY, O_WRONLY, O_RDWR: read only, write only, read and write, respectively
     -> handled on client side
   - O_CREAT: create file if it not exists, otherwise open it
   - O_EXCL: ensure that the file is newly created if O_CREAT is provided, fail with EEXIST if the fail already exists, ignore if O_CREAT is missing
   - O_TRUNC: truncate file to length 0 if writting is enabled (and the file exists)
   - O_APPEND: always write to end (i.e., append), read at seeked offset
   - O_SYNC: disabled buffered read/write
*/


scdcint_t fileio_scdc_open(fileio_scdc_t *fio, const char *path, int flags, int *error)
{
  TRACE_F_CTRL_INTRO("fileio_scdc_open: path: '%s', flags: %d, error: %" dptr_fmt, path, flags, DPTR(error));

  int do_read = ((flags & 3) == O_RDONLY) || ((flags & 3) == O_RDWR);
  int do_write = ((flags & 3) == O_WRONLY) || ((flags & 3) == O_RDWR);
  int do_create = (flags & O_CREAT) != 0;
  int do_create_excl = do_create && (flags & O_EXCL) != 0;
  int do_truncate = (flags & O_TRUNC) != 0;
  int do_sync = (flags & O_SYNC) != 0;
#if FILEIO_SCDC_APPEND
  int do_append = (flags & O_APPEND) != 0;
#endif

  TRACE_F_CTRL_BODY("fileio_scdc_open: flags:");
  TRACE_F_CTRL_BODY("  READ: %d", do_read);
  TRACE_F_CTRL_BODY("  WRITE: %d", do_write);
  TRACE_F_CTRL_BODY("  CREATE: %d", do_create);
  TRACE_F_CTRL_BODY("  CREATE_EXCL: %d", do_create_excl);
  TRACE_F_CTRL_BODY("  TRUNCATE: %d", do_truncate);
  TRACE_F_CTRL_BODY("  SYNC: %d", do_sync);

  char p[256];
  const char *f = strrchr(path, '/');

  if (f)
  {
    int pn = f - path + 1;
    strncpy(p, path, pn);
    p[pn] = '\0';
    ++f;
  }

  TRACE_F_CTRL_BODY("fileio_scdc_open: p: '%s', f: '%s'", p, f);

  *error = 0;

  scdcint_t ret = FILEIO_SCDC_FAILURE;

  fio->eof = 0;
  fio->sync = do_sync;
  fio->read = do_read;
  fio->write = do_write;

  fio->offset = 0;
  fio->eof_offset = -1;
#if FILEIO_SCDC_APPEND
  fio->append = do_append;
#endif

  fileio_scdc_init();

  /* open path without file and fail with ENOENT if it fails (i.e., does not exist) */
  fio->dataset = scdc_dataset_open(p);
  if (fio->dataset == SCDC_DATASET_NULL)
  {
    *error = ENOENT;
    goto do_return;
  }

  char cmd[256];

  /* if O_CREAT and O_EXCL then 'ls file' to check and fail with EEXIST if it exists */
  if (do_create_excl)
  {
    sprintf(cmd, "ls %s", f);
    if (scdc_dataset_cmd(fio->dataset, cmd, NULL, NULL) == SCDC_SUCCESS)
    {
      *error = EEXIST;
      goto do_return;
    }
  }

  /* if O_TRUNC and WRITEABLE then 'rm file' */
  if (do_truncate && do_write)
  {
    sprintf(cmd, "rm %s", f);
    scdc_dataset_cmd(fio->dataset, cmd, NULL, NULL);
  }

  /* if O_CREAT or (O_TRUNC and WRITE) then 'put file' without input -> create empty file */
  if (do_create || (do_truncate && do_write))
  {
    sprintf(cmd, "put %s", f);
    scdc_dataset_cmd(fio->dataset, cmd, NULL, NULL);
  }

  /* 'cd file' and fail with ENOENT if it does not exist */
  sprintf(cmd, "cd %s", f);
  if (scdc_dataset_cmd(fio->dataset, cmd, NULL, NULL) != SCDC_SUCCESS)
  {
    *error = ENOENT;
    goto do_return;
  }

#if FILEIO_SCDC_BUF
  fileio_scdc_buf_unset(&fio->buf);
#endif

  ret = FILEIO_SCDC_SUCCESS;

do_return:
  if (ret == FILEIO_SCDC_FAILURE)
  {
    scdc_dataset_close(fio->dataset);

    fio->dataset = SCDC_DATASET_NULL;
  }

  if (fio->dataset == SCDC_DATASET_NULL) fileio_scdc_release();

  TRACE_F_CTRL_OUTRO("fileio_scdc_open: return: %" scdcint_fmt, ret);

  return ret;
}


scdcint_t fileio_scdc_reopen(fileio_scdc_t *fio, const char *path, int flags, int *error)
{
  TRACE_F_CTRL_INTRO("fileio_scdc_reopen: fio: %" dptr_fmt ", path: '%s', flags: %d, error: %" dptr_fmt, DPTR(fio), path, flags, DPTR(error));

  /* FIXME: do cd and reset file info and buffer */

  scdcint_t ret = FILEIO_SCDC_FAILURE;

  fileio_scdc_close(fio);

  ret = fileio_scdc_open(fio, path, flags, error);

  TRACE_F_CTRL_OUTRO("fileio_scdc_reopen: return: %" scdcint_fmt, ret);

  return ret;
}


scdcint_t fileio_scdc_close(fileio_scdc_t *fio)
{
  TRACE_F_CTRL_INTRO("fileio_scdc_close: fio: %" dptr_fmt, DPTR(fio));

  scdcint_t ret = FILEIO_SCDC_SUCCESS;

#if !FILEIO_SCDC_SYNC_WRITE
  if (!fio->sync)
  {
    fileio_scdc_sync_buf(fio);
  }
#endif /* !FILEIO_SCDC_SYNC_WRITE */

  scdc_dataset_close(fio->dataset);

#if !FILEIO_SCDC_SYNC
  if (!fio->sync)
  {
    fileio_scdc_buf_release(&fio->buf);
  }
#endif /* !FILEIO_SCDC_SYNC */

  fileio_scdc_release();

  TRACE_F_CTRL_OUTRO("fileio_scdc_close: return: %" scdcint_fmt, ret);

  return ret;
}


static scdcint_t read_dataset_output(scdc_dataset_output_t *output, char *buf, scdcint_t size, int read_size_only)
{
  scdcint_t size_left = size;

  do
  {
    scdcint_t n = z_min(output->current_size, size_left);

    if (output->buf != buf) memcpy(buf, output->buf, n);

    /* inc. dest. */
    buf += n;
    size_left -= n;

    /* inc. source */
    output->buf_size -= n;
    output->buf = ((char *) output->buf) + n;
    output->current_size -= n;

    if (!output->next || (read_size_only && size_left <= 0)) break;

    output->buf_size = size_left;
    output->buf = buf;

    if (output->next(output) == SCDC_FAILURE) return -1;

  } while (1);

  return size - size_left;
}


#if !FILEIO_SCDC_SYNC_READ

static scdcint_t fileio_scdc_read_buf(fileio_scdc_t *fio, scdcint_t offset, void *buf, scdcint_t size)
{
  if (fio->buf.offset < 0) return 0;

  scdcint_t o = offset - fio->buf.offset;
  scdcint_t s = fio->buf.size - o;
  scdcint_t n = 0;

  /* read inside the buffer? */
  if (o >= 0 && s > 0)
  {
    n = z_min(s, size);

    TRACE_F_RD_BODY("fileio_scdc_buf_read: get %" scdcint_fmt " from buffer", n);

    if (buf) memcpy(buf, fio->buf.ptr + o, n);
  }

  return n;
}

#endif /* !FILEIO_SCDC_SYNC_READ */


scdcint_t fileio_scdc_read(fileio_scdc_t *fio, void *buf, scdcint_t size, scdcint_t keep_buf)
{
  TRACE_F_RD_INTRO("fileio_scdc_read: fio: %" dptr_fmt ", buf: %" dptr_fmt ", size: %" scdcint_fmt ", keep_buf: %" scdcint_fmt, DPTR(fio), DPTR(buf), size, keep_buf);

  scdcint_t ret = FILEIO_SCDC_SUCCESS;

#if FILEIO_SCDC_TRACE_PERF_RD
  double t = z_time_wtime();
#endif

  if (!fio->read)
  {
    ret = FILEIO_SCDC_FAILURE;
    goto do_return;
  }

  char *nbuf = buf;
  scdcint_t nleft = size;
  scdcint_t noffset = fio->offset;

  TRACE_F_RD_BODY("fileio_scdc_read: nleft: %" scdcint_fmt ", noffset: %" scdcint_fmt, nleft, noffset);

  if (nleft <= 0) goto do_return;

#if !FILEIO_SCDC_SYNC
  scdcint_t offset_old = fio->offset;

  if (!fio->sync)
  {
# if !FILEIO_SCDC_SYNC_WRITE && FILEIO_SCDC_APPEND
    /* in append mode, the offset of the buffer within the file is unknown -> flush buffer to prevent read from buffer */
    if (fio->append && fio->buf.offset >= 0 && fio->buf.mod_begin < fio->buf.mod_end) fileio_scdc_flush_buf(fio);
# endif /* !FILEIO_SCDC_SYNC_WRITE && FILEIO_SCDC_APPEND */

# if !FILEIO_SCDC_SYNC_READ
    scdcint_t n = fileio_scdc_read_buf(fio, noffset, nbuf, nleft);

    nbuf += n;
    nleft -= n;
    noffset += n;

    TRACE_F_RD_BODY("fileio_scdc_read: nleft: %" scdcint_fmt ", noffset: %" scdcint_fmt, nleft, noffset);

    if (nleft <= 0) goto do_return;
# endif /* !FILEIO_SCDC_SYNC_READ */

# if !FILEIO_SCDC_SYNC_WRITE
    /* sync before reading */
    fileio_scdc_sync_buf(fio);
# endif /* !FILEIO_SCDC_SYNC_WRITE */
  }
#endif /* !FILEIO_SCDC_SYNC */

#define READ_TO_BUF  1024*2

  scdcint_t ntotal = nleft;
#if !FILEIO_SCDC_SYNC_READ
# ifdef READ_TO_BUF
  scdcint_t nleft_old = nleft;
# endif /* READ_TO_BUF */
#endif /* !FILEIO_SCDC_SYNC_READ */

#if !FILEIO_SCDC_SYNC_READ
  if (!fio->sync)
  {
    /* init before use */
    fileio_scdc_buf_init(&fio->buf);

    if (keep_buf) fileio_scdc_compact_buf(fio, offset_old);
    else fileio_scdc_flush_buf(fio);

    ntotal = z_max(ntotal, fio->buf.max_read - fio->buf.size);

# ifdef READ_TO_BUF
    if (nleft < READ_TO_BUF) nleft = 0;
# endif /* READ_TO_BUF */
  }
#endif /* !FILEIO_SCDC_SYNC_READ */

  scdc_dataset_output_t *output = NULL;
  scdcint_t n = 0;

  if (!fio->eof && (fio->eof_offset < 0 || noffset < fio->eof_offset))
  {
    TRACE_F_RD_BODY("fileio_scdc_read: get %" scdcint_fmt " from scdc", ntotal);

    char cmd[256];
    sprintf(cmd, "get %" scdcint_fmt "F:%" scdcint_fmt, noffset, ntotal);

    scdc_dataset_output_t output_;
    output = &output_;

    scdc_dataset_output_unset(output);
    output->buf_size = nleft;
    output->buf = nbuf;

    if (scdc_dataset_cmd(fio->dataset, cmd, NULL, output) != SCDC_SUCCESS)
    {
      ret = FILEIO_SCDC_FAILURE;
      goto do_return;
    }

    n = read_dataset_output(output, nbuf, nleft, (ntotal > nleft));
    if (n < 0)
    {
      ret = FILEIO_SCDC_FAILURE;
      goto do_return;
    }
  }

#if !FILEIO_SCDC_SYNC_READ
  if (!fio->sync)
  {
#ifdef READ_TO_BUF
    nleft = nleft_old;
#endif /* READ_TO_BUF */
  }
#endif /* !FILEIO_SCDC_SYNC_READ */

  nbuf += n;
  nleft -= n;
  noffset += n;

  TRACE_F_RD_BODY("fileio_scdc_read: nleft: %" scdcint_fmt ", noffset: %" scdcint_fmt, nleft, noffset);

#if !FILEIO_SCDC_SYNC_READ
  if (!fio->sync)
  {
    scdcint_t n = 0;

    if (ntotal > nleft && output)
    {
      n = read_dataset_output(output, fio->buf.ptr + fio->buf.size, fio->buf.max_size - fio->buf.size, 0);
      if (n < 0)
      {
        ret = FILEIO_SCDC_FAILURE;
        goto do_return;
      }

      if (n > 0)
      {
        if (fio->buf.offset < 0) fio->buf.offset = noffset;
        fio->buf.size += n;
      }
    }

# ifdef READ_TO_BUF
    n = fileio_scdc_read_buf(fio, noffset, nbuf, nleft);

    nbuf += n;
    nleft -= n;
    noffset += n;

    TRACE_F_RD_BODY("fileio_scdc_read: nleft: %" scdcint_fmt ", noffset: %" scdcint_fmt, nleft, noffset);
#endif
  }
#endif /* !FILEIO_SCDC_SYNC_READ */

do_return:
  if (ret != FILEIO_SCDC_FAILURE)
  {
    ret = size - nleft;

    if (nleft > 0)
    {
      fio->eof = 1;
      fio->eof_offset = noffset;
    }

    fio->offset = noffset;
  }

#if FILEIO_SCDC_TRACE_PERF_RD
  t = z_time_wtime() - t;
  TRACE_F_RD_PERF("fileio_scdc_read: time: %f, size: %" scdcint_fmt ", rate: %f", t, ret, ret / t);
#endif

  TRACE_F_RD_OUTRO("fileio_scdc_read: return: %" scdcint_fmt, ret);

  return ret;
}


scdcint_t fileio_scdc_read_begin(fileio_scdc_t *fio, scdcint_t size, void **buf, scdcint_t *offset)
{
  TRACE_F_RD_INTRO("fileio_scdc_read_begin: fio: %" dptr_fmt ", size: %" scdcint_fmt ", buf: %" dptr_fmt ", offset: %" dptr_fmt, DPTR(fio), size, DPTR(buf), DPTR(offset));

  scdcint_t ret = FILEIO_SCDC_SUCCESS;

  if (!fio->read)
  {
    ret = FILEIO_SCDC_FAILURE;
    goto do_return;
  }

  /*
    - 
  */
  

do_return:
  if (ret != FILEIO_SCDC_FAILURE)
  {
/*    ret = ???;*/
  }

  TRACE_F_RD_OUTRO("fileio_scdc_read_begin: return: %" scdcint_fmt, ret);

  return ret;
}


scdcint_t fileio_scdc_write(fileio_scdc_t *fio, const void *buf, scdcint_t size)
{
  TRACE_F_WR_INTRO("fileio_scdc_write: fio: %" dptr_fmt ", buf: %" dptr_fmt ", size: %" scdcint_fmt, DPTR(fio), DPTR(buf), size);

#if FILEIO_SCDC_TRACE_PERF_WR
  double t = z_time_wtime();
#endif

  scdcint_t ret = FILEIO_SCDC_SUCCESS;

  if (!fio->write)
  {
    ret = FILEIO_SCDC_FAILURE;
    goto do_return;
  }

  const char *nbuf = buf;
  scdcint_t nleft = size;
  scdcint_t noffset = fio->offset;

#if FILEIO_SCDC_APPEND
  if (fio->append) noffset = 0;
#endif

  TRACE_F_WR_BODY("fileio_scdc_write: nleft: %" scdcint_fmt ", noffset: %" scdcint_fmt, nleft, noffset);

  if (nleft <= 0) goto do_return;

#if !FILEIO_SCDC_SYNC_WRITE
  if (!fio->sync)
  {
    if (fio->buf.offset >= 0)
    {
      scdcint_t o = noffset - fio->buf.offset;
      scdcint_t s = fio->buf.max_size - o;

      /* recognize current modifications */
      if (fio->buf.mod_begin < fio->buf.mod_end)
      {
# if FILEIO_SCDC_APPEND
        if (fio->append)
        {
          o = fio->buf.mod_end;
          s = fio->buf.max_size - o;

        } else
# endif /* FILEIO_SCDC_APPEND */
        {
          /* if not overlapping with current modifications, then write nothing to buffer */
          if (o > fio->buf.mod_end || o + s < fio->buf.mod_begin) s = 0;
        }
      }

      /* write inside the buffer? */
      if (o >= 0 && s > 0)
      {
        scdcint_t n = z_min(s, nleft);

        TRACE_F_WR_BODY("fileio_scdc_write: put %" scdcint_fmt " to buffer", n);

        memcpy(fio->buf.ptr + o, nbuf, n);

        fio->buf.mod_begin = z_min(o, fio->buf.mod_begin);
        fio->buf.mod_end = z_max(o + n, fio->buf.mod_end);
        fio->buf.size = z_max(fio->buf.size, fio->buf.mod_end);

        nbuf += n;
        nleft -= n;
        noffset += n;
      }
    }

    TRACE_F_WR_BODY("fileio_scdc_write: nleft: %" scdcint_fmt ", noffset: %" scdcint_fmt, nleft, noffset);

    if (nleft <= 0) goto do_return;

    /* init before use */
    fileio_scdc_buf_init(&fio->buf);

    /* flush modified buffer before writing */
    fileio_scdc_flush_buf(fio);

    if (nleft < fio->buf.max_write)
    {
      TRACE_F_WR_BODY("fileio_scdc_write: put %" scdcint_fmt " to buffer", nleft);

      scdcint_t n = z_min(nleft, fio->buf.max_size);

      if (n < nleft)
      {
        TRACE_F_WR_BODY("fileio_scdc_write: buffer of size %" scdcint_fmt " is too small", n);

      } else
      {
        memcpy(fio->buf.ptr, nbuf, n);

        fio->buf.offset = noffset;
        fio->buf.mod_begin = 0;
        fio->buf.mod_end = n;
        fio->buf.size = n;

        nbuf += n;
        nleft -= n;
        noffset += n;
      }
    }

    TRACE_F_WR_BODY("fileio_scdc_write: nleft: %" scdcint_fmt ", noffset: %" scdcint_fmt, nleft, noffset);

    if (nleft <= 0) goto do_return;
  }
#endif /* FILEIO_SCDC_SYNC_WRITE */

  scdcint_t n = nleft;

  TRACE_F_WR_BODY("fileio_scdc_write: put %" scdcint_fmt " to scdc", n);

  scdc_dataset_input_t input;

  scdc_dataset_input_unset(&input);
  input.buf_size = n;
  input.buf = (void *) nbuf;
  input.current_size = n;
  input.total_size = n;
  input.total_size_given = SCDC_DATASET_INOUT_TOTAL_SIZE_GIVEN_EXACT;

  char cmd[256];
  sprintf(cmd, "put %" scdcint_fmt "%c:%" scdcint_fmt,
#if FILEIO_SCDC_APPEND
    (fio->append)?0:
#endif
      noffset,
#if FILEIO_SCDC_APPEND
    (fio->append)?'B':
#endif
      'F',
    n);

#if FILEIO_SCDC_TRACE_SCDC_WR
  double cmd_t = z_time_wtime();
  scdcint_t cmd_size = input.current_size;
#endif

  if (scdc_dataset_cmd(fio->dataset, cmd, &input, NULL) != SCDC_SUCCESS)
  {
    ret = FILEIO_SCDC_FAILURE;
    goto do_return;
  }

#if FILEIO_SCDC_TRACE_SCDC_WR
  cmd_t = z_time_wtime() - cmd_t;
  TRACE_F_WR_SCDC("fileio_scdc_write: cmd time: %f, size: %" scdcint_fmt ", rate: %f", cmd_t, cmd_size, cmd_size / cmd_t);
#endif

  nbuf += n;
  nleft -= n;
  noffset += n;

  TRACE_F_WR_BODY("fileio_scdc_write: nleft: %" scdcint_fmt ", noffset: %" scdcint_fmt, nleft, noffset);

do_return:
  if (ret != FILEIO_SCDC_FAILURE)
  {
    ret = size - nleft;

#if FILEIO_SCDC_APPEND
    if (!fio->append)
#endif
    {
      fio->offset = noffset;
    }
  }

#if FILEIO_SCDC_TRACE_PERF_WR
  t = z_time_wtime() - t;
  TRACE_F_RD_PERF("fileio_scdc_write: time: %f, size: %" scdcint_fmt ", rate: %f", t, ret, ret / t);
#endif

  TRACE_F_WR_OUTRO("fileio_scdc_write: return: %" scdcint_fmt, ret);

  return ret;
}


scdcint_t fileio_scdc_sync(fileio_scdc_t *fio)
{
  TRACE_F_CTRL_INTRO("fileio_scdc_sync: fio: %" dptr_fmt, DPTR(fio));

  scdcint_t ret = FILEIO_SCDC_SUCCESS;

#if !FILEIO_SCDC_SYNC
  ret = fileio_scdc_flush_buf(fio);
#endif

  if (scdc_dataset_cmd(fio->dataset, "sync", NULL, NULL) != SCDC_SUCCESS)
  {
    ret = FILEIO_SCDC_FAILURE;
    goto do_return;
  }

do_return:
  TRACE_F_CTRL_OUTRO("fileio_scdc_sync: return: %" scdcint_fmt, ret);

  return ret;
}


scdcint_t fileio_scdc_seek(fileio_scdc_t *fio, scdcint_t offset, int whence, int write)
{
  TRACE_F_CTRL_INTRO("fileio_scdc_seek: fio: %" dptr_fmt ", offset: %" scdcint_fmt ", whence: %d, write: %d", DPTR(fio), offset, whence, write);

  scdcint_t ret = FILEIO_SCDC_SUCCESS;

#if FILEIO_SCDC_APPEND
  if (write && fio->append)
  {
    ret = FILEIO_SCDC_FAILURE;
    goto do_return;
  }
#endif

  if (whence == SEEK_END)
  {
    char buf[32];
    scdc_dataset_output_t output;

    scdc_dataset_output_unset(&output);
    output.buf_size = sizeof(buf) - 1;
    output.buf = buf;

    if (scdc_dataset_cmd(fio->dataset, "ls", NULL, &output) != SCDC_SUCCESS)
    {
      ret = FILEIO_SCDC_FAILURE;
      goto do_return;
    }

    z_snscanf(output.buf, output.current_size, "f:%" scdcint_fmt "|", &fio->offset);
    whence = SEEK_CUR;

    while (output.next) output.next(&output);
  }

  switch (whence)
  {
    case SEEK_SET:
      fio->offset = z_max(0, offset);
      ret = fio->offset;
      break;
    case SEEK_CUR:
      fio->offset += offset;
      ret = fio->offset;
      break;
    default:
      ret = FILEIO_SCDC_FAILURE;
      break;
  }

  if (ret != FILEIO_SCDC_FAILURE) fio->eof = 0;

do_return:
  TRACE_F_CTRL_OUTRO("fileio_scdc_seek: return: %" scdcint_fmt, ret);

  return ret;
}


static void ls2stat(scdc_dataset_output_t *output, fileio_scdc_stat_t *buf)
{
  z_snscanf(output->buf, output->current_size, "f:%" scdcint_fmt "|", &buf->size);

  while (output->next) output->next(output);
}


scdcint_t fileio_scdc_stat(fileio_scdc_t *fio, fileio_scdc_stat_t *buf, int *error)
{
  TRACE_F_CTRL_INTRO("fileio_scdc_stat: fio: %" dptr_fmt ", buf: %" dptr_fmt ", error: %" dptr_fmt, DPTR(fio), DPTR(buf), DPTR(error));

  scdcint_t ret = FILEIO_SCDC_SUCCESS;

  memset(buf, 0, sizeof(fileio_scdc_stat_t));

  char output_buf[32];
  scdc_dataset_output_t output;

  scdc_dataset_output_unset(&output);
  output.buf_size = sizeof(output_buf) - 1;
  output.buf = output_buf;

  if (scdc_dataset_cmd(SCDC_DATASET_NULL, "ls", NULL, &output) != SCDC_SUCCESS)
  {
    ret = FILEIO_SCDC_FAILURE;
    goto do_return;
  }

  ls2stat(&output, buf);

do_return:
  TRACE_F_CTRL_OUTRO("fileio_scdc_stat: return: %" scdcint_fmt, ret);

  return ret;
}


scdcint_t fileio_scdc_stat_path(const char *path, fileio_scdc_stat_t *buf, int *error)
{
  TRACE_F_CTRL_INTRO("fileio_scdc_path_stat: path: '%s', buf: %" dptr_fmt ", error: %" dptr_fmt, path, DPTR(buf), DPTR(error));

  scdcint_t ret = FILEIO_SCDC_SUCCESS;

  memset(buf, 0, sizeof(fileio_scdc_stat_t));

  fileio_scdc_init();

  char cmd[256];
  sprintf(cmd, "%s ls", path);

  char output_buf[32];
  scdc_dataset_output_t output;

  scdc_dataset_output_unset(&output);
  output.buf_size = sizeof(output_buf) - 1;
  output.buf = output_buf;

  if (scdc_dataset_cmd(SCDC_DATASET_NULL, cmd, NULL, &output) != SCDC_SUCCESS)
  {
    ret = FILEIO_SCDC_FAILURE;
    goto do_return;
  }

  ls2stat(&output, buf);

do_return:
  fileio_scdc_release();

  TRACE_F_CTRL_OUTRO("fileio_scdc_path_stat: return: %" scdcint_fmt, ret);

  return ret;
}


scdcint_t fileio_scdc_remove(const char *path, int *error)
{
  TRACE_F_CTRL_INTRO("fileio_scdc_remove: path: '%s', error: %" dptr_fmt, path, DPTR(error));

  scdcint_t ret = FILEIO_SCDC_SUCCESS;

  fileio_scdc_init();

  char cmd[256];
  sprintf(cmd, "%s rm", path);
  if (scdc_dataset_cmd(SCDC_DATASET_NULL, cmd, NULL, NULL) != SCDC_SUCCESS)
  {
    ret = FILEIO_SCDC_FAILURE;
    *error = EREMOTEIO;
  }

  fileio_scdc_release();

  TRACE_F_CTRL_OUTRO("fileio_scdc_remove: return: %" scdcint_fmt, ret);

  return ret;
}


scdcint_t fileio_scdc_move(const char *oldpath, const char *newpath, int *error)
{
  TRACE_F_CTRL_INTRO("fileio_scdc_move: oldpath: '%s', newpath: '%s', error: %" dptr_fmt, oldpath, newpath, DPTR(error));

  scdcint_t ret = FILEIO_SCDC_SUCCESS;

  fileio_scdc_init();

  int oldpath_scdc = fileio_scdc_valid(oldpath);
  int newpath_scdc = fileio_scdc_valid(newpath);

  if (oldpath_scdc && !newpath_scdc)
  {
    /* get oldpath to file newpath */

    scdc_dataset_output_t _op, *op = &_op;
    op = scdc_dataset_output_create(op, "file", newpath);
    if (op == NULL)
    {
      ret = FILEIO_SCDC_FAILURE;
      *error = ENOENT;
      goto do_release;
    }

    char cmd[256];
    sprintf(cmd, "%s get", oldpath);
    if (scdc_dataset_cmd(SCDC_DATASET_NULL, cmd, NULL, op) != SCDC_SUCCESS)
    {
      ret = FILEIO_SCDC_FAILURE;
      *error = EREMOTEIO;
    }

    scdc_dataset_output_destroy(op);

    if (ret == FILEIO_SCDC_FAILURE) goto do_release;

    /* remove oldpath */

    sprintf(cmd, "%s rm", oldpath);
    if (scdc_dataset_cmd(SCDC_DATASET_NULL, cmd, NULL, NULL) != SCDC_SUCCESS)
    {
      ret = FILEIO_SCDC_FAILURE;
      *error = EREMOTEIO;
    }

  } else if (!oldpath_scdc && newpath_scdc)
  {
    /* put file oldpath to newpath */

    scdc_dataset_input_t _ip, *ip = &_ip;
    ip = scdc_dataset_input_create(ip, "file", oldpath);
    if (ip == NULL)
    {
      ret = FILEIO_SCDC_FAILURE;
      *error = ENOENT;
      goto do_release;
    }

    char *f = strrchr(newpath, '/');
    if (f == NULL)
    {
      ret = FILEIO_SCDC_FAILURE;
      *error = ENOENT;
    }

    char cmd[256];
    sprintf(cmd, "%.*s put %s", (int) (f - newpath), newpath, f + 1);
    if (scdc_dataset_cmd(SCDC_DATASET_NULL, cmd, ip, NULL) != SCDC_SUCCESS)
    {
      ret = FILEIO_SCDC_FAILURE;
      *error = EREMOTEIO;
    }

    scdc_dataset_input_destroy(ip);

    if (ret == FILEIO_SCDC_FAILURE) goto do_release;

    /* remove oldpath */
    remove(oldpath);

  } else if (oldpath_scdc && newpath_scdc)
  {
    /* TODO: get oldpath and redirect output to input of put to newpath */

    ret = FILEIO_SCDC_FAILURE;
    *error = EFAULT;

  } else
  {
    ret = FILEIO_SCDC_FAILURE;
    *error = EFAULT;
  }

do_release:
  fileio_scdc_release();

  TRACE_F_CTRL_OUTRO("fileio_scdc_remove: return: %" scdcint_fmt, ret);

  return ret;
}
