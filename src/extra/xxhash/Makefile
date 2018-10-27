
MAKEFILE_IN:=Makefile.in
DEPS:=Makefile $(MAKEFILE_IN)

-include Makefile.in


all: xxhash.o

clean:
	rm -f xxhash.o

xxhash.o: xxhash.c xxhash.h $(DEPS)
	@touch xxhash.c
	$(MAKE) -f Makefile.original $@ CC=$(CC) #CXXFLAGS="$(CXXFLAGS)" CFLAGS="$(CFLAGS)"

info:
	@echo DEPS: $(DEPS)
	@echo CPPFLAGS: $(CPPFLAGS)
	@echo CFLAGS: $(CFLAGS)
