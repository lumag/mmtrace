#ifndef MMTRACE
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "re.h"
#define message(fmt, ...)	printf(fmt "\n", ##__VA_ARGS__)
#else
#include "mmtrace.h"
#include "mt_nvidia.h"
#endif

#include "nv_hw.h"
#include "nv_objects.h"
#include "objects.h"

const char* print_format(unsigned int value, void*format) {
	static char buf[128];
	snprintf(buf, sizeof(buf), format?format:"%x", value);
	return buf;
}

const char* print_float(unsigned int value, void*format) {
	static char buf[128];
	snprintf(buf, sizeof(buf), format?format:"%f", (double)*(float*)&value);
	return buf;
}

#define __(a)	a, # a
#define PRINT_BITFIELD(bf, func, data)	{0?bf, 1?bf, func, data}
#define PRINT_FP32	{0, 31, &print_float, NULL}
#define PRINT_X32	{0, 31, &print_format, "0x%x"}

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
		{__(NV04_DX5_TEXTURED_TRIANGLE_TEXTURE_FORMAT),	{		{}} },
		{__(NV04_DX5_TEXTURED_TRIANGLE_TEXTURE_FILTER),	{		{}} },
		{__(NV04_DX5_TEXTURED_TRIANGLE_BLEND),		{		{}} },
		{__(NV04_DX5_TEXTURED_TRIANGLE_CONTROL),	{		{}} },
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

	[NV04_CONTEXT_SURFACES_3D] = {__(NV04_CONTEXT_SURFACES_3D), {
		{}},
	},
};

// XXX
// Warning! this will work only for >= NV04!
unsigned int find_object_type(unsigned int name) {
	unsigned int htval = all_regs[NV_PFIFO_RAMHT/4];

	unsigned int ht_base;
	unsigned int ht_size;
	unsigned int ht_bits;
//	unsigned int ht_search;


	if (card_family == NV03) {
		ht_base = 0x1000 * ((htval >> 12) & 0x0f);
		ht_size = 0x1000 << ((htval >> 16) & 0x03);
		ht_bits = ((htval >> 16) & 0x03) + 9;
	} else {
		ht_base = 0x1000 * ((htval >> 4) & 0x1f);
		ht_size = 0x1000 << ((htval >> 16) & 0x03);
		ht_bits = ((htval >> 16) & 0x03) + 9;
//		ht_search = 0x10 << ((htval >> 24) & 0x03);
	}

	unsigned int hash = 0, ohash;
	unsigned int context;
	unsigned int instanceMem;
	unsigned int type;

	int i;
	unsigned int temp;

	message("Searching for object %08x", name);

	temp = name;
	for (i = 32; i > 0; i -= ht_bits) {
		hash ^= (temp & ((1 << ht_bits) - 1));
		temp >>= ht_bits;
	}
//	hash ^= chid << (ht_bits - 4);
	hash <<= 3;
	ohash = hash;

	message("Object hash is %08x", hash);

	int chan;
	int found = 0;

	// FIXME: I really dunno the number of channels
	for (chan = 0; chan < 32; chan++) {
		temp = hash ^ (chan << (ht_bits - 4 + 3));
		if (all_regs[(NV_PRAMIN + ht_base + temp) / 4] == name) {
			message("Channel is %d", chan);
			hash = temp;
			found = 1;
			break;
		}
	}

	if (found == 0) {
		message("Object not found at expected place. bruteforcing the table. PLEASE REPORT!");
		// brute force hash!
		for(i=0x00700000 + ht_base ;i<0x00700000+ht_base + ht_size; i+=4) {
			if (all_regs[i/4] == name) {
				hash = i - 0x00700000 - ht_base;
				found = 1;
				break;
			}
		}
	}

	if (found == 0) {
		message("Object not found in hash table! REPORT ASAP!!!!");
		return 0;
	}
	
	message("Object found at %08x, %08x", hash, hash ^ ohash);

	context = all_regs[(NV_PRAMIN + ht_base + hash + 4) / 4];
	message("Context is %08x", context);

	instanceMem = context & 0xffff;

	// engine
	int engine;

	switch (card_family) {
		case NV03:
			engine = (context >> 23) & 1;
			break;
		case NV04:
		case NV10:
		case NV20:
		case NV30:
			engine = (context >> 16) & 0x3;
			break;
		case NV40:
		case G70:
			engine = (context >> 20) & 0x3;
			break;
	}
	if (engine != ENGINE_GRAPHICS) {
		message("Can't decode objects with this context!");

		type = (name & 0xff00) >> 8;
	} else {
		type = all_regs[(NV_PRAMIN + (instanceMem << 4) + 0 * 4)/4] & 0xff;
	}

	for (i = 0; i < 4; i++) {
		message("InstanceMem[%d] = %08x", i, all_regs[(NV_PRAMIN + (instanceMem << 4)  + i * 4)/4]);
	}

	return type;
}

char *format_load(int object_type, int offset, unsigned int value) {
	static char buf[BUFSIZ];
	int pos = 0;
	const object_store *obj;
	const object_field_store *field;

	assert(object_type < sizeof(nv_objects)/sizeof(object_store));

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
			memset(buf+pos, ' ', 20); pos += 20;
			pos += snprintf(buf+pos, BUFSIZ-pos, "NvType0%03x", object_type);
		} else {
			pos += snprintf(buf+pos, BUFSIZ-pos, "%30s", obj->name);
		}
		memset(buf+pos, ' ', 6); pos += 6;

		pos += snprintf(buf+pos, BUFSIZ-pos, "[0x%04x/4] = ", offset);
	} else {
		pos += snprintf(buf+pos, BUFSIZ-pos, "%45s = ", obj->name);
	}

	data_print_store* dp = &field->data[0];

	if (dp[0].func == NULL) {
		dp = user_data_print;
	}

	int i;
	for (i = 0; dp[i].func != NULL; i++) {
		if (i != 0) {
			strncpy(buf+pos, " | ", 3);
			pos += 3;
		}
		const char *str = dp[i].func(
				(value >> dp[i].lowbit) & (0xFFFFFFFF >> (31-dp[i].highbit + dp[i].lowbit)),
				dp[i].data);

		strncpy(buf+pos, str, BUFSIZ-pos);
		pos += strlen(str);
	}

	return buf;
}

