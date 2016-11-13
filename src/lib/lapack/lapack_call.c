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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "z_pack.h"

#include "common.h"
#include "lapack_call.h"


#define PREFIX  "lapack_call: "


static int get_dense_matrix_size(int n0, int n1, int ld)
{
  /* column-major order: n0-1 full columns of size ld + one column of size n1 */

  return (n0 - 1) * ld + n1;
}


static int get_dense_vector_size(int n, int ld)
{
  /* column-major order: n-1 full rows of size ld + one row of size 1 */

  return (n - 1) * ld + 1;
}


#if HAVE_TRACE && HAVE_TRACE_DATA

static void print_dense_matrix_float(int n0, int n1, float *m, int ld)
{
  int i, j;
  for (i = 0; i < n0; ++i)
  {
    for (j = 0; j < n1; ++j) printf("  %f", m[j * ld + i]);
    printf("\n");
  }
}


static void print_dense_vector_float(int n, float *v, int ld)
{
  int i;
  for (i = 0; i < n; ++i) printf("  %f", v[i * ld]);
  printf("\n");
}


static void print_vector_int(int n, int *v)
{
  int i;
  for (i = 0; i < n; ++i) printf("  %d", v[i]);
  printf("\n");
}

#endif


static int lapack_call_put_inout_conf_int(char *conf, const char *id, int i)
{
  return sprintf(conf, "%s:%d ", id, i);
}


static int lapack_call_get_inout_conf_int(char *conf, const char *id, int *i)
{
  char *s = strchr(conf, ':');
  char *e = strchr(s, ' ');

  e[0] = '\0';
  sscanf(s + 1, "%d", i);
  e[0] = ' ';

  return e + 1 - conf;
}


static int lapack_call_put_inout_conf_char(char *conf, const char *id, char c)
{
  return sprintf(conf, "%s:%c ", id, c);
}


static int lapack_call_get_inout_conf_char(char *conf, const char *id, char *c)
{
  char *s = strchr(conf, ':');
  char *e = strchr(s, ' ');

  e[0] = '\0';
  sscanf(s + 1, "%c", c);
  e[0] = ' ';

  return e + 1 - conf;
}


/* client side */

void lapack_call_create_scdc(lapack_call_t *lpc, const char *op, const char *uri)
{
  lpc->client = 1;

  strcpy(lpc->op, op);
  strcpy(lpc->uri, uri);

  lpc->input_nconf = 0;
  lpc->input_conf[0] = '\0';

  lpc->output_nconf = 0;
  lpc->output_conf[0] = '\0';

  lpc->dense_nparams = 0;

  lpc->input_offset = lpc->output_offset = 0;
  lpc->input = malloc(sizeof(scdc_dataset_input_t) + sizeof(scdc_dataset_output_t));
  lpc->output = lpc->input + 1;
  scdc_dataset_input_unset(lpc->input);
  scdc_dataset_output_unset(lpc->output);

  lpc->free_nbufs = 0;
}


void lapack_call_destroy_scdc(lapack_call_t *lpc)
{
  while (lpc->output->next) lpc->output->next(lpc->output);

  free(lpc->input);

  int i;
  for (i = 0; i < lpc->free_nbufs; ++i) free(lpc->free_bufs[i]);
}


static scdcint_t do_input_next(scdc_dataset_input_t *input)
{
  lapack_call_t *lpc = input->data;

  ++lpc->input_state;

  TRACE(printf(PREFIX "do_input_next: state %d of %d\n", lpc->input_state, lpc->dense_nparams););

  if (lpc->input_state < lpc->dense_nparams)
  {
    lpc->input->buf = lpc->dense_params[lpc->input_state].buf;
    lpc->input->buf_size = lpc->dense_params[lpc->input_state].size;

    lpc->input->current_size = lpc->dense_params[lpc->input_state].size;

  } else
  {
    input->next = NULL;
  }

  return SCDC_SUCCESS;
}


