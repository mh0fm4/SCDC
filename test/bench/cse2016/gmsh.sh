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

HOST=localhost
CLI_N=1
CLI_D=

STORE="store/"
SCP_STORE="store/"

while [ -n "$1" ] ; do

  case "$1" in
    "-h")
      HOST=$2
      shift
      ;;
    "-n")
      CLI_N=$2
      shift
      ;;
    "-d")
      CLI_D=$2
      shift
      ;;
    "-s")
      STORE=$2
      shift
      ;;
    "sto")
      ARG_STO=yes
      modes="store"
      ;;
    "rly")
      ARG_RLY=yes
      modes="relay"
      ;;
    "srv")
      ARG_SRV=yes
      ;;
    "scp")
      ARG_SCP=yes
      ;;
    *)
      ARG_CLI=yes
      CLI_RUNS=yes
      modes="${modes} $1"
      ;;
    esac

  shift
done

GMSH_CLI=/usr/bin/gmsh
GMSH_SCP_CLI=./gmsh_scp.sh
GMSH_SRV=./cse2016_srv

GMSH_CLI_IN_FILE=porsche.stl
GMSH_CLI_OUT_FILE=porsche.msh

case "${CLI_D}" in
  0tsm)
    GMSH_CLI_IN_FILE=porsche_0t.stl
    GMSH_CLI_OUT_FILE=porsche_0t_out.msh
    ;;
  0bsm)
    CLI_OPTS="${CLI_OPTS} -bin"
    GMSH_CLI_IN_FILE=porsche_0b.stl
    GMSH_CLI_OUT_FILE=porsche_0b_out.msh
    ;;
  0tms)
    GMSH_CLI_IN_FILE=porsche_0t.msh
    GMSH_CLI_OUT_FILE=porsche_0t_out.stl
    ;;
  0bms)
    CLI_OPTS="${CLI_OPTS} -bin"
    GMSH_CLI_IN_FILE=porsche_0b.msh
    GMSH_CLI_OUT_FILE=porsche_0b_out.stl
    ;;
  1tsm)
    GMSH_CLI_IN_FILE=porsche_1t.stl
    GMSH_CLI_OUT_FILE=porsche_1t_out.msh
    ;;
  1bsm)
    CLI_OPTS="${CLI_OPTS} -bin"
    GMSH_CLI_IN_FILE=porsche_1b.stl
    GMSH_CLI_OUT_FILE=porsche_1b_out.msh
    ;;
  1tms)
    GMSH_CLI_IN_FILE=porsche_1t.msh
    GMSH_CLI_OUT_FILE=porsche_1t_out.stl
    ;;
  1bms)
    CLI_OPTS="${CLI_OPTS} -bin"
    GMSH_CLI_IN_FILE=porsche_1b.msh
    GMSH_CLI_OUT_FILE=porsche_1b_out.stl
    ;;
esac


if [ "${HOST}" = "gupta" ] ; then
  SCP_STORE="/mnt/data_ssd/mhofma/store/"
fi

CLI=${ARG_CLI}
CLI_OPTS="${CLI_OPTS}"
SRV=${ARG_SRV}
SRV_OPTS="${SRV_OPTS} -n tcp"
STO=${ARG_STO}
STO_OPTS="${STO_OPTS} -s store ${STORE}"
RLY=${ARG_RLY}
RLY_OPTS="${RLY_OPTS} -r relay store scdc+tcp://${HOST}/store"
#RLY_OPTS="${RLY_OPTS} -r relay store scdc:/store"
SCP=${ARG_SCP}

if [ -n "${RLY}" ] ; then
  REMOTE="scdc+tcp://${HOST}/relay/"
else
  REMOTE="scdc+tcp://${HOST}/"
#  REMOTE="scdc:/"
fi

for m in ${modes} ; do

  echo "#mode: ${m}"

  case "${m}" in

    local_local)
      CLI_IN="${STORE}"
      CLI_OUT="${STORE}"
      ;;

    remote_local)
      CLI_IN="${REMOTE}${STORE}"
      CLI_OUT="${STORE}"
      if [ -n "${SCP}" ] ; then
        CLI_IN="${HOST}:${SCP_STORE}"
      fi
      ;;

    local_remote)
      CLI_IN="${STORE}"
      CLI_OUT="${REMOTE}${STORE}"
      if [ -n "${SCP}" ] ; then
        CLI_OUT="${HOST}:${SCP_STORE}"
      fi
      ;;

    remote_remote)
      CLI_IN="${REMOTE}${SCP_STORE}"
      CLI_OUT="${REMOTE}${SCP_STORE}"
      if [ -n "${SCP}" ] ; then
        CLI_IN="${HOST}:${SCP_STORE}"
        CLI_OUT="${HOST}:${SCP_STORE}"
      fi
      ;;

    *)
      [ -n "${CLI_RUNS}" ] && continue
      ;;

  esac

  if [ -n "${SRV}" ] ; then

    OPTS="${SRV_OPTS}"

    if [ -n "${STO}" ] ; then
      echo "#sto"
      OPTS="${OPTS} ${STO_OPTS}"
    fi

    if [ -n "${RLY}" ] ; then
      echo "#rly"
      OPTS="${OPTS} ${RLY_OPTS}"
    fi

    echo "#srv: ${GMSH_SRV} ${OPTS}"
    ${GMSH_SRV} ${OPTS}

  elif [ -n "${CLI}" ] ; then

    echo "#cli"

    OPTS="${CLI_OPTS} -0 -o ${CLI_OUT}${GMSH_CLI_OUT_FILE} ${CLI_IN}${GMSH_CLI_IN_FILE}"

    N="${CLI_N}"
    while [ "${N}" -gt "0" ] ; do
      echo "#N: ${N}"

      if [ -n "${SCP}" ] ; then
        echo "#${GMSH_SCP_CLI} ${OPTS}"
        /usr/bin/time ${GMSH_SCP_CLI} ${OPTS}
      else
        echo "#${GMSH_CLI} ${OPTS}"
        LD_PRELOAD=../../src/lib/fileio/libfileio_scdc.so \
        LIBFILEIO_SCDC_LOCAL_BASE="store" \
        LIBFILEIO_SCDC_LOCAL_PATH="store" \
        /usr/bin/time ${GMSH_CLI} ${OPTS}
      fi

      N=$((N-1))
    done

  fi

done
