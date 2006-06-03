PKG_CONFIG = pkg-config --print-errors

ARCH = $(shell $(PKG_CONFIG) valgrind --variable=arch)
OS = $(shell $(PKG_CONFIG) valgrind --variable=os)
PLATFORM = $(subst -,_,$(shell $(PKG_CONFIG) valgrind --variable=platform))

LOAD_ADDR = $(shell $(PKG_CONFIG) valgrind --variable=valt_load_address)

CFLAGS = \
	$(shell $(PKG_CONFIG) valgrind --cflags) \
	-DVGA_$(ARCH)=1 \
	-DVGO_$(OS)=1 \
	-DVGP_$(PLATFORM)=1 \
	-g -Wall -Wno-pointer-sign -Werror

LDLIBS = \
	$(shell $(PKG_CONFIG) valgrind --libs)

LDFLAGS = \
	-static -Wl,-defsym,valt_load_address=$(LOAD_ADDR) \
	-nodefaultlibs -nostartfiles -u _start \
	-Wl,-T,valt_load_address_x86_linux.lds

OBJECTS = mt_main.o mt_nvidia.o

all: mmtrace

include .depend

mmtrace: $(OBJECTS)
	$(LINK.o) $^ $(LOADLIBES) $(LDLIBS) -o $@

clean:
	-rm -f $(OBJECTS) mmtrace

.PHONY: .depend
.depend:
	$(CC) $(CFLAGS) -M -MP $(OBJECTS:.o=.c) > .depend
