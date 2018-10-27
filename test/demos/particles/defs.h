
#ifndef __DEFS_H__
#define __DEFS_H__


#if USE_MPI
# include <mpi.h>
#endif


#define Z_NOP()       do { } while (0)
#define Z_MOP(_mop_)  do { _mop_ } while (0)

// #define PARTICLES_TRACE_PREFIX  "PARTICLES_TRACE: "
#define PARTICLES_TRACE_PREFIX  ""
#define PARTICLES_TRACE(_s_...)  Z_MOP(printf(PARTICLES_TRACE_PREFIX PARTICLES_LOG_PREFIX _s_); printf("\n");)

#if USE_MPI
# define PARTICLES_MPI_ROOT  0
void mpi_root_parallel_cmd(MPI_Comm comm, int tag);
#endif

#define MAX_STRING_SIZE  64

struct _compD_t;
struct _compG_t;
struct _compL_t;
struct _compC_t;
struct _compI_t;

#if USE_COMPU
# define COMPU_IFELSE(_if_, _else_)  _if_
#else
# define COMPU_IFELSE(_if_, _else_)  _else_
#endif

#if USE_COMPD
# define COMPD_IFELSE(_if_, _else_)  _if_
#else
# define COMPD_IFELSE(_if_, _else_)  _else_
#endif

#if USE_COMPG
# define COMPG_IFELSE(_if_, _else_)  _if_
#else
# define COMPG_IFELSE(_if_, _else_)  _else_
#endif

#if USE_COMPL
# define COMPL_IFELSE(_if_, _else_)  _if_
#else
# define COMPL_IFELSE(_if_, _else_)  _else_
#endif

#if USE_COMPC
# define COMPC_IFELSE(_if_, _else_)  _if_
#else
# define COMPC_IFELSE(_if_, _else_)  _else_
#endif

#if USE_COMPI
# define COMPI_IFELSE(_if_, _else_)  _if_
#else
# define COMPI_IFELSE(_if_, _else_)  _else_
#endif


#endif /* __DEFS_H__ */
