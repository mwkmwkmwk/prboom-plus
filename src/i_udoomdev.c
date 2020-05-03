#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include "v_video.h"
#include "i_udoomdev.h"
#include "udoomdev.h"
#include "uharddoom.h"
#include "lprintf.h"
#include "r_draw.h"
#include "r_main.h"
#include "w_wad.h"

#define UDOOMDEV_NUM_CMD_BUFS		16
#define UDOOMDEV_CMD_BUF_SIZE		0x10000
#define UDOOMDEV_CMD_BATCH_RESERVE	0x1000

int udoom_fd = -1;
uint32_t *udoomdev_cmd_ptr[UDOOMDEV_NUM_CMD_BUFS];
uint32_t udoomdev_cmd_addr[UDOOMDEV_NUM_CMD_BUFS];
int udoomdev_cmd_buf_idx;
int udoomdev_cmd_buf_pos;
uint32_t udoomdev_translations_addr;
uint32_t udoomdev_transmap_addr;

int udoomdev_surf_width;
int udoomdev_surf_height;

uint32_t *udoomdev_cur_header;

void I_DoomDevFlushCmd(void)
{
  struct udoomdev_ioctl_run run = {udoomdev_cmd_addr[udoomdev_cmd_buf_idx], udoomdev_cmd_buf_pos * 4};
  struct udoomdev_ioctl_wait wait = {UDOOMDEV_NUM_CMD_BUFS - 1};
  if (!udoomdev_cmd_buf_pos)
    return;
  if (ioctl(udoom_fd, UDOOMDEV_IOCTL_RUN, &run))
    I_Error("udoomdev run fail");
  if (ioctl(udoom_fd, UDOOMDEV_IOCTL_WAIT, &wait))
    I_Error("udoomdev wait for space fail");
  udoomdev_cmd_buf_idx++;
  udoomdev_cmd_buf_idx %= UDOOMDEV_NUM_CMD_BUFS;
  udoomdev_cmd_buf_pos = 0;
  udoomdev_cur_header = 0;
}

static inline void I_DoomDevReserve(int num) {
  udoomdev_cur_header = 0;
  if (udoomdev_cmd_buf_pos + num > UDOOMDEV_CMD_BUF_SIZE)
    I_DoomDevFlushCmd();
}

static inline uint32_t *I_DoomDevCurCmd() {
  return udoomdev_cmd_ptr[udoomdev_cmd_buf_idx] + udoomdev_cmd_buf_pos;
}

static inline void I_DoomDevSend(uint32_t word) {
  udoomdev_cmd_ptr[udoomdev_cmd_buf_idx][udoomdev_cmd_buf_pos++] = word;
}

void I_DoomDevSync(void) {
  struct udoomdev_ioctl_wait wait = {0};
  I_DoomDevFlushCmd();
  if (udoom_fd != -1 && ioctl(udoom_fd, UDOOMDEV_IOCTL_WAIT, &wait))
    I_Error("udoomdev sync fail");
}

void I_DoomDevRead(int idx)
{
  int y;
  I_DoomDevSync();
  for (y = 0; y < screens[idx].height; y++) {
    memcpy(screens[idx].data + screens[idx].byte_pitch * y, screens[idx].doomdev_ptr + udoomdev_surf_width * y, screens[idx].width);
  }
}

void *I_DoomDevCreateBuf(uint32_t size, uint32_t *dev_addr, int rdonly) {
  struct udoomdev_ioctl_create_buffer cb = {size};
  struct udoomdev_ioctl_map_buffer mb = {0, rdonly};
  long addr;
  void *mdata;
  int fd = ioctl(udoom_fd, UDOOMDEV_IOCTL_CREATE_BUFFER, &cb);
  if (fd < 0)
    I_Error("udoomdev create buffer fail");
  mb.buf_fd = fd;
  addr = ioctl(udoom_fd, UDOOMDEV_IOCTL_MAP_BUFFER, &mb);
  if (addr == -1)
    I_Error("udoomdev map buffer fail");
  *dev_addr = addr;
  mdata = mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (mdata == MAP_FAILED)
    I_Error("udoomdev mmap fail");
  close(fd);
  return mdata;
}

uint32_t I_DoomDevUploadBuf(const void *data, uint32_t size) {
  uint32_t res;
  void *ptr = I_DoomDevCreateBuf(size, &res, 1);
  memcpy(ptr, data, size);
  munmap(ptr, size);
  return res;
}

