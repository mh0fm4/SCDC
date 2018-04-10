
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "conf.h"

#if USE_MPI
# include <mpi.h>
#endif

#if USE_SCDC
# include "scdc.h"
#endif

#include "compU.h"
#include "compD.h"
#include "compG.h"
#include "compL.h"
#include "compC.h"
#include "compI.h"
#include "particles.h"

#define PARTICLES_LOG_PREFIX  "particles: "


particles_t particles;


#if USE_MPI

void mpi_root_start(MPI_Comm comm)
{
  PARTICLES_TRACE("mpi_root_start");

  MPI_Barrier(comm);
}


void mpi_root_signal(MPI_Comm comm, int tag)
{
  PARTICLES_TRACE("mpi_root_signal: tag: %d", tag);

  MPI_Request req;
  MPI_Ibcast(&tag, 1, MPI_INT, PARTICLES_MPI_ROOT, comm, &req);
  MPI_Wait(&req, MPI_STATUS_IGNORE);
}


void mpi_root_stop(MPI_Comm comm)
{
  PARTICLES_TRACE("mpi_root_stop");

  mpi_root_signal(comm, -1);
}


void mpi_handle_tag(MPI_Comm comm, int tag)
{
#if USE_COMPC
  if (compC_mpi_cmd(particles.compC, tag));
  else
#endif
  {
    PARTICLES_TRACE("mpi_non_root_run: unhandled tag %d", tag);
    Z_NOP();
  }
}


void mpi_non_root_run(MPI_Comm comm)
{
  PARTICLES_TRACE("mpi_non_root_run");

  MPI_Barrier(comm);

  while (1)
  {
    int tag;
    MPI_Request req;

    PARTICLES_TRACE("mpi_non_root_run: await new incoming tag");
    MPI_Ibcast(&tag, 1, MPI_INT, PARTICLES_MPI_ROOT, comm, &req);

    while (1)
    {
      int flag;
      MPI_Test(&req, &flag, MPI_STATUS_IGNORE);
      PARTICLES_TRACE("mpi_non_root_run: request completed: %d", flag);
      if (flag) break;
      usleep(PARTICLES_MPI_POLL_INTERVAL * 1000);
    }

    if (tag == -1) break;

    mpi_handle_tag(comm, tag);
  }
}


void mpi_root_parallel_cmd(MPI_Comm comm, int tag)
{
  mpi_root_signal(comm, tag);
  mpi_handle_tag(comm, tag);
}


#endif /* USE_MPI */


#if USE_SCDC

scdc_t *scdc_create()
{
  PARTICLES_TRACE("scdc_create");

  scdc_init(SCDC_INIT_DEFAULT);

  scdc_t *s = malloc(sizeof(scdc_t));

#if USE_SCDC_UDS
  s->np_uds = scdc_nodeport_open("uds");
#endif

#if USE_SCDC_TCP
  s->np_tcp = scdc_nodeport_open("tcp");
#endif

  return s;
}


void scdc_start(scdc_t *s)
{
  PARTICLES_TRACE("scdc_start");

#if USE_SCDC_UDS
  scdc_nodeport_start(s->np_uds, SCDC_NODEPORT_START_ASYNC_UNTIL_CANCEL);
#endif

#if USE_SCDC_TCP
  scdc_nodeport_start(s->np_tcp, SCDC_NODEPORT_START_ASYNC_UNTIL_CANCEL);
#endif
}


void scdc_stop(scdc_t *s)
{
  PARTICLES_TRACE("scdc_stop");

#if USE_SCDC_UDS
  scdc_nodeport_stop(s->np_uds);
#endif

#if USE_SCDC_TCP
  scdc_nodeport_stop(s->np_tcp);
#endif
}


void scdc_destroy(scdc_t *s)
{
  PARTICLES_TRACE("scdc_destroy");

#if USE_SCDC_UDS
  scdc_nodeport_close(s->np_uds);
#endif

#if USE_SCDC_TCP
  scdc_nodeport_close(s->np_tcp);
#endif
}

#endif /* USE_SCDC */


