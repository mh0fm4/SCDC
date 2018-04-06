
#include <stdio.h>
#include <stdlib.h>

#include "defs.h"
#include "compD.h"


#define PARTICLES_LOG_PREFIX  "compD: "


compD_t *compD_create()
{
  PARTICLES_TRACE("create");

  compD_t *c = malloc(sizeof(compD_t));

  return c;
}


void compD_destroy(compD_t *c)
{
  PARTICLES_TRACE("destroy");

  if (c) free(c);
}


void compD_cmd_get(compD_t *c, const char *id, compD_data_t *data)
{
  PARTICLES_TRACE("get: id: '%s'", id);

  data->start = 0.0;
  data->max_inc = 1.0;
}


void compD_cmd_put(compD_t *c, const char *id, compD_data_t *data)
{
  PARTICLES_TRACE("put: id: '%s'", id);

  PARTICLES_TRACE("put: writting result %f", data->end);
}