void lapack_call_do(lapack_call_t *lpc)
{
  char cmd[512];

  sprintf(cmd, "%s %s %s", lpc->uri, lpc->op, lpc->input_conf);

  lpc->input->next = do_input_next;
  lpc->input->data = lpc;
  lpc->input_state = -1;

  lpc->input->next(lpc->input);

  TRACE(printf(PREFIX "cmd: '%s'\n", cmd););

  scdc_dataset_cmd(SCDC_DATASET_NULL, cmd, lpc->input, lpc->output);

  /* extract conf output from output */
  char *s = lpc->output->buf;
  char *e = strchr(s, '|');

  lpc->output_nconf = 0;
  e[0] = '\0';
  strncpy(lpc->output_conf, s, e - s);
  e[0] = ' ';

  lpc->output_offset = e - s + 1;
}


void lapack_call_put_input_conf_int(lapack_call_t *lpc, const char *id, int i)
{
  lpc->input_nconf += lapack_call_put_inout_conf_int(lpc->input_conf + lpc->input_nconf, id, i);
}


void lapack_call_get_output_conf_int(lapack_call_t *lpc, const char *id, int *i)
{
  lpc->output_nconf += lapack_call_get_inout_conf_int(lpc->output_conf + lpc->output_nconf, id, i);
}


void lapack_call_put_input_conf_char(lapack_call_t *lpc, const char *id, char c)
{
  lpc->input_nconf += lapack_call_put_inout_conf_char(lpc->input_conf + lpc->input_nconf, id, c);
}


void lapack_call_get_output_conf_char(lapack_call_t *lpc, const char *id, char *c)
{
  lpc->output_nconf += lapack_call_get_inout_conf_char(lpc->output_conf + lpc->output_nconf, id, c);
}


/* server side */

void lapack_call_init_scdc(lapack_call_t *lpc, const char *params, scdc_dataset_input_t *input, scdc_dataset_output_t *output)
{
  lpc->client = 0;

  lpc->input_nconf = 0;
  strcpy(lpc->input_conf, params);

  lpc->output_nconf = 0;
  lpc->output_conf[0] = '\0';

  lpc->dense_nparams = 0;

  lpc->input_offset = lpc->output_offset = 0;
  lpc->input = input;
  lpc->output = output;

  lpc->free_nbufs = 0;
}


static scdcint_t release_scdc_output_next(scdc_dataset_output_t *output)
{
  lapack_call_t *lpc = output->data;

  ++lpc->output_state;

  TRACE(printf(PREFIX "release_scdc_output_next: state %d of %d\n", lpc->output_state, lpc->dense_nparams););

  if (lpc->output_state < lpc->dense_nparams)
  {
    lpc->output->buf = lpc->dense_params[lpc->output_state].buf;
    lpc->output->buf_size = lpc->dense_params[lpc->output_state].size;

    lpc->output->current_size = lpc->dense_params[lpc->output_state].size;

  } else
  {
    output->next = NULL;

    int i;
    for (i = 0; i < lpc->free_nbufs; ++i) free(lpc->free_bufs[i]);
  }

  return SCDC_SUCCESS;
}


void lapack_call_release_scdc(lapack_call_t *lpc)
{
  /* consume remaining input */
  while (lpc->input->next) lpc->input->next(lpc->input);

  strcpy(lpc->output_conf + lpc->output_nconf, "|");
  lpc->output_nconf += 1;

  lpc->output->buf = lpc->output_conf;
  lpc->output->buf_size = lpc->output_nconf;

  lpc->output->current_size = lpc->output_nconf;

  lpc->output->next = release_scdc_output_next;
  lpc->output->data = lpc;
  lpc->output_state = -1;
}


void lapack_call_get_input_conf_int(lapack_call_t *lpc, const char *id, int *i)
{
  lpc->input_nconf += lapack_call_get_inout_conf_int(lpc->input_conf + lpc->input_nconf, id, i);
}


void lapack_call_put_output_conf_int(lapack_call_t *lpc, const char *id, int i)
{
  lpc->output_nconf += lapack_call_put_inout_conf_int(lpc->output_conf + lpc->output_nconf, id, i);
}


void lapack_call_get_input_conf_char(lapack_call_t *lpc, const char *id, char *c)
{
  lpc->input_nconf += lapack_call_get_inout_conf_char(lpc->input_conf + lpc->input_nconf, id, c);
}


void lapack_call_put_output_conf_char(lapack_call_t *lpc, const char *id, char c)
{
  lpc->output_nconf += lapack_call_put_inout_conf_char(lpc->output_conf + lpc->output_nconf, id, c);
}

/* client and server side */

