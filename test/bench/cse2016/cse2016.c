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


#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>
       
#include "scdc.h"
#include "z_pack.h"


#define BENCH_BUF_SIZE    1*1024*1024
/*#define BENCH_BW_SIZE     1024*1024*1024
#define BENCH_LT_SIZE     1024*1024*1024*/
#define BENCH_BW_SIZE     1024*1024*16
#define BENCH_LT_SIZE     1024*1024*16
#define BENCH_LT_NRW      1024

#define BENCH_BW_NREP  1
#define BENCH_LT_NREP  1

#define BENCH_TMIN  1


typedef long long dsint_t;
#define dsint_fmt   "lld"
#define DSINT(_x_)  ((dsint_t) _x_)

typedef unsigned long long duint_t;
#define duint_fmt   "llu"
#define DUINT(_x_)  ((duint_t) _x_)


static size_t strlen_safe(const char *s)
{
  if (s) return strlen(s);

  return 0;
}


#define MAX_STORES  16

static int nstores = 0;
static scdc_dataprov_t stores[MAX_STORES];

void add_store(const char *base, const char *path)
{
  if (nstores >= MAX_STORES) return;

  stores[nstores] = scdc_dataprov_open(base, "fs:access", path);

  if (stores[nstores] != SCDC_DATAPROV_NULL) ++nstores;
  else fprintf(stderr, "opening store '%s' with path '%s' failed\n", base, path);
}


void add_relay(const char *base, const char *relbase, const char *reltarget)
{
  if (nstores >= MAX_STORES) return;

  stores[nstores] = scdc_dataprov_open(base, "relay");

  if (stores[nstores] != SCDC_DATAPROV_NULL)
  {
    char cmd[256];

    sprintf(cmd, "scdc:/%s/CONFIG put relay %s %s", base, relbase, reltarget);

    if (scdc_dataset_cmd(SCDC_DATASET_NULL, cmd, NULL, NULL) != SCDC_SUCCESS)
      fprintf(stderr, "adding relay '%s' with base '%s' and target '%s' failed\n", base, relbase, reltarget);

    ++nstores;

  } else fprintf(stderr, "opening relay '%s' failed\n", base);
}


void release_stores()
{
  while (nstores > 0)
  {
    scdc_dataprov_close(stores[nstores]);
    --nstores;
  }
}


#define MAX_PORTS  16

static int nports = 0;
static scdc_nodeport_t ports[MAX_PORTS];

void add_port(const char *port)
{
  if (nports >= MAX_STORES) return;

  if (strcmp(port, "uds") == 0) ports[nports] = scdc_nodeport_open("uds");
  else if (strcmp(port, "tcp") == 0) ports[nports] = scdc_nodeport_open("tcp");
/*  else if (strcmp(port, "mpi") == 0) ports[nports] = scdc_nodeport_open("mpi:publ", "hpcs");*/
  else if (strcmp(port, "mpi") == 0) ports[nports] = scdc_nodeport_open("mpi:world");

  if (ports[nports] != SCDC_NODEPORT_NULL) ++nports;
  else fprintf(stderr, "opening port '%s' failed\n", port);
}


void run_ports()
{
  int i;

  for (i = 0; i < nports; ++i) scdc_nodeport_start(ports[i], SCDC_NODEPORT_START_ASYNC_UNTIL_CANCEL);

  if (nports > 0)
  {
    printf("running %d ports: ENTER to exit", nports);
    getchar();
    printf("quiting %d ports\n", nports);
  }

  for (i = 0; i < nports; ++i) scdc_nodeport_stop(ports[i]);
}


void release_ports()
{
  while (nports > 0)
  {
    scdc_nodeport_close(ports[nports]);
    --nports;
  }
}



void set_libfileio_scdc_buf_size(dsint_t n)
{
  char s[32];
  sprintf(s, "%" dsint_fmt, n);

  setenv("LIBFILEIO_SCDC_BUFFER_SIZE", s, 1);
  setenv("LIBFILEIO_SCDC_BUFFER_SIZE_READ", s, 1);
  setenv("LIBFILEIO_SCDC_BUFFER_SIZE_WRITE", s, 1);
}


void check_read_target(const char *target)
{
}


void check_write_target(const char *target)
{
/*  int fd = open(target, O_WRONLY|O_CREAT, S_IRUSR|S_IWUSR);
  int e = fallocate(fd, 0, 0, size_max);
  close(fd);*/

  z_fs_rm(target);
}


