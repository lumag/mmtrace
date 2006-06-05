#define NV_IOCTL_MAGIC      'F'

static Bool this_ioctl_trace = False;
static int ioctl_dir;
static int ioctl_type;
static int ioctl_nr;
static int ioctl_size;
static UChar *ioctl_data;

void ML_(trace_pre_ioctl)(Int fd, Int request, void* arg) {

	ioctl_dir = _VKI_IOC_DIR(request);
	ioctl_type = _VKI_IOC_TYPE(request);
	ioctl_nr = _VKI_IOC_NR(request);
	ioctl_size = _VKI_IOC_SIZE(request);
	ioctl_data = (UChar*) arg;

	if (ioctl_dir == _VKI_IOC_NONE
	 || ioctl_type != NV_IOCTL_MAGIC) {
		return;
	}

	VG_(message)(Vg_UserMsg, "ioctl: fd=%d nr=%d, arg=%08x, size=%d", fd, ioctl_nr, arg, ioctl_size);

	this_ioctl_trace = True;
	if ((ioctl_dir & _VKI_IOC_WRITE) && ioctl_size > 0) {
		print_buffer("ioctl in:  ", ioctl_data, ioctl_size);
	}
}

void ML_(trace_post_ioctl)(SysRes res) {
	if (!this_ioctl_trace) {
		return;
	}

	this_ioctl_trace = False;
	VG_(message)(Vg_DebugMsg, "ioctl returned %d", res.val);
	if ((ioctl_dir & _VKI_IOC_READ) && ioctl_size > 0) {
		print_buffer("ioctl out: ", ioctl_data, ioctl_size);
	}
}