void I_DoomDevAllocScreens(void)
{
  int i;
  int width = screens[0].width;
  byte *trans = malloc((CR_LIMIT + 3) * 256);
  for (i = 0; i < CR_LIMIT; i++)
    memcpy(trans + i * 256, colrngs[i], 256);
  memcpy(trans + CR_LIMIT * 256, translationtables, 256 * 3);
  if (width & 0x3f) {
    width |= 0x3f;
    width++;
  }
  udoomdev_surf_width = width;
  udoomdev_surf_height = screens[0].height;
  udoom_fd = open("/dev/udoom0", O_RDWR);
  if (udoom_fd < 0)
    I_Error("doomdev2 open fail");

  for (i = 0; i < UDOOMDEV_NUM_CMD_BUFS; i++)
    udoomdev_cmd_ptr[i] = I_DoomDevCreateBuf(UDOOMDEV_CMD_BUF_SIZE * 4, &udoomdev_cmd_addr[i], 1);
  for (i = 0; i < NUM_SCREENS; i++)
    screens[i].doomdev_ptr = I_DoomDevCreateBuf(udoomdev_surf_width * udoomdev_surf_height, &screens[i].doomdev_addr, 0);
  udoomdev_translations_addr = I_DoomDevUploadBuf(trans, (CR_LIMIT + 3) * 0x100);
  udoomdev_transmap_addr = I_DoomDevUploadBuf(main_tranmap, 0x10000);
  free(trans);
  I_DoomDevUploadColormap(boom_cm);
  udoomdev_cmd_buf_idx = 0;
  udoomdev_cmd_buf_pos = 0;
  udoomdev_cur_header = 0;
}

void I_DoomDevFreeScreens(void)
{
  int i;
  I_DoomDevSync();
  R_FlushHardPatches();
  for (i = 0; i < numlumps; i++)
    lumpinfo[i].flat_doomdev_addr = -1;
  for (i = 0; i < numcolormaps; i++)
    colormap_doomdev_addr[i] = -1;
  for (i = 0; i < UDOOMDEV_NUM_CMD_BUFS; i++) {
    if (udoomdev_cmd_ptr[i]) {
      munmap(udoomdev_cmd_ptr[i], UDOOMDEV_CMD_BUF_SIZE * 4);
      udoomdev_cmd_ptr[i] = 0;
    }
  }
  for (i = 0; i < NUM_SCREENS; i++) {
    if (screens[i].doomdev_ptr) {
      munmap(screens[i].doomdev_ptr, udoomdev_surf_width * udoomdev_surf_height);
      screens[i].doomdev_ptr = 0;
    }
    screens[i].doomdev_addr = -1;
  }
  udoomdev_translations_addr = -1;
  udoomdev_transmap_addr = -1;
  if (udoom_fd != -1)
    close(udoom_fd);
  udoom_fd = -1;
}

void I_DoomDevFreeBuf(uint32_t addr) {
  struct udoomdev_ioctl_unmap_buffer ub = {addr};
  if (ioctl(udoom_fd, UDOOMDEV_IOCTL_UNMAP_BUFFER, &ub))
    I_Error("udoomdev unmap buffer fail");
}

void I_DoomDevUploadPatch(rpatch_t *patch)
{
  if (patch->doomdev_addr != -1)
    return;
  patch->doomdev_addr = I_DoomDevUploadBuf(patch->pixels, patch->height * patch->width);
}

void I_DoomDevClosePatch(rpatch_t *patch)
{
  I_DoomDevSync();
  if (patch->doomdev_addr != -1)
    I_DoomDevFreeBuf(patch->doomdev_addr);
  patch->doomdev_addr = -1;
}

void I_DoomDevUploadFlat(int lump, const byte *data)
{
  if (lumpinfo[lump].flat_doomdev_addr != -1)
    return;
  lumpinfo[lump].flat_doomdev_addr = I_DoomDevUploadBuf(data, 0x1000);
}

void I_DoomDevUploadColormap(int idx)
{
  if (colormap_doomdev_addr[idx] != -1)
    return;
  colormap_doomdev_addr[idx] = I_DoomDevUploadBuf(colormaps[idx], 34 * 0x100);
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
  I_DoomDevReserve(5);
  I_DoomDevSend(UHARDDOOM_USER_FILL_RECT_HEADER(colour));
  I_DoomDevSend(screens[scrn].doomdev_addr);
  I_DoomDevSend(udoomdev_surf_width);
  I_DoomDevSend(UHARDDOOM_USER_FILL_RECT_W3(x, y));
  I_DoomDevSend(UHARDDOOM_USER_FILL_RECT_W4(width, height));
}

