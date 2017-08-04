/*
 *  Copyright (C) 2014, 2015, 2016, 2017 Michael Hofmann
 *  
 *  This file is part of the Simulation Component and Data Coupling (SCDC) library.
 *  
 *  The SCDC library is free software: you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser Public License as published
 *  by the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *  
 *  The SCDC library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser Public License for more details.
 *  
 *  You should have received a copy of the GNU Lesser Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef __SCDC_CONFIG_HH__
#define __SCDC_CONFIG_HH__


#include "z_pack.h"
#include "scdc_defs.h"


#if HAVE_SCDC_DEBUG
# define USE_DEBUG  1
#else
# define USE_DEBUG  0
#endif

#ifdef HAVE_ZLIB_H
# define USE_ZLIB  1
#else
# define USE_ZLIB  0
#endif

#ifdef HAVE_MYSQL_H
# define USE_MYSQL  1
#else
# define USE_MYSQL  0
#endif

#ifdef HAVE_MPI_H
# define USE_MPI  1
#else
# define USE_MPI  0
#endif

#if defined(HAVE_NEON_H) || defined(HAVE_DAVIX_HPP)
# define USE_WEBDAV  1
#else
# define USE_WEBDAV  0
#endif

#ifdef HAVE_LIBNFS_H
# define USE_NFS  1
#else
# define USE_NFS  0
#endif


#define USE_NODE_DIRECT  1
#define USE_NODE_TCP     1
#define USE_NODE_UDS     1
#if USE_MPI
# define USE_NODE_MPI    1
#endif
#define USE_NODE_STREAM  1
#define USE_NODE_TIMER   1

#define DEFAULT_TRANSPORT_ALLOC_INPUT_SIZE   16*1024*DEFAULT_SIZE_FACTOR
#define DEFAULT_TRANSPORT_ALLOC_OUTPUT_SIZE  16*1024*DEFAULT_SIZE_FACTOR

#define DEFAULT_TRANSPORT_ALLOC_INCOMING_SIZE  32*1024*DEFAULT_SIZE_FACTOR
#define DEFAULT_TRANSPORT_ALLOC_OUTGOING_SIZE  32*1024*DEFAULT_SIZE_FACTOR

#define DEFAULT_TRANSPORT_SEND_OUTGOING_DATASET_INOUT_THRESHOLD  1024

#define DEFAULT_DATASET_INOUT_BUF_SIZE  16*1024*DEFAULT_SIZE_FACTOR

#define DEFAULT_LIBSCDC_BUF_SIZE  16*1024*DEFAULT_SIZE_FACTOR

#if HAVE_SCDC_PERFOPT
# define DEFAULT_SIZE_FACTOR  1024
#else
# define DEFAULT_SIZE_FACTOR  1
#endif


#define TRANSPORT_TCP_LISTEN_BACKLOG  16

#define NODE_TCP_HOST_DEFAULT  "localhost"
#define NODE_TCP_PORT_DEFAULT  0x5CDC  // 23772

#define TRANSPORT_UDS_LISTEN_BACKLOG  16

#define NODE_UDS_SOCKETFILE_DEFAULT  "/tmp/scdc_%s.uds"
#define NODE_UDS_SOCKETNAME_DEFAULT  "default"


#define TRACE_DATA_SIZE_MAX  1024

/* logging */
#define SCDC_TRACE_COMMON  0

#define SCDC_TRACE_LOG  0

#define SCDC_TRACE_DATA  0

#define SCDC_TRACE_TRANSPORT         0
#define SCDC_TRACE_TRANSPORT_UDS     0
#define SCDC_TRACE_TRANSPORT_TCP     0
#define SCDC_TRACE_TRANSPORT_MPI     0
#define SCDC_TRACE_TRANSPORT_STREAM  0

#define SCDC_TRACE_COMPCOUP            0
#define SCDC_TRACE_COMPCOUP_DIRECT     0
#define SCDC_TRACE_COMPCOUP_TRANSPORT  0

#define SCDC_TRACE_NODEPORT       0
#define SCDC_TRACE_NODEPORT_POOL  0

#define SCDC_TRACE_NODECONN       0
#define SCDC_TRACE_NODECONN_POOL  0

#define SCDC_TRACE_NODE_DIRECT  0
#define SCDC_TRACE_NODE_UDS     0
#define SCDC_TRACE_NODE_TCP     0
#define SCDC_TRACE_NODE_MPI     0
#define SCDC_TRACE_NODE_STREAM  0

#define SCDC_TRACE_DATAPROV         0
#define SCDC_TRACE_DATAPROV_POOL    0
#define SCDC_TRACE_DATAPROV_HOOK    0
#define SCDC_TRACE_DATAPROV_JOBRUN  0
#define SCDC_TRACE_DATAPROV_FS      0

#define SCDC_TRACE_DATASET          0
#define SCDC_TRACE_DATASET_INOUT    0

#define SCDC_TRACE_LIBSCDC  0

#endif /* __SCDC_CONFIG_HH__ */
