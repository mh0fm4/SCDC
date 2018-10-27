
#ifndef __COMPC_H__
#define __COMPC_H__


#if USE_MPI
# include <mpi.h>
#endif

#if USE_SCDC
# include "scdc.h"
#endif


#include "defs.h"

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

#if USE_MPI
  MPI_Comm comm;
  int comm_size, comm_rank;
  const char *id;
#endif

} compC_t;


compC_t *compC_create(
#if USE_MPI
  MPI_Comm comm
#endif
);
void compC_destroy(compC_t *c);

#if USE_MPI
int compC_mpi_cmd(compC_t *c, int tag);
#endif

void compC_cmd_put(compC_t *c, const char *id, compC_data_t *data);
void compC_cmd_get(compC_t *c, const char *id, compC_data_t *data);
void compC_cmd_compute(compC_t *c, const char *id);


#endif /* __COMPC_H__ */