void I_DoomDevCopyRect(int srcscrn, int destscrn, int x, int y, int width, int height, enum patch_translation_e flags)
{
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
  I_DoomDevReserve(9);
  I_DoomDevSend(UHARDDOOM_USER_BLIT_HEADER(0x10, 0x10));
  I_DoomDevSend(screens[destscrn].doomdev_addr);
  I_DoomDevSend(udoomdev_surf_width);
  I_DoomDevSend(UHARDDOOM_USER_BLIT_W3(x, y));
  I_DoomDevSend(UHARDDOOM_USER_BLIT_W4(width, height));
  I_DoomDevSend(screens[srcscrn].doomdev_addr);
  I_DoomDevSend(udoomdev_surf_width);
  I_DoomDevSend(UHARDDOOM_USER_BLIT_W7(x, y));
  I_DoomDevSend(UHARDDOOM_USER_BLIT_W8(width, height));
}

void I_DoomDevDrawLine(fline_t* fl, int color)
{
  I_DoomDevReserve(5);
  I_DoomDevSend(UHARDDOOM_USER_DRAW_LINE_HEADER(color));
  I_DoomDevSend(screens[0].doomdev_addr);
  I_DoomDevSend(udoomdev_surf_width);
  I_DoomDevSend(UHARDDOOM_USER_DRAW_LINE_W3(fl->a.x, fl->a.y));
  I_DoomDevSend(UHARDDOOM_USER_DRAW_LINE_W4(fl->b.x, fl->b.y));
}

