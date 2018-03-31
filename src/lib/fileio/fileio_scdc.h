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


#ifndef __FILEIO_SCDC_H__
#define __FILEIO_SCDC_H__


#include "scdc.h"


#define FILEIO_SCDC_BUF_ENV          1
#if 0
#define FILEIO_SCDC_BUF              0
#else
#define FILEIO_SCDC_BUF              1024*1024
#endif
#define FILEIO_SCDC_BUF_READ_CHUNK   1024*1024
#define FILEIO_SCDC_BUF_WRITE_CHUNK  1024*1024

#define FILEIO_SCDC_SYNC_READ        (!(FILEIO_SCDC_BUF && FILEIO_SCDC_BUF_READ_CHUNK))
#define FILEIO_SCDC_SYNC_WRITE       (!(FILEIO_SCDC_BUF && FILEIO_SCDC_BUF_WRITE_CHUNK))
#define FILEIO_SCDC_SYNC             (FILEIO_SCDC_SYNC_READ && FILEIO_SCDC_SYNC_WRITE)

#define FILEIO_SCDC_APPEND     1

typedef struct
{
  scdcint_t max_size, max_read, max_write;
  char *ptr;

  scdcint_t offset, size;
#if FILEIO_SCDC_BUF_WRITE_CHUNK
  scdcint_t mod_begin, mod_end;
#endif

} fileio_scdc_buf_t;

typedef struct
{
  int eof, sync, read, write;

  scdcint_t offset, eof_offset;
#if FILEIO_SCDC_APPEND
  int append;
#endif
  scdc_dataset_t dataset;

#if FILEIO_SCDC_BUF
  fileio_scdc_buf_t buf;
#endif

} fileio_scdc_t;


#define FILEIO_SCDC_SUCCESS  0
#define FILEIO_SCDC_FAILURE  -1


scdcint_t fileio_scdc_local(const char *base, const char *path);

scdcint_t fileio_scdc_valid(const char *path);

scdcint_t fileio_scdc_open(fileio_scdc_t *fio, const char *path, int flags, int *error);
scdcint_t fileio_scdc_reopen(fileio_scdc_t *fio, const char *path, int flags, int *error);
scdcint_t fileio_scdc_close(fileio_scdc_t *fio);

scdcint_t fileio_scdc_read(fileio_scdc_t *fio, void *buf, scdcint_t size, scdcint_t keep_buf);
scdcint_t fileio_scdc_write(fileio_scdc_t *fio, const void *buf, scdcint_t size);

scdcint_t fileio_scdc_sync(fileio_scdc_t *fio);
scdcint_t fileio_scdc_seek(fileio_scdc_t *fio, scdcint_t offset, int whence, int write);

typedef struct
{
  scdcint_t size;

} fileio_scdc_stat_t;

scdcint_t fileio_scdc_stat(fileio_scdc_t *fio, fileio_scdc_stat_t *buf, int *error);

scdcint_t fileio_scdc_stat_path(const char *path, fileio_scdc_stat_t *buf, int *error);
scdcint_t fileio_scdc_remove(const char *path, int *error);
scdcint_t fileio_scdc_move(const char *oldpath, const char *newpath, int *error);


#endif /* __FILEIO_SCDC_H__ */
