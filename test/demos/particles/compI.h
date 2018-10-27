
#ifndef __COMPI_H__
#define __COMPI_H__


#include "defs.h"

typedef struct _compI_data_t
{
  float current, inc;
  
} compI_data_t;

typedef struct _compI_t
{
  compI_data_t data;

} compI_t;


compI_t *compI_create();
void compI_destroy(compI_t *c);

void compI_cmd_put(compI_t *c, const char *id, compI_data_t *data);
void compI_cmd_get(compI_t *c, const char *id, compI_data_t *data);
void compI_cmd_compute(compI_t *c, const char *id);


#endif /* __COMPI_H__ */
