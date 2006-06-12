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
	-DMMTRACE=1 \
	-Invidia/ \
	-g -Wall -Wno-pointer-sign -Werror

LDLIBS = \
	$(shell $(PKG_CONFIG) valgrind --libs)

LDFLAGS = \
	-static -Wl,-defsym,valt_load_address=$(LOAD_ADDR) \
	-nodefaultlibs -nostartfiles -u _start \
	-Wl,-T,valt_load_address_x86_linux.lds

OBJECTS = mt_main.o mt_nvidia.o mt_nvidia_fifo.o mt_nvidia_ioctl.o mt_nvidia_objects.o
#OBJECTS = mt_main.o mt_client.o

all: mmtrace

include .depend

mmtrace: $(OBJECTS)
	$(LINK.o) $^ $(LOADLIBES) $(LDLIBS) -o $@

clean:
	-rm -f $(OBJECTS) mmtrace

.PHONY: .depend
.depend: $(OBJECTS:.o=.c)
	-$(CC) $(CFLAGS) -M -MP $(OBJECTS:.o=.c) > .depend

mt_nvidia_objects.c:
	[ -r $@ ] || ln -s nvidia/objects.c $@
