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
from mpi4py import MPI

import scdc
import simpat
from simpat import * 
from simpat import SIMPAT_LOG_PREFIX

simpat.SIMPAT_LOG_PREFIX = "srv: "


scdc.log_init("log_FILE", sys.stdout, sys.stderr)

scdc.init()

simpat_init()

np = None

if len(sys.argv) <= 1:
  SIMPAT_TRACE("setup: none")

elif sys.argv[1] == "uds":
  SIMPAT_TRACE("setup: UDS")
  socketname = "simpat" if len(sys.argv) <= 2 else sys.argv[2]
  np = scdc.nodeport_open("uds:socketname:max_connections", socketname, 2)

elif sys.argv[1] == "tcp":
  SIMPAT_TRACE("setup: TCP")
  np = scdc.nodeport_open("tcp:max_connections", 2)

elif sys.argv[1] == "mpi":
  SIMPAT_TRACE("setup: MPI")
  np = scdc.nodeport_open("mpi")

else:
  SIMPAT_TRACE("unknown mode: " + argv[1])


SIMPAT_TRACE("start")
scdc.nodeport_start(np, scdc.NODEPORT_START_ASYNC_UNTIL_CANCEL)
#scdc.nodeport_start(np, scdc.NODEPORT_START_LOOP_UNTIL_CANCEL)

try:
  raw_input("Press <ENTER> to quit!\n")
except:
  pass

SIMPAT_TRACE("stop")
scdc.nodeport_stop(np)

SIMPAT_TRACE("release")
scdc.nodeport_close(np)

simpat_release()

scdc.release()

scdc.log_release()
