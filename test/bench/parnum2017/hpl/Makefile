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


HPL:=hpl-2.1

all:
	make -C $(HPL) arch=Linux clean
	make -C $(HPL) arch=Linux
	mv $(HPL)/bin/Linux/xhpl $(HPL)/bin/Linux/xhpl_openblas
	make -C $(HPL) arch=Linux clean
	make -C $(HPL) arch=Linux SCDC=yes
	mv $(HPL)/bin/Linux/xhpl $(HPL)/bin/Linux/xhpl_blas_scdc

clean:
	make -C $(HPL) arch=Linux clean
	rm -f $(HPL)/bin/Linux/xhpl*
