#include "pub_tool_basics.h"
#include "pub_tool_libcassert.h"
#include "pub_tool_libcbase.h"
#include "pub_tool_libcprint.h"

#include "mt_client_common.h"
#include "mt_nvidia.h"
#include "nvidia/nv_regs.h"
#include "nvidia/nv_objects.h"

static const char* user_format = "%08x";

static const char* print_format(unsigned int value, void*format) {
	static char buf[128];
	VG_(snprintf)(buf, sizeof(buf), format?format:user_format, value);
	return buf;
}

typedef struct {
	unsigned int id;
	const char* name;
} data_store;

typedef const char* (*PrintFunc)(unsigned int value, void*data);

typedef struct {
	int lowbit;
	int highbit;

	PrintFunc func;
	void *data;
} data_print_store;

typedef struct {
	unsigned int offset;
	const char* name;

	// max 32 bits, so max 32 subfields
	data_print_store data[32];
} object_field_store;

typedef struct {
	unsigned int id;
	const char *name;

	// You can freely increase this array size, if it's full
	object_field_store fields[100];
} object_store;

#define __(a)	a, # a
#define PRINT_BITFIELD(bf, func, data)	{0?bf, 1?bf, func, data}
#define PRINT_FP32	{0, 31, &print_format, "%f"}
#define PRINT_X32	{0, 31, &print_format, "0x%x"}

#define PRINT_USER	{0, 31, &print_format, NULL}

// temporary
#define PRINT_OBJECT	{0, 31, &print_format, "object %08x"}

