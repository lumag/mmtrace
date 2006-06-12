#include "mmtrace.h"
#include "mt_client_common.h"

#include "mt_nvidia.h"

#define NV_IOCTL_MAGIC      'F'

static Bool this_ioctl_trace = False;
static int ioctl_dir;
static int ioctl_type;
static int ioctl_nr;
static int ioctl_size;
static void *ioctl_data;

static int mt_nv_ctrl_fd = -1;
static int mt_nv_dev_fd = -1;

void ML_(trace_pre_ioctl)(int fd, Int request, void* arg) {

	ioctl_dir = _VKI_IOC_DIR(request);
	ioctl_type = _VKI_IOC_TYPE(request);
	ioctl_nr = _VKI_IOC_NR(request);
	ioctl_size = _VKI_IOC_SIZE(request);
	ioctl_data = arg;

	if (ioctl_dir == _VKI_IOC_NONE
	 || ioctl_type != NV_IOCTL_MAGIC) {
		return;
	}

	VG_(message)(Vg_UserMsg, "ioctl: fd=%d nr=%d, arg=%08x, size=%d", fd, ioctl_nr, arg, ioctl_size);

	this_ioctl_trace = True;
	if ((ioctl_dir & _VKI_IOC_WRITE) && ioctl_size > 0) {
		print_buffer_4("ioctl in:  ", ioctl_data, ioctl_size/4);
	}
}

void ML_(trace_post_ioctl)(SysRes res) {
	int i;

	if (!this_ioctl_trace) {
		return;
	}

	this_ioctl_trace = False;
	VG_(message)(Vg_DebugMsg, "ioctl returned %d", res.val);
	if ((ioctl_dir & _VKI_IOC_READ) && ioctl_size > 0) {
		print_buffer_4("ioctl out: ", ioctl_data, ioctl_size/4);
	}

	switch (ioctl_nr) {
		case 200:
			VG_(memcpy)(&ML_(nvinfo), ioctl_data, sizeof(ML_(nvinfo)));

			for (i = 0; i < 8; i++) {
				if (ML_(nvinfo).devices[i].unknown != 0) {
					VG_(message)(Vg_DebugMsg, "Reported card %04x:%04x at %x:%02x. regs %08x:%08x, fb %08x:%08x",
							ML_(nvinfo).devices[i].vid,
							ML_(nvinfo).devices[i].pid,
							ML_(nvinfo).devices[i].bus,
							ML_(nvinfo).devices[i].slot,
							ML_(nvinfo).devices[i].reg_addr,
							ML_(nvinfo).devices[i].reg_addr + ML_(nvinfo).devices[i].reg_size - 1,
							ML_(nvinfo).devices[i].fb_addr,
							ML_(nvinfo).devices[i].fb_addr + ML_(nvinfo).devices[i].reg_size - 1
							);
				}
			}

			break;
	}
}

void ML_(trace_pre_open)(HChar *name, UInt flags) {
	if (!VG_(strcmp)("/dev/nvidiactl", name)) {
		VG_(message)(Vg_UserMsg, "Control device open...");
		mt_nv_ctrl_fd = -2;
	} else if (!VG_(strncmp)("/dev/nvidia", name, 11)) {
		VG_(message)(Vg_UserMsg, "Card device open...");

		if (mt_nv_dev_fd == -1) {
			mt_nv_dev_fd = -2;
			ML_(nvcard) = name[11] - '0';
		}
	}
}

void ML_(trace_post_open)(SysRes res) {
	if (mt_nv_ctrl_fd == -2) {
		mt_nv_ctrl_fd = res.val;
		VG_(message)(Vg_UserMsg, "Opened at fd %d", res.val);
	} else if (mt_nv_dev_fd == -2) {
		mt_nv_dev_fd = res.val;
		VG_(message)(Vg_UserMsg, "Opened at fd %d", res.val);
		ML_(device_selected)(mt_nv_dev_fd);
	}
}

void ML_(trace_pre_close)(int fd) {
	if (fd == mt_nv_ctrl_fd) {
		mt_nv_ctrl_fd = -1;
		VG_(message)(Vg_UserMsg, "Closed control device");
	} else if (fd == mt_nv_dev_fd) {
		mt_nv_dev_fd = -1;
		VG_(message)(Vg_UserMsg, "Closed card device");
	}
}
void ML_(trace_post_close)(SysRes res) {
}
