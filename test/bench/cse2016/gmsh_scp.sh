#!/bin/sh

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

#set -xv

GMSH=/usr/bin/gmsh

while [ -n "$1" ] ; do

  case "$1" in
    -o)
      shift
      if [ "${1#*:}" != "$1" ] ; then
        GMSH_DST="gmsh_dst.${1##*.}"
        cmd="${cmd} -o ${GMSH_DST}"
        SCP_DST="$1"
      else
        cmd="${cmd} -o $1"
      fi
      ;;

    *)
      if [ "${1#*:}" != "$1" ] ; then
        SCP_SRC="$1"
        GMSH_SRC="gmsh_src.${1##*.}"
        cmd="${cmd} ${GMSH_SRC}"
      else
        cmd="${cmd} $1"
      fi
  esac

  shift

done


echo "SCP_SRC: ${SCP_SRC}"
echo "GMSH_SRC: ${GMSH_SRC}"
echo "GMSH: ${GMSH} ${cmd}"
echo "GMSH_DST: ${GMSH_DST}"
echo "SCP_DST: ${SCP_DST}"

#scp -o 'ProxyCommand ssh mhofma@fiona nc %h %p'

if [ -n "${SCP_SRC}" -a -n "${GMSH_SRC}" ] ; then
  scp -o 'ProxyCommand ssh fiona nc %h %p' "${SCP_SRC}" "${GMSH_SRC}"
fi

${GMSH} ${cmd}

if [ -n "${GMSH_DST}" -a -n "${SCP_DST}" ] ; then
  scp -o 'ProxyCommand ssh fiona nc %h %p' "${GMSH_DST}" "${SCP_DST}"
fi

[ -n "${GMSH_SRC}" ] && rm -f "${GMSH_SRC}"
[ -n "${GMSH_DST}" ] && rm -f "${GMSH_DST}"
