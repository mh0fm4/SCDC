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


#ifndef __LIBFILEIO_SCDC_ORIGS_H__
#define __LIBFILEIO_SCDC_ORIGS_H__


#include <stdio.h>
#include <dlfcn.h>

typedef int fprintf_f(FILE *stream, const char *format, ...);
typedef int vfprintf_f(FILE *stream, const char *format, va_list ap);

#if LIBFILEIO_SCDC_PROVIDE_STAT
typedef int access_f(const char *pathname, int mode);
typedef int stat_f(const char *pathname, struct stat *buf);
typedef int lstat_f(const char *pathname, struct stat *buf);
#endif /* LIBFILEIO_SCDC_PROVIDE_STAT */

#if LIBFILEIO_SCDC_PROVIDE_FD
typedef int open_f(const char *pathname, int flags, ...);
typedef int creat_f(const char *pathname, mode_t mode);
typedef int close_f(int fd);
typedef ssize_t read_f(int fd, void *buf, size_t count);
typedef ssize_t write_f(int fd, const void *buf, size_t count);
typedef ssize_t pread_f(int fd, void *buf, size_t count, off_t offset);
typedef ssize_t pwrite_f(int fd, const void *buf, size_t count, off_t offset);
typedef ssize_t readv_f(int fd, const struct iovec *iov, int iovcnt);
typedef ssize_t writev_f(int fd, const struct iovec *iov, int iovcnt);
typedef ssize_t preadv_f(int fd, const struct iovec *iov, int iovcnt, off_t offset);
typedef ssize_t pwritev_f(int fd, const struct iovec *iov, int iovcnt, off_t offset);
typedef int fsync_f(int fd);
typedef off_t lseek_f(int fd, off_t offset, int whence);
typedef int fstat_f(int fd, struct stat *buf);
/* ftruncate, fchmod, fchown */
# ifdef LIBFILEIO_SCDC_PROVIDE_64
typedef open_f open64_f;
typedef creat_f creat64_f;
typedef off64_t lseek64_f(int fd, off64_t offset, int whence);
typedef ssize_t pread64_f(int fd, void *buf, size_t count, off64_t offset);
typedef ssize_t pwrite64_f(int fd, const void *buf, size_t count, off64_t offset);
typedef ssize_t preadv64_f(int fd, const struct iovec *iov, int iovcnt, off64_t offset);
typedef ssize_t pwritev64_f(int fd, const struct iovec *iov, int iovcnt, off64_t offset);
typedef int fstat64_f(int fd, struct stat64 *buf);
# endif /* LIBFILEIO_SCDC_PROVIDE_64 */
#endif /* LIBFILEIO_SCDC_PROVIDE_FD */

#if LIBFILEIO_SCDC_PROVIDE_FILE
typedef FILE *fopen_f(const char *path, const char *mode);
typedef FILE *fdopen_f(int fd, const char *mode);
typedef FILE *freopen_f(const char *path, const char *mode, FILE *stream);
typedef int fclose_f(FILE *stream);
typedef size_t fread_f(void *ptr, size_t size, size_t nmemb, FILE *stream);
typedef size_t fwrite_f(const void *ptr, size_t size, size_t nmemb, FILE *stream);
typedef int fflush_f(FILE *stream);
typedef int fseek_f(FILE *stream, long offset, int whence);
typedef long ftell_f(FILE *stream);
typedef int fseeko_f(FILE *stream, off_t offset, int whence);
typedef off_t ftello_f(FILE *stream);
/*typedef int fgetpos_f(FILE *stream, fpos_t *pos);
typedef int fsetpos_f(FILE *stream, const fpos_t *pos);*/
typedef void rewind_f(FILE *stream);
typedef void clearerr_f(FILE *stream);
typedef int feof_f(FILE *stream);
typedef int ferror_f(FILE *stream);
typedef int fileno_f(FILE *stream);
typedef int fgetc_f(FILE *stream);
#if LIBFILEIO_SCDC_PROVIDE_FILE_
typedef int getc_f(FILE *stream);
#endif
/*typedef int ungetc_f(int c, FILE *stream);*/
typedef char *fgets_f(char *s, int size, FILE *stream);
typedef int fputc_f(int c, FILE *stream);
#if LIBFILEIO_SCDC_PROVIDE_FILE_
typedef int putc_f(int c, FILE *stream);
#endif
typedef int fputs_f(const char *s, FILE *stream);
typedef int vfscanf_f(FILE *stream, const char *format, va_list ap);
# ifdef LIBFILEIO_SCDC_PROVIDE_64
typedef int fseeko64_f(FILE *stream, off64_t offset, int whence);
typedef off64_t ftello64_f(FILE *stream);
/*typedef int fgetpos64_f(FILE *stream, fpos64_t *pos);
typedef int fsetpos64_f(FILE *stream, const fpos64_t *pos);*/
# endif /* LIBFILEIO_SCDC_PROVIDE_64 */
#endif /* LIBFILEIO_SCDC_PROVIDE_FILE */

