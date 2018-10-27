
#ifndef __COMPL_H__
#define __COMPL_H__


#include "defs.h"

typedef struct _compL_data_t
{
  float start, max_inc, end;
  
} compL_data_t;

typedef struct _compL_t
{
  compL_data_t data;

#if USE_SCDC
  char compC_uri[MAX_STRING_SIZE];
#else
  struct _compC_t *compC;
#endif

  struct _compI_t *compI;
  struct _compG_t *compG;

} compL_t;


compL_t *compL_create();
void compL_destroy(compL_t *c);

void compL_cmd_put(compL_t *c, const char *id, compL_data_t *data);
void compL_cmd_get(compL_t *c, const char *id, compL_data_t *data);
void compL_cmd_run(compL_t *c, const char *id, const char *params);


#endif /* __COMPL_H__ */
