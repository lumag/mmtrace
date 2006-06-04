#include "pub_tool_basics.h"
#include "pub_tool_aspacemgr.h"
#include "pub_tool_libcassert.h"
#include "pub_tool_libcbase.h"
#include "pub_tool_libcprint.h"

#include "mmtrace.h"
#include "mt_client_common.h"

#define FIFO_SIZE1 0x102000 
#define FIFO_SIZE2 0x103000 
#define FIFO_SIZE3 0x101000 

static void stub_1(Char *name, ULong offset, UChar data) {VG_(tool_panic)("1");}
static void stub_2(Char *name, ULong offset, UShort data) {VG_(tool_panic)("2");}
static void stub_4(Char *name, ULong offset, UInt data) {VG_(tool_panic)("4");}
static void stub_8(Char *name, ULong offset, ULong data) {VG_(tool_panic)("8");}
static void stub_16(Char *name, ULong offset, U128 data) {VG_(tool_panic)("16");}

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
	smart_store_4_flush();
	if (data == 0) {
		zero_start = offset;
		zero_expect = offset + 4;
		zero_fname = name;
		return;
	}

	store_4(name, offset, data);
}


static ULong fifo_cmdstart = 0;
static ULong fifo_expected = 0;
static int fifo_nops = 0;

static int fifo_cmdlen = 0;
static int fifo_channel = 0;
static int fifo_cmd = 0;

#define BUFSIZ 1024

static void fifo_flush() {
	if (fifo_nops != 0) {
		if (fifo_nops == 1) {
			VG_(message) (Vg_UserMsg, "FIFO: [%08llx] = NOP");
		} else {
			VG_(message) (Vg_UserMsg, "     %d NOP skipped at %08llx", fifo_nops, fifo_expected - fifo_nops*4);
		}
		fifo_nops = 0;
	}
}

static void fifo_store_4(Char *name, ULong offset, UInt data) {
	static char buf[BUFSIZ];
	int pos = 0;

	pos += VG_(snprintf)(buf+pos, BUFSIZ-pos, "FIFO: [%08llx] = %08x", offset, data);

	if (fifo_cmdstart + fifo_cmdlen*4 < offset || offset <= fifo_cmdstart) {

		fifo_cmdstart = offset;

		fifo_cmdlen = (data & 0x1ffc0000) >> 18;
		fifo_channel = (data & 0x0000e000) >> 13;
		fifo_cmd = data & 0x1FFF;

		if (data == 0) {
			if (offset != fifo_expected) {
				fifo_flush();
			}

			fifo_nops ++;
			fifo_expected = offset + 4;
			return;
		} else {
			fifo_flush();

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

static void fifo_store_16(Char *name, ULong offset, U128 data) {
	VG_(message)(Vg_DebugMsg, "FIFO: emulating fast 16-byte write by 4 typical ones");
	int i;
	for (i = 0; i < 4; i++)
		fifo_store_4(name, offset + i * 4, data[i]);
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