void bench_bw_rd(const char *target, int sync, dsint_t size_min, dsint_t size_max)
{
  const dsint_t size_f = 10;

  dsint_t buf_size = z_min(size_max, BENCH_BUF_SIZE);
  char *buf = malloc(buf_size);

  memset(buf, 'X', buf_size);
  buf[buf_size - 1] = '\0';

  dsint_t i, l = 0;
  for (i = size_min; i <= size_max || l < size_max; i *= size_f)
  {
    i = l = z_min(i, size_max);

    dsint_t nrep = BENCH_BW_NREP;
    dsint_t n, m, j;

    double t0 = z_time_wtime();

#if BENCH_TMIN
do_bench:
#endif
    for (j = 0; j < nrep; ++j)
    {
      FILE *f = fopen(target, "r");

      if (!f) break;

      n = m = 0;

      while (n < i)
      {
        dsint_t nin = z_min(i - n, buf_size);

        dsint_t nout = fread(buf, 1, nin, f);

        ++m;

        n += nout;

        if (nout < nin) break;
      }

      if (sync) fsync(fileno(f));

      fclose(f);
    }

    double t1 = z_time_wtime();
    double t = t1 - t0;

#if BENCH_TMIN
    if (t < BENCH_TMIN)
    {
      nrep = ceil(1.0 / t * nrep);
      goto do_bench;
    }
#endif

    printf("%" dsint_fmt "  %" dsint_fmt "  %f  %f  %e%s\n", i, n, t / nrep, n * 1e-6 / t * nrep, t / m / nrep, (j < nrep)?"  #error opening file":"");
  }

  free(buf);
}


void bench_bw_wr(const char *target, int sync, dsint_t size_min, dsint_t size_max)
{
  const dsint_t size_f = 10;

  dsint_t buf_size = z_min(size_max, BENCH_BUF_SIZE);
  char *buf = malloc(buf_size);

  memset(buf, 'X', buf_size);
  buf[buf_size - 1] = '\0';

  dsint_t i, l = 0;
  for (i = size_min; i <= size_max || l < size_max; i *= size_f)
  {
    i = l = z_min(i, size_max);

    dsint_t nrep = BENCH_BW_NREP;
    dsint_t n, m, j;

    check_write_target(target);

    double t0 = z_time_wtime();

#if BENCH_TMIN
do_bench:
#endif
    for (j = 0; j < nrep; ++j)
    {
      FILE *f = fopen(target, "w");

      if (!f) break;

      n = m = 0;

      while (n < i)
      {
        dsint_t nin = z_min(i - n, buf_size);

        dsint_t nout = fwrite(buf, 1, nin, f);

        ++m;

        n += nout;

        if (nout < nin) break;
      }

      if (sync) fsync(fileno(f));

      fclose(f);
    }

    double t1 = z_time_wtime();
    double t = t1 - t0;

#if BENCH_TMIN
    if (t < BENCH_TMIN)
    {
      nrep = ceil(1.0 / t * nrep);
      goto do_bench;
    }
#endif

    printf("%" dsint_fmt "  %" dsint_fmt "  %f  %f  %e%s\n", i, n, t / nrep, n * 1e-6 / t * nrep, t / m / nrep, (j < nrep)?"  #error opening file":"");
  }

  free(buf);
}


void bench_lt_rd(const char *target, int sync, dsint_t size_max, dsint_t nrd_min, dsint_t nrd_max, int rand)
{
  const dsint_t nrd_f = 10;

  if (rand) srandom(2501);

  dsint_t pos = 0;

  dsint_t i, l = 0;
  for (i = nrd_min; i <= nrd_max || l < nrd_max; i *= nrd_f)
  {
    i = l = z_min(i, nrd_max);

    dsint_t nrep = BENCH_LT_NREP;
    dsint_t n, m, j;

    double t0 = z_time_wtime();

#if BENCH_TMIN
do_bench:
#endif
    for (j = 0; j < nrep; ++j)
    {
      FILE *f = fopen(target, "r");

      if (!f) break;

      n = m = 0;

      while (n < i)
      {
        if (rand)
        {
          pos = (pos + random()) % size_max;
          fseeko(f, pos, SEEK_SET);
        }

        int c = fgetc(f);

        ++m;

        if (c == EOF) break;

        n += 1;
      }

      if (sync) fsync(fileno(f));

      fclose(f);
    }

    double t1 = z_time_wtime();
    double t = t1 - t0;

#if BENCH_TMIN
    if (t < BENCH_TMIN)
    {
      nrep = ceil(1.0 / t * nrep);
      goto do_bench;
    }
#endif

    printf("%" dsint_fmt "  %" dsint_fmt "  %f  %f  %e%s\n", i, n, t / nrep, n * 1e-6 / t * nrep, t / m / nrep, (j < nrep)?"  #error opening file":"");
  }
}


void bench_lt_wr(const char *target, int sync, dsint_t size_max, dsint_t nwr_min, dsint_t nwr_max, int rand)
{
  const dsint_t nwr_f = 10;

  if (rand) srandom(2501);

  dsint_t pos = 0;

  dsint_t i, l = 0;
  for (i = nwr_min; i <= nwr_max || l < nwr_max; i *= nwr_f)
  {
    i = l = z_min(i, nwr_max);

    dsint_t nrep = BENCH_LT_NREP;
    dsint_t n, m, j;

    check_write_target(target);

    double t0 = z_time_wtime();

#if BENCH_TMIN
do_bench:
#endif
    for (j = 0; j < nrep; ++j)
    {
      FILE *f = fopen(target, "w");

      if (!f) break;

      n = m = 0;

      while (n < i)
      {
        if (rand)
        {
          pos = (pos + random()) % size_max;
          fseeko(f, pos, SEEK_SET);
        }

        int c = fputc('X', f);

        ++m;

        if (c == EOF) break;

        ++n;
      }

      if (sync) fsync(fileno(f));

      fclose(f);
    }

    double t1 = z_time_wtime();
    double t = t1 - t0;

#if BENCH_TMIN
    if (t < BENCH_TMIN)
    {
      nrep = ceil(1.0 / t * nrep);
      goto do_bench;
    }
#endif

    printf("%" dsint_fmt "  %" dsint_fmt "  %f  %f  %e%s\n", i, n, t / nrep, n * 1e-6 / t * nrep, t / m / nrep, (j < nrep)?"  #error opening file":"");
  }
}


