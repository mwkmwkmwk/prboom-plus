#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include "v_video.h"
#include "i_doomdev.h"
#include "doomdev.h"
#include "lprintf.h"
#include "r_draw.h"
#include "r_main.h"
#include "w_wad.h"

int doom_fd = -1;
int doomdev_surf_width;
int doomdev_surf_height;
int translations_fd = -1;

enum batch_mode {
  BATCH_NONE,
  BATCH_FILL,
  BATCH_COPY,
  BATCH_LINES,
  BATCH_COLUMNS,
  BATCH_SPANS,
};

#define MAX_BATCH_SIZE 0x400

int batch_scrn_dst, batch_scrn_src;
struct doomdev_fill_rect batch_fill[MAX_BATCH_SIZE];
struct doomdev_copy_rect batch_copy[MAX_BATCH_SIZE];
struct doomdev_line batch_lines[MAX_BATCH_SIZE];
struct doomdev_column batch_columns[MAX_BATCH_SIZE];
struct doomdev_column batch_columns_bot[MAX_BATCH_SIZE];
struct doomdev_span batch_spans[MAX_BATCH_SIZE];

enum batch_mode batch_mode = BATCH_NONE;
int batch_size = 0;
int batch_size_bot = 0;
int batch_colormap_fd;

const byte *batch_texture;
const byte *batch_texture_bot;
int batch_texture_fd;
int batch_texture_fd_bot;
int batch_draw_flags;
int batch_translation;

void I_DoomDevFlushBatch(void)
{
  int done = 0;
  if (batch_mode == BATCH_NONE)
    return;
  while (done < batch_size) {
    int res = -1;
    switch (batch_mode) {
    case BATCH_FILL:
    {
      struct doomdev_surf_ioctl_fill_rects param = {
        .rects_ptr = (uint64_t)(batch_fill + done),
        .rects_num = batch_size - done,
      };
      res = ioctl(screens[batch_scrn_dst].doomdev_fd, DOOMDEV_SURF_IOCTL_FILL_RECTS, &param);
      break;
    }
    case BATCH_COPY:
    {
      struct doomdev_surf_ioctl_copy_rects param = {
	.surf_src_fd = screens[batch_scrn_src].doomdev_fd,
        .rects_ptr = (uint64_t)(batch_copy + done),
        .rects_num = batch_size - done,
      };
      res = ioctl(screens[batch_scrn_dst].doomdev_fd, DOOMDEV_SURF_IOCTL_COPY_RECTS, &param);
      break;
    }
    case BATCH_LINES:
    {
      struct doomdev_surf_ioctl_draw_lines param = {
        .lines_ptr = (uint64_t)(batch_lines + done),
        .lines_num = batch_size - done,
      };
      res = ioctl(screens[batch_scrn_dst].doomdev_fd, DOOMDEV_SURF_IOCTL_DRAW_LINES, &param);
      break;
    }
    case BATCH_COLUMNS:
    {
      struct doomdev_surf_ioctl_draw_columns param = {
	.texture_fd = batch_texture_fd,
	.colormaps_fd = batch_colormap_fd,
	.draw_flags = batch_draw_flags,
	.translations_fd = translations_fd,
	.translation_idx = batch_translation,
        .columns_ptr = (uint64_t)(batch_columns + done),
        .columns_num = batch_size - done,
      };
      res = ioctl(screens[batch_scrn_dst].doomdev_fd, DOOMDEV_SURF_IOCTL_DRAW_COLUMNS, &param);
      break;
    }
    case BATCH_SPANS:
    {
      struct doomdev_surf_ioctl_draw_spans param = {
        .flat_fd = batch_texture_fd,
	.colormaps_fd = batch_colormap_fd,
	.draw_flags = DOOMDEV_DRAW_FLAGS_COLORMAP,
        .spans_ptr = (uint64_t)(batch_spans + done),
        .spans_num = batch_size - done,
      };
      res = ioctl(screens[batch_scrn_dst].doomdev_fd, DOOMDEV_SURF_IOCTL_DRAW_SPANS, &param);
      break;
    }
    }
    if (res <= 0)
      I_Error("doomdev render fail");
    done += res;
  }
  done = 0;
  while (done < batch_size_bot) {
    struct doomdev_surf_ioctl_draw_columns param = {
      .texture_fd = batch_texture_fd_bot,
      .colormaps_fd = batch_colormap_fd,
      .draw_flags = batch_draw_flags,
      .translations_fd = translations_fd,
      .translation_idx = batch_translation,
      .columns_ptr = (uint64_t)(batch_columns_bot + done),
      .columns_num = batch_size_bot - done,
    };
    int res = ioctl(screens[batch_scrn_dst].doomdev_fd, DOOMDEV_SURF_IOCTL_DRAW_COLUMNS, &param);
    if (res <= 0)
      I_Error("doomdev render fail");
    done += res;
  }
  batch_mode = BATCH_NONE;
  batch_size = 0;
  batch_size_bot = 0;
  batch_texture = 0;
  batch_texture_bot = 0;
}

