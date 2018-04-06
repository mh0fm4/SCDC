
#ifndef __COMPG_H__
#define __COMPG_H__


typedef struct _compG_data_t
{
  float value;
  
} compG_data_t;

typedef struct _compG_t
{
  compG_data_t data;

} compG_t;


compG_t *compG_create();
void compG_destroy(compG_t *c);

void compG_cmd_put(compG_t *c, const char *id, compG_data_t *data);
void compG_cmd_show(compG_t *c, const char *id);


#endif /* __COMPG_H__ */
