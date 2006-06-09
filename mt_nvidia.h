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

UInt ML_(find_object_type)(UInt name, UInt chid);

void ML_(device_selected)(int fd);

void ML_(fifo_flush)();
void ML_(fifo_store_4)(Char *name, ULong offset, UInt data);
void ML_(fifo_store_16)(Char *name, ULong offset, U128 data);
void ML_(fifo_load_4)(Char *name, ULong offset, UInt data);

#endif
