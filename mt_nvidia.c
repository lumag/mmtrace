#include "mmtrace.h"
#include "mt_client_common.h"
#include "mt_nvidia.h"
#include "nvidia/nv_hw.h"
#include "nvidia/objects.h"

// from pub_core_aspacemgr.h
extern SysRes VG_(am_mmap_file_float_valgrind)
   ( SizeT length, UInt prot, Int fd, Off64T offset );
extern SysRes VG_(am_munmap_valgrind)( Addr start, SizeT length );
// end

static Char* zero_fname = "";
static ULong zero_start = 0;
static ULong zero_expect = 0;

nvidia_info ML_(nvinfo);
int ML_(nvcard) = -1;
UInt* all_regs = NULL;
UInt card_family = 0;

data_print_store user_data_print[] = {
	{0, 31, print_format, "0x%08x"},
	{}
};

void ML_(device_selected)(int fd) {
	VG_(message)(Vg_UserMsg, "Selected card %d", ML_(nvcard));
	switch (ML_(nvinfo).devices[ML_(nvcard)].pid & 0x0FF0) {
		case 0x0020: /* TNT1/TNT2 */
		case 0x00A0: /* Aladdin TNT2 */
			card_family=NV04;
			break;
		case 0x0100: /* GeForce 256 */
		case 0x0110: /* GeForce2 MX */
		case 0x01A0: /* nForce */
			card_family=NV10;
			break;
		case 0x0150: /* GeForce2 */
		case 0x0170: /* GeForce4 MX */
		case 0x0180: /* GeForce4 MX (8x AGP) */
		case 0x01F0: /* nForce2 */
			card_family=NV15;
			break;
		case 0x0200: /* GeForce3 */
		case 0x0250: /* GeForce4 Ti */
		case 0x0280: /* GeForce4 Ti (8x AGP) */
			card_family=NV20;
			break;
		case 0x0300: /* GeForceFX 5800 */
		case 0x0310: /* GeForceFX 5600 */
		case 0x0320: /* GeForceFX 5200 */
		case 0x0330: /* GeForceFX 5900 */
		case 0x0340: /* GeForceFX 5700 */
			card_family=NV30;
			break;
		case 0x0040: /* GeForce 6800 */
		case 0x00C0: /* GeForce 6800 */
		case 0x0120: /* GeForce 6800 */
		case 0x0210: /* GeForce 6800 */
		case 0x0140: /* GeForce 6600 */
		case 0x0160: /* GeForce 6200 */
		case 0x0220: /* GeForce 6200 */
		case 0x0240: /* GeForce 6100 */
			card_family=NV40;
			break;
		case 0x01D0: /* GeForce 7200, 7300, 7400 */
		case 0x0090: /* GeForce 7800 */
		case 0x0290: /* GeForce 7900 */
		case 0x02E0: /* GeForce 7600 */
		case 0x0390: /* GeForce 7600 */
			card_family=G70;
			break;
		case 0x00F0: /* misc PCI-E cards */
			switch(ML_(nvinfo).devices[ML_(nvcard)].pid & 0xFF)
			{
				case 0xFF: /* Geforce 4300 */
					card_family=NV15;
					break;
				case 0xFA: /* GeForceFX 5750 */
				case 0xFB: /* GeForceFX 5900 */
				case 0xFC: /* QuadroFX 330/GeForceFX 5300 */
				case 0xFD: /* QuadroFX 330/NVS 280 */
				case 0xFE: /* QuadroFX 1300 */
					card_family=NV30;
					break;
				case 0xF0: /* GeForce 6800 */
				case 0xF1: /* GeForce 6600 */
				case 0xF2: /* GeForce 6600 */
				case 0xF3: /* GeForce 6200 */
				case 0xF4: /* GeForce 6600 */
				case 0xF6: /* GeForce 6600 */
				case 0xF7: /* guess */
				case 0xF8: /* QuadroFX 3400/4400 */
				case 0xF9: /* GeForce 6800 */
					card_family=NV40;
					break;
				case 0xF5: /* GeForce 7800 */
					card_family=G70;
					break;
			}
			break;
		default:
			message("Unknown card");
			card_family=G70;
			break;
	}
	SysRes res = VG_(am_mmap_file_float_valgrind)(
			ML_(nvinfo).devices[ML_(nvcard)].reg_size,
			VKI_PROT_READ | VKI_PROT_WRITE,
			fd,
			ML_(nvinfo).devices[ML_(nvcard)].reg_addr);
	if (res.isError) {
		VG_(tool_panic)("Failed mmapping device registers");
	}
	all_regs = (UInt*)res.val;
}

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

	.load_1 = &load_1,
	.load_2 = &load_2,
	.load_4 = &load_4,
	.load_8 = &load_8,
	.load_16 = &load_16,
};


static const mt_mmap_trace_t stub_trace = {
	.store_1 = &stub_1,
	.store_2 = &stub_2,
	.store_4 = &stub_4,
	.store_8 = &stub_8,
	.store_16 = &stub_16,

	.load_1 = &stub_1,
	.load_2 = &stub_2,
	.load_4 = &stub_4,
	.load_8 = &stub_8,
	.load_16 = &stub_16,
};


static const mt_mmap_trace_t fifo_trace = {
	.store_1 = &stub_1,
	.store_2 = &stub_2,
	.store_4 = &ML_(fifo_store_4),
	.store_8 = &stub_8,
	.store_16 = &ML_(fifo_store_16),

	.load_1 = &stub_1,
	.load_2 = &stub_2,
	.load_4 = &ML_(fifo_load_4),
	.load_8 = &stub_8,
	.load_16 = &stub_16,
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
	ML_(fifo_flush)();
	smart_store_4_flush();
}