void I_DoomDevFillFlat(int lump, int scrn, int x, int y, int width, int height, enum patch_translation_e flags)
{
  lump += firstflat;
  if (lumpinfo[lump].flat_doomdev_addr == -1)
  {
    const byte *data = W_CacheLumpNum(lump);
    I_DoomDevUploadFlat(lump, data);
    W_UnlockLumpNum(lump);
  }
  I_DoomDevReserve(9);
  I_DoomDevSend(UHARDDOOM_USER_BLIT_HEADER(6, 6));
  I_DoomDevSend(screens[scrn].doomdev_addr);
  I_DoomDevSend(udoomdev_surf_width);
  I_DoomDevSend(UHARDDOOM_USER_BLIT_W3(0, 0));
  I_DoomDevSend(UHARDDOOM_USER_BLIT_W4(udoomdev_surf_width, udoomdev_surf_height));
  I_DoomDevSend(lumpinfo[lump].flat_doomdev_addr);
  I_DoomDevSend(0x40);
  I_DoomDevSend(UHARDDOOM_USER_BLIT_W7(0, 0));
  I_DoomDevSend(UHARDDOOM_USER_BLIT_W8(udoomdev_surf_width, udoomdev_surf_height));
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

static inline int I_DoomDevDrawGenericColumnBatch(pdraw_column_vars_s dcvars, int translation, int transparent) {
  int num_cols = 0;
  if (!udoomdev_cur_header)
    return 0;
  if (udoomdev_cmd_buf_pos + 6 > UDOOMDEV_CMD_BUF_SIZE)
    return 0;
  if (UHARDDOOM_USER_CMD_HEADER_EXTR_TYPE(udoomdev_cur_header[0]) != UHARDDOOM_USER_CMD_TYPE_DRAW_COLUMNS)
    return 0;
  if (UHARDDOOM_USER_DRAW_COLUMNS_HEADER_EXTR_CMAP_A_EN(udoomdev_cur_header[0]) != (translation != -1))
    return 0;
  if (UHARDDOOM_USER_DRAW_COLUMNS_HEADER_EXTR_CMAP_B_EN(udoomdev_cur_header[0]) != 1)
    return 0;
  if (UHARDDOOM_USER_DRAW_COLUMNS_HEADER_EXTR_TRANS_EN(udoomdev_cur_header[0]) != transparent)
    return 0;
  num_cols = UHARDDOOM_USER_DRAW_COLUMNS_HEADER_EXTR_NUM_COLS(udoomdev_cur_header[0]);
  if (num_cols == 0xffff)
    return 0;
  if (udoomdev_cur_header[1] != screens[drawvars.screen].doomdev_addr)
    return 0;
  if (translation != -1) {
    if (udoomdev_cur_header[3] != udoomdev_translations_addr + translation * 0x100)
      return 0;
  }
  num_cols++;
  udoomdev_cur_header[0] = UHARDDOOM_USER_DRAW_COLUMNS_HEADER(translation != -1, 1, transparent, num_cols);
  return 1;
}

void I_DoomDevDrawGenericColumn(pdraw_column_vars_s dcvars, int translation, int transparent) {
  int ulog = 0;
  if (dcvars->yl > dcvars->yh)
    return;
  if (!I_DoomDevDrawGenericColumnBatch(dcvars, translation, transparent)) {
    I_DoomDevReserve(UDOOMDEV_CMD_BATCH_RESERVE);
    udoomdev_cur_header = I_DoomDevCurCmd();
    I_DoomDevSend(UHARDDOOM_USER_DRAW_COLUMNS_HEADER(translation != -1, 1, transparent, 1));
    I_DoomDevSend(screens[drawvars.screen].doomdev_addr);
    I_DoomDevSend(udoomdev_surf_width);
    if (translation != -1)
      I_DoomDevSend(udoomdev_translations_addr + translation * 0x100);
    if (transparent)
      I_DoomDevSend(udoomdev_transmap_addr);
  }
  if (dcvars->texheight)
    while ((1 << ulog) < dcvars->texheight)
      ulog++;
  else
    ulog = 0x10;
  I_DoomDevSend(UHARDDOOM_USER_DRAW_COLUMNS_WR0(dcvars->x + drawvars.xoff, ulog));
  I_DoomDevSend(UHARDDOOM_USER_DRAW_COLUMNS_WR1(dcvars->yl + drawvars.yoff, dcvars->yh + drawvars.yoff));
  I_DoomDevSend(dcvars->texture_doomdev_addr + (dcvars->source - dcvars->texture_base));
  I_DoomDevSend(I_DoomDevComputeColumnStart(dcvars));
  I_DoomDevSend(dcvars->iscale);
  I_DoomDevSend(colormap_doomdev_addr[boom_cm] + dcvars->colormap - colormaps[boom_cm]);
}

void I_DoomDevDrawColumn(pdraw_column_vars_s dcvars)
{
  I_DoomDevDrawGenericColumn(dcvars, -1, 0);
}

void I_DoomDevDrawTransparentColumn(pdraw_column_vars_s dcvars)
{
  I_DoomDevDrawGenericColumn(dcvars, -1, 1);
}

static inline int I_DoomDevDrawFuzzBatch(pdraw_column_vars_s dcvars) {
  int num_cols = 0;
  if (!udoomdev_cur_header)
    return 0;
  if (udoomdev_cmd_buf_pos + 2 > UDOOMDEV_CMD_BUF_SIZE)
    return 0;
  if (UHARDDOOM_USER_CMD_HEADER_EXTR_TYPE(udoomdev_cur_header[0]) != UHARDDOOM_USER_CMD_TYPE_DRAW_FUZZ)
    return 0;
  num_cols = UHARDDOOM_USER_DRAW_FUZZ_HEADER_EXTR_NUM_COLS(udoomdev_cur_header[0]);
  if (num_cols == 0xffff)
    return 0;
  if (udoomdev_cur_header[1] != screens[drawvars.screen].doomdev_addr)
    return 0;
  if (udoomdev_cur_header[4] != colormap_doomdev_addr[boom_cm] + 0x600)
    return 0;
  num_cols++;
  udoomdev_cur_header[0] = UHARDDOOM_USER_DRAW_FUZZ_HEADER(num_cols);
  return 1;
}

void I_DoomDevDrawFuzzColumn(pdraw_column_vars_s dcvars)
{
  static int fuzzpos = 0;
  if (dcvars->yl > dcvars->yh)
    return;
  if (!I_DoomDevDrawFuzzBatch(dcvars)) {
    I_DoomDevReserve(UDOOMDEV_CMD_BATCH_RESERVE);
    udoomdev_cur_header = I_DoomDevCurCmd();
    I_DoomDevSend(UHARDDOOM_USER_DRAW_FUZZ_HEADER(1));
    I_DoomDevSend(screens[drawvars.screen].doomdev_addr);
    I_DoomDevSend(udoomdev_surf_width);
    I_DoomDevSend(UHARDDOOM_USER_DRAW_FUZZ_W3(0, udoomdev_surf_height-1));
    I_DoomDevSend(colormap_doomdev_addr[boom_cm] + 0x600);
  }
  I_DoomDevSend(UHARDDOOM_USER_DRAW_FUZZ_WR0(dcvars->x + drawvars.xoff, fuzzpos));
  I_DoomDevSend(UHARDDOOM_USER_DRAW_FUZZ_WR1(dcvars->yl + drawvars.yoff, dcvars->yh + drawvars.yoff));
  fuzzpos += dcvars->yh - dcvars->yl + 1;
  fuzzpos %= 50;
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

static inline int I_DoomDevDrawSpanBatch(draw_span_vars_t *dsvars) {
  int y = dsvars->y + drawvars.yoff;
  int y0, y1;
  if (!udoomdev_cur_header)
    return 0;
  if (udoomdev_cmd_buf_pos + 6 > UDOOMDEV_CMD_BUF_SIZE)
    return 0;
  if (udoomdev_cur_header[0] != UHARDDOOM_USER_DRAW_SPANS_HEADER(1, 0, 6, 6))
    return 0;
  if (udoomdev_cur_header[1] != screens[drawvars.screen].doomdev_addr)
    return 0;
  if (udoomdev_cur_header[4] != dsvars->flat_doomdev_addr)
    return 0;
  y0 = UHARDDOOM_USER_DRAW_SPANS_W3_EXTR_Y0(udoomdev_cur_header[3]);
  y1 = UHARDDOOM_USER_DRAW_SPANS_W3_EXTR_Y1(udoomdev_cur_header[3]);
  if (y0 <= y1 && y == y1 + 1) {
    udoomdev_cur_header[3] = UHARDDOOM_USER_DRAW_SPANS_W3(y0, y);
    return 1;
  } else if (y0 >= y1 && y == y1 - 1) {
    udoomdev_cur_header[3] = UHARDDOOM_USER_DRAW_SPANS_W3(y0, y);
    return 1;
  } else {
    return 0;
  }
}

void I_DoomDevDrawSpan(draw_span_vars_t *dsvars)
{
  int y = dsvars->y + drawvars.yoff;
  int cmap_idx = (dsvars->colormap - colormaps[boom_cm]) >> 8;
  if (!I_DoomDevDrawSpanBatch(dsvars)) {
    I_DoomDevReserve(UDOOMDEV_CMD_BATCH_RESERVE);
    udoomdev_cur_header = I_DoomDevCurCmd();
    I_DoomDevSend(UHARDDOOM_USER_DRAW_SPANS_HEADER(1, 0, 6, 6));
    I_DoomDevSend(screens[drawvars.screen].doomdev_addr);
    I_DoomDevSend(udoomdev_surf_width);
    I_DoomDevSend(UHARDDOOM_USER_DRAW_SPANS_W3(y, y));
    I_DoomDevSend(dsvars->flat_doomdev_addr);
    I_DoomDevSend(0x40);
  }
  I_DoomDevSend(UHARDDOOM_USER_DRAW_SPANS_WR0(dsvars->x1 + drawvars.xoff, dsvars->x2 + drawvars.xoff));
  I_DoomDevSend(dsvars->xfrac);
  I_DoomDevSend(dsvars->yfrac);
  I_DoomDevSend(dsvars->xstep);
  I_DoomDevSend(dsvars->ystep);
  I_DoomDevSend(colormap_doomdev_addr[boom_cm] + cmap_idx * 0x100);
}


void I_DoomDevMeltColumns(int destscrn, int srcascrn, int srcbscrn, int *yoff) {
  I_DoomDevReserve(9 + SCREENWIDTH);
  I_DoomDevSend(UHARDDOOM_USER_WIPE_HEADER);
  I_DoomDevSend(screens[destscrn].doomdev_addr);
  I_DoomDevSend(udoomdev_surf_width);
  I_DoomDevSend(UHARDDOOM_USER_WIPE_W3(0, 0));
  I_DoomDevSend(UHARDDOOM_USER_WIPE_W4(SCREENWIDTH, SCREENHEIGHT));
  I_DoomDevSend(screens[srcascrn].doomdev_addr);
  I_DoomDevSend(udoomdev_surf_width);
  I_DoomDevSend(screens[srcbscrn].doomdev_addr);
  I_DoomDevSend(udoomdev_surf_width);
  for (int i = 0; i < SCREENWIDTH; i++)
    if (yoff[i] < 0)
      I_DoomDevSend(0);
    else
      I_DoomDevSend(yoff[i]);
}
