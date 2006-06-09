#include "pub_tool_basics.h"
#include "pub_tool_aspacemgr.h"
#include "pub_tool_libcassert.h"
#include "pub_tool_libcprint.h"

#include "mmtrace.h"
#include "mt_client_common.h"
#include "mt_nvidia.h"

typedef struct {
	UInt name;
	UInt type;
} fifo_descriptor;

static fifo_descriptor fifo_channels[8];

static ULong fifo_expected = 0;
static int fifo_nops = 0;

static ULong fifo_cmdstart = 0;
static int fifo_cmdlen = 0;

static int fifo_channel = 0;
static int fifo_object_offset = 0;

void ML_(fifo_flush)() {
	if (fifo_nops != 0) {
		if (fifo_nops < 5) {
			ULong o = fifo_expected - fifo_nops*4;
			int i;
			for (i = 0; i < fifo_nops; i++) {
				VG_(message) (Vg_UserMsg, "FIFO: [%08llx] = NOP", o + i*4);
			}
		} else {
			VG_(message) (Vg_UserMsg, "     %d NOP skipped at %08llx", fifo_nops, fifo_expected - fifo_nops*4);
		}
		fifo_nops = 0;
	}
}

void ML_(fifo_store_4)(Char *name, ULong offset, UInt data) {
	static char buf[BUFSIZ];
	int pos = 0;

	pos += VG_(snprintf)(buf+pos, BUFSIZ-pos, "FIFO: [%08llx] = %08x", offset, data);

	if (fifo_cmdstart + fifo_cmdlen*4 < offset || offset <= fifo_cmdstart) {

		fifo_cmdstart = offset;

		fifo_cmdlen = (data & 0x1ffc0000) >> 18;
		fifo_channel = (data & 0x0000e000) >> 13;
		fifo_object_offset = data & 0x1FFF;

		if (data == 0) {
			if (offset != fifo_expected) {
				ML_(fifo_flush)();
			}

			fifo_nops ++;
			fifo_expected = offset + 4;
			return;
		} else {
			ML_(trace_flush)();

			pos += VG_(snprintf)(buf+pos, BUFSIZ-pos, "  {size: 0x%-3x   channel: 0x%-1x}", fifo_cmdlen, fifo_channel);

			fifo_expected = offset + 4;
		}
	} else {
		if (fifo_object_offset == 0) {
			fifo_channels[fifo_channel].name = data;
			fifo_channels[fifo_channel].type = ML_(find_object_type)(data, 2);
		}
		pos += VG_(snprintf)(buf+pos, BUFSIZ-pos, "  object: %08x type=%02x  offset=%04x",
				fifo_channels[fifo_channel].name,
				fifo_channels[fifo_channel].type,
				fifo_object_offset);
		fifo_object_offset += 4;
	}

	VG_(message)(Vg_UserMsg, buf);
}

void ML_(fifo_store_16)(Char *name, ULong offset, U128 data) {
	VG_(message)(Vg_DebugMsg, "FIFO: emulating fast 16-byte write by 4 typical ones");
	int i;
	for (i = 0; i < 4; i++)
		ML_(fifo_store_4)(name, offset + i * 4, data[i]);
}

void ML_(fifo_load_4)(Char *name, ULong offset, UInt data) {
	VG_(message)(Vg_UserMsg, "FIFO load: [%08llx] = %08x", offset, data);
}
