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

if [ "$1" = "srv" ] ; then
  ARG_SRV=yes
  shift
else
  ARG_CLI=yes
fi

modes=$*

TCP_HOST=ws1g

HPCS2016_CLI=./hpcs2016
HPCS2016_SRV=./hpcs2016_srv


for m in ${modes} ; do

  echo "#mode: ${m}"

  CLI=${ARG_CLI}
  CLI_OPTS=
  SRV=${ARG_SRV}
  SRV_OPTS=

  case "${m}" in

    direct_tmp)
      CLI_DST="/tmp/mhofma/store/"
      ;;

    direct_nfs)
      CLI_DST="store/"
      ;;

    scdc_direct_tmp)
      CLI_DST="scdc:///store/"
      CLI_OPTS="-s store /tmp/mhofma/store/"
      ;;

    scdc_direct_nfs)
      CLI_DST="scdc:///store/"
      CLI_OPTS="-s store store/"
      ;;

    scdc_uds)
      CLI_DST="scdc+uds:///store/"
      SRV_OPTS="-s store /tmp/mhofma/store/ -n uds"
      ;;

    scdc_tcp)
      CLI_DST="scdc+tcp://${TCP_HOST}/store/"
      SRV_OPTS="-s store /tmp/mhofma/store/ -n tcp"
      ;;

    scdc_mpi)
      CLI_DST="scdc+mpi://publ:hpcs:0/store/"
      SRV_OPTS="-s store /tmp/mhofma/store/ -n mpi"
      ;;

  esac

  if [ -n "${SRV}" ] ; then

    echo "#srv"

    ${HPCS2016_SRV} ${SRV_OPTS}

  fi

  if [ -n "${CLI}" ] ; then

    echo "#cli"

    BW_RD_SIZE=24000000000
    BW_WR_SIZE=12000000000
#    ${HPCS2016_CLI} ${CLI_OPTS} -z ${BW_RD_SIZE} bw:rd ${CLI_DST}bench_rd.dat
#    ${HPCS2016_CLI} ${CLI_OPTS} -z ${BW_WR_SIZE} bw:wr ${CLI_DST}bench_wr.dat

    LT_RD_SIZE_CONT=24000000000
    LT_WR_SIZE_CONT=12000000000
    LT_RD_NRW_CONT=1000000000
    LT_WR_NRW_CONT=1000000000
#    ${HPCS2016_CLI} ${CLI_OPTS} -z ${LT_RD_SIZE_CONT} -r ${LT_RD_NRW_CONT} lt:rd:cont ${CLI_DST}bench_rd.dat
#    ${HPCS2016_CLI} ${CLI_OPTS} -z ${LT_WR_SIZE_CONT} -r ${LT_WR_NRW_CONT} lt:wr:cont ${CLI_DST}bench_wr.dat

    LT_RD_SIZE_RAND=24000000000
    LT_WR_SIZE_RAND=12000000000
    LT_RD_NRW_RAND=100000
    LT_WR_NRW_RAND=100000
    ${HPCS2016_CLI} ${CLI_OPTS} -z ${LT_RD_SIZE_RAND} -r ${LT_RD_NRW_RAND} lt:rd:rand ${CLI_DST}bench_rd.dat
    ${HPCS2016_CLI} ${CLI_OPTS} -z ${LT_WR_SIZE_RAND} -r ${LT_WR_NRW_RAND} lt:wr:rand ${CLI_DST}bench_wr.dat

  fi

done
