#include "pub_tool_basics.h"
#include "pub_tool_libcassert.h"
#include "pub_tool_libcprint.h"

#include "mt_client_common.h"
#include "mt_nvidia.h"
#include "mt_nvidia_regs.h"

// Warning! this will work only for >= NV04!
UInt ML_(find_object_type)(UInt name, UInt chid) {
	UInt htval = ML_(all_regs)[NV_PFIFO_RAMHT/4];

	UInt ht_base = 0x1000 * ((htval >> 4) & 0x1f);
	UInt ht_size = 0x1000 << ((htval >> 16) & 0x03);
	UInt ht_bits = ((htval >> 16) & 0x03) + 9;
//	UInt ht_search = 0x10 << ((htval >> 24) & 0x03);

	UInt hash = 0;
	UInt context;
	UInt instanceMem;
	UInt type;

	int i;
	UInt temp;

	VG_(message)(Vg_UserMsg, "Searching for object %08x", name);

	temp = name;
	for (i = 32; i > 0; i -= ht_bits) {
		hash ^= (temp & ((1 << ht_bits) - 1));
		temp >>= ht_bits;
	}
	hash ^= chid << (ht_bits - 4);
	hash <<= 3;

	VG_(message)(Vg_UserMsg, "Hash is %08x", hash);

	if (ML_(all_regs)[(NV_PRAMIN + ht_base + hash) / 4] != name) {
		VG_(message)(Vg_UserMsg, "Object not found at expected place. bruteforcing the table. PLEASE REPORT!");
		// brute force hash!
		for(i=0x00700000 + ht_base ;i<0x00700000+ht_base + ht_size; i+=4) {
			if (ML_(all_regs)[i/4] == name) {
				hash = i - 0x00700000 - ht_base;
				VG_(message)(Vg_UserMsg, "Object found at %08x", hash);
				break;
			}
		}
	}

	context = ML_(all_regs)[(NV_PRAMIN + ht_base + hash + 4) / 4];
	VG_(message)(Vg_UserMsg, "Context is %08x", context);

	instanceMem = context & 0xffff;

	// engine
	if (((context >> 16) & 0x3) != 1) {
		VG_(message) (Vg_UserMsg, "Can't decode objects with this context!");

		type = (name & 0xff00) >> 8;
	} else {
		type = ML_(all_regs)[(NV_PRAMIN + (instanceMem << 4) + 0 * 4)/4] & 0xffff;
	}

	for (i = 0; i < 4; i++) {
		VG_(message)(Vg_UserMsg, "InstanceMem[%d] = %08x", i, ML_(all_regs)[(NV_PRAMIN + (instanceMem << 4)  + i * 4)/4]);
	}

	return type;
}
