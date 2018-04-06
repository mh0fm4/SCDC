
#ifndef __COMPL_H__
#define __COMPL_H__


#include "defs.h"

typedef struct _compL_data_t
{
  float start, max_inc, end;
  
} compL_data_t;

struct _compC_t;
struct _compI_t;
struct _compG_t;

typedef struct _compL_t
{
  compL_data_t data;

  struct _compC_t *compC;
  struct _compI_t *compI;
  struct _compG_t *compG;
  char compC_uri[MAX_STRING_SIZE], compI_uri[MAX_STRING_SIZE], compG_uri[MAX_STRING_SIZE];

} compL_t;


compL_t *compL_create();
void compL_destroy(compL_t *c);

void compL_cmd_put(compL_t *c, const char *id, compL_data_t *data);
void compL_cmd_get(compL_t *c, const char *id, compL_data_t *data);
void compL_cmd_run(compL_t *c, const char *id, const char *params);


#endif /* __COMPL_H__ */
