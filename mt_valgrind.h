#include "pub_tool_basics.h"
#include "pub_tool_aspacemgr.h"
#include "pub_tool_libcassert.h"
#include "pub_tool_libcbase.h"
#include "pub_tool_libcprint.h"

#define snprintf		VG_(snprintf)
#define strlen			VG_(strlen)
#define strncpy			VG_(strncpy)
#define memset			VG_(memset)

#define assert			tl_assert

#define message(fmt, ...)	VG_(message)(Vg_UserMsg, fmt , ## __VA_ARGS__)
