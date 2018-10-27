
#include <stdio.h>
#include <stdlib.h>

#include "defs.h"
#include "compC.h"
#include "compI.h"
#include "compG.h"
#include "compL.h"


#define PARTICLES_LOG_PREFIX  "compL: "


compL_t *compL_create()
{
  PARTICLES_TRACE("create");

  compL_t *c = malloc(sizeof(compL_t));

  return c;
}


void compL_destroy(compL_t *c)
{
  PARTICLES_TRACE("destroy");

  if (c) free(c);
}


void compL_cmd_put(compL_t *c, const char *id, compL_data_t *data)
{
  PARTICLES_TRACE("put: id: '%s'", id);

  c->data.start = data->start;
  c->data.max_inc = data->max_inc;
}


void compL_cmd_get(compL_t *c, const char *id, compL_data_t *data)
{
  PARTICLES_TRACE("get: id: '%s'", id);

  data->end = c->data.end;
}


void compL_cmd_run(compL_t *c, const char *id, const char *params)
{
  PARTICLES_TRACE("run: id: '%s', params: '%s'", id, params);

  int nsteps = atoi(params);

  float current = c->data.start;

  int i;
  for (i = 1; i <= nsteps; ++i)
  {
    PARTICLES_TRACE("run: step: %d", i);

    compC_data_t dataC;
    dataC.max_inc = c->data.max_inc;
#if USE_SCDC
    {
      char cmd[256];
      scdc_dataset_input_t input;
      scdc_dataset_output_t output;

      /* put input data */
      sprintf(cmd, "%s put %s", c->compC_uri, id);
      scdc_dataset_input_unset(&input);
      SCDC_DATASET_INOUT_BUF_SET(&input, &dataC.max_inc, sizeof(dataC.max_inc), sizeof(dataC.max_inc));
      scdc_dataset_cmd(SCDC_DATASET_NULL, cmd, &input, NULL);

      /* compute result */
      sprintf(cmd, "%s compute %s", c->compC_uri, id);
      scdc_dataset_cmd(SCDC_DATASET_NULL, cmd, NULL, NULL);

      /* get result */
      sprintf(cmd, "%s get %s", c->compC_uri, id);
      scdc_dataset_output_unset(&output);
      SCDC_DATASET_INOUT_BUF_SET(&output, &dataC.inc, sizeof(dataC.inc), 0);
      scdc_dataset_cmd(SCDC_DATASET_NULL, cmd, NULL, &output);
      dataC.inc = *((float *) SCDC_DATASET_INOUT_BUF_PTR(&output));
    }
#else
    if (c->compC)
    {
      compC_cmd_put(c->compC, id, &dataC);
      compC_cmd_compute(c->compC, id);
      compC_cmd_get(c->compC, id, &dataC);

    } else PARTICLES_TRACE("run: have no component C");
#endif /* USE_SCDC */

    compI_data_t dataI;
    dataI.current = current;
    dataI.inc = dataC.inc;
    if (c->compI)
    {
      compI_cmd_put(c->compI, id, &dataI);
      compI_cmd_compute(c->compI, id);
      compI_cmd_get(c->compI, id, &dataI);
    }
    current = dataI.current;

    compG_data_t dataG;
    dataG.value = current;
    if (c->compG)
    {
      compG_cmd_put(c->compG, id, &dataG);
      compG_cmd_show(c->compG, id);
    }

    PARTICLES_TRACE("run: step: %d done", i);
  }

  c->data.end = current;

  PARTICLES_TRACE("run: return");
}
