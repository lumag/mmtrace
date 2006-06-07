#define BUFSIZ 1024

#ifndef UNUSED
#define UNUSED __attribute__((unused))
#endif

static void UNUSED stub_1(Char *name, ULong offset, UChar data) {VG_(tool_panic)("memaccess_1");}
static void UNUSED stub_2(Char *name, ULong offset, UShort data) {VG_(tool_panic)("memaccess_2");}
static void UNUSED stub_4(Char *name, ULong offset, UInt data) {VG_(tool_panic)("memaccess_4");}
static void UNUSED stub_8(Char *name, ULong offset, ULong data) {VG_(tool_panic)("memaccess_8");}
static void UNUSED stub_16(Char *name, ULong offset, U128 data) {VG_(tool_panic)("memaccess_16");}

static void UNUSED store_1(Char *name, ULong offset, UChar data) {
	VG_(message)(Vg_UserMsg, "store: [%s][%08llx] = %02x", name, offset, data);
}
static void UNUSED store_2(Char *name, ULong offset, UShort data) {
	VG_(message)(Vg_UserMsg, "store: [%s][%08llx] = %04x", name, offset, data);
}
static void UNUSED store_4(Char *name, ULong offset, UInt data) {
	VG_(message)(Vg_UserMsg, "store: [%s][%08llx] = %08x", name, offset, data);
}
static void UNUSED store_8(Char *name, ULong offset, ULong data) {
	VG_(message)(Vg_UserMsg, "store: [%s][%08llx] = %016llx", name, offset, data);
}
static void UNUSED store_16(Char *name, ULong offset, U128 data) {
	VG_(message)(Vg_UserMsg, "store: [%s][%08llx] = %08x%08x%08x%08x", name, offset, data[3], data[2], data[1], data[0]);
}

static void UNUSED load_1(Char *name, ULong offset, UChar data) {
	VG_(message)(Vg_UserMsg, "load:  [%s][%08llx] = %02x", name, offset, data);
}
static void UNUSED load_2(Char *name, ULong offset, UShort data) {
	VG_(message)(Vg_UserMsg, "load:  [%s][%08llx] = %04x", name, offset, data);
}
static void UNUSED load_4(Char *name, ULong offset, UInt data) {
	VG_(message)(Vg_UserMsg, "load:  [%s][%08llx] = %08x", name, offset, data);
}
static void UNUSED load_8(Char *name, ULong offset, ULong data) {
	VG_(message)(Vg_UserMsg, "load:  [%s][%08llx] = %016llx", name, offset, data);
}
static void UNUSED load_16(Char *name, ULong offset, U128 data) {
	VG_(message)(Vg_UserMsg, "load:  [%s][%08llx] = %08x%08x%08x%08x", name, offset, data[3], data[2], data[1], data[0]);
}

static UNUSED void print_buffer(HChar* prefix, UChar *data, Int size) {
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

static UNUSED void print_buffer_4(HChar* prefix, UInt *data, Int size) {
	static char buf[BUFSIZ];
	int pos = 0;
	int i;

	pos = VG_(snprintf)(buf+pos, BUFSIZ-pos, prefix);
	for (i = 0; i < size; i++) {
		if (i % 4 == 2 || i % 2 == 1) {
			buf[(pos++)] = ' ';
		}
		pos += VG_(snprintf)(buf+pos, BUFSIZ-pos, " %08x", data[i]);

		if ((i+1)% 4 == 0) {
			VG_(message)(Vg_UserMsg, buf);
			pos = VG_(snprintf)(buf+pos, BUFSIZ-pos, prefix);
		}
	}
	
	if ( i % 4 != 0) {
		VG_(message)(Vg_UserMsg, buf);
	}
}

