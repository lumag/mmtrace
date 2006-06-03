#include "pub_tool_basics.h"
#include "pub_tool_aspacemgr.h"
#include "pub_tool_libcassert.h"
#include "pub_tool_libcbase.h"
#include "pub_tool_libcprint.h"

#include "mmtrace.h"

#define FIFO_SIZE1 0x102000 
#define FIFO_SIZE2 0x103000 

static void stub_1(Char *name, ULong offset, UChar data) {VG_(tool_panic)("1");}
static void stub_2(Char *name, ULong offset, UShort data) {VG_(tool_panic)("2");}
static void stub_4(Char *name, ULong offset, UInt data) {VG_(tool_panic)("4");}
static void stub_8(Char *name, ULong offset, ULong data) {VG_(tool_panic)("8");}
static void stub_16(Char *name, ULong offset, U128 data) {VG_(tool_panic)("16");}

static ULong fifo_cmdstart = 0;
static ULong fifo_expected = 0;
static int fifo_nops = 0;

static int fifo_cmdlen = 0;
static int fifo_channel = 0;
static int fifo_cmd = 0;

#define BUFSIZ 1024

static void fifo_store_4(Char *name, ULong offset, UInt data) {
	static char buf[BUFSIZ];
	int pos = 0;

	pos += VG_(snprintf)(buf+pos, BUFSIZ-pos, "FIFO: [%08llx] = %08x", offset, data);

	if (fifo_cmdstart + fifo_cmdlen*4 < offset || offset <= fifo_cmdstart) {

		fifo_cmdstart = offset;

		fifo_cmdlen = (data & 0x1ffc0000) >> 18;
		fifo_channel = (data & 0x0000e000) >> 13;
		fifo_cmd = data & 0x1FFF;

		if (data == 0 && offset == fifo_expected) {
			fifo_nops ++;
			fifo_expected += 4;
			return;
		} else {
			if (fifo_nops != 0) {
				VG_(message) (Vg_UserMsg, "     %d NOP skipped at %llx\n", fifo_nops, fifo_expected - fifo_nops*4);
				fifo_nops = 0;
			}

			pos += VG_(snprintf)(buf+pos, BUFSIZ-pos, "  {size: 0x%-3x   channel: 0x%-1x   cmd:   ", fifo_cmdlen, fifo_channel);
			// FIXME: cmd name
			pos += VG_(snprintf)(buf+pos, BUFSIZ-pos, "0x%04x", fifo_cmd);

			fifo_expected = offset + 4;
		}
	} else {
		pos += VG_(snprintf)(buf+pos, BUFSIZ-pos, "  cmd=0x%08x, arg=%lld", fifo_cmd, (offset - fifo_cmdstart)>>2);
	}

	VG_(message)(Vg_UserMsg, buf);
}

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
	.store_16 = &stub_16,
};

const mt_mmap_trace_t *ML_(get_mmap_trace)(Addr addr, SizeT len, NSegment *seg) {
	if (seg == NULL
		|| seg->kind != SkFileC) {
		return NULL;
	}

	Char *name = VG_(am_get_filename) (seg);
	tl_assert(name != NULL);

	if (VG_(strncmp)("/dev/", name, 5)
	|| !VG_(strcmp)("/dev/zero", name)) {
		return 0;
	}

	VG_(message)(Vg_UserMsg, "map: %08x %08llx size %x", seg->start, seg->offset, len);

	if (len == FIFO_SIZE1 || len == FIFO_SIZE2) {
		VG_(message)(Vg_UserMsg, "detected FIFO map");
		return &fifo_trace;
	}

	return NULL;
}
