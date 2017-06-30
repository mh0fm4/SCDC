#  
#  Copyright (C) 2014, 2015, 2016, 2017 Michael Hofmann
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
CFILES:=$(wildcard *fileio_*.c)
OFILES:=$(addsuffix .o,$(basename $(CFILES)))

LIB:=libfileio_scdc.a
LIBSO:=libfileio_scdc.so
EXEC:=libfileio_scdc_srv test_fileio test_fileio_linked

CFLAGS+=-fPIC -std=c99
#CFLAGS+=-Wpedantic

libfileio_scdc.so_LIBS:=-ldl

linked_DEPS:=$(LIBSO)
linked_LIBS:=$(CURDIR)/$(LIBSO) -ldl

DEPS:=Makefile Makefile.in

LIBS:=

-include Makefile.in

SCDC_DEPS:=../scdc/libscdc.a
SCDC_LIBS:=$(LIBS)

CC_:=$(CC)
CXX_:=$(CXX)
LD_:=$(LD)
LDCXX_:=$(LDCXX)
CFLAGS_:=$(CFLAGS)
CXXFLAGS_:=$(CXXFLAGS)
LDFLAGS_:=$(LDFLAGS)
LDCXXFLAGS_:=$(LDCXXFLAGS)
ifneq ($(USE_MPI),)
 CC_:=$(MPICC)
 CXX_:=$(MPICXX)
 LD_:=$(MPILD)
 LDCXX_:=$(MPILDCXX)
 CFLAGS_+=$(MPICFLAGS)
 CXXFLAGS_+=$(MPICXXFLAGS)
 LDFLAGS_+=$(MPILDFLAGS)
 LDCXXFLAGS_+=$(MPILDCXXFLAGS)
endif


all: $(LIB) $(LIBSO) $(EXEC)

%.o: %.c $(HFILES) $(DEPS)
	$(CC_) $(CPPFLAGS) $(CFLAGS_) -c -o $@ $<

$(LIB): $(OFILES) $(DEPS)
	$(AR) r $@ $(OFILES)

$(LIBSO): $(OFILES) $(SCDC_DEPS) $(DEPS)
	$(LD_) $(LDFLAGS_) -shared -o $@ $(OFILES) $($@_LIBS) $(SCDC_LIBS)

libfileio_scdc_srv: libfileio_scdc_srv.c $(SCDC_DEPS) $(DEPS)
	$(CC_) $(CPPFLAGS) $(CFLAGS_) $(LDFLAGS_) -o $@ $< $($@_LIBS) $(SCDC_LIBS)

%: %.c $(DEPS)
	$(CC_) $(CPPFLAGS) $(CFLAGS_) $(LDFLAGS_) -o $@ $< $($@_LIBS)

%_linked: %.c $(linked_DEPS) $(DEPS)
	$(CC_) $(CPPFLAGS) $(CFLAGS_) $(LDFLAGS_) -o $@ $< $($@_LIBS) $(linked_LIBS)

clean: Makefile
	rm -rf $(OFILES) $(LIB) $(LIBSO) $(EXEC)

info: 
	@echo "HFILES: $(HFILES)"
	@echo "CFILES: $(CFILES)"
	@echo "OFILES: $(OFILES)"
