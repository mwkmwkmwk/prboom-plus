#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include "v_video.h"
#include "i_fdoomdev.h"
#include "fdoomdev.h"
#include "fharddoom.h"
#include "lprintf.h"
#include "r_draw.h"
#include "r_main.h"
#include "w_wad.h"

#define FDOOMDEV_NUM_CMD_BUFS		16
#define FDOOMDEV_CMD_BUF_SIZE		0x10000
#define FDOOMDEV_CMD_BATCH_RESERVE	0x1000

int fdoom_fd = -1;
uint32_t *fdoomdev_cmd_ptr[FDOOMDEV_NUM_CMD_BUFS];
int fdoomdev_cmd_fd[FDOOMDEV_NUM_CMD_BUFS];
int fdoomdev_cmd_buf_idx;
int fdoomdev_cmd_buf_pos;
int fdoomdev_translations_fd;
int fdoomdev_translations_idx;
int fdoomdev_transmap_fd;
int fdoomdev_transmap_idx;

int fdoomdev_bound_fds[FDOOMDEV_MAX_USER_SLOTS];
int fdoomdev_used_fds;

int fdoomdev_surf_width;
int fdoomdev_surf_height;

uint32_t *fdoomdev_cols_header;
uint32_t *fdoomdev_fuzz_header;
uint32_t *fdoomdev_spans_header;
int fdoomdev_spans_flat_fd;
int fdoomdev_batch_screen;
int fdoomdev_batch_screen_slot;
int fdoomdev_batch_cmap_fd;
int fdoomdev_batch_cmap_slot;
int fdoomdev_cols_translation;
int fdoomdev_cols_transparent;


struct poolbuf {
	struct poolbuf *next;
	int fd;
	uint32_t used;
	uint32_t size;
	void *ptr;
};

struct poolbuf *fdoomdev_pool_flats;
struct poolbuf *fdoomdev_pool_colormaps;
struct poolbuf *fdoomdev_pool_transmaps;
struct poolbuf *fdoomdev_pool_textures;

// XXX bump later
#define POOL_MAX	0x10000

void *I_DoomDevCreateBuf(uint32_t size, uint32_t pitch, int *pfd) {
  struct fdoomdev_ioctl_create_buffer cb = {size, pitch};
  long addr;
  void *mdata;
  int fd = ioctl(fdoom_fd, FDOOMDEV_IOCTL_CREATE_BUFFER, &cb);
  if (fd < 0)
    I_Error("fdoomdev create buffer fail");
  *pfd = fd;
  mdata = mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (mdata == MAP_FAILED)
    I_Error("fdoomdev mmap fail");
  return mdata;
}

void I_DoomDevResizeBuf(int fd, uint32_t size) {
	struct fdoomdev_ioctl_buffer_resize br = {size};
	int res = ioctl(fd, FDOOMDEV_IOCTL_BUFFER_RESIZE, &br);
	if (res < 0)
		I_Error("fdoomdev buffer resize fail");
}

void I_DoomDevUploadBuf(const void *data, uint32_t size, struct poolbuf **phead, int *res_fd, uint32_t *res_offset) {
	struct poolbuf *buf;
	if (*phead && (*phead)->used + size <= POOL_MAX) {
		buf = *phead;
		*res_offset = buf->used;
		buf->used += size;
		if (buf->size < buf->used) {
			munmap(buf->ptr, buf->size);
			while (buf->size < buf->used)
				buf->size *= 2;
			I_DoomDevResizeBuf(buf->fd, buf->size);
			buf->ptr = mmap(0, buf->size, PROT_READ | PROT_WRITE, MAP_SHARED, buf->fd, 0);
			if (buf->ptr == MAP_FAILED)
				I_Error("fdoomdev mmap fail");
		}
	} else {
		buf = malloc(sizeof *buf);
		buf->next = *phead;
		buf->used = size;
		buf->size = 0x1000;
		*res_offset = 0;
		while (buf->size < size)
			buf->size *= 2;
		*phead = buf;
		buf->ptr = I_DoomDevCreateBuf(buf->size, 0x40, &buf->fd);
	}
	*res_fd = buf->fd;
	memcpy((char *)buf->ptr + *res_offset, data, size);
}

