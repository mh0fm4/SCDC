
#ifndef __COMPU_H__
#define __COMPU_H__


#include "defs.h"

typedef struct _compU_t
{
  char id[MAX_STRING_SIZE];
  int nsteps;

  struct _compD_t *compD;
  struct _compG_t *compG;
  struct _compL_t *compL;
  struct _compC_t *compC;
  struct _compI_t *compI;

#if USE_SCDC
  char compD_uri[MAX_STRING_SIZE], compG_uri[MAX_STRING_SIZE], compL_uri[MAX_STRING_SIZE], compC_uri[MAX_STRING_SIZE], compI_uri[MAX_STRING_SIZE];
#endif

} compU_t;

compU_t *compU_create();
void compU_destroy(compU_t *c);

void compU_print(compU_t *c);
void compU_main(compU_t *c);


#endif /* __COMPU_H__ */
