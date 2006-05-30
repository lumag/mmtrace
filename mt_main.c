#include "pub_tool_basics.h"
#include "pub_tool_aspacemgr.h"
#include "pub_tool_libcassert.h"
#include "pub_tool_libcbase.h"
#include "pub_tool_libcprint.h"
#include "pub_tool_mallocfree.h"
#include "pub_tool_tooliface.h"

static void mt_post_clo_init(void)
{
}

static
IRBB* mt_instrument(IRBB* bb, VexGuestLayout* layout, 
                    Addr64 orig_addr_noredir, VexGuestExtents* vge,
                    IRType gWordTy, IRType hWordTy)
{
    return bb;
}

static void mt_fini(Int exitcode)
{
}

static
void mt_new_mem_mmap ( Addr a, SizeT len, Bool rr, Bool ww, Bool xx )
{
	NSegment* seg = VG_(am_find_nsegment) (a);
	if (seg->kind != SkFileC) {
		return;
	}
	Char *name;
	{
		HChar *hname = VG_(am_get_filename) (seg);
		tl_assert(hname != NULL);
		name = VG_(malloc)(VG_(strlen)(hname));
		VG_(strcpy)(name, hname);
	}
	if (VG_(strncmp)("/dev/", name, 5)) {
		return;
	}
	if (!VG_(strcmp)("/dev/zero", name)) {
		return;
	}
	VG_(printf)("mmap: %s %x of size %x (%s%s%s)\n", name, a, len,
			rr?"r":"-",
			ww?"w":"-",
			xx?"x":"-"
			);

	VG_(free)(name);
}

static
void mt_die_mem_munmap ( Addr a, SizeT len ) {
	VG_(printf)("munmap: %x of size %x\n", a, len);
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
   VG_(track_die_mem_munmap)	(&mt_die_mem_munmap);
}

VG_DETERMINE_INTERFACE_VERSION(mt_pre_clo_init)

/*--------------------------------------------------------------------*/
/*--- end                                                          ---*/
/*--------------------------------------------------------------------*/
