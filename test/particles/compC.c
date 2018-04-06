
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if USE_SCDC
# include "scdc.h"
#endif

#include "defs.h"
#include "compC.h"


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
  printf(PARTICLES_LOG_PREFIX "compC_scdc_dataset_cmd:\n");
  printf(PARTICLES_LOG_PREFIX "  dataprov: '%p'\n", dataprov);
  printf(PARTICLES_LOG_PREFIX "  dataset: '%p'\n", dataset);
  printf(PARTICLES_LOG_PREFIX "  cmd: '%s'\n", cmd);
  printf(PARTICLES_LOG_PREFIX "  params: '%s'\n", params);
  printf(PARTICLES_LOG_PREFIX "  "); scdc_dataset_input_print(input); printf("\n");
  printf(PARTICLES_LOG_PREFIX "  "); scdc_dataset_output_print(output); printf("\n");

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

  printf(PARTICLES_LOG_PREFIX "  "); scdc_dataset_output_print(output); printf("\n");

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


compC_t *compC_create()
{
  PARTICLES_TRACE("create");

  compC_t *c = malloc(sizeof(compC_t));

  srandom(10);

#if USE_SCDC
  c->dp = scdc_dataprov_open("compC", "hook:info", &compC_scdc_hook, c);
#endif

  return c;
}


void compC_destroy(compC_t *c)
{
  PARTICLES_TRACE("destroy");

#if USE_SCDC
  scdc_dataprov_close(c->dp);
#endif

  if (c) free(c);
}


void compC_cmd_put(compC_t *c, const char *id, compC_data_t *data)
{
  PARTICLES_TRACE("put: id: '%s'", id);

  c->data.max_inc = data->max_inc;
}


void compC_cmd_get(compC_t *c, const char *id, compC_data_t *data)
{
  PARTICLES_TRACE("get: id: '%s'", id);

  data->inc = c->data.inc;
}


void compC_cmd_compute(compC_t *c, const char *id)
{
  PARTICLES_TRACE("compute: id: '%s'", id);

  c->data.inc = c->data.max_inc * random() / RAND_MAX;
}