int main(int argc, char *argv[])
{
  --argc; ++argv;

  int mode = -1;

/*  printf("sizeof(long) = %zu\n", sizeof(long));
  printf("sizeof(off_t) = %zu\n", sizeof(off_t));
  printf("sizeof(fpos_t) = %zu\n", sizeof(fpos_t));*/

  const char *s;
  int n;
  dsint_t size = -1;
  dsint_t nrw = -1;

  scdc_init(SCDC_INIT_DEFAULT);

  while (argc > 0)
  {
    s = argv[0];
    n = strlen(s) - strlen_safe(strchr(s, ':'));

    --argc; ++argv;

/*    printf("#arg: %s\n", s);*/

    if (strcmp("-s", s) == 0)
    {
      add_store(argv[0], argv[1]);

      argc -= 2; argv += 2;

    } else if (strcmp("-r", s) == 0)
    {
      add_relay(argv[0], argv[1], argv[2]);

      argc -= 3; argv += 3;

    } else if (strcmp("-n", s) == 0)
    {
      add_port(argv[0]);

      --argc; ++argv;

    } else if (strcmp("-z", s) == 0)
    {
      size = strtol(argv[0], NULL, 10);

      --argc; ++argv;

    } else if (strcmp("-r", s) == 0)
    {
      nrw = strtol(argv[0], NULL, 10);

      --argc; ++argv;

    } else if (strncmp("bw", s, n) == 0)
    {
      mode = 100;

      if ((s = strchr(s, ':')))
      {
        ++s;
        n = strlen(s) - strlen_safe(strchr(s, ':'));

        if (strncmp("rd", s, n) == 0) mode += 10;
        else if (strncmp("wr", s, n) == 0) mode += 20;
        else if (strncmp("rw", s, n) == 0) mode += 30;

      } else mode += 30;

      break;

    } else if (strncmp("lt", s, n) == 0)
    {
      mode = 200;

      if ((s = strchr(s, ':')))
      {
        ++s;
        n = strlen(s) - strlen_safe(strchr(s, ':'));

        if (strncmp("rd", s, n) == 0) mode += 10;
        else if (strncmp("wr", s, n) == 0) mode += 20;
        else if (strncmp("rw", s, n) == 0) mode += 30;

        if ((s = strchr(s, ':')))
        {
          ++s;
          n = strlen(s) - strlen_safe(strchr(s, ':'));

          if (strncmp("cont", s, n) == 0) mode += 0;
          else if (strncmp("rand", s, n) == 0) mode += 1;

        } else mode += 0;

      } else mode += 30;

      break;

    } else
    {
      printf("error: unknown option '%s'\n", s);
    }
  }

  while (argc > 0)
  {
    const char *target = argv[0];
    int done = 0;

    printf("#target: '%s'\n", target);

    if (mode == 110 || mode == 130)
    {
      if (size < 0) size = BENCH_BW_SIZE;

      printf("#bandwidth read '%s', size: %" dsint_fmt "\n", target, size);
      bench_bw_rd(target, 1, size, size);
      done = 1;
    }

    if (mode == 120 || mode == 130)
    {
      if (size < 0) size = BENCH_BW_SIZE;

      printf("#bandwidth write '%s', size: %" dsint_fmt "\n", target, size);
      bench_bw_wr(target, 1, size, size);
      done = 1;
    }

    if (mode - (mode % 10) == 210 || mode - (mode % 10) == 230)
    {
      if (size < 0) size = BENCH_LT_SIZE;
      if (nrw < 0) nrw = BENCH_LT_NRW;

      printf("#latency read %s '%s', size: %" dsint_fmt ", nrw: %" dsint_fmt "\n", (mode % 10)?"rand":"cont", target, size, nrw);
      bench_lt_rd(target, 1, size, 1, nrw, mode % 10);
      done = 1;
    }

    if (mode - (mode % 10) == 220 || mode - (mode % 10) == 230)
    {
      if (size < 0) size = BENCH_LT_SIZE;
      if (nrw < 0) nrw = BENCH_LT_NRW;

      printf("#latency write %s '%s', size: %" dsint_fmt ", nrw: %" dsint_fmt "\n", (mode % 10)?"rand":"cont", target, size, nrw);
      bench_lt_wr(target, 1, size, 1, nrw, mode % 10);
      done = 1;
    }

    if (!done)
    {
      printf("#unknown mode %d\n", mode);
    }

    --argc; ++argv;
  }

  run_ports();

  release_ports();

  release_stores();

  scdc_release();

  return 0;
}
