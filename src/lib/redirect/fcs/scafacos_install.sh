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


rm -rf scafacos_build scafacos_install

SCAFACOS=`pwd`/scafacos
SCAFACOS_BUILD=`pwd`/scafacos_build
SCAFACOS_INSTALL=`pwd`/scafacos_install

mkdir -p ${SCAFACOS_BUILD}

cd ${SCAFACOS_BUILD}

${SCAFACOS}/configure -C --enable-fcs-solvers=direct --disable-doc --disable-fcs-fortran --prefix="${SCAFACOS_INSTALL}" CFLAGS='-Wall -ggdb' CXXFLAGS='-Wall -ggdb' FCFLAGS='-Wall -ggdb'

make -j 4 && make install
