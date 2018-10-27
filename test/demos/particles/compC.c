
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if USE_MPI
# include <mpi.h>
#endif

#if USE_SCDC
# include "scdc.h"
#endif

#include "defs.h"
#include "compC.h"


#if USE_MPI
# define COMPC_MPI_TAG_PUT      1
# define COMPC_MPI_TAG_GET      2
# define COMPC_MPI_TAG_COMPUTE  3
#endif


#define PARTICLES_LOG_PREFIX  "compC: "


#if USE_SCDC

/* hook function for opening data provider of component C */
void *compC_scdc_open(const char *conf, va_list ap)
{
  printf(PARTICLES_LOG_PREFIX "compC_scdc_open: conf: '%s'\n", conf);

  compC_t *c = NULL;

  c = va_arg(ap, compC_t *);

  return c;
}


/* hook function for closing data provider of component C */
scdcint_t compC_scdc_close(void *dataprov)
{
  printf(PARTICLES_LOG_PREFIX "compC_scdc_open: dataprov: '%p'\n", dataprov);
  
  return SCDC_SUCCESS;
}


/* hook function for executing commands of component C */
scdcint_t compC_scdc_dataset_cmd(void *dataprov, void *dataset, const char *cmd, const char *params, scdc_dataset_input_t *input, scdc_dataset_output_t *output)
{
  // printf(PARTICLES_LOG_PREFIX "compC_scdc_dataset_cmd:\n");
  // printf(PARTICLES_LOG_PREFIX "  dataprov: '%p'\n", dataprov);
  // printf(PARTICLES_LOG_PREFIX "  dataset: '%p'\n", dataset);
  // printf(PARTICLES_LOG_PREFIX "  cmd: '%s'\n", cmd);
  // printf(PARTICLES_LOG_PREFIX "  params: '%s'\n", params);
  // printf(PARTICLES_LOG_PREFIX "  "); scdc_dataset_input_print(input); printf("\n");
  // printf(PARTICLES_LOG_PREFIX "  "); scdc_dataset_output_print(output); printf("\n");

  compC_t *c = dataprov;
  const char *id = params;

  if (strcmp(cmd, "put") == 0) /* put command */
  {
    /* retrieve data from input object */
    c->tmp_data.max_inc = *((float *) SCDC_DATASET_INOUT_BUF_PTR(input));
    /* perform put command */
    compC_cmd_put(c, id, &c->tmp_data);

  } else if (strcmp(cmd, "get") == 0) /* get command */
  {
    /* perform get command */
    compC_cmd_get(c, id, &c->tmp_data);
    /* set data to output object */
    SCDC_DATASET_INOUT_BUF_SET(output, &c->tmp_data.inc, sizeof(c->tmp_data.inc), sizeof(c->tmp_data.inc));

  } else if (strcmp(cmd, "compute") == 0) /* compute command */
  {
    /* perform compute command */
    compC_cmd_compute(c, id);

  } else printf(PARTICLES_LOG_PREFIX "error: unknown command '%s'\n", cmd);

  // printf(PARTICLES_LOG_PREFIX "  "); scdc_dataset_output_print(output); printf("\n");

  return SCDC_SUCCESS;
}


const static scdc_dataprov_hook_t compC_scdc_hook = {
  compC_scdc_open, /* open */
  compC_scdc_close, /* close */
  0, /* config */
  0, /* dataset_open */
  0, /* dataset_close */
  0, /* dataset_open_read_state */
  0, /* dataset_close_write_state */
  compC_scdc_dataset_cmd,
};

#endif /* USE_SCDC */


