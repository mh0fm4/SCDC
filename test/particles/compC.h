
#ifndef __COMPC_H__
#define __COMPC_H__


#if USE_SCDC
# include "scdc.h"
#endif


typedef struct _compC_data_t
{
  float max_inc, inc;
  
} compC_data_t;

typedef struct _compC_t
{
  compC_data_t data;

#if USE_SCDC
  scdc_dataprov_t dp;

  compC_data_t tmp_data;
#endif

} compC_t;


compC_t *compC_create();
void compC_destroy(compC_t *c);

void compC_cmd_put(compC_t *c, const char *id, compC_data_t *data);
void compC_cmd_get(compC_t *c, const char *id, compC_data_t *data);
void compC_cmd_compute(compC_t *c, const char *id);


#endif /* __COMPC_H__ */
