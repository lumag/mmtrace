#include "pub_tool_basics.h"
#include "pub_tool_aspacemgr.h"
#include "pub_tool_libcassert.h"
#include "pub_tool_libcbase.h"
#include "pub_tool_libcprint.h"
#include "pub_tool_mallocfree.h"
#include "pub_tool_tooliface.h"

#include <sys/syscall.h>

#include "mmtrace.h"

#define ADDR_SIZE	(sizeof(Addr)*8 - VKI_PAGE_SHIFT)
#define ADDR_FIRST_SHIFT	(ADDR_SIZE/2)
#define ADDR_MAP_FIRST_SIZE	(1 << ADDR_FIRST_SHIFT)
#define ADDR_MAP_SECOND_SIZE	(1 << (ADDR_SIZE - ADDR_FIRST_SHIFT))

static const mt_mmap_trace_t**	mt_mmap_trace_table[ADDR_MAP_FIRST_SIZE];

static inline const mt_mmap_trace_t* mt_mmap_trace_get(Addr a) {
	a = a >> VKI_PAGE_SHIFT;
	const mt_mmap_trace_t** t2 = mt_mmap_trace_table[a >> ADDR_FIRST_SHIFT];
	if (t2 == NULL) {
		return NULL;
	} else {
		return t2[ a & (ADDR_MAP_FIRST_SIZE - 1)];
	}
	return NULL;
}

static inline void mt_mmap_trace_set(Addr a, const mt_mmap_trace_t *entry) {
	a = a >> VKI_PAGE_SHIFT;
	if (mt_mmap_trace_table[a >> ADDR_FIRST_SHIFT] == NULL) {
		mt_mmap_trace_table[a >> ADDR_FIRST_SHIFT] = VG_(calloc)(ADDR_MAP_SECOND_SIZE, sizeof(mt_mmap_trace_t**));
	}
	mt_mmap_trace_table[a >> ADDR_FIRST_SHIFT][a & (ADDR_MAP_FIRST_SIZE-1)] = entry;
}


#define MT_STORE_COMMON							\
	const mt_mmap_trace_t *entry = mt_mmap_trace_get(addr);		\
	if (entry == NULL) {						\
		return;							\
	}								\
									\
	NSegment *seg = VG_(am_find_nsegment)(addr);			\
	tl_assert(seg != NULL);						\
									\
	Char *name = VG_(am_get_filename) (seg);			\
	ULong offset = addr - seg->start + seg->offset;


static VG_REGPARM(2) void mt_store_1(Addr addr, UInt data) {
	MT_STORE_COMMON

	entry->store_1(name, offset, data);
}

static VG_REGPARM(2) void mt_store_2(Addr addr, UInt data) {
	MT_STORE_COMMON

	entry->store_2(name, offset, data);
}

static VG_REGPARM(2) void mt_store_4(Addr addr, UInt data) {
	MT_STORE_COMMON

	entry->store_4(name, offset, data);
}

static void mt_store_8(Addr addr, ULong data) {
	MT_STORE_COMMON

	entry->store_8(name, offset, data);
}

static void mt_store_16(Addr addr, ULong dataLo, ULong dataHi) {
	MT_STORE_COMMON

	U128 data;
	data[3] = dataHi >> 32;
	data[2] = dataHi & 0xffffffff;
	data[1] = dataLo >> 32;
	data[0] = dataLo & 0xffffffff;

	entry->store_16(name, offset, data);
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
							temp = newIRTemp(bb->tyenv, Ity_I32);
							addStmtToIRBB(bb, IRStmt_Tmp(temp,  IRExpr_Unop(Iop_ReinterpF32asI32, st->Ist.Store.data)));
							di = unsafeIRDirty_0_N (0, "mt_store_4",
									&mt_store_4, mkIRExprVec_2(st->Ist.Store.addr, IRExpr_Tmp(temp)));
							break;
 						case Ity_F64:
							temp = newIRTemp(bb->tyenv, Ity_I64);
							addStmtToIRBB(bb, IRStmt_Tmp(temp,  IRExpr_Unop(Iop_ReinterpF64asI64, st->Ist.Store.data)));
							di = unsafeIRDirty_0_N (0, "mt_store_8",
									&mt_store_8, mkIRExprVec_2(st->Ist.Store.addr, IRExpr_Tmp(temp)));
							break;
						case Ity_I128:
							temp = newIRTemp(bb->tyenv, Ity_I64);
							addStmtToIRBB(bb, IRStmt_Tmp(temp,  IRExpr_Unop(Iop_128to64, st->Ist.Store.data)));
							temp2 = newIRTemp(bb->tyenv, Ity_I64);
							addStmtToIRBB(bb, IRStmt_Tmp(temp2,  IRExpr_Unop(Iop_128HIto64, st->Ist.Store.data)));
							di = unsafeIRDirty_0_N (0, "mt_store_16",
									&mt_store_16, mkIRExprVec_3(st->Ist.Store.addr, IRExpr_Tmp(temp), IRExpr_Tmp(temp2)));
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
	ML_(trace_flush)();
	int i;

	for (i = 0; i < ADDR_MAP_FIRST_SIZE; i++) {
		if (mt_mmap_trace_table[i] != NULL) {
			VG_(free) (mt_mmap_trace_table[i]);
			mt_mmap_trace_table[i] = NULL;
		}
	}
}