void I_DoomDevFreePool(struct poolbuf **phead) {
	struct poolbuf *buf = *phead;
	*phead = 0;
	while (buf) {
		struct poolbuf *next = buf->next;
		munmap(buf->ptr, buf->size);
		close(buf->fd);
		free(buf);
		buf = next;
	}
}

void I_DoomDevFlushCmd(void)
{
  struct fdoomdev_ioctl_run run = {
    .cmd_fd = fdoomdev_cmd_fd[fdoomdev_cmd_buf_idx],
    .cmd_addr = 0,
    .cmd_size = fdoomdev_cmd_buf_pos * 4,
    .buffers_num = fdoomdev_used_fds,
  };
  int i;
  struct fdoomdev_ioctl_wait wait = {FDOOMDEV_NUM_CMD_BUFS - 1};
  if (!fdoomdev_cmd_buf_pos)
    return;
  for (i = 0; i < fdoomdev_used_fds; i++)
    run.buffer_fd[i] = fdoomdev_bound_fds[i];
  if (ioctl(fdoom_fd, FDOOMDEV_IOCTL_RUN, &run))
    I_Error("fdoomdev run fail");
  if (ioctl(fdoom_fd, FDOOMDEV_IOCTL_WAIT, &wait))
    I_Error("fdoomdev wait for space fail");
  fdoomdev_cmd_buf_idx++;
  fdoomdev_cmd_buf_idx %= FDOOMDEV_NUM_CMD_BUFS;
  fdoomdev_cmd_buf_pos = 0;
  fdoomdev_cols_header = 0;
  fdoomdev_fuzz_header = 0;
  fdoomdev_spans_header = 0;
  fdoomdev_used_fds = 0;
}

static inline int I_DoomDevXlatSlots(int nslots, int *fds, int *slots) {
  int i, j;
  int orig = fdoomdev_used_fds;
  for (i = 0; i < nslots; i++) {
    int fd = fds[i];
    for (j = 0; j < fdoomdev_used_fds; j++) {
      if (fdoomdev_bound_fds[j] == fd) {
        slots[i] = j;
        goto ok;
      }
    }
    if (fdoomdev_used_fds >= FDOOMDEV_MAX_USER_SLOTS) {
      fdoomdev_used_fds = orig;
      return 0;
    }
    fdoomdev_bound_fds[fdoomdev_used_fds] = fd;
    slots[i] = fdoomdev_used_fds++;
ok:;
  }
  return 1;
}

static inline void I_DoomDevReserve(int num, int nslots, int *fds, int *slots) {
  fdoomdev_cols_header = 0;
  fdoomdev_fuzz_header = 0;
  fdoomdev_spans_header = 0;
  if (fdoomdev_cmd_buf_pos + num > FDOOMDEV_CMD_BUF_SIZE)
    I_DoomDevFlushCmd();
  if (!I_DoomDevXlatSlots(nslots, fds, slots)) {
    I_DoomDevFlushCmd();
    assert(I_DoomDevXlatSlots(nslots, fds, slots));
  }
}

static inline uint32_t *I_DoomDevCurCmd() {
  return fdoomdev_cmd_ptr[fdoomdev_cmd_buf_idx] + fdoomdev_cmd_buf_pos;
}

static inline void I_DoomDevSend(uint32_t word) {
  fdoomdev_cmd_ptr[fdoomdev_cmd_buf_idx][fdoomdev_cmd_buf_pos++] = word;
}

void I_DoomDevSync(void) {
  struct fdoomdev_ioctl_wait wait = {0};
  I_DoomDevFlushCmd();
  if (fdoom_fd != -1 && ioctl(fdoom_fd, FDOOMDEV_IOCTL_WAIT, &wait))
    I_Error("fdoomdev sync fail");
}

void I_DoomDevRead(int idx)
{
  int y;
  I_DoomDevSync();
  for (y = 0; y < screens[idx].height; y++) {
    memcpy(screens[idx].data + screens[idx].byte_pitch * y, screens[idx].doomdev_ptr + fdoomdev_surf_width * y, screens[idx].width);
  }
}