compC_t *compC_create(
#if USE_MPI
  MPI_Comm comm
#endif
)
{
  PARTICLES_TRACE("create");

  compC_t *c = malloc(sizeof(compC_t));

#if USE_MPI
  c->comm = comm;
  MPI_Comm_size(comm, &c->comm_size);
  MPI_Comm_rank(comm, &c->comm_rank);

  PARTICLES_TRACE("create: MPI: comm_size: %d, comm_rank: %d", c->comm_size, c->comm_rank);
#endif

  srandom(
#if USE_MPI
    c->comm_rank +
#endif
    1
  );

#if USE_SCDC
  c->dp = SCDC_DATAPROV_NULL;
# if USE_MPI
  if (c->comm_rank == PARTICLES_MPI_ROOT)
#endif
  {
    c->dp = scdc_dataprov_open("compC", "hook:info", &compC_scdc_hook, c);
  }
#endif

  return c;
}


void compC_destroy(compC_t *c)
{
  PARTICLES_TRACE("destroy");

#if USE_SCDC
  if (c->dp != SCDC_DATAPROV_NULL) scdc_dataprov_close(c->dp);
#endif

  if (c) free(c);
}


#if USE_MPI

void compC_compute(compC_t *c, const char *id);

int compC_mpi_cmd(compC_t *c, int tag)
{
  PARTICLES_TRACE("mpi_cmd: %d: tag: %d", c->comm_rank, tag);

  if (tag == COMPC_MPI_TAG_PUT)
  {
    PARTICLES_TRACE("mpi_cmd: %d: put", c->comm_rank);

    MPI_Bcast(&c->data.max_inc, 1, MPI_FLOAT, PARTICLES_MPI_ROOT, c->comm);

  } else if (tag == COMPC_MPI_TAG_GET)
  {
    PARTICLES_TRACE("mpi_cmd: %d: get", c->comm_rank);

    float x = c->data.inc;
    MPI_Reduce(&x, &c->data.inc, 1, MPI_FLOAT, MPI_MAX, PARTICLES_MPI_ROOT, c->comm);

  } else if (tag == COMPC_MPI_TAG_COMPUTE)
  {
    PARTICLES_TRACE("mpi_cmd: %d: compute", c->comm_rank);

    char id[MAX_STRING_SIZE];
    if (c->comm_rank == PARTICLES_MPI_ROOT) strncpy(id, c->id, MAX_STRING_SIZE);
    MPI_Bcast(id, MAX_STRING_SIZE, MPI_CHAR, PARTICLES_MPI_ROOT, c->comm);

    compC_compute(c, id);

  } else return 0;

  return 1;
}

#endif /* USE_MPI */


void compC_cmd_put(compC_t *c, const char *id, compC_data_t *data)
{
  PARTICLES_TRACE("cmd_put: id: '%s'", id);

  PARTICLES_TRACE("cmd_put: id: '%s', max_inc: %f", id, data->max_inc);

  c->data.max_inc = data->max_inc;

#if USE_MPI
  mpi_root_parallel_cmd(c->comm, COMPC_MPI_TAG_PUT);
#endif
}


void compC_cmd_get(compC_t *c, const char *id, compC_data_t *data)
{
  PARTICLES_TRACE("cmd_get: id: '%s'", id);

#if USE_MPI
  mpi_root_parallel_cmd(c->comm, COMPC_MPI_TAG_GET);
#endif

  data->inc = c->data.inc;

  PARTICLES_TRACE("cmd_get: id: '%s', inc: %f", id, data->inc);
}


void compC_compute(compC_t *c, const char *id)
{
  PARTICLES_TRACE("compute: id: '%s'", id);

  c->data.inc = c->data.max_inc * random() / RAND_MAX;

  PARTICLES_TRACE("compute: id: '%s', inc: %f", id, c->data.inc);
}


void compC_cmd_compute(compC_t *c, const char *id)
{
  PARTICLES_TRACE("cmd_compute: id: '%s'", id);

#if USE_MPI
  c->id = id;
  mpi_root_parallel_cmd(c->comm, COMPC_MPI_TAG_COMPUTE);
#else
  compC_compute(c, id);
#endif

  PARTICLES_TRACE("cmd_compute: id: '%s', inc: %f", id, c->data.inc);
}
