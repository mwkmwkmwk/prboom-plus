#ifndef UDOOMDEV_H
#define UDOOMDEV_H

#ifdef __KERNEL__
#include <linux/kernel.h>
#else
#include <stdint.h>
#endif

#include <linux/ioctl.h>

/* Creates a new buffer on the device, returns its fd.  */
struct udoomdev_ioctl_create_buffer {
	uint32_t size;
};
#define UDOOMDEV_IOCTL_CREATE_BUFFER _IOW('D', 0x00, struct udoomdev_ioctl_create_buffer)

/* Maps a given buffer somewhere in the current context, returns the address.  */
struct udoomdev_ioctl_map_buffer {
	uint32_t buf_fd;
	/* Actually a bool.  */
	uint32_t map_rdonly;
};

/* Unmaps a buffer from the current context by its address, returns 0.  */
struct udoomdev_ioctl_unmap_buffer {
	uint32_t addr;
};

/* Runs a batch of commands on the current context, returns 0.  */
struct udoomdev_ioctl_run {
	uint32_t addr;
	uint32_t size;
};

/* Waits for all but `num_back` last commands submitted on this context
 * to complete, returns 0.  */
struct udoomdev_ioctl_wait {
	uint32_t num_back;
};

#define UDOOMDEV_IOCTL_MAP_BUFFER _IOW('D', 0x01, struct udoomdev_ioctl_map_buffer)
#define UDOOMDEV_IOCTL_UNMAP_BUFFER _IOW('D', 0x02, struct udoomdev_ioctl_unmap_buffer)
#define UDOOMDEV_IOCTL_RUN _IOW('D', 0x03, struct udoomdev_ioctl_run)
#define UDOOMDEV_IOCTL_WAIT _IOW('D', 0x04, struct udoomdev_ioctl_wait)

#endif