void I_DoomDevAllocScreens(void)
{
  int i;
  int width = screens[0].width;
  uint32_t tmp;
  byte *trans = malloc((CR_LIMIT + 3) * 256);
  for (i = 0; i < CR_LIMIT; i++)
    memcpy(trans + i * 256, colrngs[i], 256);
  memcpy(trans + CR_LIMIT * 256, translationtables, 256 * 3);
  if (width & 0x3f) {
    width |= 0x3f;
    width++;
  }
  fdoomdev_surf_width = width;
  fdoomdev_surf_height = screens[0].height;
  fdoom_fd = open("/dev/fdoom0", O_RDWR);
  if (fdoom_fd < 0)
    I_Error("doomdev2 open fail");

  for (i = 0; i < FDOOMDEV_NUM_CMD_BUFS; i++)
    fdoomdev_cmd_ptr[i] = I_DoomDevCreateBuf(FDOOMDEV_CMD_BUF_SIZE * 4, 0, &fdoomdev_cmd_fd[i]);
  for (i = 0; i < NUM_SCREENS; i++)
    screens[i].doomdev_ptr = I_DoomDevCreateBuf(fdoomdev_surf_width * fdoomdev_surf_height, fdoomdev_surf_width, &screens[i].doomdev_fd);
  I_DoomDevUploadBuf(trans, (CR_LIMIT + 3) * 0x100, &fdoomdev_pool_colormaps, &fdoomdev_translations_fd, &tmp);
  assert(!(tmp & 0xff));
  fdoomdev_translations_idx = tmp >> 8;
  I_DoomDevUploadBuf(main_tranmap, 1 << 16, &fdoomdev_pool_transmaps, &fdoomdev_transmap_fd, &tmp);
  assert(!(tmp & 0xffff));
  fdoomdev_transmap_idx = tmp >> 16;
  free(trans);
  I_DoomDevUploadColormap(boom_cm);
  fdoomdev_cmd_buf_idx = 0;
  fdoomdev_cmd_buf_pos = 0;
  fdoomdev_cols_header = 0;
  fdoomdev_fuzz_header = 0;
  fdoomdev_spans_header = 0;
}

void I_DoomDevFreeScreens(void)
{
  int i;
  I_DoomDevSync();
  R_FlushHardPatches();
  for (i = 0; i < numlumps; i++)
    lumpinfo[i].flat_doomdev_fd = -1;
  for (i = 0; i < numcolormaps; i++)
    colormap_doomdev_fd[i] = -1;
  I_DoomDevFreePool(&fdoomdev_pool_textures);
  I_DoomDevFreePool(&fdoomdev_pool_flats);
  I_DoomDevFreePool(&fdoomdev_pool_colormaps);
  I_DoomDevFreePool(&fdoomdev_pool_transmaps);
  for (i = 0; i < FDOOMDEV_NUM_CMD_BUFS; i++) {
    if (fdoomdev_cmd_ptr[i]) {
      munmap(fdoomdev_cmd_ptr[i], FDOOMDEV_CMD_BUF_SIZE * 4);
      fdoomdev_cmd_ptr[i] = 0;
      close(fdoomdev_cmd_fd[i]);
    }
  }
  for (i = 0; i < NUM_SCREENS; i++) {
    if (screens[i].doomdev_ptr) {
      munmap(screens[i].doomdev_ptr, fdoomdev_surf_width * fdoomdev_surf_height);
      screens[i].doomdev_ptr = 0;
      close(screens[i].doomdev_fd);
    }
    screens[i].doomdev_fd = -1;
  }
  if (fdoom_fd != -1)
    close(fdoom_fd);
  fdoom_fd = -1;
}

void I_DoomDevUploadPatch(rpatch_t *patch)
{
  if (patch->doomdev_fd != -1)
    return;
  I_DoomDevUploadBuf(patch->pixels, patch->height * patch->width, &fdoomdev_pool_textures, &patch->doomdev_fd, &patch->doomdev_addr);
}

void I_DoomDevUploadFlat(int lump, const byte *data)
{
  uint32_t tmp;
  if (lumpinfo[lump].flat_doomdev_fd != -1)
    return;
  I_DoomDevUploadBuf(data, 0x1000, &fdoomdev_pool_flats, &lumpinfo[lump].flat_doomdev_fd, &tmp);
  assert(!(tmp & 0xfff));
  lumpinfo[lump].flat_doomdev_idx = tmp >> 12;
}