void I_DoomDevRead(int idx)
{
  int res;
  int y;
  I_DoomDevFlushBatch();
  for (y = 0; y < screens[idx].height; y++) {
    off_t offset = doomdev_surf_width * y;
    byte *ptr = screens[idx].data + screens[idx].byte_pitch * y;
    int size = screens[idx].width;
    int copied = 0;
    while (copied < size) {
      res = pread(screens[idx].doomdev_fd, ptr + copied, size - copied, offset + copied);
      if (res <= 0) {
        I_Error("doomdev read fail");
      }
      copied += res;
    }
  }
}

void I_DoomDevAllocScreens(void)
{
  int i;
  int width = screens[0].width;
  byte *trans = malloc((CR_LIMIT + 3) * 256);
  struct doomdev_ioctl_create_colormaps xlat_param = {
    .data_ptr = (uint64_t)(uintptr_t)trans,
    .num = CR_LIMIT + 3,
  };
  for (i = 0; i < CR_LIMIT; i++)
    memcpy(trans + i * 256, colrngs[i], 256);
  memcpy(trans + CR_LIMIT * 256, translationtables, 256 * 3);
  if (width & 0x3f) {
    width |= 0x3f;
    width++;
  }
  doomdev_surf_width = width;
  doomdev_surf_height = screens[0].height;
  doom_fd = open("/dev/doom0", O_RDWR);
  if (doom_fd < 0) {
    I_Error("doomdev open fail");
  }
  for (i = 0; i < NUM_SCREENS; i++) {
    struct doomdev_ioctl_create_surface param = {
      .width = doomdev_surf_width,
      .height = doomdev_surf_height,
    };
    int fd = ioctl(doom_fd, DOOMDEV_IOCTL_CREATE_SURFACE, &param);
    if (fd < 0)
      I_Error("doomdev create_surface fail");
    screens[i].doomdev_fd = fd;
  }
  translations_fd = ioctl(doom_fd, DOOMDEV_IOCTL_CREATE_COLORMAPS, &xlat_param);
  if (translations_fd < 0)
    I_Error("doomdev create_colormap translation fail");
  free(trans);
  I_DoomDevUploadColormap(boom_cm);
}

void I_DoomDevFreeScreens(void)
{
  int i;
  I_DoomDevFlushBatch();
  R_FlushHardPatches();
  for (i = 0; i < numlumps; i++)
    if (lumpinfo[i].flat_fd != -1) {
      close(lumpinfo[i].flat_fd);
      lumpinfo[i].flat_fd = -1;
    }
  for (i = 0; i < numcolormaps; i++) {
    if (colormap_fd[i] != -1)
      close(colormap_fd[i]);
    colormap_fd[i] = -1;
  }
  for (i = 0; i < NUM_SCREENS; i++) {
    if (screens[i].doomdev_fd != -1)
      close(screens[i].doomdev_fd);
    screens[i].doomdev_fd = -1;
  }
  if (translations_fd != -1)
    close(translations_fd);
  translations_fd = -1;
  if (doom_fd != -1)
    close(doom_fd);
  doom_fd = -1;
}

