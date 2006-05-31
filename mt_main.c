/**
 * TODO:
 * 	use V-bits for marking 'interesting' zones.
 * 	evaluate using instructions instead of checking in the mt_find_segment
 *
 * 	ioctl
 * 	write
 *
 * 	LOAD
 * 	read
 */

#include "pub_tool_basics.h"
#include "pub_tool_aspacemgr.h"
#include "pub_tool_libcassert.h"
#include "pub_tool_libcbase.h"
#include "pub_tool_libcprint.h"
#include "pub_tool_mallocfree.h"
#include "pub_tool_tooliface.h"

static NSegment *mt_find_segment(ULong *addr, Char **filename) {
	NSegment *seg = VG_(am_find_nsegment) ((Addr)addr);
	if (seg == NULL || seg->kind != SkFileC) {
		return NULL;
	}

	Char *name = VG_(am_get_filename) (seg);
	tl_assert(name != NULL);

	if (filename != NULL) {
		*filename = VG_(malloc)(VG_(strlen)(name));
		VG_(strcpy)(*filename, name);
	}

	if (VG_(strncmp)("/dev/", name, 5)
	|| !VG_(strcmp)("/dev/zero", name)) {
		return NULL;
	}
	return seg;
}

static VG_REGPARM(2) void mt_store_1(ULong *addr, UInt data) {
	Char *name;
	NSegment *seg = mt_find_segment(addr, &name);
	if (seg == NULL) {
		return;
	}
	Addr realAddr = (Addr)addr - seg->start + seg->offset;
	VG_(message)(Vg_UserMsg, "store: [%s][%08x] = %02x", name, realAddr, (UChar)data);
}

static VG_REGPARM(2) void mt_store_2(ULong *addr, UInt data) {
	Char *name;
	NSegment *seg = mt_find_segment(addr, &name);
	if (seg == NULL) {
		return;
	}
	Addr realAddr = (Addr)addr - seg->start + seg->offset;
	VG_(message)(Vg_UserMsg, "store: [%s][%08x] = %04x", name, realAddr, (UShort)data);
}

static VG_REGPARM(2) void mt_store_4(ULong *addr, UInt data) {
	Char *name;
	NSegment *seg = mt_find_segment(addr, &name);
	if (seg == NULL) {
		return;
	}
//	if (data == 0) {
//		return;
//	}
	Addr realAddr = (Addr)addr - seg->start + seg->offset;
	VG_(message)(Vg_UserMsg, "store: [%s][%08x] = %08x", name, realAddr, data);
}

static void mt_store_8(ULong *addr, ULong data) {
	Char *name;
	NSegment *seg = mt_find_segment(addr, &name);
	if (seg == NULL) {
		return;
	}
	Addr realAddr = (Addr)addr - seg->start + seg->offset;
	VG_(message)(Vg_UserMsg, "store: [%s][%08x] = %016llx", name, realAddr, data);
}

static void mt_store_16(ULong *addr, ULong dataLo, ULong dataHi) {
	Char *name;
	NSegment *seg = mt_find_segment(addr, &name);
	if (seg == NULL) {
		return;
	}
	Addr realAddr = (Addr)addr - seg->start + seg->offset;
	VG_(message)(Vg_UserMsg, "store: [%s][%08x] = %016llx%016llx", name, realAddr, dataHi, dataLo);
}

static void mt_post_clo_init(void)
{
}