void lapack_call_put_input_param_bytes(lapack_call_t *lpc, const char *id, int n, char *b)
{
  /* client only */
  ASSERT(lpc->client);

  lpc->dense_params[lpc->dense_nparams].buf = b;
  lpc->dense_params[lpc->dense_nparams].size = n;

  ++lpc->dense_nparams;
}


void lapack_call_get_output_param_bytes(lapack_call_t *lpc, const char *id, int n, char **b)
{
  if (lpc->client)
  {
    ASSERT(*b);

    /* daten aus output in puffer kopieren */

    char *src = lpc->output->buf;
    char *dst = (char *) *b;

    do
    {
      int t = z_min(n, lpc->output->current_size - lpc->output_offset);
      if (dst != src + lpc->output_offset) memcpy(dst, src + lpc->output_offset, t);
      lpc->output_offset += t;
      dst += t;
      n -= t;

      if (n <= 0 || lpc->output->next == 0 || lpc->output->next(lpc->output) != SCDC_SUCCESS) break;

      src = lpc->output->buf;
      lpc->output_offset = 0;

    } while (1);

  } else
  {
    if (*b == NULL)
    {
#if 0
      if (lpc->output->buf && lpc->output->buf_size - lpc->output_offset >= s)
      {
        /* puffer aus output.buf verwenden */
        /* FIXME */
        /* output_offset hochsetzen */

      } else
#endif
      {
        /* puffer allokieren */
        *b = malloc(n);

        lpc->free_bufs[lpc->free_nbufs] = *b;
        ++lpc->free_nbufs;
      }
    }
  }
}


void lapack_call_get_input_param_bytes(lapack_call_t *lpc, const char *id, int n, char **b)
{
  /* server only */
  ASSERT(!lpc->client);

  if (*b == NULL)
  {
    /* puffer allokieren */
    *b = malloc(n);

    lpc->free_bufs[lpc->free_nbufs] = *b;
    ++lpc->free_nbufs;
  }

  /* daten aus input in puffer kopieren */
  char *src = lpc->input->buf;
  char *dst = (char *) *b;

  do
  {
    int t = z_min(n, lpc->input->current_size - lpc->input_offset);
    if (dst != src + lpc->output_offset) memcpy(dst, src + lpc->input_offset, t);
    lpc->input_offset += t;
    dst += t;
    n -= t;

    if (n <= 0 || lpc->input->next == 0 || lpc->input->next(lpc->input) != SCDC_SUCCESS) break;

    src = lpc->input->buf;
    lpc->input_offset = 0;

  } while (1);
}


void lapack_call_put_output_param_bytes(lapack_call_t *lpc, const char *id, int n, char *b)
{
  if (lpc->client)
  {
    /* puffer in output hinterlegen */
    /* FIXME */

  } else
  {
    /* ausgabe in output schreiben */
    lpc->dense_params[lpc->dense_nparams].buf = b;
    lpc->dense_params[lpc->dense_nparams].size = n;

    ++lpc->dense_nparams;
  }
}


void lapack_call_put_input_param_matrix_float(lapack_call_t *lpc, const char *id, int n0, int n1, float *m, int ld)
{
  /* client only */
  ASSERT(lpc->client);

  TRACE_DATA(
    printf("lapack_call_put_input_param_matrix_float:\n");
    print_dense_matrix_float(n0, n1, m, ld);
  );

  lapack_call_put_input_conf_int(lpc, "ldm", ld);

  lapack_call_put_input_param_bytes(lpc, id, get_dense_matrix_size(n0, n1, ld) * sizeof(float), (char *) m);
}


void lapack_call_get_input_param_matrix_float(lapack_call_t *lpc, const char *id, int n0, int n1, float **m, int *ld)
{
  /* server only */
  ASSERT(!lpc->client);

  lapack_call_get_input_conf_int(lpc, "ldm", ld);

  lapack_call_get_input_param_bytes(lpc, id, get_dense_matrix_size(n0, n1, *ld) * sizeof(float), (char **) m);

  TRACE_DATA(
    printf("lapack_call_get_input_param_matrix_float:\n");
    print_dense_matrix_float(n0, n1, *m, *ld);
  );
}