void I_DoomDevUploadPatch(rpatch_t *patch)
{
  struct doomdev_ioctl_create_texture param = {
    .data_ptr = (uint64_t)(uintptr_t)patch->pixels,
    .size = patch->height * patch->width,
    .height = patch->height,
  };
  int fd;
  if (patch->doomdev_fd != -1)
    return;
  fd = ioctl(doom_fd, DOOMDEV_IOCTL_CREATE_TEXTURE, &param);
  if (fd < 0)
    I_Error("doomdev create_texture patch fail");
  patch->doomdev_fd = fd;
}

void I_DoomDevClosePatch(rpatch_t *patch)
{
  if (patch->doomdev_fd != -1)
    close(patch->doomdev_fd);
  patch->doomdev_fd = -1;
}

void I_DoomDevUploadFlat(int lump, const byte *data)
{
  struct doomdev_ioctl_create_flat param = {
    .data_ptr = (uint64_t)(uintptr_t)data,
  };
  int fd;
  if (lumpinfo[lump].flat_fd != -1)
    return;
  fd = ioctl(doom_fd, DOOMDEV_IOCTL_CREATE_FLAT, &param);
  if (fd < 0)
    I_Error("doomdev create_flat fail");
  lumpinfo[lump].flat_fd = fd;
}

void I_DoomDevUploadColormap(int idx)
{
  struct doomdev_ioctl_create_colormaps param = {
    .data_ptr = (uint64_t)(uintptr_t)colormaps[idx],
    .num = 34,
  };
  int fd;
  if (colormap_fd[idx] != -1)
    return;
  fd = ioctl(doom_fd, DOOMDEV_IOCTL_CREATE_COLORMAPS, &param);
  if (fd < 0)
    I_Error("doomdev create_colormaps fail");
  colormap_fd[idx] = fd;
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
  if (batch_mode != BATCH_FILL || batch_size >= MAX_BATCH_SIZE || batch_scrn_dst != scrn)
    I_DoomDevFlushBatch();
  batch_mode = BATCH_FILL;
  batch_scrn_dst = scrn;
  batch_fill[batch_size].pos_dst_x = x;
  batch_fill[batch_size].pos_dst_y = y;
  batch_fill[batch_size].width = width;
  batch_fill[batch_size].height = height;
  batch_fill[batch_size].color = colour;
  batch_size++;
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
  if (batch_mode != BATCH_COPY || batch_size >= MAX_BATCH_SIZE || batch_scrn_dst != destscrn || batch_scrn_src != srcscrn)
    I_DoomDevFlushBatch();
  batch_mode = BATCH_COPY;
  batch_scrn_dst = destscrn;
  batch_scrn_src = srcscrn;
  batch_copy[batch_size].pos_dst_x = x;
  batch_copy[batch_size].pos_dst_y = y;
  batch_copy[batch_size].pos_src_x = x;
  batch_copy[batch_size].pos_src_y = y;
  batch_copy[batch_size].width = width;
  batch_copy[batch_size].height = height;
  batch_size++;
}

void I_DoomDevDrawLine(fline_t* fl, int color)
{
  if (batch_mode != BATCH_LINES || batch_size >= MAX_BATCH_SIZE || batch_scrn_dst != 0)
    I_DoomDevFlushBatch();
  batch_mode = BATCH_LINES;
  batch_scrn_dst = 0;
  batch_lines[batch_size].pos_a_x = fl->a.x;
  batch_lines[batch_size].pos_a_y = fl->a.y;
  batch_lines[batch_size].pos_b_x = fl->b.x;
  batch_lines[batch_size].pos_b_y = fl->b.y;
  batch_lines[batch_size].color = color;
  batch_size++;
}

void I_DoomDevFillFlat(int lump, int scrn, int x, int y, int width, int height, enum patch_translation_e flags)
{
  int res;
  I_DoomDevFlushBatch();
  lump += firstflat;
  if (lumpinfo[lump].flat_fd == -1)
  {
    byte *data = W_CacheLumpNum(lump);
    I_DoomDevUploadFlat(lump, data);
    W_UnlockLumpNum(lump);
  }
  struct doomdev_surf_ioctl_draw_background param = {
    .flat_fd = lumpinfo[lump].flat_fd,
  };
  res = ioctl(screens[scrn].doomdev_fd, DOOMDEV_SURF_IOCTL_DRAW_BACKGROUND, &param);
  if (res < 0)
    I_Error("doomdev fill_flat fail");
}

