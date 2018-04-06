
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "defs.h"
#include "compD.h"
#include "compL.h"
#include "compG.h"
#include "compU.h"


#define PARTICLES_LOG_PREFIX  "compU: "


compU_t *compU_create()
{
  PARTICLES_TRACE("create");

  compU_t *c = malloc(sizeof(compU_t));

  return c;
}


void compU_destroy(compU_t *c)
{
  PARTICLES_TRACE("destroy");

  if (c) free(c);
}


void compU_print(compU_t *c)
{
#if USE_SCDC
  printf("components:\n");
  printf("  compD: %s\n", c->compD_uri);
  printf("  compG: %s\n", c->compG_uri);
  printf("  compL: %s\n", c->compL_uri);
  printf("  compC: %s\n", c->compC_uri);
  printf("  compI: %s\n", c->compI_uri);
#else
  printf("components: single programm\n");
#endif

  printf("particle simulation:\n");
  printf("  id: %s\n", c->id);
  printf("  nsteps: %d\n", c->nsteps);
}


void compU_main(compU_t *c)
{
  PARTICLES_TRACE("main");

  compD_data_t dataD;

  /* get input data from component D */
  if (c->compD) compD_cmd_get(c->compD, c->id, &dataD);
  else PARTICLES_TRACE("main: have no component D");

  /* prepare component L */
  c->compL->compC = c->compC;
  strncpy(c->compL->compC_uri, c->compC_uri, MAX_STRING_SIZE);
  c->compL->compI = c->compI;
  strncpy(c->compL->compI_uri, c->compI_uri, MAX_STRING_SIZE);
  c->compL->compG = c->compG;
  strncpy(c->compL->compG_uri, c->compG_uri, MAX_STRING_SIZE);

  /* prepare input data of component L */
  compL_data_t dataL;
  dataL.start = dataD.start;
  dataL.max_inc = dataD.max_inc;

  /* prepare run parameter of component L */
  char params[MAX_STRING_SIZE];
  sprintf(params, "%d", c->nsteps);

  if (c->compL)
  {
    /* put input data to component L */
    compL_cmd_put(c->compL, c->id, &dataL);
    /* run component L */
    compL_cmd_run(c->compL, c->id, params);
    /* get result data from component L */
    compL_cmd_get(c->compL, c->id, &dataL);

  } else PARTICLES_TRACE("main: have no component L");

  /* prepare input data of component G */
  compG_data_t dataG;
  dataG.value = dataL.end;

  if (c->compG)
  {
    /* put input data to component G */
    compG_cmd_put(c->compG, c->id, &dataG);
    /* show data with component G */
    compG_cmd_show(c->compG, c->id);

  } else PARTICLES_TRACE("main: have no component G");

  /* prepare input data of component D */
  dataD.end = dataL.end;

  if (c->compD) compD_cmd_put(c->compD, c->id, &dataD);
  else PARTICLES_TRACE("main: have no component D");

  PARTICLES_TRACE("main: return");
}
