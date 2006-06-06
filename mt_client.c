#include "pub_tool_basics.h"
#include "pub_tool_aspacemgr.h"
#include "pub_tool_libcassert.h"
#include "pub_tool_libcbase.h"
#include "pub_tool_libcprint.h"

#include "mmtrace.h"
#include "mt_client_common.h"

static const mt_mmap_trace_t full_trace = {
	.store_1 = &store_1,
	.store_2 = &store_2,
	.store_4 = &store_4,
	.store_8 = &store_8,
	.store_16 = &store_16,
	.load_1 = &load_1,
	.load_2 = &load_2,
	.load_4 = &load_4,
	.load_8 = &load_8,
	.load_16 = &load_16,
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

	return &full_trace;
}

void ML_(trace_flush)() {
}

void ML_(trace_pre_ioctl)(Int fd, Int request, void* arg) {
	VG_(message)(Vg_DebugMsg, "ioctl: fd=%d  request=%08x, arg=%08x", fd, request, arg);

	int ioctl_dir = _VKI_IOC_DIR(request);
	int ioctl_type = _VKI_IOC_TYPE(request);
	int ioctl_nr = _VKI_IOC_NR(request);
	int ioctl_size = _VKI_IOC_SIZE(request);

	VG_(message)(Vg_DebugMsg, "dir=%d, type=%d, nr=%d, size=%d", ioctl_dir, ioctl_type, ioctl_nr, ioctl_size);
}

void ML_(trace_post_ioctl)(SysRes res) {
	VG_(message)(Vg_DebugMsg, "ioctl returned %d", res.val);
}

