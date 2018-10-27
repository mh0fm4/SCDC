
#include <stdio.h>
#include <stdlib.h>

#include "defs.h"
#include "compG.h"


#define PARTICLES_LOG_PREFIX  "compG: "


compG_t *compG_create()
{
  PARTICLES_TRACE("create");

  compG_t *c = malloc(sizeof(compG_t));

  return c;
}


void compG_destroy(compG_t *c)
{
  PARTICLES_TRACE("destroy");

  if (c) free(c);
}


void compG_cmd_put(compG_t *c, const char *id, compG_data_t *data)
{
  PARTICLES_TRACE("put: id: '%s'", id);

  c->data.value = data->value;
}


void compG_cmd_show(compG_t *c, const char *id)
{
  PARTICLES_TRACE("show: id: '%s'", id);

  PARTICLES_TRACE("show: showing result %f", c->data.value);
}
