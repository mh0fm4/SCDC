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

set -xv

if [ "$1" = "srv" ] ; then
  ARG_SRV=yes
  shift
else
  ARG_CLI=yes
fi

modes=$*

TCP_HOST=ws1g

#BONNIE_CLI=/usr/sbin/bonnie
BONNIE_CLI=/home/mhofma/bonnie++-1.97.3/bonnie++
BONNIE_SRV=./cse2016_srv


for m in ${modes} ; do

  echo "#mode: ${m}"

  CLI=${ARG_CLI}
  CLI_OPTS=
  SRV=${ARG_SRV}
  SRV_OPTS=

  case "${m}" in

    direct_tmp)
      CLI_DST="/tmp/mhofma/store/"
      CLI_OPTS="-d ${CLI_DST}"
      ;;

    direct_nfs)
      CLI_DST="store/"
      CLI_OPTS="-d ${CLI_DST}"
      ;;

    scdc_direct_tmp)
      CLI_DST="scdc:///store/"
      SRV_OPTS="/tmp/mhofma/store/"
      ;;

    scdc_direct_nfs)
      CLI_DST="scdc:///store/"
      SRV_OPTS="store/"
      ;;

    scdc_uds)
      CLI_DST="scdc+uds:///store/"
      SRV_OPTS="-s store /tmp/mhofma/store/ -n uds"
      ;;

    scdc_tcp)
      CLI_DST="scdc+tcp://${TCP_HOST}/store/"
      SRV_OPTS="-s store /tmp/mhofma/store/ -n tcp"
      ;;

#    scdc_mpi)
#      CLI_DST="scdc+mpi://world:0/store/"
#      SRV_OPTS="-s store /tmp/mhofma/store/ -n mpi"
#      ;;

  esac

  if [ -n "${SRV}" ] ; then

    echo "#srv"

    ${BONNIE_SRV} ${SRV_OPTS}

  fi

  if [ -n "${CLI}" ] ; then

    echo "#cli"

    RAM="10"
    SIZE="20"

    CLI_OPTS="${CLI_OPTS} -n 0 -r ${RAM} -s ${SIZE}"

    LD_PRELOAD=../../src/lib/fileio/libfileio_scdc.so \
    LIBFILEIO_SCDC_PATH_MATCH="./Bonnie." \
    LIBFILEIO_SCDC_PATH_UNPREFIX="./" \
    LIBFILEIO_SCDC_PATH_PREFIX="${CLI_DST}" \
    LIBFILEIO_SCDC_LOCAL_BASE="store" \
    LIBFILEIO_SCDC_LOCAL_PATH="${SRV_OPTS}" \
    ${BONNIE_CLI} ${CLI_OPTS}

  fi

done
