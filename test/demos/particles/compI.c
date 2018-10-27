
#include <stdio.h>
#include <stdlib.h>

#include "defs.h"
#include "compI.h"


#define PARTICLES_LOG_PREFIX  "compI: "


compI_t *compI_create()
{
  PARTICLES_TRACE("create");

  compI_t *c = malloc(sizeof(compI_t));

  return c;
}


void compI_destroy(compI_t *c)
{
  PARTICLES_TRACE("destroy");

  if (c) free(c);
}


void compI_cmd_put(compI_t *c, const char *id, compI_data_t *data)
{
  PARTICLES_TRACE("put: id: '%s'", id);

  c->data.current = data->current;
  c->data.inc = data->inc;
}


void compI_cmd_get(compI_t *c, const char *id, compI_data_t *data)
{
  PARTICLES_TRACE("get: id: '%s'", id);

  data->current = c->data.current;
}


void compI_cmd_compute(compI_t *c, const char *id)
{
  PARTICLES_TRACE("compute: id: '%s'", id);

  c->data.current += c->data.inc;
}
