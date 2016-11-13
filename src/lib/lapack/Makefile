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


HFILES:=$(wildcard *.h)
CFILES:=$(wildcard *lapack_*.c)
OFILES:=$(addsuffix .o,$(basename $(CFILES)))

LIB:=liblapack_scdc.a
EXEC:=liblapack_scdc_srv test_sgesv

LIBS:=$(LIB)
LIBS+=-lopenblas

-include Makefile.in


all: $(LIB) $(EXEC)

$(LIB): $(OFILES)
	$(AR) r $@ $(OFILES)

%.o: %.c $(HFILES) Makefile
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

%: %.c $(LIB)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) -o $@ $< $(LIBS)

clean: Makefile
	rm -rf $(OFILES) $(LIB) $(EXEC)

info: 
	@echo "HFILES: $(HFILES)"
	@echo "CFILES: $(CFILES)"
	@echo "OFILES: $(OFILES)"
