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


ALL:=

CPPFLAGS:=$(CPPFLAGS)
CFLAGS:=$(CFLAGS)
CXXFLAGS:=$(CXXFLAGS)
LDFLAGS:=$(LDFLAGS)
LDCXXFLAGS:=$(LDCXXFLAGS)
LIBS:=$(LIBS)

DEPS:=
LIB:=
CEXEC:=
CXXEXEC:=

CFILES:=
HFILES:=
CEXEC_CFILES:=
CEXEC_HFILES:=
CXXFILES:=
HXXFILES:=
TCCFILES:=
CXXEXEC_CXXFILES:=
CXXEXEC_HXXFILES:=
CXXEXEC_TCCFILES:=
OFILES:=
CEXEC_OFILES:=
CXXEXEC_OFILES:=
PYFILES:=
PYCFILES:=

CLEAN:=

all: _all

-include Makefile.in
-include Makefile.local
-include Makefile.global

DEPS+=Makefile Makefile.in

CFILES+=$(wildcard *.c) $(wildcard $(addsuffix /*.c,$(SUBDIRS)))
HFILES+=$(wildcard *.h) $(wildcard $(addsuffix /*.h,$(SUBDIRS)))
CXXFILES+=$(wildcard *.cc) $(wildcard $(addsuffix /*.cc,$(SUBDIRS)))
HXXFILES+=$(wildcard *.hh) $(wildcard $(addsuffix /*.hh,$(SUBDIRS)))
TCCFILES+=$(wildcard *.tcc) $(wildcard $(addsuffix /*.tcc,$(SUBDIRS)))
PYFILES+=$(wildcard *.py) $(wildcard $(addsuffix /*.py,$(SUBDIRS)))

CEXEC_CFILES:=$(wildcard $(addsuffix .c,$(CEXEC)))
CEXEC_HFILES:=$(wildcard $(addsuffix .h,$(CEXEC)))
CXXEXEC_CXXFILES:=$(wildcard $(addsuffix .cc,$(CXXEXEC)))
CXXEXEC_HXXFILES:=$(wildcard $(addsuffix .hh,$(CXXEXEC)))
CXXEXEC_TCCFILES:=$(wildcard $(addsuffix .tcc,$(CXXEXEC)))

CFILES:=$(filter-out $(CFILES_IGNORE) $(CEXEC_CFILES),$(CFILES))
HFILES:=$(filter-out $(HFILES_IGNORE) $(CEXEC_HFILES),$(HFILES))
CXXFILES:=$(filter-out $(CXXFILES_IGNORE) $(CXXEXEC_CXXFILES),$(CXXFILES))
HXXFILES:=$(filter-out $(HXXFILES_IGNORE) $(CXXEXEC_HXXFILES),$(HXXFILES))
TCCFILES:=$(filter-out $(TCCFILES_IGNORE) $(CXXEXEC_TCCFILES),$(TCCFILES))
PYFILES:=$(filter-out $(PYFILES_IGNORE),$(PYFILES))

OFILES+=$(addsuffix .o,$(basename $(CFILES) $(CXXFILES)))
CEXEC_OFILES+=$(addsuffix .o,$(basename $(CEXEC_CFILES)))
CXXEXEC_OFILES+=$(addsuffix .o,$(basename $(CXXEXEC_CXXFILES)))

PYCFILES+=$(addsuffix .pyc,$(basename $(PYFILES)))

PYMOD_BUILD:=build
PYMOD_INSTALL:=install

CLEAN+=$(LIB) $(CEXEC) $(CXXEXEC) $(OFILES) $(CEXEC_OFILES) $(CXXEXEC_OFILES) $(PYCFILES) $(PYMOD_BUILD) $(PYMOD_INSTALL)

ifeq ($(ALL),)
 ALL:=sub lib exec
endif

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


_all: $(ALL)

sub:
	$(foreach d,$(wildcard $(addsuffix /Makefile,$(SUBDIRS))),$(MAKE) -C $(dir $(d)) ;)

sub_clean:
	$(foreach d,$(wildcard $(addsuffix /Makefile,$(SUBDIRS))),$(MAKE) -C $(dir $(d)) clean;)

sub_info:
	@echo "SUBDIRS: $(SUBDIRS)"
	$(foreach d,$(wildcard $(addsuffix /Makefile,$(SUBDIRS))),$(MAKE) -C $(dir $(d)) info;)

lib: $(LIB)

exec: sub $(CEXEC) $(CXXEXEC)

pymod: setup.py
	PYMOD_INCDIRS="$(PYMOD_INCDIRS)" PYMOD_LIBS="$(PYMOD_LIBS)" PYMOD_LIBDIRS="$(PYMOD_LIBDIRS)" PYMOD_SOURCES="$(CFILES) $(PYMOD_SOURCES)" PYMOD_DEPENDS="$(HFILES) $(PYMOD_DEPENDS)" PYMOD_MACROS="$(PYMOD_MACROS)" OPT="$(CFLAGS)" $(PYTHON) setup.py build --build-base=$(PYMOD_BUILD)
	PYMOD_INCDIRS="$(PYMOD_INCDIRS)" PYMOD_LIBS="$(PYMOD_LIBS)" PYMOD_LIBDIRS="$(PYMOD_LIBDIRS)" PYMOD_SOURCES="$(CFILES) $(PYMOD_SOURCES)" PYMOD_DEPENDS="$(HFILES) $(PYMOD_DEPENDS)" PYMOD_MACROS="$(PYMOD_MACROS)" OPT="$(CFLAGS)" $(PYTHON) setup.py install --home=$(PYMOD_INSTALL)

%.o: %.c $(HFILES) $(CEXEC_HFILES) $(DEPS)
	$(CC_) $(CPPFLAGS) $($(notdir $<)_CPPFLAGS) $(CFLAGS_) $($(notdir $<)_CFLAGS) -c -o $@ $<

%.o: %.cc $(HFILES) $(CEXEC_HFILES) $(HXXFILES) $(CXXEXEC_HXXFILES) $(TCCFILES) $(CXXEXEC_TCCFILES) $(DEPS)
	$(CXX_) $(CPPFLAGS) $($(notdir $<)_CPPFLAGS) $(CXXFLAGS_) $($(notdir $<)_CXXFLAGS) -c -o $@ $<

$(LIB): $(OFILES) $(AFILES)
	$(AR) r $@ $(OFILES)
	$(if $(AFILES),rm -rf .libs && mkdir .libs && (cd .libs $(foreach a,$(AFILES),&& $(AR) x ../$(a)) ) && $(AR) r $@ .libs/*.o && rm -rf .libs)

$(CEXEC): $(OFILES) $(CEXEC_OFILES) $(AFILES) $(DEPS)
	$(LD_) $($@_LDFLAGS) $(LDFLAGS_) -o $@ $@.o $(OFILES) $(AFILES) $($@_LIBS) $(LIBS)

$(CXXEXEC): $(OFILES) $(CXXEXEC_OFILES) $(AFILES) $(DEPS)
	$(LDCXX_) $($@_LDFLAGS) $(LDFLAGS_) $($@_LDCXXFLAGS) $(LDCXXFLAGS_) -o $@ $@.o $(OFILES) $(AFILES) $($@_LIBS) $(LIBS)

clean: sub_clean
ifneq ($(wildcard $(CLEAN)),)
	rm -rf $(wildcard $(CLEAN))
endif

info: sub_info
	@echo "DEPS: $(DEPS)"
	@echo "LIB: $(LIB)"
	@echo "CEXEC: $(CEXEC)"
	@echo "CFILES: $(CFILES)"
	@echo "HFILES: $(HFILES)"
	@echo "CEXEC_CFILES: $(CEXEC_CFILES)"
	@echo "CEXEC_HFILES: $(CEXEC_HFILES)"
	@echo "CXXEXEC: $(CXXEXEC)"
	@echo "CXXFILES: $(CXXFILES)"
	@echo "HXXFILES: $(HXXFILES)"
	@echo "TCCFILES: $(TCCFILES)"
	@echo "CXXEXEC_CXXFILES: $(CXXEXEC_CXXFILES)"
	@echo "CXXEXEC_HXXFILES: $(CXXEXEC_HXXFILES)"
	@echo "CXXEXEC_TCCFILES: $(CXXEXEC_TCCFILES)"
	@echo "OFILES: $(OFILES)"
	@echo "CEXEC_OFILES: $(CEXEC_OFILES)"
	@echo "CXXEXEC_OFILES: $(CXXEXEC_OFILES)"
	@echo "PYFILES: $(PYFILES)"
	@echo "PYCFILES: $(PYCFILES)"
	@echo "CLEAN: $(CLEAN)"
