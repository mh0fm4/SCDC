
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "conf.h"

#if USE_SCDC
# include "scdc.h"
#endif

#include "compU.h"
#include "compD.h"
#include "compG.h"
#include "compL.h"
#include "compC.h"
#include "compI.h"
#include "particles.h"

#define PARTICLES_LOG_PREFIX  "particles: "


#if USE_SCDC

scdc_t *scdc_create()
{
  PARTICLES_TRACE("scdc_create");

  scdc_init(SCDC_INIT_DEFAULT);

  scdc_t *s = malloc(sizeof(scdc_t));

#if USE_SCDC_UDS
  s->np_uds = scdc_nodeport_open("uds");
#endif

#if USE_SCDC_TCP
  s->np_tcp = scdc_nodeport_open("tcp");
#endif

  return s;
}


void scdc_start(scdc_t *s)
{
  PARTICLES_TRACE("scdc_start");

#if USE_SCDC_UDS
  scdc_nodeport_start(s->np_uds, SCDC_NODEPORT_START_ASYNC_UNTIL_CANCEL);
#endif

#if USE_SCDC_TCP
  scdc_nodeport_start(s->np_tcp, SCDC_NODEPORT_START_ASYNC_UNTIL_CANCEL);
#endif
}


void scdc_stop(scdc_t *s)
{
  PARTICLES_TRACE("scdc_stop");

#if USE_SCDC_UDS
  scdc_nodeport_stop(s->np_uds);
#endif

#if USE_SCDC_TCP
  scdc_nodeport_stop(s->np_tcp);
#endif
}


void scdc_destroy(scdc_t *s)
{
  PARTICLES_TRACE("scdc_destroy");

#if USE_SCDC_UDS
  scdc_nodeport_close(s->np_uds);
#endif

#if USE_SCDC_TCP
  scdc_nodeport_close(s->np_tcp);
#endif
}

#endif /* USE_SCDC */


int main(int argc, char *argv[])
{
  /* init SCDC library */
#if USE_SCDC
  scdc_t *scdc = scdc_create();
#endif

  /* init components */
#if USE_COMPU
  compU_t *compU = compU_create();
#endif
#if USE_COMPD
  compD_t *compD = compD_create();
#endif
#if USE_COMPG
  compG_t *compG = compG_create();
#endif
#if USE_COMPL
  compL_t *compL = compL_create();
#endif
#if USE_COMPC
  compC_t *compC = compC_create();
#endif
#if USE_COMPI
  compI_t *compI = compI_create();
#endif

  /* start SCDC service */
#if USE_SCDC
  scdc_start(scdc);
#endif

#if USE_COMPU
  /* setup component distribution */
  strncpy(compU->compD_uri, COMPD_SCDC_URI, MAX_STRING_SIZE);
  strncpy(compU->compG_uri, COMPG_SCDC_URI, MAX_STRING_SIZE);
  strncpy(compU->compL_uri, COMPL_SCDC_URI, MAX_STRING_SIZE);
  strncpy(compU->compC_uri, COMPC_SCDC_URI, MAX_STRING_SIZE);
  strncpy(compU->compI_uri, COMPI_SCDC_URI, MAX_STRING_SIZE);

  /* setup particle simulation */
  strncpy(compU->id, "test", MAX_STRING_SIZE);
  compU->nsteps = 5;

  /* setup component objects for local calls */
#if USE_COMPD
  compU->compD = compD;
#endif
#if USE_COMPG
  compU->compG = compG;
#endif
#if USE_COMPL
  compU->compL = compL;
#endif
#if USE_COMPC
  compU->compC = compC;
#endif
#if USE_COMPI
  compU->compI = compI;
#endif

  compU_print(compU);

  /* run main component U */
  compU_main(compU);
#else
  /* keep service alive */
  printf("Press <ENTER> to quit!");
  getchar();
#endif

  /* stop SCDC service */
#if USE_SCDC
  scdc_stop(scdc);
#endif

  /* release components */
#if USE_COMPU
  compU_destroy(compU);
#endif
#if USE_COMPD
  compD_destroy(compD);
#endif
#if USE_COMPG
  compG_destroy(compG);
#endif
#if USE_COMPL
  compL_destroy(compL);
#endif
#if USE_COMPC
  compC_destroy(compC);
#endif
#if USE_COMPI
  compI_destroy(compI);
#endif

  /* release SCDC library */
#if USE_SCDC
  scdc_destroy(scdc);
#endif

  return 0;
}