static uint32_t I_DoomDevComputeColumnStart(pdraw_column_vars_s dcvars)
{
	int res = dcvars->texturemid + (dcvars->yl - centery) * dcvars->iscale;
	if (dcvars->texheight) {
		res %= dcvars->texheight << 16;
		if (res < 0)
			res += dcvars->texheight << 16;
	}
	return res & 0x3ffffff;
}

void I_DoomDevDrawColumn(pdraw_column_vars_s dcvars)
{
  if (dcvars->yl > dcvars->yh)
    return;
  int draw_flags = DOOMDEV_DRAW_FLAGS_COLORMAP;
  struct doomdev_column *columns;
  int *size;
  if (dcvars->flags & DRAW_COLUMN_ISPATCH)
    draw_flags = 0;
  if (batch_mode != BATCH_COLUMNS || batch_size >= MAX_BATCH_SIZE ||
	  batch_scrn_dst != drawvars.screen || batch_size_bot >= MAX_BATCH_SIZE ||
	  batch_draw_flags != draw_flags)
    I_DoomDevFlushBatch();
  if (dcvars->flags & DRAW_COLUMN_ISBOT) {
    if (batch_texture_bot && batch_texture_bot != dcvars->texture_base)
      I_DoomDevFlushBatch();
    batch_texture_bot = dcvars->texture_base;
    batch_texture_fd_bot = dcvars->texture_fd;
    columns = batch_columns_bot;
    size = &batch_size_bot;
  } else {
    if (batch_texture && batch_texture != dcvars->texture_base)
      I_DoomDevFlushBatch();
    batch_texture = dcvars->texture_base;
    batch_texture_fd = dcvars->texture_fd;
    columns = batch_columns;
    size = &batch_size;
  }
  batch_mode = BATCH_COLUMNS;
  batch_draw_flags = draw_flags;
  batch_scrn_dst = drawvars.screen;
  batch_colormap_fd = colormap_fd[boom_cm];
  columns[*size].texture_offset = dcvars->source - dcvars->texture_base;
  columns[*size].ustart = I_DoomDevComputeColumnStart(dcvars);
  columns[*size].ustep = dcvars->iscale & 0x3ffffff;
  columns[*size].y1 = dcvars->yl + drawvars.yoff;
  columns[*size].y2 = dcvars->yh + drawvars.yoff;
  columns[*size].x = dcvars->x + drawvars.xoff;
  columns[*size].colormap_idx = (dcvars->colormap - colormaps[boom_cm]) >> 8;
  (*size)++;
}

void I_DoomDevDrawFuzzColumn(pdraw_column_vars_s dcvars)
{
  if (dcvars->yl > dcvars->yh)
    return;
  int draw_flags = DOOMDEV_DRAW_FLAGS_FUZZ;
  if (batch_mode != BATCH_COLUMNS || batch_size >= MAX_BATCH_SIZE ||
	  batch_scrn_dst != drawvars.screen || batch_texture != dcvars->texture_base ||
	  batch_draw_flags != draw_flags)
    I_DoomDevFlushBatch();
  batch_mode = BATCH_COLUMNS;
  batch_draw_flags = draw_flags;
  batch_scrn_dst = drawvars.screen;
  batch_texture_fd = dcvars->texture_fd;
  batch_colormap_fd = colormap_fd[boom_cm];
  batch_texture = dcvars->texture_base;
  batch_columns[batch_size].texture_offset = dcvars->source - dcvars->texture_base;
  batch_columns[batch_size].ustart = I_DoomDevComputeColumnStart(dcvars);
  batch_columns[batch_size].ustep = dcvars->iscale & 0x3ffffff;
  batch_columns[batch_size].y1 = dcvars->yl + drawvars.yoff;
  batch_columns[batch_size].y2 = dcvars->yh + drawvars.yoff;
  batch_columns[batch_size].x = dcvars->x + drawvars.xoff;
  batch_columns[batch_size].colormap_idx = 6;
  batch_size++;
}

