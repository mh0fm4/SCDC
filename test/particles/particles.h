
#ifndef __PARTICLES_H__
#define __PARTICLES_H__


#if USE_SCDC

#include "scdc.h"

typedef struct _scdc_t
{
  scdc_nodeport_t np_uds, np_tcp;

} scdc_t;

#endif /* USE_SCDC */


#endif /* __PARTICLES_H__ */
