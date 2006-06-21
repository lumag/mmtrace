#include "pub_tool_basics.h"
#include "pub_tool_aspacemgr.h"
#include "pub_tool_libcassert.h"
#include "pub_tool_libcbase.h"
#include "pub_tool_libcprint.h"
#include "pub_tool_mallocfree.h"

#define malloc			VG_(malloc)
#define calloc			VG_(calloc)
#define free			VG_(free)

#define snprintf		VG_(snprintf)
#define sprintf			VG_(sprintf)
#define printf			VG_(printf)

#define strlen			VG_(strlen)
#define strncpy			VG_(strncpy)
#define strncat			VG_(strncat)
#define memset			VG_(memset)

#define assert			tl_assert

#define message(fmt, ...)	VG_(message)(Vg_UserMsg, fmt , ## __VA_ARGS__)