static void mt_pre_syscall(ThreadId tid, UInt syscallno, SyscallArgs* args) {
	ML_(trace_flush)();
//	VG_(message)(Vg_DebugMsg, "syscall: %d, %d, %d ...", syscallno, args->arg1, args->arg2);
	if (syscallno == SYS_ioctl) {
		ML_(trace_pre_ioctl)(args->arg1, args->arg2, (void*)args->arg3);
	}
}

static void mt_post_syscall(ThreadId tid, UInt syscallno, SysRes res) {
	if (syscallno == SYS_ioctl) {
		ML_(trace_post_ioctl)(res);
	}
}

static
void mt_new_mem_mmap ( Addr a, SizeT len, Bool rr, Bool ww, Bool xx )
{
	ML_(trace_flush)();
	NSegment *seg = VG_(am_find_nsegment)(a);
	tl_assert(seg != NULL);

	Char *name = VG_(am_get_filename) (seg);
	VG_(message)(Vg_DebugMsg, "mmap: %s at %08llx to %08x size %x (%s%s%s)",
			(name!=NULL)?name:(Char*)"<none>",
			seg->offset + a - seg->start, a, len,
			rr?"r":"-",
			ww?"w":"-",
			xx?"x":"-"
			);

	Addr cur;
	for (cur = a; cur < a + len; cur += VKI_PAGE_SIZE) {
		mt_mmap_trace_set(cur, ML_(get_mmap_trace)(a, len, seg));
	}
}

static
void mt_die_mem_munmap ( Addr a, SizeT len ) {
	ML_(trace_flush)();
	// There segment has been already marked as free, so we cannot get original mappings.
	VG_(message)(Vg_DebugMsg, "munmap: %08x of size %x", a, len);

	Addr cur;
	for (cur = a; cur < a + len; cur += VKI_PAGE_SIZE) {
		mt_mmap_trace_set(cur, NULL);
	}
}

static void mt_copy_mem_remap( Addr from, Addr to, SizeT len ) {
	ML_(trace_flush)();
	VG_(message)(Vg_DebugMsg, "mremap: %08x -> %08x of size %x", from, to, len);

	Addr cur, curnew;
	for (cur = from, curnew = to; cur < from + len; cur += VKI_PAGE_SIZE, curnew += VKI_PAGE_SIZE) {
		mt_mmap_trace_set(cur, mt_mmap_trace_get(curnew));
		mt_mmap_trace_set(curnew, NULL);
	}
}

static void mt_pre_clo_init(void)
{
   int i;

   VG_(details_name)		("MMTrace");
   VG_(details_version)		(NULL);
   VG_(details_description)	("a binary JIT-compiler");
   VG_(details_copyright_author)(
      "Copyright (C) 2006, and GNU GPL'd, by Dmitry Baryshkov.");
   VG_(details_bug_reports_to)	("dbaryshkov@gmail.com");

   VG_(basic_tool_funcs)	(mt_post_clo_init,
		   		 mt_instrument,
				 mt_fini);

   for (i = 0; i < ADDR_MAP_FIRST_SIZE; i++) {
	   mt_mmap_trace_table[i] = NULL;
   }

   VG_(needs_core_errors)	();
   VG_(needs_libc_freeres)	();
   VG_(track_new_mem_startup)	(&mt_new_mem_mmap);
   VG_(track_new_mem_mmap)	(&mt_new_mem_mmap);
   VG_(track_copy_mem_remap)	(&mt_copy_mem_remap);
   VG_(track_die_mem_munmap)	(&mt_die_mem_munmap);

   VG_(needs_syscall_wrapper)	(&mt_pre_syscall,
		   		 &mt_post_syscall);
}

VG_DETERMINE_INTERFACE_VERSION(mt_pre_clo_init)

/*--------------------------------------------------------------------*/
/*--- end                                                          ---*/
/*--------------------------------------------------------------------*/
