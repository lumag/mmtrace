#include "pub_tool_basics.h"
#include "pub_tool_aspacemgr.h"
#include "pub_tool_libcassert.h"
#include "pub_tool_libcbase.h"
#include "pub_tool_libcprint.h"

#include "mmtrace.h"
#include "mt_client_common.h"

#include "mt_nvidia_fifo.h"
#include "mt_nvidia_ioctl.h"

static Char* zero_fname = "";
static ULong zero_start = 0;
static ULong zero_expect = 0;

static void smart_store_4_flush() {
	if (zero_start != zero_expect) {
		if (zero_expect - zero_start > 8) {
				VG_(message)(Vg_UserMsg, "store: [%s] zeroed at %08llx - %08llx (size %08llx)", zero_fname, zero_start, zero_expect - 1, zero_expect - zero_start);
		} else {
			ULong o;
			for (o = zero_start; o < zero_expect; o += 4) {
				store_4(zero_fname, o, 0);
			}
		}

		zero_start = zero_expect = 0;
		zero_fname = "";
	}
}

static void smart_store_4(Char *name, ULong offset, UInt data) {
	if (data == 0 && offset == zero_expect && !VG_(strcmp)(zero_fname, name)) {
		zero_expect += 4;
		return;
	}
	ML_(trace_flush)();
	if (data == 0) {
		zero_start = offset;
		zero_expect = offset + 4;
		zero_fname = name;
		return;
	}

	store_4(name, offset, data);
}


static const mt_mmap_trace_t store_trace = {
	.store_1 = &store_1,
	.store_2 = &store_2,
	.store_4 = &smart_store_4,
	.store_8 = &store_8,
	.store_16 = &store_16,
};


static const mt_mmap_trace_t stub_trace = {
	.store_1 = &stub_1,
	.store_2 = &stub_2,
	.store_4 = &stub_4,
	.store_8 = &stub_8,
	.store_16 = &stub_16,
};


static const mt_mmap_trace_t fifo_trace = {
	.store_1 = &stub_1,
	.store_2 = &stub_2,
	.store_4 = &fifo_store_4,
	.store_8 = &stub_8,
	.store_16 = &fifo_store_16,
};

#define FIFO_SIZE1 0x102000 
#define FIFO_SIZE2 0x103000 
#define FIFO_SIZE3 0x101000 

const mt_mmap_trace_t *ML_(get_mmap_trace)(Addr addr, SizeT len, NSegment *seg) {
	if (seg == NULL
		|| seg->kind != SkFileC) {
		return NULL;
	}

	Char *name = VG_(am_get_filename) (seg);
	tl_assert(name != NULL);

	if (VG_(strncmp)("/dev/nvidia", name, 11)) {
		return 0;
	}

//	VG_(message)(Vg_UserMsg, "map: %08x %08llx size %x", seg->start, seg->offset, len);

	if (len == FIFO_SIZE1 || len == FIFO_SIZE2 || len == FIFO_SIZE3) {
//		VG_(message)(Vg_UserMsg, "detected FIFO map");
		return &fifo_trace;
	}

	return &store_trace;
}

void ML_(trace_flush)() {
	fifo_flush();
	smart_store_4_flush();
}
