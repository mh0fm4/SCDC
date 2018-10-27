
#ifndef __PARTICLES_H__
#define __PARTICLES_H__


#if USE_MPI
# define PARTICLES_MPI_POLL_INTERVAL  100
#endif

#if USE_SCDC

#include "scdc.h"

typedef struct _scdc_t
{
  scdc_nodeport_t np_uds, np_tcp;

} scdc_t;

#endif /* USE_SCDC */


typedef struct _particles_t
{
  struct _compU_t *compU;
  struct _compD_t *compD;
  struct _compG_t *compG;
  struct _compL_t *compL;
  struct _compC_t *compC;
  struct _compI_t *compI;

  char compD_uri[MAX_STRING_SIZE], compG_uri[MAX_STRING_SIZE], compL_uri[MAX_STRING_SIZE], compC_uri[MAX_STRING_SIZE], compI_uri[MAX_STRING_SIZE];

} particles_t;


#endif /* __PARTICLES_H__ */