void I_DoomDevUploadColormap(int idx)
{
  uint32_t tmp;
  if (colormap_doomdev_fd[idx] != -1)
    return;
  I_DoomDevUploadBuf(colormaps[idx], 34 * 0x100, &fdoomdev_pool_colormaps, &colormap_doomdev_fd[idx], &tmp);
  assert(!(tmp & 0xff));
  colormap_doomdev_idx[idx] = tmp >> 8;
}

void I_DoomDevPlotPixelWu(int scrn, int x, int y, byte color, int weight)
{
  I_DoomDevPlotPixel(scrn, x, y, color);
}

void I_DoomDevPlotPixel(int scrn, int x, int y, byte color)
{
  I_DoomDevFillRect(scrn, x, y, 1, 1, color);
}

void I_DoomDevFillRect(int scrn, int x, int y, int width, int height, byte colour)
{
  int fds[1] = {
    screens[scrn].doomdev_fd,
  };
  int slots[1];
  I_DoomDevReserve(3, 1, fds, slots);
  I_DoomDevSend(FHARDDOOM_USER_FILL_RECT_HEADER(slots[0], colour));
  I_DoomDevSend(FHARDDOOM_USER_FILL_RECT_W1(x, y));
  I_DoomDevSend(FHARDDOOM_USER_FILL_RECT_W2(width, height));
}

void I_DoomDevCopyRect(int srcscrn, int destscrn, int x, int y, int width, int height, enum patch_translation_e flags)
{
  int fds[2] = {
    screens[destscrn].doomdev_fd,
    screens[srcscrn].doomdev_fd,
  };
  int slots[2];
  if (flags & VPT_STRETCH_MASK)
  {
    stretch_param_t *params;
    int sx = x;
    int sy = y;

    params = &stretch_params[flags & VPT_ALIGN_MASK];

    x  = params->video->x1lookup[x];
    y  = params->video->y1lookup[y];
    width  = params->video->x2lookup[sx + width - 1] - x + 1;
    height = params->video->y2lookup[sy + height - 1] - y + 1;
    x += params->deltax1;
    y += params->deltay1;
  }
  I_DoomDevReserve(5, 2, fds, slots);
  I_DoomDevSend(FHARDDOOM_USER_BLIT_HEADER(slots[0], slots[1], 0x10, 0x10));
  I_DoomDevSend(FHARDDOOM_USER_BLIT_W1(x, y));
  I_DoomDevSend(FHARDDOOM_USER_BLIT_W2(width, height));
  I_DoomDevSend(FHARDDOOM_USER_BLIT_W3(x, y));
  I_DoomDevSend(FHARDDOOM_USER_BLIT_W4(width, height));
}

void I_DoomDevDrawLine(fline_t* fl, int color)
{
  int fds[1] = {
    screens[0].doomdev_fd,
  };
  int slots[1];
  I_DoomDevReserve(3, 1, fds, slots);
  I_DoomDevSend(FHARDDOOM_USER_DRAW_LINE_HEADER(slots[0], color));
  I_DoomDevSend(FHARDDOOM_USER_DRAW_LINE_W1(fl->a.x, fl->a.y));
  I_DoomDevSend(FHARDDOOM_USER_DRAW_LINE_W2(fl->b.x, fl->b.y));
}

void I_DoomDevFillFlat(int lump, int scrn, int x, int y, int width, int height, enum patch_translation_e flags)
{
  lump += firstflat;
  if (lumpinfo[lump].flat_doomdev_fd == -1)
  {
    const byte *data = W_CacheLumpNum(lump);
    I_DoomDevUploadFlat(lump, data);
    W_UnlockLumpNum(lump);
  }
  int fds[2] = {
    screens[scrn].doomdev_fd,
    lumpinfo[lump].flat_doomdev_fd,
  };
  int slots[2];
  I_DoomDevReserve(5, 2, fds, slots);
  I_DoomDevSend(FHARDDOOM_USER_BLIT_HEADER(slots[0], slots[1], 6, 6));
  I_DoomDevSend(FHARDDOOM_USER_BLIT_W1(0, 0));
  I_DoomDevSend(FHARDDOOM_USER_BLIT_W2(fdoomdev_surf_width, fdoomdev_surf_height));
  I_DoomDevSend(FHARDDOOM_USER_BLIT_W3(0, lumpinfo[lump].flat_doomdev_idx << 6));
  I_DoomDevSend(FHARDDOOM_USER_BLIT_W4(fdoomdev_surf_width, fdoomdev_surf_height));
}