int main(int argc, char *argv[])
{
  /* init MPI */
#if USE_MPI
# if USE_SCDC
  int provided;
  MPI_Init_thread(&argc, &argv, MPI_THREAD_SERIALIZED, &provided);
  PARTICLES_TRACE("MPI_Init_thread: required: %d, provided: %d", MPI_THREAD_SERIALIZED, provided);
# else
  MPI_Init(&argc, &argv);
# endif

  MPI_Comm comm = MPI_COMM_WORLD;

  int comm_size, comm_rank;
  MPI_Comm_size(comm, &comm_size);
  MPI_Comm_rank(comm, &comm_rank);
#endif

  /* init SCDC library */
#if USE_SCDC
  scdc_t *scdc = NULL;
# if USE_MPI
  if (comm_rank == PARTICLES_MPI_ROOT)
# endif
  {
    scdc = scdc_create();
  }
#endif

  /* init components */
  particles.compU = COMPU_IFELSE(compU_create(), NULL);
  particles.compD = COMPD_IFELSE(compD_create(), NULL);
  particles.compG = COMPG_IFELSE(compG_create(), NULL);
  particles.compL = COMPL_IFELSE(compL_create(), NULL);
#if USE_MPI
  particles.compC = COMPC_IFELSE(compC_create(comm), NULL);
#else
  particles.compC = COMPC_IFELSE(compC_create(), NULL);
#endif
  particles.compI = COMPI_IFELSE(compI_create(), NULL);

  /* start SCDC service */
#if USE_SCDC
  if (scdc) scdc_start(scdc);
#endif

#if USE_MPI
  if (comm_rank != PARTICLES_MPI_ROOT)
  {
    mpi_non_root_run(comm);

  } else
#endif
  {
#if USE_MPI
    mpi_root_start(comm);
#endif

#if USE_COMPU
    /* setup particle simulation */
    strncpy(particles.compU->id, "test", MAX_STRING_SIZE);
    particles.compU->nsteps = 5;

    /* setup component objects for local calls */
    particles.compU->compD = COMPD_IFELSE(particles.compD, NULL);
    particles.compU->compG = COMPG_IFELSE(particles.compG, NULL);
    particles.compU->compL = COMPL_IFELSE(particles.compL, NULL);
    particles.compU->compC = COMPC_IFELSE(particles.compC, NULL);
    particles.compU->compI = COMPI_IFELSE(particles.compI, NULL);

#if USE_SCDC
    /* setup component distribution */
    strncpy(particles.compU->compD_uri, COMPD_SCDC_URI, MAX_STRING_SIZE);
    strncpy(particles.compU->compG_uri, COMPG_SCDC_URI, MAX_STRING_SIZE);
    strncpy(particles.compU->compL_uri, COMPL_SCDC_URI, MAX_STRING_SIZE);
    strncpy(particles.compU->compC_uri, COMPC_SCDC_URI, MAX_STRING_SIZE);
    strncpy(particles.compU->compI_uri, COMPI_SCDC_URI, MAX_STRING_SIZE);
#endif

    compU_print(particles.compU);

    /* run main component U */
    compU_main(particles.compU);
#else /* USE_COMPU */
    /* keep service alive */
    printf("Press <ENTER> to quit!");
    getchar();
#endif /* USE_COMPU */

#if USE_MPI
    mpi_root_stop(comm);
#endif
  }

  /* stop SCDC service */
#if USE_SCDC
  if (scdc) scdc_stop(scdc);
#endif

  /* release components */
  COMPU_IFELSE(compU_destroy(particles.compU), Z_NOP());
  COMPD_IFELSE(compD_destroy(particles.compD), Z_NOP());
  COMPG_IFELSE(compG_destroy(particles.compG), Z_NOP());
  COMPL_IFELSE(compL_destroy(particles.compL), Z_NOP());
  COMPC_IFELSE(compC_destroy(particles.compC), Z_NOP());
  COMPI_IFELSE(compI_destroy(particles.compI), Z_NOP());

  /* release SCDC library */
#if USE_SCDC
  if (scdc) scdc_destroy(scdc);
#endif

  /* release MPI */
#if USE_MPI
  MPI_Finalize();
#endif

  return 0;
}