static const object_store nv_objects[0x100] = {
	[NV04_DX5_TEXTURED_TRIANGLE] = {__(NV04_DX5_TEXTURED_TRIANGLE),{
		{__(NV04_DX5_TEXTURED_TRIANGLE_NOTIFY),		{PRINT_OBJECT,	{}} },
		{__(NV04_DX5_TEXTURED_TRIANGLE_DMA_1),		{PRINT_OBJECT,	{}} },
		{__(NV04_DX5_TEXTURED_TRIANGLE_DMA_2),		{PRINT_OBJECT,	{}} },
		{__(NV04_DX5_TEXTURED_TRIANGLE_SURFACE),	{PRINT_OBJECT,	{}} },

		{__(NV04_DX5_TEXTURED_TRIANGLE_COLOR_KEY),	{PRINT_X32,	{}} },
		{__(NV04_DX5_TEXTURED_TRIANGLE_TEXTURE_OFFSET),	{PRINT_X32,	{}} },
		{__(NV04_DX5_TEXTURED_TRIANGLE_TEXTURE_FORMAT),	{PRINT_USER,	{}} },
		{__(NV04_DX5_TEXTURED_TRIANGLE_TEXTURE_FILTER),	{PRINT_USER,	{}} },
		{__(NV04_DX5_TEXTURED_TRIANGLE_BLEND),		{PRINT_USER,	{}} },
		{__(NV04_DX5_TEXTURED_TRIANGLE_CONTROL),	{PRINT_USER,	{}} },
		{__(NV04_DX5_TEXTURED_TRIANGLE_FOG_COLOR),	{PRINT_X32,	{}} },

		{__(NV04_DX5_TEXTURED_TRIANGLE_TLVERTEX_SX(0)),	{PRINT_FP32,	{}} },
		{__(NV04_DX5_TEXTURED_TRIANGLE_TLVERTEX_SY(0)),	{PRINT_FP32,	{}} },
		{__(NV04_DX5_TEXTURED_TRIANGLE_TLVERTEX_SZ(0)),	{PRINT_FP32,	{}} },
		{__(NV04_DX5_TEXTURED_TRIANGLE_INV_W(0)),	{PRINT_FP32,	{}} },
		{__(NV04_DX5_TEXTURED_TRIANGLE_COLOR(0)),	{PRINT_X32,	{}} },
		{__(NV04_DX5_TEXTURED_TRIANGLE_SPECULAR(0)),	{PRINT_X32,	{}} },
		{__(NV04_DX5_TEXTURED_TRIANGLE_TEXTURE_S(0)),	{PRINT_FP32,	{}} },
		{__(NV04_DX5_TEXTURED_TRIANGLE_TEXTURE_T(0)),	{PRINT_FP32,	{}} },

		{__(NV04_DX5_TEXTURED_TRIANGLE_TLVERTEX_SX(1)),	{PRINT_FP32,	{}} },
		{__(NV04_DX5_TEXTURED_TRIANGLE_TLVERTEX_SY(1)),	{PRINT_FP32,	{}} },
		{__(NV04_DX5_TEXTURED_TRIANGLE_TLVERTEX_SZ(1)),	{PRINT_FP32,	{}} },
		{__(NV04_DX5_TEXTURED_TRIANGLE_INV_W(1)),	{PRINT_FP32,	{}} },
		{__(NV04_DX5_TEXTURED_TRIANGLE_COLOR(1)),	{PRINT_X32,	{}} },
		{__(NV04_DX5_TEXTURED_TRIANGLE_SPECULAR(1)),	{PRINT_X32,	{}} },
		{__(NV04_DX5_TEXTURED_TRIANGLE_TEXTURE_S(1)),	{PRINT_FP32,	{}} },
		{__(NV04_DX5_TEXTURED_TRIANGLE_TEXTURE_T(1)),	{PRINT_FP32,	{}} },

		{__(NV04_DX5_TEXTURED_TRIANGLE_TLVERTEX_SX(2)),	{PRINT_FP32,	{}} },
		{__(NV04_DX5_TEXTURED_TRIANGLE_TLVERTEX_SY(2)),	{PRINT_FP32,	{}} },
		{__(NV04_DX5_TEXTURED_TRIANGLE_TLVERTEX_SZ(2)),	{PRINT_FP32,	{}} },
		{__(NV04_DX5_TEXTURED_TRIANGLE_INV_W(2)),	{PRINT_FP32,	{}} },
		{__(NV04_DX5_TEXTURED_TRIANGLE_COLOR(2)),	{PRINT_X32,	{}} },
		{__(NV04_DX5_TEXTURED_TRIANGLE_SPECULAR(2)),	{PRINT_X32,	{}} },
		{__(NV04_DX5_TEXTURED_TRIANGLE_TEXTURE_S(2)),	{PRINT_FP32,	{}} },
		{__(NV04_DX5_TEXTURED_TRIANGLE_TEXTURE_T(2)),	{PRINT_FP32,	{}} },

		{__(NV04_DX5_TEXTURED_TRIANGLE_TLVERTEX_SX(3)),	{PRINT_FP32,	{}} },
		{__(NV04_DX5_TEXTURED_TRIANGLE_TLVERTEX_SY(3)),	{PRINT_FP32,	{}} },
		{__(NV04_DX5_TEXTURED_TRIANGLE_TLVERTEX_SZ(3)),	{PRINT_FP32,	{}} },
		{__(NV04_DX5_TEXTURED_TRIANGLE_INV_W(3)),	{PRINT_FP32,	{}} },
		{__(NV04_DX5_TEXTURED_TRIANGLE_COLOR(3)),	{PRINT_X32,	{}} },
		{__(NV04_DX5_TEXTURED_TRIANGLE_SPECULAR(3)),	{PRINT_X32,	{}} },
		{__(NV04_DX5_TEXTURED_TRIANGLE_TEXTURE_S(3)),	{PRINT_FP32,	{}} },
		{__(NV04_DX5_TEXTURED_TRIANGLE_TEXTURE_T(3)),	{PRINT_FP32,	{}} },

		{__(NV04_DX5_TEXTURED_TRIANGLE_TLVERTEX_SX(4)),	{PRINT_FP32,	{}} },
		{__(NV04_DX5_TEXTURED_TRIANGLE_TLVERTEX_SY(4)),	{PRINT_FP32,	{}} },
		{__(NV04_DX5_TEXTURED_TRIANGLE_TLVERTEX_SZ(4)),	{PRINT_FP32,	{}} },
		{__(NV04_DX5_TEXTURED_TRIANGLE_INV_W(4)),	{PRINT_FP32,	{}} },
		{__(NV04_DX5_TEXTURED_TRIANGLE_COLOR(4)),	{PRINT_X32,	{}} },
		{__(NV04_DX5_TEXTURED_TRIANGLE_SPECULAR(4)),	{PRINT_X32,	{}} },
		{__(NV04_DX5_TEXTURED_TRIANGLE_TEXTURE_S(4)),	{PRINT_FP32,	{}} },
		{__(NV04_DX5_TEXTURED_TRIANGLE_TEXTURE_T(4)),	{PRINT_FP32,	{}} },

		{__(NV04_DX5_TEXTURED_TRIANGLE_TLVERTEX_SX(5)),	{PRINT_FP32,	{}} },
		{__(NV04_DX5_TEXTURED_TRIANGLE_TLVERTEX_SY(5)),	{PRINT_FP32,	{}} },
		{__(NV04_DX5_TEXTURED_TRIANGLE_TLVERTEX_SZ(5)),	{PRINT_FP32,	{}} },
		{__(NV04_DX5_TEXTURED_TRIANGLE_INV_W(5)),	{PRINT_FP32,	{}} },
		{__(NV04_DX5_TEXTURED_TRIANGLE_COLOR(5)),	{PRINT_X32,	{}} },
		{__(NV04_DX5_TEXTURED_TRIANGLE_SPECULAR(5)),	{PRINT_X32,	{}} },
		{__(NV04_DX5_TEXTURED_TRIANGLE_TEXTURE_S(5)),	{PRINT_FP32,	{}} },
		{__(NV04_DX5_TEXTURED_TRIANGLE_TEXTURE_T(5)),	{PRINT_FP32,	{}} },

		{__(NV04_DX5_TEXTURED_TRIANGLE_DRAW), {
				PRINT_BITFIELD(NV04_DX5_TEXTURED_TRIANGLE_DRAW_V0, print_format, "%d"),
				PRINT_BITFIELD(NV04_DX5_TEXTURED_TRIANGLE_DRAW_V1, print_format, "%d"),
				PRINT_BITFIELD(NV04_DX5_TEXTURED_TRIANGLE_DRAW_V2, print_format, "%d"),
				PRINT_BITFIELD(NV04_DX5_TEXTURED_TRIANGLE_DRAW_V3, print_format, "%d"),
				PRINT_BITFIELD(NV04_DX5_TEXTURED_TRIANGLE_DRAW_V4, print_format, "%d"),
				PRINT_BITFIELD(NV04_DX5_TEXTURED_TRIANGLE_DRAW_V5, print_format, "%d"),
									      	{}} },
		{}},
	},
};