static uint32_t I_DoomDevComputeColumnStart(pdraw_column_vars_s dcvars)
{
	int res = dcvars->texturemid + (dcvars->yl - centery) * dcvars->iscale;
	if (dcvars->flags & DRAW_COLUMN_ISPATCH)
		return ((dcvars->yl - dcvars->dy) * dcvars->iscale) & 0xFFFF;
	if (dcvars->texheight) {
		res %= dcvars->texheight << 16;
		if (res < 0)
			res += dcvars->texheight << 16;
	}
	return res;
}

static inline int I_DoomDevDrawGenericColumnBatch(pdraw_column_vars_s dcvars, int translation, int transparent, int *slot_tex) {
  int num_cols = 0;
  if (!fdoomdev_cols_header)
    return 0;
  if (fdoomdev_cmd_buf_pos + 6 > FDOOMDEV_CMD_BUF_SIZE)
    return 0;
  if (fdoomdev_cols_translation != translation)
    return 0;
  if (fdoomdev_cols_transparent != transparent)
    return 0;
  num_cols = FHARDDOOM_USER_DRAW_COLUMNS_HEADER_EXTR_NUM_COLS(fdoomdev_cols_header[0]);
  if (num_cols == 0xffff)
    return 0;
  if (fdoomdev_batch_screen != drawvars.screen)
    return 0;
  if (fdoomdev_batch_cmap_fd != colormap_doomdev_fd[boom_cm])
    return 0;
  if (!I_DoomDevXlatSlots(1, &dcvars->texture_doomdev_fd, slot_tex))
    return 0;
  num_cols++;
  fdoomdev_cols_header[0] = FHARDDOOM_USER_DRAW_COLUMNS_HEADER(fdoomdev_batch_screen_slot, translation != -1, 1, transparent, num_cols);
  return 1;
}

void I_DoomDevDrawGenericColumn(pdraw_column_vars_s dcvars, int translation, int transparent) {
  int slot_tex;
  if (dcvars->yl > dcvars->yh)
    return;
  assert(!transparent || translation == -1);
  if (!I_DoomDevDrawGenericColumnBatch(dcvars, translation, transparent, &slot_tex)) {
    int fds[4] = {
      screens[drawvars.screen].doomdev_fd,
      colormap_doomdev_fd[boom_cm],
      dcvars->texture_doomdev_fd,
      transparent ? fdoomdev_transmap_fd : fdoomdev_translations_fd,
    };
    int slots[4];
    I_DoomDevReserve(FDOOMDEV_CMD_BATCH_RESERVE, (translation != -1 || transparent) ? 4 : 3, fds, slots);
    fdoomdev_cols_header = I_DoomDevCurCmd();
    fdoomdev_batch_screen = drawvars.screen;
    fdoomdev_batch_screen_slot = slots[0];
    fdoomdev_batch_cmap_fd = colormap_doomdev_fd[boom_cm];
    fdoomdev_batch_cmap_slot = slots[1];
    fdoomdev_cols_translation = translation;
    fdoomdev_cols_transparent = transparent;
    I_DoomDevSend(FHARDDOOM_USER_DRAW_COLUMNS_HEADER(slots[0], translation != -1, 1, transparent, 1));
    if (transparent)
      I_DoomDevSend(FHARDDOOM_USER_DRAW_COLUMNS_W1(0, 0, slots[3], fdoomdev_transmap_idx));
    else if (translation != -1)
      I_DoomDevSend(FHARDDOOM_USER_DRAW_COLUMNS_W1(slots[3], fdoomdev_translations_idx + translation, 0, 0));
    slot_tex = slots[2];
  }
  I_DoomDevSend(FHARDDOOM_USER_DRAW_COLUMNS_WR0(dcvars->x + drawvars.xoff, dcvars->texheight));
  I_DoomDevSend(FHARDDOOM_USER_DRAW_COLUMNS_WR1(dcvars->yl + drawvars.yoff, dcvars->yh + drawvars.yoff));
  I_DoomDevSend(FHARDDOOM_USER_DRAW_COLUMNS_WR2(slot_tex, dcvars->texture_doomdev_addr + (dcvars->source - dcvars->texture_base)));
  I_DoomDevSend(I_DoomDevComputeColumnStart(dcvars));
  I_DoomDevSend(dcvars->iscale);
  I_DoomDevSend(FHARDDOOM_USER_DRAW_COLUMNS_WR5(fdoomdev_batch_cmap_slot, colormap_doomdev_idx[boom_cm] + ((dcvars->colormap - colormaps[boom_cm]) >> 8)));
}

