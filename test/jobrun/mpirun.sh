#!/bin/sh

#  
#  Copyright (C) 2014, 2015, 2016 Michael Hofmann
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


np=0
machinefile=
hosts=

while [ -n "$1" ] ; do
  if [ "$1" = "-np" ] ; then
    np="$2"
    shift 2
  elif [ "$1" = "-machinefile" ] ; then
    machinefile="$2"
    shift 2
  elif [ "$1" = "-hosts" ] ; then
    hosts="$2"
    shift 2
  else
    cmd="$*"
    break
  fi
done


echo "mpirun.sh: executing command '${cmd}' on ${np} process(es) with hosts '${hosts}' or machinefile '${machinefile}'"

[ -n "${cmd}" ] && eval ${cmd}
