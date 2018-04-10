
#ifndef __COMPD_H__
#define __COMPD_H__


#include "defs.h"

typedef struct _compD_data_t
{
  float start, max_inc, end;
  
} compD_data_t;

typedef struct _compD_t
{
  int dummy;

} compD_t;

compD_t *compD_create();
void compD_destroy(compD_t *c);

void compD_cmd_get(compD_t *c, const char *id, compD_data_t *data);
void compD_cmd_put(compD_t *c, const char *id, compD_data_t *data);


#endif /* __COMPD_H__ */