void I_DoomDevDrawColumn(pdraw_column_vars_s dcvars)
{
  I_DoomDevDrawGenericColumn(dcvars, -1, 0);
}

void I_DoomDevDrawTransparentColumn(pdraw_column_vars_s dcvars)
{
  I_DoomDevDrawGenericColumn(dcvars, -1, 1);
}

void I_DoomDevDrawTranslatedColumn(pdraw_column_vars_s dcvars)
{
  int i;
  int xlat = 0;
  for (i = 0; i < 3; i++)
    if (dcvars->translation == translationtables + i * 256)
      xlat = CR_LIMIT + i;
  for (i = 0; i < CR_LIMIT; i++)
    if (dcvars->translation == colrngs[i])
      xlat = i;
  I_DoomDevDrawGenericColumn(dcvars, xlat, 0);
}

static inline int I_DoomDevDrawFuzzBatch(pdraw_column_vars_s dcvars) {
  int num_cols = 0;
  if (!fdoomdev_fuzz_header)
    return 0;
  if (fdoomdev_cmd_buf_pos + 2 > FDOOMDEV_CMD_BUF_SIZE)
    return 0;
  num_cols = FHARDDOOM_USER_DRAW_FUZZ_HEADER_EXTR_NUM_COLS(fdoomdev_fuzz_header[0]);
  if (num_cols == 0xffff)
    return 0;
  if (fdoomdev_batch_screen != drawvars.screen)
    return 0;
  if (fdoomdev_batch_cmap_fd != colormap_doomdev_fd[boom_cm])
    return 0;
  num_cols++;
  fdoomdev_fuzz_header[0] = FHARDDOOM_USER_DRAW_FUZZ_HEADER(fdoomdev_batch_screen_slot, num_cols);
  return 1;
}

void I_DoomDevDrawFuzzColumn(pdraw_column_vars_s dcvars)
{
  static int fuzzpos = 0;
  if (dcvars->yl > dcvars->yh)
    return;
  if (!I_DoomDevDrawFuzzBatch(dcvars)) {
    int fds[2] = {
      screens[drawvars.screen].doomdev_fd,
      colormap_doomdev_fd[boom_cm],
    };
    int slots[2];
    I_DoomDevReserve(FDOOMDEV_CMD_BATCH_RESERVE, 2, fds, slots);
    fdoomdev_fuzz_header = I_DoomDevCurCmd();
    fdoomdev_batch_screen = drawvars.screen;
    fdoomdev_batch_screen_slot = slots[0];
    fdoomdev_batch_cmap_fd = colormap_doomdev_fd[boom_cm];
    fdoomdev_batch_cmap_slot = slots[1];
    I_DoomDevSend(FHARDDOOM_USER_DRAW_FUZZ_HEADER(slots[0], 1));
    I_DoomDevSend(FHARDDOOM_USER_DRAW_FUZZ_W1(0, fdoomdev_surf_height-1));
    I_DoomDevSend(FHARDDOOM_USER_DRAW_FUZZ_W2(slots[1], colormap_doomdev_idx[boom_cm] + 6));
  }
  I_DoomDevSend(FHARDDOOM_USER_DRAW_FUZZ_WR0(dcvars->x + drawvars.xoff, fuzzpos));
  I_DoomDevSend(FHARDDOOM_USER_DRAW_FUZZ_WR1(dcvars->yl + drawvars.yoff, dcvars->yh + drawvars.yoff));
  fuzzpos += dcvars->yh - dcvars->yl + 1;
  fuzzpos %= 50;
}

