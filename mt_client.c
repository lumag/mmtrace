#include "pub_tool_basics.h"
#include "pub_tool_aspacemgr.h"
#include "pub_tool_libcassert.h"
#include "pub_tool_libcbase.h"
#include "pub_tool_libcprint.h"

#include "mmtrace.h"

static void store_1(Char *name, ULong offset, UChar data) {
	VG_(message)(Vg_UserMsg, "store: [%s][%08llx] = %02x", name, offset, data);
}
static void store_2(Char *name, ULong offset, UShort data) {
	VG_(message)(Vg_UserMsg, "store: [%s][%08llx] = %04x", name, offset, data);
}
static void store_4(Char *name, ULong offset, UInt data) {
//	if (data == 0) {
//		return;
//	}
	VG_(message)(Vg_UserMsg, "store: [%s][%08llx] = %08x", name, offset, data);
}
static void store_8(Char *name, ULong offset, ULong data) {
	VG_(message)(Vg_UserMsg, "store: [%s][%08llx] = %016llx", name, offset, data);
}
static void store_16(Char *name, ULong offset, U128 data) {
	VG_(message)(Vg_UserMsg, "store: [%s][%08llx] = %08x%08x%08x%08x", name, offset, data[3], data[2], data[1], data[0]);
}

static const mt_mmap_trace_t store_trace = {
	.store_1 = &store_1,
	.store_2 = &store_2,
	.store_4 = &store_4,
	.store_8 = &store_8,
	.store_16 = &store_16,
};

const mt_mmap_trace_t *ML_(get_mmap_trace)(Addr addr, SizeT len, NSegment *seg) {
	if (seg == NULL
		|| seg->kind != SkFileC) {
		return NULL;
	}

	Char *name = VG_(am_get_filename) (seg);
	tl_assert(name != NULL);

/*	if (VG_(strncmp)("/dev/", name, 5)
	|| !VG_(strcmp)("/dev/zero", name)) {
		return 0;
	}*/

	return &store_trace;
}
