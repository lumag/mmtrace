CFLAGS = -I/usr/include/valgrind \
	 -DVGA_x86=1 \
	 -DVGO_linux=1 \
	 -DVGP_x86_linux=1 \
	 -g -Wall -Wno-pointer-sign
LDFLAGS = -L/usr/lib/valgrind/x86-linux -lcoregrind -lvex -lgcc \
	-static -Wl,-defsym,valt_load_address=0xa0000000 \
	-nodefaultlibs -nostartfiles -u _start \
	-Wl,-T,valt_load_address_x86_linux.lds

OBJECTS = mt_main.o

all: mmtrace

mmtrace: $(OBJECTS)
	$(LINK.o) $^ $(LOADLIBES) $(LDLIBS) -o $@

clean:
	-rm -f $(OBJECTS) mmtrace

