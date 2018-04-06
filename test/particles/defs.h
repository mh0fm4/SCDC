
#ifndef __DEFS_H__
#define __DEFS_H__


#define Z_MOP(_mop_)  do { _mop_ } while (0)

// #define PARTICLES_TRACE_PREFIX  "PARTICLES_TRACE: "
#define PARTICLES_TRACE_PREFIX  ""
#define PARTICLES_TRACE(_s_...)  Z_MOP(printf(PARTICLES_TRACE_PREFIX PARTICLES_LOG_PREFIX _s_); printf("\n");)

#define MAX_STRING_SIZE  64


#endif /* __DEFS_H__ */
