static ULong fifo_cmdstart = 0;
static ULong fifo_expected = 0;
static int fifo_nops = 0;

static int fifo_cmdlen = 0;
static int fifo_channel = 0;
static int fifo_cmd = 0;

static void fifo_flush() {
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
			ML_(trace_flush)();

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

