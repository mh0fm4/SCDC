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

import scdc


SIMPAT_TRACE_PREFIX = "SIMPAT_TRACE: "
SIMPAT_LOG_PREFIX = ""

def SIMPAT_TRACE_N(x):
  if sys.version_info[0] < 3:
    sys.stdout.write(SIMPAT_TRACE_PREFIX + SIMPAT_LOG_PREFIX + x)
  else:
#    print(SIMPAT_TRACE_PREFIX + x, end = "")
    sys.stdout.write(SIMPAT_TRACE_PREFIX + SIMPAT_LOG_PREFIX + x)

def SIMPAT_TRACE(x):
  SIMPAT_TRACE_N(x)
  print("")


simpat_base = "simpat"
simpat_dp = None


def simpat_init():
  simpat_dp = scdc.dataprov_open(simpat_base, "bench")


def simpat_release():
  scdc.dataprov_close(simpat_dp)
