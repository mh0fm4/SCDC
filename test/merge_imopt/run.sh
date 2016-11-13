#!/bin/bash

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

#set -xv

run_conf=${0%/*}/run.conf

echo -n "# " && uname -a
echo -n "# " && date
echo "# $0 $*"
echo "# conf: ${run_conf}"

args=$*

set ""

[ -f "${run_conf}" ] && . "${run_conf}"

[ -n "${args}" ] && set ${args}


tofile()
{
  cat - > "$1"
}


dummy()
{
  [ $# -lt 1 ] && return

  outfile=run.out
  teeornot=tee
  maxoutsize=1GB
  dir=

  while [ "$1" != "" ] ; do
    if [ "$1" = "-o" -a $# -ge 2 ] ; then outfile="$2" ; shift 2
    elif [ "$1" = "-q" -a $# -ge 1 ] ; then teeornot=tofile ; shift 1
    elif [ "$1" = "-x" -a $# -ge 2 ] ; then maxoutsize="$2" ; shift 2
    elif [ "$1" = "-d" -a $# -ge 2 ] ; then dir="$2" ; shift 2
    else break
    fi
  done

  _pwd=`pwd`

  (
    [ -n "${dir}" ] && cd ${dir}

    echo -n "# "
    date
    tstart=`date "+%s"`

    uname -a

    while [ -n "$1" ] ; do
      if [ "$1" = "-s" ] ; then
        sleep $2
        shift
      else
        if [ -f "$1" ] ; then
          f="$1"
          echo "# cat: '${f}'"
          cat "$f"
        elif [ -d "$1" ] ; then
          echo "# dir: '$1'"
          for f in $1/* ; do
            if [ -f "${f}" ] ; then
              echo "# cat: '${f}'"
              cat ${f}
            fi
          done
        fi
      fi
      shift
    done

    echo "VTK output of DUMMY: none"
    touch ${_pwd}/${outfile}.vtk

    echo -n "# "
    date
    tend=`date "+%s"`
    tdiff=`echo "${tend}-${tstart}" | bc`
    echo "time: ${tdiff}"

    cd ${_pwd}
  ) | ${teeornot} ${outfile}
}


imf()
{
  [ $# -lt 1 ] && return

  outfile=run.out
  teeornot=tee
  maxoutsize=1GB
  dir=

  while [ "$1" != "" ] ; do
    if [ "$1" = "-o" -a $# -ge 2 ] ; then outfile="$2" ; shift 2
    elif [ "$1" = "-q" -a $# -ge 1 ] ; then teeornot=tofile ; shift 1
    elif [ "$1" = "-x" -a $# -ge 2 ] ; then maxoutsize="$2" ; shift 2
    elif [ "$1" = "-d" -a $# -ge 2 ] ; then dir="$2" ; shift 2
    else break
    fi
  done

  _pwd=`pwd`

  (
    [ -n "${dir}" ] && cd ${dir}

    echo -n "# "
    date
    tstart=`date "+%s"`

    echo ""
    ${imf} -case "." 2>&1 | head -c ${maxoutsize}
    ${imf_foamToVTK}
    imfout=`ls -G VTK/*.vtk | tail -n 1`
    if [ -f "${imfout}" ] ; then
      echo "VTK output of IMF: ${imfout}"
      mv ${imfout} ${_pwd}/${outfile}.vtk
    else
      echo "VTK output of IMF: none"
      touch ${_pwd}/${outfile}.vtk
    fi
    echo ""

    echo -n "# "
    date
    tend=`date "+%s"`
    tdiff=`echo "${tend}-${tstart}" | bc`
    echo "time: ${tdiff}"

    cd ${_pwd}
  ) | ${teeornot} ${outfile}
}


spc()
{
  [ $# -lt 1 ] && return

  outfile=run.out
  teeornot=tee
  maxoutsize=1GB
  dir=
  spc=${spc_teani}
  vtk=
  mesh=

  while [ "$1" != "" ] ; do
    if [ "$1" = "-o" -a $# -ge 2 ] ; then outfile="$2" ; shift 2
    elif [ "$1" = "-q" -a $# -ge 1 ] ; then teeornot=tofile ; shift 1
    elif [ "$1" = "-x" -a $# -ge 2 ] ; then maxoutsize="$2" ; shift 2
    elif [ "$1" = "-d" -a $# -ge 2 ] ; then dir="$2" ; shift 2
    elif [ "$1" = "-TEAni" -a $# -ge 1 ] ; then spc=${spc_teani} ; shift 1
    elif [ "$1" = "-Mer" -a $# -ge 2 ] ; then spc=${spc_mer} ; vtk="$2" ; shift 2
    else
      mesh="$1"
      break
    fi
  done

  _pwd=`pwd`

  (
    [ -n "${dir}" ] && cd ${dir}

    echo -n "# "
    date
    tstart=`date "+%s"`

    echo ""
    if [ -n "${vtk}" ] ; then
      ${imf_vtk_to_spc} -i ${_pwd}/${vtk} -o Probleme/${mesh}/Faserorientierung.vtk
      ln -s Faserorientierung.vtk Probleme/${mesh}/Temperatur.vtk
    fi
    OMP_NUM_THREADS=1 ${spc} < aquad.cmds 2>&1 | head -c ${maxoutsize}
    spcout=Output/${mesh}_def.vtk
    if [ -f "${spcout}" ] ; then
      echo "VTK output of SPC: ${spcout}"
      mv ${spcout} ${_pwd}/${outfile}.vtk
    else
      echo "VTK output of SPC: none"
      touch ${_pwd}/${outfile}.vtk
    fi
    echo ""

    echo -n "# "
    date
    tend=`date "+%s"`
    tdiff=`echo "${tend}-${tstart}" | bc`
    echo "time: ${tdiff}"

    cd ${_pwd}
  ) | ${teeornot} ${outfile}
}


[ $# -lt 1 ] && return

cmd=$1 ; shift

if [ "${cmd}" = "dummy" ] ; then
  dummy $*
elif [ "${cmd}" = "imf" ] ; then
  imf $*
elif [ "${cmd}" = "spc" ] ; then
  spc $*
fi