void lapack_call_put_output_param_matrix_float(lapack_call_t *lpc, const char *id, int n0, int n1, float *m, int ld)
{
  if (!lpc->client)
  {
    TRACE_DATA(
      printf("lapack_call_put_output_param_matrix_float:\n");
      print_dense_matrix_float(n0, n1, m, ld);
    );
  }

  lapack_call_put_output_param_bytes(lpc, id, get_dense_matrix_size(n0, n1, ld) * sizeof(float), (char *) m);
}


void lapack_call_get_output_param_matrix_float(lapack_call_t *lpc, const char *id, int n0, int n1, float **m, int *ld)
{
  lapack_call_get_output_param_bytes(lpc, id, get_dense_matrix_size(n0, n1, *ld) * sizeof(float), (char **) m);

  if (lpc->client)
  {
    TRACE_DATA(
      printf("lapack_call_get_output_param_matrix_float:\n");
      print_dense_matrix_float(n0, n1, *m, *ld);
    );
  }
}


void lapack_call_put_input_param_vector_float(lapack_call_t *lpc, const char *id, int n, float *v, int ld)
{
  /* client only */
  ASSERT(lpc->client);

  TRACE_DATA(
    printf("lapack_call_put_input_param_vector_float:\n");
    print_dense_vector_float(n, v, ld);
  );

  lapack_call_put_input_conf_int(lpc, "ldv", ld);

  lapack_call_put_input_param_bytes(lpc, id, get_dense_vector_size(n, ld) * sizeof(float), (char *) v);
}


void lapack_call_get_input_param_vector_float(lapack_call_t *lpc, const char *id, int n, float **v, int *ld)
{
  /* server only */
  ASSERT(!lpc->client);

  lapack_call_get_input_conf_int(lpc, "ldv", ld);

  lapack_call_get_input_param_bytes(lpc, id, get_dense_vector_size(n, *ld) * sizeof(float), (char **) v);

  TRACE_DATA(
    printf("lapack_call_get_input_param_vector_float:\n");
    print_dense_vector_float(n, *v, *ld);
  );
}


void lapack_call_get_output_param_vector_float(lapack_call_t *lpc, const char *id, int n, float **v, int *ld)
{
  lapack_call_get_output_param_bytes(lpc, id, get_dense_vector_size(n, *ld) * sizeof(float), (char **) v);

  if (lpc->client)
  {
    TRACE_DATA(
      printf("lapack_call_get_output_param_vector_float:\n");
      print_dense_vector_float(n, *v, *ld);
    );
  }
}


void lapack_call_put_output_param_vector_float(lapack_call_t *lpc, const char *id, int n, float *v, int ld)
{
  if (!lpc->client)
  {
    TRACE_DATA(
      printf("lapack_call_put_output_param_vector_float:\n");
      print_dense_vector_float(n, v, ld);
    );
  }

  lapack_call_put_output_param_bytes(lpc, id, get_dense_vector_size(n, ld) * sizeof(float), (char *) v);
}


void lapack_call_put_input_param_vector_int(lapack_call_t *lpc, const char *id, int n, int *v)
{
  /* client only */
  ASSERT(lpc->client);

  TRACE_DATA(
    printf("lapack_call_put_input_param_vector_int:\n");
    print_vector_int(n, v);
  );

  lapack_call_put_input_param_bytes(lpc, id, n * sizeof(int), (char *) v);
}


void lapack_call_get_input_param_vector_int(lapack_call_t *lpc, const char *id, int n, int **v)
{
  /* server only */
  ASSERT(!lpc->client);

  lapack_call_get_input_param_bytes(lpc, id, n * sizeof(int), (char **) v);

  TRACE_DATA(
    printf("lapack_call_get_input_param_vector_int:\n");
    print_vector_int(n, *v);
  );
}


void lapack_call_put_output_param_vector_int(lapack_call_t *lpc, const char *id, int n, int *v)
{
  if (!lpc->client)
  {
    TRACE_DATA(
      printf("lapack_call_put_output_param_vector_int:\n");
      print_vector_int(n, v);
    );
  }

  lapack_call_put_output_param_bytes(lpc, id, n * sizeof(int), (char *) v);
}


void lapack_call_get_output_param_vector_int(lapack_call_t *lpc, const char *id, int n, int **v)
{
  lapack_call_get_output_param_bytes(lpc, id, n * sizeof(int), (char **) v);

  if (lpc->client)
  {
    TRACE_DATA(
      printf("lapack_call_get_output_param_vector_int:\n");
      print_vector_int(n, *v);
    );
  }
}
