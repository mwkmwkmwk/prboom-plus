#ifndef DOOMDEV_H
#define DOOMDEV_H

#ifdef __KERNEL__
#include <linux/kernel.h>
#else
#include <stdint.h>
#endif

#include <linux/ioctl.h>

/* /dev/doom* ioctls.  */

struct doomdev_ioctl_create_surface {
	uint16_t width;
	uint16_t height;
};

struct doomdev_ioctl_create_texture {
	uint64_t data_ptr;
	uint32_t size;
	uint16_t height;
	uint16_t _pad;
};

struct doomdev_ioctl_create_flat {
	uint64_t data_ptr;
};

struct doomdev_ioctl_create_colormaps {
	uint64_t data_ptr;
	uint32_t num;
	uint32_t _pad;
};

#define DOOMDEV_IOCTL_CREATE_SURFACE _IOW('D', 0x00, struct doomdev_ioctl_create_surface)
#define DOOMDEV_IOCTL_CREATE_TEXTURE _IOW('D', 0x01, struct doomdev_ioctl_create_texture)
#define DOOMDEV_IOCTL_CREATE_FLAT _IOW('D', 0x02, struct doomdev_ioctl_create_flat)
#define DOOMDEV_IOCTL_CREATE_COLORMAPS _IOW('D', 0x03, struct doomdev_ioctl_create_colormaps)

struct doomdev_surf_ioctl_copy_rects {
	uint64_t rects_ptr;
	uint32_t surf_src_fd;
	uint16_t rects_num;
	uint16_t _pad;
};

struct doomdev_copy_rect {
	uint16_t pos_dst_x;
	uint16_t pos_dst_y;
	uint16_t pos_src_x;
	uint16_t pos_src_y;
	uint16_t width;
	uint16_t height;
};

struct doomdev_surf_ioctl_fill_rects {
	uint64_t rects_ptr;
	uint16_t rects_num;
	uint16_t _pad[3];
};

struct doomdev_fill_rect {
	uint16_t pos_dst_x;
	uint16_t pos_dst_y;
	uint16_t width;
	uint16_t height;
	uint8_t color;
	uint8_t _pad;
};

struct doomdev_surf_ioctl_draw_lines {
	uint64_t lines_ptr;
	uint16_t lines_num;
	uint16_t _pad[3];
};

struct doomdev_line {
	uint16_t pos_a_x;
	uint16_t pos_a_y;
	uint16_t pos_b_x;
	uint16_t pos_b_y;
	uint8_t color;
	uint8_t _pad;
};

struct doomdev_surf_ioctl_draw_background {
	uint32_t flat_fd;
};

struct doomdev_surf_ioctl_draw_columns {
	uint64_t columns_ptr;
	uint32_t texture_fd;
	uint32_t translations_fd;
	uint32_t colormaps_fd;
	uint16_t columns_num;
	uint8_t draw_flags;
	uint8_t translation_idx;
};

struct doomdev_column {
	uint32_t texture_offset;
	uint32_t ustart;
	uint32_t ustep;
	uint16_t y1;
	uint16_t y2;
	uint16_t x;
	uint8_t colormap_idx;
	uint8_t _pad;
};

struct doomdev_surf_ioctl_draw_spans {
	uint64_t spans_ptr;
	uint32_t flat_fd;
	uint32_t translations_fd;
	uint32_t colormaps_fd;
	uint16_t spans_num;
	uint8_t draw_flags;
	uint8_t translation_idx;
};

struct doomdev_span {
	uint32_t ustart;
	uint32_t vstart;
	uint32_t ustep;
	uint32_t vstep;
	uint16_t x1, x2;
	uint16_t y;
	uint8_t colormap_idx;
	uint8_t _pad;
};

#define DOOMDEV_SURF_IOCTL_COPY_RECTS _IOW('D', 0x10, struct doomdev_surf_ioctl_copy_rects)
#define DOOMDEV_SURF_IOCTL_FILL_RECTS _IOW('D', 0x11, struct doomdev_surf_ioctl_fill_rects)
#define DOOMDEV_SURF_IOCTL_DRAW_LINES _IOW('D', 0x12, struct doomdev_surf_ioctl_draw_lines)
#define DOOMDEV_SURF_IOCTL_DRAW_BACKGROUND _IOW('D', 0x13, struct doomdev_surf_ioctl_draw_background)
#define DOOMDEV_SURF_IOCTL_DRAW_COLUMNS _IOW('D', 0x14, struct doomdev_surf_ioctl_draw_columns)
#define DOOMDEV_SURF_IOCTL_DRAW_SPANS _IOW('D', 0x15, struct doomdev_surf_ioctl_draw_spans)

#define DOOMDEV_DRAW_FLAGS_FUZZ		0x01
#define DOOMDEV_DRAW_FLAGS_TRANSLATE	0x02
#define DOOMDEV_DRAW_FLAGS_COLORMAP	0x04

#endif