static
IRBB* mt_instrument(IRBB* bb_in, VexGuestLayout* layout, 
                    Addr64 orig_addr_noredir, VexGuestExtents* vge,
                    IRType gWordTy, IRType hWordTy)
{
	IRBB *bb;
	int i;

	if (gWordTy != hWordTy) {
		/* We don't currently support this case. */
		VG_(tool_panic)("host/guest word size mismatch");
	}

	/*
	 * Set up BB
	 */
	bb = emptyIRBB();
	bb->tyenv = dopyIRTypeEnv(bb_in->tyenv);
	bb->next = dopyIRExpr(bb_in->next);
	bb->jumpkind = bb_in->jumpkind;

	for (i = 0; i < bb_in->stmts_used; i++) {
		IRStmt *st = bb_in->stmts[i];
		IRDirty *di;
		IRTemp temp, temp2;

		switch (st->tag) {
			case Ist_Store:
				{
					di = NULL;
					IRType type = typeOfIRExpr(bb->tyenv, st->Ist.Store.data);
					switch (type) {
						case Ity_I8:
							temp = newIRTemp(bb->tyenv, Ity_I32);
							addStmtToIRBB(bb, IRStmt_Tmp(temp,  IRExpr_Unop(Iop_8Uto32, st->Ist.Store.data)));
							di = unsafeIRDirty_0_N (2, "mt_store_1",
									&mt_store_1, mkIRExprVec_2(st->Ist.Store.addr, IRExpr_Tmp(temp)));
							break;
						case Ity_I16:
							temp = newIRTemp(bb->tyenv, Ity_I32);
							addStmtToIRBB(bb, IRStmt_Tmp(temp,  IRExpr_Unop(Iop_16Uto32, st->Ist.Store.data)));
							di = unsafeIRDirty_0_N (2, "mt_store_2",
									&mt_store_2, mkIRExprVec_2(st->Ist.Store.addr, IRExpr_Tmp(temp)));
							break;
						case Ity_I32:
							di = unsafeIRDirty_0_N (2, "mt_store_4",
									&mt_store_4, mkIRExprVec_2(st->Ist.Store.addr, st->Ist.Store.data));
							break;
						case Ity_I64:
							di = unsafeIRDirty_0_N (0, "mt_store_8",
									&mt_store_8, mkIRExprVec_2(st->Ist.Store.addr, st->Ist.Store.data));
							break;
 						case Ity_F32:
/*
 * FIXME: enable when Iop_ReinterpF32asI32 is supported in the VEX
							temp = newIRTemp(bb->tyenv, Ity_I32);
							addStmtToIRBB(bb, IRStmt_Tmp(temp,  IRExpr_Unop(Iop_ReinterpF32asI32, st->Ist.Store.data)));
							di = unsafeIRDirty_0_N (0, "mt_store_4",
									&mt_store_4, mkIRExprVec_2(st->Ist.Store.addr, IRExpr_Tmp(temp)));
									*/
							break;
 						case Ity_F64:
							temp = newIRTemp(bb->tyenv, Ity_I64);
							addStmtToIRBB(bb, IRStmt_Tmp(temp,  IRExpr_Unop(Iop_ReinterpF64asI64, st->Ist.Store.data)));
							di = unsafeIRDirty_0_N (0, "mt_store_8",
									&mt_store_8, mkIRExprVec_2(st->Ist.Store.addr, IRExpr_Tmp(temp)));
							break;
						case Ity_V128:
							temp = newIRTemp(bb->tyenv, Ity_I64);
							addStmtToIRBB(bb, IRStmt_Tmp(temp,  IRExpr_Unop(Iop_V128to64, st->Ist.Store.data)));
							temp2 = newIRTemp(bb->tyenv, Ity_I64);
							addStmtToIRBB(bb, IRStmt_Tmp(temp2,  IRExpr_Unop(Iop_V128HIto64, st->Ist.Store.data)));
							di = unsafeIRDirty_0_N (0, "mt_store_16",
									&mt_store_16, mkIRExprVec_3(st->Ist.Store.addr, IRExpr_Tmp(temp), IRExpr_Tmp(temp2)));
							break;
						default:
							{
								Char *buf = VG_(malloc)(128);
								VG_(snprintf)(buf, 128, "unhandled type: %x", type);
								VG_(tool_panic)(buf);
							}
							break;
					}
					if (di != NULL) {
						addStmtToIRBB(bb, IRStmt_Dirty(di));
					}
					addStmtToIRBB(bb, st);
				}
			default:
				addStmtToIRBB(bb, st);
				break;
		}
	}

	return bb;
}

static void mt_fini(Int exitcode)
{
}

static
void mt_new_mem_mmap ( Addr a, SizeT len, Bool rr, Bool ww, Bool xx )
{
	Char *name = NULL;
	/* NSegment *seg = */mt_find_segment((ULong*) a, &name);

	VG_(message)(Vg_DebugMsg, "mmap: %s at %08x size %x (%s%s%s)", name, a, len,
			rr?"r":"-",
			ww?"w":"-",
			xx?"x":"-"
			);

	if (name != NULL) {
		VG_(free)(name);
	}
}

static
void mt_die_mem_munmap ( Addr a, SizeT len ) {
	VG_(message)(Vg_DebugMsg, "munmap: %08x of size %x", a, len);
}

static void mt_copy_mem_remap( Addr from, Addr to, SizeT len ) {
	VG_(message)(Vg_DebugMsg, "mremap: %08x -> %08x of size %x", from, to, len);
}

static void mt_pre_clo_init(void)
{
   VG_(details_name)		("MMTrace");
   VG_(details_version)		(NULL);
   VG_(details_description)	("a binary JIT-compiler");
   VG_(details_copyright_author)(
      "Copyright (C) 2006, and GNU GPL'd, by Dmitry Baryshkov.");
   VG_(details_bug_reports_to)	(VG_BUGS_TO);

   VG_(basic_tool_funcs)	(mt_post_clo_init,
		   		 mt_instrument,
				 mt_fini);

   VG_(needs_core_errors)	();
   VG_(needs_libc_freeres)	();
   VG_(track_new_mem_startup)	(&mt_new_mem_mmap);
   VG_(track_new_mem_mmap)	(&mt_new_mem_mmap);
   VG_(track_copy_mem_remap)	(&mt_copy_mem_remap);
   VG_(track_die_mem_munmap)	(&mt_die_mem_munmap);
}

VG_DETERMINE_INTERFACE_VERSION(mt_pre_clo_init)

/*--------------------------------------------------------------------*/
/*--- end                                                          ---*/
/*--------------------------------------------------------------------*/
