#include "pub_tool_basics.h"
#include "pub_tool_aspacemgr.h"
#include "pub_tool_libcassert.h"
#include "pub_tool_libcbase.h"
#include "pub_tool_libcprint.h"

#include "mmtrace.h"
#include "mt_client_common.h"

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

void ML_(trace_flush)() {
}

void ML_(trace_pre_ioctl)(Int fd, Int request, void* arg) {
	VG_(message)(Vg_DebugMsg, "ioctl: fd=%d  request=%08x, arg=%08x", fd, request, arg);
}

void ML_(trace_post_ioctl)(SysRes res) {
	VG_(message)(Vg_DebugMsg, "ioctl returned %d", res.val);
}