static inline int I_DoomDevDrawSpanBatch(draw_span_vars_t *dsvars) {
  int y = dsvars->y + drawvars.yoff;
  int y0, y1;
  if (!fdoomdev_spans_header)
    return 0;
  if (fdoomdev_cmd_buf_pos + 6 > FDOOMDEV_CMD_BUF_SIZE)
    return 0;
  if (fdoomdev_batch_screen != drawvars.screen)
    return 0;
  if (fdoomdev_spans_flat_fd != dsvars->flat_doomdev_fd)
    return 0;
  if (fdoomdev_batch_cmap_fd != colormap_doomdev_fd[boom_cm])
    return 0;
  y0 = FHARDDOOM_USER_DRAW_SPANS_W2_EXTR_Y0(fdoomdev_spans_header[1]);
  y1 = FHARDDOOM_USER_DRAW_SPANS_W2_EXTR_Y1(fdoomdev_spans_header[1]);
  if (y0 <= y1 && y == y1 + 1) {
    fdoomdev_spans_header[1] = FHARDDOOM_USER_DRAW_SPANS_W2(y0, y);
    return 1;
  } else if (y0 >= y1 && y == y1 - 1) {
    fdoomdev_spans_header[1] = FHARDDOOM_USER_DRAW_SPANS_W2(y0, y);
    return 1;
  } else {
    return 0;
  }
}

void I_DoomDevDrawSpan(draw_span_vars_t *dsvars)
{
  int y = dsvars->y + drawvars.yoff;
  int cmap_idx = (dsvars->colormap - colormaps[boom_cm]) >> 8;
  int slot_cmap;
  if (!I_DoomDevDrawSpanBatch(dsvars)) {
    int fds[3] = {
      screens[drawvars.screen].doomdev_fd,
      dsvars->flat_doomdev_fd,
      colormap_doomdev_fd[boom_cm],
    };
    int slots[3];
    I_DoomDevReserve(FDOOMDEV_CMD_BATCH_RESERVE, 3, fds, slots);
    fdoomdev_spans_header = I_DoomDevCurCmd();
    fdoomdev_batch_screen = drawvars.screen;
    fdoomdev_batch_screen_slot = slots[0];
    fdoomdev_spans_flat_fd = dsvars->flat_doomdev_fd;
    fdoomdev_batch_cmap_fd = colormap_doomdev_fd[boom_cm];
    fdoomdev_batch_cmap_slot = slots[2];
    I_DoomDevSend(FHARDDOOM_USER_DRAW_SPANS_HEADER(slots[0], 0, 1, 0, slots[1], 6, 6));
    I_DoomDevSend(FHARDDOOM_USER_DRAW_SPANS_W2(y, y));
  }
  I_DoomDevSend(FHARDDOOM_USER_DRAW_SPANS_WR0(dsvars->x1 + drawvars.xoff, dsvars->x2 + drawvars.xoff));
  I_DoomDevSend(dsvars->xfrac & 0x3fffff);
  I_DoomDevSend((dsvars->yfrac & 0x3fffff) | dsvars->flat_doomdev_idx << 22);
  I_DoomDevSend(dsvars->xstep);
  I_DoomDevSend(dsvars->ystep);
  I_DoomDevSend(FHARDDOOM_USER_DRAW_SPANS_WR5(fdoomdev_batch_cmap_slot, colormap_doomdev_idx[boom_cm] + cmap_idx));
}


void I_DoomDevMeltColumns(int destscrn, int srcascrn, int srcbscrn, int *yoff) {
  int fds[3] = {
    screens[destscrn].doomdev_fd,
    screens[srcascrn].doomdev_fd,
    screens[srcbscrn].doomdev_fd,
  };
  int slots[3];
  I_DoomDevReserve(3 + SCREENWIDTH, 3, fds, slots);
  I_DoomDevSend(FHARDDOOM_USER_WIPE_HEADER(slots[0], slots[1], slots[2]));
  I_DoomDevSend(FHARDDOOM_USER_WIPE_W1(0, 0));
  I_DoomDevSend(FHARDDOOM_USER_WIPE_W2(SCREENWIDTH, SCREENHEIGHT));
  for (int i = 0; i < SCREENWIDTH; i++)
    if (yoff[i] < 0)
      I_DoomDevSend(0);
    else
      I_DoomDevSend(yoff[i]);
}