// XXX
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
//	hash ^= chid << (ht_bits - 4);
	hash <<= 3;

	VG_(message)(Vg_UserMsg, "Object hash is %08x", hash);

	int chan;
	int found = 0;

	// FIXME: I really dunno the number of channels
	for (chan = 0; chan < 32; chan++) {
		temp = hash ^ (chan << (ht_bits - 4 + 3));
		if (ML_(all_regs)[(NV_PRAMIN + ht_base + temp) / 4] == name) {
			VG_(message)(Vg_UserMsg, "Channel is %d", chan);
			hash = temp;
			found = 1;
			break;
		}
	}

	if (found == 0) {
		VG_(message)(Vg_UserMsg, "Object not found at expected place. bruteforcing the table. PLEASE REPORT!");
		// brute force hash!
		for(i=0x00700000 + ht_base ;i<0x00700000+ht_base + ht_size; i+=4) {
			if (ML_(all_regs)[i/4] == name) {
				hash = i - 0x00700000 - ht_base;
				found = 1;
				break;
			}
		}
	}

	if (found == 0) {
		VG_(message)(Vg_UserMsg, "Object not found in hash table! REPORT ASAP!!!!");
		return 0;
	}
	
	VG_(message)(Vg_UserMsg, "Object found at %08x", hash);

	context = ML_(all_regs)[(NV_PRAMIN + ht_base + hash + 4) / 4];
	VG_(message)(Vg_UserMsg, "Context is %08x", context);

	instanceMem = context & 0xffff;

	// engine
	if (((context >> 16) & 0x3) != 1) {
		VG_(message) (Vg_UserMsg, "Can't decode objects with this context!");

		type = (name & 0xff00) >> 8;
	} else {
		type = ML_(all_regs)[(NV_PRAMIN + (instanceMem << 4) + 0 * 4)/4] & 0xff;
	}

	for (i = 0; i < 4; i++) {
		VG_(message)(Vg_UserMsg, "InstanceMem[%d] = %08x", i, ML_(all_regs)[(NV_PRAMIN + (instanceMem << 4)  + i * 4)/4]);
	}

	return type;
}

char *ML_(format_load)(int object_type, int offset, unsigned int value) {
	static char buf[BUFSIZ];
	int pos = 0;
	const object_store *obj;
	const object_field_store *field;

	tl_assert(object_type < sizeof(nv_objects)/sizeof(object_store));

	obj = &nv_objects[object_type];

	field = &obj->fields[0];

	while (field->name != NULL) {
		if (field->offset == offset) {
			break;
		}

		field ++;
	}

	if (field->name == NULL) {
		if (obj->name == NULL) {
			pos += VG_(snprintf)(buf+pos, BUFSIZ-pos, "NvType%02x", object_type);
		} else {
			VG_(strncpy)(buf+pos, obj->name, BUFSIZ-pos);
			pos += VG_(strlen)(buf+pos);
		}

		pos += VG_(snprintf)(buf+pos, BUFSIZ-pos, "[0x%04x/4] = ", offset);
	} else {
		VG_(strncpy)(buf+pos, field->name, BUFSIZ-pos);
		pos += VG_(strlen)(buf+pos);

		VG_(strncpy)(buf+pos, " = ", BUFSIZ-pos);
		pos += 3;
	}

	if (field->data[0].func == NULL) {
		pos += VG_(snprintf)(buf+pos, BUFSIZ-pos, user_format, value);
	} else {
		int i;

		for (i = 0; field->data[i].func != NULL; i++) {
			if (i != 0) {
				VG_(strncpy)(buf+pos, " | ", 3);
				pos += 3;
			}
			const char *str = field->data[i].func(
					(value >> field->data[i].lowbit) & (0xFFFFFFFF >> (31-field->data[i].highbit + field->data[i].lowbit)),
					field->data[i].data);

			VG_(strncpy)(buf+pos, str, BUFSIZ-pos);
			pos += VG_(strlen)(str);
		}
	}


	return buf;
}
