#ifndef MT_NVIDIA_H
#define MT_NVIDIA_H

typedef struct {
	UInt unknown;
	UInt bus;
	UInt slot;
	UInt vid;
	UInt pid;
	UInt irq;
	UInt reg_addr;
	UInt reg_size;
	UInt fb_addr;
	UInt fb_size;
} info_ioctl_struct;

typedef union {
	info_ioctl_struct devices[8];
	UChar bytes[320];
	UInt words[320/4];
} nvidia_info;

extern nvidia_info ML_(nvinfo);
extern int ML_(nvcard);
extern int ML_(nvarch);

extern UInt* ML_(all_regs);
#define all_regs ML_(all_regs)

extern UInt ML_(card_family);
#define card_family ML_(card_family)

#define find_object_type	ML_(find_object_type)
#define format_load		ML_(format_load)
#define print_format		ML_(print_format)
#define print_float		ML_(print_float)
#define user_data_print		ML_(user_data_print)
#define record_offset_in	ML_(record_offset_in)
#define record_offset_out	ML_(record_offset_out)
#define record_length_in	ML_(record_length_in)
#define record_dma_op		ML_(record_dma_op)

#define gpu_ofs_to_agp		ML_(gpu_ofs_to_agp)
#define find_dma_src_from_dest	ML_(find_dma_src_from_dest)
unsigned int *gpu_ofs_to_agp(unsigned long ofs);
unsigned int  find_dma_src_from_dest(unsigned int to);

#define object_create		ML_(object_create)
void object_create(unsigned int parent, unsigned int name, unsigned int type);


void ML_(device_selected)(int fd);

void ML_(fifo_flush)();
void ML_(fifo_store_4)(Char *name, ULong offset, UInt data);
void ML_(fifo_store_16)(Char *name, ULong offset, U128 data);
void ML_(fifo_load_4)(Char *name, ULong offset, UInt data);

#ifndef BUFSIZ
#define BUFSIZ 1024
#endif


#endif
