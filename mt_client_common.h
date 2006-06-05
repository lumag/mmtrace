#define BUFSIZ 1024

static void stub_1(Char *name, ULong offset, UChar data) {VG_(tool_panic)("store_1");}
static void stub_2(Char *name, ULong offset, UShort data) {VG_(tool_panic)("store_2");}
static void stub_4(Char *name, ULong offset, UInt data) {VG_(tool_panic)("store_4");}
static void stub_8(Char *name, ULong offset, ULong data) {VG_(tool_panic)("store_8");}
static void stub_16(Char *name, ULong offset, U128 data) {VG_(tool_panic)("store_16");}

static void store_1(Char *name, ULong offset, UChar data) {
	VG_(message)(Vg_UserMsg, "store: [%s][%08llx] = %02x", name, offset, data);
}
static void store_2(Char *name, ULong offset, UShort data) {
	VG_(message)(Vg_UserMsg, "store: [%s][%08llx] = %04x", name, offset, data);
}
static void store_4(Char *name, ULong offset, UInt data) {
	VG_(message)(Vg_UserMsg, "store: [%s][%08llx] = %08x", name, offset, data);
}
static void store_8(Char *name, ULong offset, ULong data) {
	VG_(message)(Vg_UserMsg, "store: [%s][%08llx] = %016llx", name, offset, data);
}
static void store_16(Char *name, ULong offset, U128 data) {
	VG_(message)(Vg_UserMsg, "store: [%s][%08llx] = %08x%08x%08x%08x", name, offset, data[3], data[2], data[1], data[0]);
}

static void print_buffer(HChar* prefix, UChar *data, Int size) {
	static char buf[BUFSIZ];
	int pos = 0;
	int i;

	pos = VG_(snprintf)(buf+pos, BUFSIZ-pos, prefix);
	for (i = 0; i < size; i++) {
		if (i % 16 == 8 || i % 8 == 4) {
			buf[(pos++)] = ' ';
			buf[(pos++)] = '-';
		}
		pos += VG_(snprintf)(buf+pos, BUFSIZ-pos, " %02x", (UInt)data[i]);

		if ((i+1)% 16 == 0) {
			VG_(message)(Vg_UserMsg, buf);
			pos = VG_(snprintf)(buf+pos, BUFSIZ-pos, prefix);
		}
	}
	
	if ( i % 16 != 0) {
		VG_(message)(Vg_UserMsg, buf);
	}
}

