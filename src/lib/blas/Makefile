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


NAME:=BLAS
name:=blas

LIB:=lib$(name)_scdc.a
LIBSO:=lib$(name)_scdc.so
EXEC:=lib$(name)_scdc_srv test_$(name)_native test_$(name)_scdc test_$(name)_scdc_linked
EXEC_FILTER:=test_$(name)%

HFILES:=$(wildcard *.h)
CFILES:=$(wildcard *.c)
HXXFILES:=$(wildcard *.hh)
CXXFILES:=$(wildcard *.cc)

HFILES_EXEC:=$(filter $(addsuffix %,$(EXEC)) $(EXEC_FILTER),$(HFILES))
CFILES_EXEC:=$(filter $(addsuffix %,$(EXEC)) $(EXEC_FILTER),$(CFILES))
HXXFILES_EXEC:=$(filter $(addsuffix %,$(EXEC)) $(EXEC_FILTER),$(HXXFILES))
CXXFILES_EXEC:=$(filter $(addsuffix %,$(EXEC)) $(EXEC_FILTER),$(CXXFILES))

HFILES_LIB:=$(filter-out $(HFILES_EXEC), $(HFILES))
CFILES_LIB:=$(filter-out $(CFILES_EXEC), $(CFILES))
HXXFILES_LIB:=$(filter-out $(HXXFILES_EXEC), $(HXXFILES))
CXXFILES_LIB:=$(filter-out $(CXXFILES_EXEC), $(CXXFILES))

OFILES:=$(addsuffix .o,$(basename $(CFILES) $(CXXFILES)))
OFILES_LIB:=$(addsuffix .o,$(basename $(CFILES_LIB) $(CXXFILES_LIB)))
OFILES_EXEC:=$(addsuffix .o,$(basename $(CFILES_EXEC) $(CXXFILES_EXEC)))
OFILES_EXEC_COMMON:=$(foreach f,$(basename $(CFILES_EXEC) $(CXXFILES_EXEC)),$(filter $(f)_%.o,$(OFILES_EXEC)))

$(NAME)_LDFLAGS:=
$(NAME)_LIBS:=-lopenblas

REDIRECT_DEPENDS:=../redirect/redirect_call.h ../redirect/libredirect_scdc.a
REDIRECT_CPPFLAGS:=-I../redirect
REDIRECT_LDFLAGS:=-L../redirect
REDIRECT_LIBS:=-lredirect_scdc

-include Makefile.in

DEPS+=Makefile Makefile.in $(REDIRECT_DEPENDS)
#CPPFLAGS+=-DLIB$(NAME)_SCDC_PREFIX=1
CPPFLAGS+=$(REDIRECT_CPPFLAGS)
CFLAGS+=-fPIC #-std=c99
#CFLAGS+=-Wpedantic
LDFLAGS+=

$(NAME)_SCDC_DEPENDS:=$(LIB) $(LIBSO)
$(NAME)_SCDC_CPPFLAGS:=
$(NAME)_SCDC_LDFLAGS:=$(REDIRECT_LDFLAGS) $($(NAME)_LDFLAGS)
$(NAME)_SCDC_LIBS:=-ldl $(REDIRECT_LIBS) $($(NAME)_LIBS)
$(NAME)_SCDC_A_LIBS:=lib$(name)_scdc.a $($(NAME)_SCDC_LIBS)
$(NAME)_SCDC_SO_LIBS:=lib$(name)_scdc.so -ldl

LIBS+=-lm


all: $(LIB) $(LIBSO) $(EXEC)

%.o: %.c $(HFILES) $(DEPS)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

%.o: %.cc $(HFILES) $(HXXFILES) $(DEPS)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $<

$(LIB): $(OFILES_LIB)
	$(AR) r $@ $(OFILES_LIB)

$(LIBSO): $(OFILES_LIB)
	$(LD) $(LDFLAGS) $($(NAME)_SCDC_LDFLAGS) -shared -o $@ $(OFILES_LIB) $($(NAME)_SCDC_LIBS) $(LIBS)

lib$(name)_scdc_srv: lib$(name)_scdc_srv.c $(CFILES_EXEC) $(DEPS) $($(NAME)_SCDC_DEPENDS)
	$(CC) $(CPPFLAGS) $(LDFLAGS) $($(NAME)_SCDC_LDFLAGS) -o $@ $< $($(NAME)_SCDC_A_LIBS) $(LIBS)

%_native: %.c $(OFILES_EXEC_COMMON) $(DEPS)
	$(CC) $(CPPFLAGS) -DLIB$(NAME)_SCDC=0 $(LDFLAGS) $($(NAME)_LDFLAGS) -o $@ $< $(OFILES_EXEC_COMMON) $($(NAME)_LIBS) $(LIBS)

%_scdc: %.c $(OFILES_EXEC_COMMON) $($(NAME)_SCDC_DEPENDS) $(DEPS)
	$(CC) $(CPPFLAGS) -DLIB$(NAME)_SCDC=1 $(LDFLAGS) $($(NAME)_SCDC_LDFLAGS) -o $@ $< $(OFILES_EXEC_COMMON) $($(NAME)_SCDC_A_LIBS) $(LIBS)

%_scdc_linked: %.c $(OFILES_EXEC_COMMON) $($(NAME)_SCDC_DEPENDS) $(DEPS)
	$(CC) $(CPPFLAGS) -DLIB$(NAME)_SCDC=1 $(LDFLAGS) $($(NAME)_SCDC_LDFLAGS) -o $@ $< $(OFILES_EXEC_COMMON) $($(NAME)_SCDC_SO_LIBS) $(LIBS)

clean: Makefile
	rm -rf $(OFILES) $(LIB) $(LIBSO) $(EXEC)

info: 
	@echo "HFILES: $(HFILES)"
	@echo "CFILES: $(CFILES)"
	@echo "HXXFILES: $(HXXFILES)"
	@echo "CXXFILES: $(CXXFILES)"
	@echo "OFILES: $(OFILES)"
	@echo "HFILES_LIB: $(HFILES_LIB)"
	@echo "CFILES_LIB: $(CFILES_LIB)"
	@echo "HXXFILES_LIB: $(HXXFILES_LIB)"
	@echo "CXXFILES_LIB: $(CXXFILES_LIB)"
	@echo "OFILES_LIB: $(OFILES_LIB)"
	@echo "HFILES_EXEC: $(HFILES_EXEC)"
	@echo "CFILES_EXEC: $(CFILES_EXEC)"
	@echo "HXXFILES_EXEC: $(HXXFILES_EXEC)"
	@echo "CXXFILES_EXEC: $(CXXFILES_EXEC)"
	@echo "OFILES_EXEC: $(OFILES_EXEC)"
	@echo "OFILES_EXEC_COMMON: $(OFILES_EXEC_COMMON)"
	@echo "DEPS: $(DEPS)"
	@echo "CPPFLAGS: $(CPPFLAGS)"
	@echo "CFLAGS: $(CFLAGS)"
	@echo "CXXFLAGS: $(CXXFLAGS)"
	@echo "LDFLAGS: $(LDFLAGS)"
	@echo "LIBS: $(LIBS)"