void I_DoomDevDrawTranslatedColumn(pdraw_column_vars_s dcvars)
{
  if (dcvars->yl > dcvars->yh)
    return;
  int i;
  int xlat = 0;
  int draw_flags = DOOMDEV_DRAW_FLAGS_COLORMAP | DOOMDEV_DRAW_FLAGS_TRANSLATE;
  for (i = 0; i < 3; i++)
    if (dcvars->translation == translationtables + i * 256)
      xlat = CR_LIMIT + i;
  for (i = 0; i < CR_LIMIT; i++)
    if (dcvars->translation == colrngs[i])
      xlat = i;
  if (dcvars->flags & DRAW_COLUMN_ISPATCH)
    draw_flags = DOOMDEV_DRAW_FLAGS_TRANSLATE;
  if (batch_mode != BATCH_COLUMNS || batch_size >= MAX_BATCH_SIZE ||
	  batch_scrn_dst != drawvars.screen || batch_texture != dcvars->texture_base ||
	  batch_draw_flags != draw_flags || batch_translation != xlat)
    I_DoomDevFlushBatch();
  batch_mode = BATCH_COLUMNS;
  batch_draw_flags = draw_flags;
  batch_scrn_dst = drawvars.screen;
  batch_texture_fd = dcvars->texture_fd;
  batch_colormap_fd = colormap_fd[boom_cm];
  batch_texture = dcvars->texture_base;
  batch_translation = xlat;
  batch_columns[batch_size].texture_offset = dcvars->source - dcvars->texture_base;
  batch_columns[batch_size].ustart = I_DoomDevComputeColumnStart(dcvars);
  batch_columns[batch_size].ustep = dcvars->iscale & 0x3ffffff;
  batch_columns[batch_size].y1 = dcvars->yl + drawvars.yoff;
  batch_columns[batch_size].y2 = dcvars->yh + drawvars.yoff;
  batch_columns[batch_size].x = dcvars->x + drawvars.xoff;
  batch_columns[batch_size].colormap_idx = (dcvars->colormap - colormaps[boom_cm]) >> 8;
  batch_size++;
}

void I_DoomDevDrawSpan(draw_span_vars_t *dsvars)
{
  if (batch_mode != BATCH_SPANS || batch_size >= MAX_BATCH_SIZE ||
	  batch_scrn_dst != drawvars.screen || batch_texture_fd != dsvars->flat_fd)
    I_DoomDevFlushBatch();
  batch_mode = BATCH_SPANS;
  batch_scrn_dst = drawvars.screen;
  batch_texture_fd = dsvars->flat_fd;
  batch_colormap_fd = colormap_fd[boom_cm];
  batch_spans[batch_size].ustart = dsvars->xfrac & 0x3fffff;
  batch_spans[batch_size].ustep = dsvars->xstep & 0x3fffff;
  batch_spans[batch_size].vstart = dsvars->yfrac & 0x3fffff;
  batch_spans[batch_size].vstep = dsvars->ystep & 0x3fffff;
  batch_spans[batch_size].x1 = dsvars->x1 + drawvars.xoff;
  batch_spans[batch_size].x2 = dsvars->x2 + drawvars.xoff;
  batch_spans[batch_size].y = dsvars->y + drawvars.yoff;
  batch_spans[batch_size].colormap_idx = (dsvars->colormap - colormaps[boom_cm]) >> 8;
  batch_size++;
}

void I_DoomDevMeltColumn(int srcscrn, int destscrn, int x, int yoff, int height)
{
  if (batch_mode != BATCH_COPY || batch_size >= MAX_BATCH_SIZE || batch_scrn_dst != destscrn || batch_scrn_src != srcscrn)
    I_DoomDevFlushBatch();
  batch_mode = BATCH_COPY;
  batch_scrn_dst = destscrn;
  batch_scrn_src = srcscrn;
  batch_copy[batch_size].pos_dst_x = x;
  batch_copy[batch_size].pos_dst_y = yoff;
  batch_copy[batch_size].pos_src_x = x;
  batch_copy[batch_size].pos_src_y = 0;
  batch_copy[batch_size].width = 1;
  batch_copy[batch_size].height = height;
  batch_size++;
}
