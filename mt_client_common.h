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


