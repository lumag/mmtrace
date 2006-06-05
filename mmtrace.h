typedef struct mt_mmap_trace_s mt_mmap_trace_t;

struct mt_mmap_trace_s {
	void (*store_1)(Char *name, ULong offset, UChar data);
	void (*store_2)(Char *name, ULong offset, UShort data);
	void (*store_4)(Char *name, ULong offset, UInt data);
	void (*store_8)(Char *name, ULong offset, ULong data);
	void (*store_16)(Char *name, ULong offset, U128 data);
};

void ML_(trace_pre_ioctl)(Int fd, Int request, void* arg);
void ML_(trace_post_ioctl)(SysRes res);


const mt_mmap_trace_t* ML_(get_mmap_trace)(Addr addr, SizeT len, NSegment *seg);
void ML_(trace_flush)();

