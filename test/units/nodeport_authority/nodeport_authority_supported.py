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


import os
import sys

from mpi4py import MPI

import scdc


scdc.log_init("log_FILE", sys.stdout, sys.stderr)

#scdc.init()

print scdc.nodeport_authority("direct", "ID")
print scdc.nodeport_supported("scdc:/")

print scdc.nodeport_authority("tcp", "hostname")
print scdc.nodeport_authority("tcp:port", "hostname", 45678)
print scdc.nodeport_supported("scdc+tcp://localhost/")

print scdc.nodeport_authority("uds", "socketname")
print scdc.nodeport_supported("scdc+uds:/")

if scdc.USE_MPI:
  rank = 1
  print scdc.nodeport_authority("mpi", rank)
  print scdc.nodeport_authority("mpi:world", rank)
  print scdc.nodeport_authority("mpi:comm", MPI.COMM_WORLD, rank)
  print scdc.nodeport_authority("mpi:port", "PORTNAME", rank)
  print scdc.nodeport_authority("mpi:publ", "PUBLNAME", rank)
#  print scdc.nodeport_authority("mpi:fd", ???, rank)

print scdc.nodeport_supported("scdc+mpi://world:1/")


#scdc.release()

scdc.log_release()