#if LIBFILEIO_SCDC_PROVIDE_FS
typedef int stat_f(const char *pathname, struct stat *buf);
typedef int lstat_f(const char *pathname, struct stat *buf);
# ifdef LIBFILEIO_SCDC_PROVIDE_64
typedef int stat64_f(const char *pathname, struct stat64 *buf);
typedef int lstat64_f(const char *pathname, struct stat64 *buf);
# endif /* LIBFILEIO_SCDC_PROVIDE_64 */
/* copy? */
typedef int remove_f(const char *pathname);
typedef int unlink_f(const char *pathname);
typedef int rename_f(const char *oldpath, const char *newpath);
/* truncate, chmod, chown, lchown */
#endif /* LIBFILEIO_SCDC_PROVIDE_FS */

static struct
{
  fprintf_f *fprintf;
  vfprintf_f *vfprintf;

#if LIBFILEIO_SCDC_PROVIDE_FD
  open_f *open;
  creat_f *creat;
  close_f *close;
  read_f *read;
  write_f *write;
/*
pread_f
pwrite_f
readv_f
writev_f
preadv_f
pwritev_f
*/
  fsync_f *fsync;
  lseek_f *lseek;
  fstat_f *fstat;
# ifdef LIBFILEIO_SCDC_PROVIDE_64
  open64_f *open64;
  creat64_f *creat64;
  lseek64_f *lseek64;
/*
  pread64_f
  pwrite64_f
  preadv64_f
  pwritev64_f
*/
  fstat64_f *fstat64;
# endif /* LIBFILEIO_SCDC_PROVIDE_64 */
#endif /* LIBFILEIO_SCDC_PROVIDE_FD */

#if LIBFILEIO_SCDC_PROVIDE_FILE
  fopen_f *fopen;
  fdopen_f *fdopen;
  freopen_f *freopen;
  fclose_f *fclose;
  fread_f *fread;
  fwrite_f *fwrite;
  fflush_f *fflush;
  fseek_f *fseek;
  ftell_f *ftell;
  fseeko_f *fseeko;
  ftello_f *ftello;
/*  fgetpos_f *fgetpos;
  fsetpos_f *fsetpos;*/
  rewind_f *rewind;
  clearerr_f *clearerr;
  feof_f *feof;
  ferror_f *ferror;
  fileno_f *fileno;
  fgetc_f *fgetc;
#if LIBFILEIO_SCDC_PROVIDE_FILE_
  getc_f *getc_;
#endif
/*  ungetc_f *ungetc;*/
  fgets_f *fgets;
  fputc_f *fputc;
#if LIBFILEIO_SCDC_PROVIDE_FILE_
  putc_f *putc_;
#endif
  fputs_f *fputs;
  vfscanf_f *vfscanf;
# ifdef LIBFILEIO_SCDC_PROVIDE_64
  fseeko64_f *fseeko64;
  ftello64_f *ftello64;
/*  fgetpos64_f *fgetpos64;
  fsetpos64_f *fsetpos64;*/
# endif /* LIBFILEIO_SCDC_PROVIDE_64 */
#endif /* LIBFILEIO_SCDC_PROVIDE_FILE */

#if LIBFILEIO_SCDC_PROVIDE_FS
  stat_f *stat;
  lstat_f *lstat;
# ifdef LIBFILEIO_SCDC_PROVIDE_64
  stat64_f *stat64;
  lstat64_f *lstat64;
# endif /* LIBFILEIO_SCDC_PROVIDE_64 */
  remove_f *remove;
  unlink_f *unlink;
  rename_f *rename;
#endif /* LIBFILEIO_SCDC_PROVIDE_FS */

  int this_is_not_an_empty_struct;

} libfileio_scdc_originals = { NULL };


#define LIBFILEIO_SCDC_ORIGS_INIT()        Z_MOP(int i; for (i = 0; i < sizeof(libfileio_scdc_originals) / sizeof(void *); ++i) ((void **) &libfileio_scdc_originals)[i] = NULL;)
#define LIBFILEIO_SCDC_ORIGS_RELEASE()     Z_NOP()
#define LIBFILEIO_SCDC_ORIGS_F_INIT(_f_)   Z_MOP(if (libfileio_scdc_originals._f_ == NULL) *((void **) &libfileio_scdc_originals._f_) = dlsym(RTLD_NEXT, #_f_);)
#define LIBFILEIO_SCDC_ORIGS_F(_f_)        libfileio_scdc_originals._f_
#define LIBFILEIO_SCDC_ORIGS_F_INIT_(_f_)  Z_MOP(if (libfileio_scdc_originals.Z_CONCAT(_f_, _) == NULL) *((void **) &libfileio_scdc_originals.Z_CONCAT(_f_, _)) = dlsym(RTLD_NEXT, #_f_);)
#define LIBFILEIO_SCDC_ORIGS_F_(_f_)       libfileio_scdc_originals.Z_CONCAT(_f_, _)


#endif /* __LIBFILEIO_SCDC_ORIGS_H__ */
