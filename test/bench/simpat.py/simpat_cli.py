#!/usr/bin/python

#  
#  Copyright (C) 2014, 2015, 2016, 2017, 2018 Michael Hofmann
#  
#  This file is part of the Simulation Component and Data Coupling (SCDC) library.
#  
#  The SCDC library is free software: you can redistribute it and/or
#  modify it under the terms of the GNU Lesser Public License as published
#  by the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#  
#  The SCDC library is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU Lesser Public License for more details.
#  
#  You should have received a copy of the GNU Lesser Public License
#  along with this program.  If not, see <http://www.gnu.org/licenses/>.
#  


import sys
import time
from mpi4py import MPI

import scdc
import simpat
from simpat import * 
from simpat import SIMPAT_LOG_PREFIX, simpat_base

simpat.SIMPAT_LOG_PREFIX = "cli: "


scdc.log_init("log_FILE", sys.stdout, sys.stderr)

scdc.init()

simpat_init()

if len(sys.argv) <= 1:
  SIMPAT_TRACE("test: direct")
  uri = "scdc:"

elif sys.argv[1] == "uds":
  SIMPAT_TRACE("test: UDS")
  socketname = "simpat" if len(sys.argv) <= 2 else sys.argv[2]
  uri = "scdc+uds://" + socketname

elif sys.argv[1] == "tcp":
  SIMPAT_TRACE("test: TCP")
  hostname = "localhost" if len(sys.argv) <= 2 else sys.argv[2]
  uri = "scdc+tcp://" + hostname

elif sys.argv[1] == "mpi":
  SIMPAT_TRACE("test: MPI")
  uri = "scdc+mpi://" + sys.argv[2]

else:
  SIMPAT_TRACE("unknown mode: " + argv[1])
  uri = None


buf_size = 1024 * 1024

bw_put = True
bw_get = True
#bw_size = 2000000
bw_size = 1000000000

lat_ds = True
lat_ah = True
lat_size = 1
lat_num = 100

if uri:
  mode = "memory/zero"

  # bandwidth: put
  if bw_put:
    ds = scdc.dataset_open(uri + "/" + simpat.simpat_base + "/" + mode)

    input = scdc.dataset_input_create("produce:size", bw_size, buf_size)

    t = time.time()
    ret = scdc.dataset_cmd(ds, "put " + str(bw_size), input, None)
    t = time.time() - t

    scdc.dataset_input_destroy(input)

    scdc.dataset_close(ds)

    if ret:
      SIMPAT_TRACE("bandwith put: " + str(bw_size / t * 1e-6))
    else:
      SIMPAT_TRACE("bandwith put: FAILED")

  # bandwidth: get
  if bw_get:
    ds = scdc.dataset_open(uri + "/" + simpat.simpat_base + "/" + mode)

    output = scdc.dataset_output_create("consume:size", buf_size)

    t = time.time()
    ret = scdc.dataset_cmd(ds, "get " + str(bw_size), None, output)
    t = time.time() - t

    scdc.dataset_output_destroy(output)

    scdc.dataset_close(ds)

    if ret:
      SIMPAT_TRACE("bandwith get: " + str(bw_size / t * 1e-6))
    else:
      SIMPAT_TRACE("bandwith get: FAILED")

  # latency: dataset put
  if lat_ds:
    ds = scdc.dataset_open(uri + "/" + simpat.simpat_base + "/" + mode)

    input = scdc.dataset_input_create("produce:size", lat_size, buf_size)

    ret = True

    t = time.time()
    for i in xrange(lat_num):
      ret = ret and scdc.dataset_cmd(ds, "put " + str(lat_size), input, None)
    t = time.time() - t

    scdc.dataset_input_destroy(input)

    scdc.dataset_close(ds)

    if ret:
      SIMPAT_TRACE("latency dataset put: " + str(t / lat_num * 1e3))
    else:
      SIMPAT_TRACE("latency dataset put: FAILED")

  # latency: dataset get
  if lat_ds:
    ds = scdc.dataset_open(uri + "/" + simpat.simpat_base + "/" + mode)

    output = scdc.dataset_output_create("consume:size", buf_size)

    ret = True

    t = time.time()
    for i in xrange(lat_num):
      ret = ret and scdc.dataset_cmd(ds, "get " + str(lat_size), None, output)
    t = time.time() - t

    scdc.dataset_output_destroy(output)

    scdc.dataset_close(ds)

    if ret:
      SIMPAT_TRACE("latency dataset get: " + str(t / lat_num * 1e3))
    else:
      SIMPAT_TRACE("latency dataset get: FAILED")

  # latency: ad-hoc put
  if lat_ah:
    scdc.dataset_cmd(scdc.DATASET_NULL, "scdc:////CONFIG put nodeconn_cache_connections 0", None, None)

    input = scdc.dataset_input_create("produce:size", lat_size, buf_size)

    ret = True

    t = time.time()
    for i in xrange(lat_num):
      ret = ret and scdc.dataset_cmd(scdc.DATASET_NULL, uri + "/" + simpat.simpat_base + "/" + mode + " put " + str(lat_size), input, None)
    t = time.time() - t

    scdc.dataset_input_destroy(input)

    if ret:
      SIMPAT_TRACE("latency ad-hoc put: " + str(t / lat_num * 1e3))
    else:
      SIMPAT_TRACE("latency ad-hoc put: FAILED")

  # latency: ad-hoc get
  if lat_ah:
    scdc.dataset_cmd(scdc.DATASET_NULL, "scdc:////CONFIG put nodeconn_cache_connections 0", None, None)

    output = scdc.dataset_output_create("consume:size", buf_size)

    ret = True

    t = time.time()
    for i in xrange(lat_num):
      ret = ret and scdc.dataset_cmd(scdc.DATASET_NULL, uri + "/" + simpat.simpat_base + "/" + mode + " get " + str(lat_size), None, output)
    t = time.time() - t

    scdc.dataset_output_destroy(output)

    if ret:
      SIMPAT_TRACE("latency ad-hoc get: " + str(t / lat_num * 1e3))
    else:
      SIMPAT_TRACE("latency ad-hoc get: FAILED")

simpat_release()

scdc.release()

scdc.log_release()
