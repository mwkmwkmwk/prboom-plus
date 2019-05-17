#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include "v_video.h"
#include "i_doomdev.h"
#include "doomdev2.h"
#include "lprintf.h"
#include "r_draw.h"
#include "r_main.h"
#include "w_wad.h"

int doom_fd = -1;
int doomdev2_surf_width;
int doomdev2_surf_height;
int translations_fd = -1;
int tranmap_fd = -1;

#define MAX_BATCH_SIZE 0x400

int last_scrn_dst;
int last_scrn_src;
int last_tex_fd;
int last_flat_fd;
int last_cm_fd;

int batch_scrn_dst, batch_scrn_src;
struct doomdev2_cmd batch_cmds[MAX_BATCH_SIZE];
struct doomdev2_cmd batch_cmds_bot[MAX_BATCH_SIZE];

int batch_size = 0;
int batch_size_bot = 0;

int batch_texture_fd;
int batch_texture_bot_fd;
int batch_flat_fd;
int batch_colormap_fd;

void I_DoomDevPrepBatch(int destscrn, int srcscrn, int tex_fd, int tex_bot_fd, int flat_fd, int cm_fd) {
  if (batch_size >= MAX_BATCH_SIZE || batch_size_bot >= MAX_BATCH_SIZE)
    I_DoomDevFlushBatch();
  if (batch_scrn_dst != -1 && batch_scrn_dst != destscrn)
    I_DoomDevFlushBatch();
  if (batch_scrn_src != -1 && srcscrn != -1 && batch_scrn_src != srcscrn)
    I_DoomDevFlushBatch();
  if (batch_texture_fd != -1 && tex_fd != -1 && tex_fd != batch_texture_fd)
    I_DoomDevFlushBatch();
  if (batch_texture_bot_fd != -1 && tex_bot_fd != -1 && tex_bot_fd != batch_texture_bot_fd)
    I_DoomDevFlushBatch();
  if (batch_flat_fd != -1 && flat_fd != -1 && flat_fd != batch_flat_fd)
    I_DoomDevFlushBatch();
  if (batch_colormap_fd != -1 && cm_fd != -1 && cm_fd != batch_colormap_fd)
    I_DoomDevFlushBatch();
  batch_scrn_dst = destscrn;
  if (srcscrn != -1)
    batch_scrn_src = srcscrn;
  if (tex_fd != -1)
    batch_texture_fd = tex_fd;
  if (tex_bot_fd != -1)
    batch_texture_bot_fd = tex_bot_fd;
  if (flat_fd != -1)
    batch_flat_fd = flat_fd;
  if (cm_fd != -1)
    batch_colormap_fd = cm_fd;
}

void I_DoomDevSetup(int tex_fd) {
  int doit = 0;
  if (batch_scrn_dst != last_scrn_dst) {
    last_scrn_dst = batch_scrn_dst;
    doit = 1;
  }
  if (batch_scrn_src != last_scrn_src && batch_scrn_src != -1) {
    last_scrn_src = batch_scrn_src;
    doit = 1;
  }
  if (tex_fd != -1 && last_tex_fd != tex_fd) {
    last_tex_fd = tex_fd;
    doit = 1;
  }
  if (batch_flat_fd != -1 && last_flat_fd != batch_flat_fd) {
    last_flat_fd = batch_flat_fd;
    doit = 1;
  }
  if (batch_colormap_fd != -1 && last_cm_fd != batch_colormap_fd) {
    last_cm_fd = batch_colormap_fd;
    doit = 1;
  }
  if (doit) {
    struct doomdev2_ioctl_setup param = {
      screens[last_scrn_dst].doomdev_fd,
      last_scrn_src == -1 ? -1 : screens[last_scrn_src].doomdev_fd,
      last_tex_fd,
      last_flat_fd,
      last_cm_fd,
      translations_fd,
      tranmap_fd,
    };
    int res = ioctl(doom_fd, DOOMDEV2_IOCTL_SETUP, &param);
    if (res < 0)
      I_Error("doomdev2 setup fail");
  }
}

void I_DoomDevFlushBatch(void)
{
  int done = 0;
  if (batch_size)
    I_DoomDevSetup(batch_texture_fd);
  while (done < batch_size) {
    ssize_t res = write(doom_fd, batch_cmds + done, sizeof *batch_cmds * (batch_size - done));
    if (res <= 0)
      I_Error("doomdev2 render fail");
    done += res / sizeof *batch_cmds;
  }
  done = 0;
  if (batch_size_bot)
    I_DoomDevSetup(batch_texture_bot_fd);
  while (done < batch_size_bot) {
    ssize_t res = write(doom_fd, batch_cmds_bot + done, sizeof *batch_cmds_bot * (batch_size_bot - done));
    if (res <= 0)
      I_Error("doomdev2 render fail");
    done += res / sizeof *batch_cmds;
  }
  batch_size = 0;
  batch_size_bot = 0;
  batch_scrn_dst = -1;
  batch_scrn_src = -1;
  batch_texture_fd = -1;
  batch_texture_bot_fd = -1;
  batch_flat_fd = -1;
  batch_colormap_fd = -1;
}

void I_DoomDevRead(int idx)
{
  int res;
  int y;
  I_DoomDevFlushBatch();
  for (y = 0; y < screens[idx].height; y++) {
    off_t offset = doomdev2_surf_width * y;
    byte *ptr = screens[idx].data + screens[idx].byte_pitch * y;
    int size = screens[idx].width;
    int copied = 0;
    while (copied < size) {
      res = pread(screens[idx].doomdev_fd, ptr + copied, size - copied, offset + copied);
      if (res <= 0) {
        I_Error("doomdev2 read fail");
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
  struct doomdev2_ioctl_create_buffer xlat_param = {
    .size = (CR_LIMIT + 3) * 0x100,
  };
  ssize_t res;
  for (i = 0; i < CR_LIMIT; i++)
    memcpy(trans + i * 256, colrngs[i], 256);
  memcpy(trans + CR_LIMIT * 256, translationtables, 256 * 3);
  if (width & 0x3f) {
    width |= 0x3f;
    width++;
  }
  doomdev2_surf_width = width;
  doomdev2_surf_height = screens[0].height;
  doom_fd = open("/dev/doom0", O_RDWR);
  if (doom_fd < 0) {
    I_Error("doomdev2 open fail");
  }
  for (i = 0; i < NUM_SCREENS; i++) {
    struct doomdev2_ioctl_create_surface param = {
      .width = doomdev2_surf_width,
      .height = doomdev2_surf_height,
    };
    int fd = ioctl(doom_fd, DOOMDEV2_IOCTL_CREATE_SURFACE, &param);
    if (fd < 0)
      I_Error("doomdev2 create_surface fail");
    screens[i].doomdev_fd = fd;
  }
  translations_fd = ioctl(doom_fd, DOOMDEV2_IOCTL_CREATE_BUFFER, &xlat_param);
  if (translations_fd < 0)
    I_Error("doomdev2 create_colormap translation fail");
  res = write(translations_fd, trans, xlat_param.size);
  if (res != xlat_param.size)
    I_Error("doomdev2 create_colormap translation fail");
  xlat_param.size = 0x10000;
  tranmap_fd = ioctl(doom_fd, DOOMDEV2_IOCTL_CREATE_BUFFER, &xlat_param);
  if (tranmap_fd < 0)
    I_Error("doomdev2 create_tranmap fail");
  res = write(tranmap_fd, main_tranmap, xlat_param.size);
  if (res != xlat_param.size)
    I_Error("doomdev2 create_tranmap fail");
  free(trans);
  I_DoomDevUploadColormap(boom_cm);
  last_scrn_dst = -1;
  last_scrn_src = -1;
  last_tex_fd = -1;
  last_flat_fd = -1;
  last_cm_fd = -1;
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
  if (tranmap_fd != -1)
    close(tranmap_fd);
  tranmap_fd = -1;
  if (doom_fd != -1)
    close(doom_fd);
  doom_fd = -1;
}

void I_DoomDevUploadPatch(rpatch_t *patch)
{
  struct doomdev2_ioctl_create_buffer param = {
    .size = patch->height * patch->width,
  };
  int fd;
  ssize_t res;
  if (patch->doomdev_fd != -1)
    return;
  fd = ioctl(doom_fd, DOOMDEV2_IOCTL_CREATE_BUFFER, &param);
  if (fd < 0)
    I_Error("doomdev2 create_texture patch fail");
  res = write(fd, patch->pixels, param.size);
  if (res != param.size)
    I_Error("doomdev2 create_flat fail");
  patch->doomdev_fd = fd;
}

void I_DoomDevClosePatch(rpatch_t *patch)
{
  I_DoomDevFlushBatch();
  last_tex_fd = -1;
  if (patch->doomdev_fd != -1)
    close(patch->doomdev_fd);
  patch->doomdev_fd = -1;
}

void I_DoomDevUploadFlat(int lump, const byte *data)
{
  struct doomdev2_ioctl_create_buffer param = {
    .size = 0x1000,
  };
  int fd;
  ssize_t res;
  if (lumpinfo[lump].flat_fd != -1)
    return;
  fd = ioctl(doom_fd, DOOMDEV2_IOCTL_CREATE_BUFFER, &param);
  if (fd < 0)
    I_Error("doomdev2 create_flat fail");
  res = write(fd, data, param.size);
  if (res != param.size)
    I_Error("doomdev2 create_flat fail");
  lumpinfo[lump].flat_fd = fd;
}

void I_DoomDevUploadColormap(int idx)
{
  struct doomdev2_ioctl_create_buffer param = {
    .size = 34 * 0x100,
  };
  int fd;
  ssize_t res;
  if (colormap_fd[idx] != -1)
    return;
  fd = ioctl(doom_fd, DOOMDEV2_IOCTL_CREATE_BUFFER, &param);
  if (fd < 0)
    I_Error("doomdev2 create_colormaps fail");
  res = write(fd, colormaps[idx], param.size);
  if (res != param.size)
    I_Error("doomdev2 create_colormaps fail");
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
  I_DoomDevPrepBatch(scrn, -1, -1, -1, -1, -1);
  batch_cmds[batch_size].fill_rect = ((struct doomdev2_cmd_fill_rect){
	.type = DOOMDEV2_CMD_TYPE_FILL_RECT,
	.fill_color = colour,
	.pos_x = x,
	.pos_y = y,
	.width = width,
	.height = height,
  });
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
  I_DoomDevPrepBatch(destscrn, srcscrn, -1, -1, -1, -1);
  batch_cmds[batch_size].copy_rect = ((struct doomdev2_cmd_copy_rect){
	.type = DOOMDEV2_CMD_TYPE_COPY_RECT,
	.pos_dst_x = x,
	.pos_dst_y = y,
	.pos_src_x = x,
	.pos_src_y = y,
	.width = width,
	.height = height,
  });
  batch_size++;
}

void I_DoomDevDrawLine(fline_t* fl, int color)
{
  I_DoomDevPrepBatch(0, -1, -1, -1, -1, -1);
  batch_cmds[batch_size].draw_line = ((struct doomdev2_cmd_draw_line){
	.type = DOOMDEV2_CMD_TYPE_DRAW_LINE,
	.fill_color = color,
	.pos_a_x = fl->a.x,
	.pos_a_y = fl->a.y,
	.pos_b_x = fl->b.x,
	.pos_b_y = fl->b.y,
  });
  batch_size++;
}

void I_DoomDevFillFlat(int lump, int scrn, int x, int y, int width, int height, enum patch_translation_e flags)
{
  int res;
  I_DoomDevFlushBatch();
  lump += firstflat;
  if (lumpinfo[lump].flat_fd == -1)
  {
    const byte *data = W_CacheLumpNum(lump);
    I_DoomDevUploadFlat(lump, data);
    W_UnlockLumpNum(lump);
  }
  I_DoomDevPrepBatch(scrn, -1, -1, -1, lumpinfo[lump].flat_fd, -1);
  batch_cmds[batch_size].draw_background = ((struct doomdev2_cmd_draw_background){
	.type = DOOMDEV2_CMD_TYPE_DRAW_BACKGROUND,
	.flat_idx = 0,
	.width = doomdev2_surf_width,
	.height = doomdev2_surf_height,
	.pos_x = 0,
	.pos_y = 0,
  });
  batch_size++;
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

void I_DoomDevDrawColumn(pdraw_column_vars_s dcvars)
{
  int draw_flags = DOOMDEV2_CMD_FLAGS_COLORMAP;
  struct doomdev2_cmd *cmds;
  int *size;
  if (dcvars->yl > dcvars->yh)
    return;
  if (dcvars->flags & DRAW_COLUMN_ISPATCH)
    draw_flags = 0;
  if (dcvars->flags & DRAW_COLUMN_ISBOT) {
    I_DoomDevPrepBatch(drawvars.screen, -1, -1, dcvars->texture_fd, -1, colormap_fd[boom_cm]);
    cmds = batch_cmds_bot;
    size = &batch_size_bot;
  } else {
    I_DoomDevPrepBatch(drawvars.screen, -1, dcvars->texture_fd, -1, -1, colormap_fd[boom_cm]);
    cmds = batch_cmds;
    size = &batch_size;
  }
  cmds[*size].draw_column = ((struct doomdev2_cmd_draw_column){
	.type = DOOMDEV2_CMD_TYPE_DRAW_COLUMN,
	.flags = draw_flags,
	.texture_offset = dcvars->source - dcvars->texture_base,
	.ustart = I_DoomDevComputeColumnStart(dcvars),
	.ustep = dcvars->iscale,
	.pos_a_y = dcvars->yl + drawvars.yoff,
	.pos_b_y = dcvars->yh + drawvars.yoff,
	.pos_x = dcvars->x + drawvars.xoff,
	.texture_height = dcvars->texheight,
	.colormap_idx = (dcvars->colormap - colormaps[boom_cm]) >> 8,
  });
  (*size)++;
}

void I_DoomDevDrawTransparentColumn(pdraw_column_vars_s dcvars)
{
  int draw_flags = DOOMDEV2_CMD_FLAGS_COLORMAP | DOOMDEV2_CMD_FLAGS_TRANMAP;
  struct doomdev2_cmd *cmds;
  int *size;
  if (dcvars->yl > dcvars->yh)
    return;
  if (dcvars->flags & DRAW_COLUMN_ISPATCH)
    draw_flags = DOOMDEV2_CMD_FLAGS_TRANMAP;
  if (dcvars->flags & DRAW_COLUMN_ISBOT) {
    I_DoomDevPrepBatch(drawvars.screen, -1, -1, dcvars->texture_fd, -1, colormap_fd[boom_cm]);
    cmds = batch_cmds_bot;
    size = &batch_size_bot;
  } else {
    I_DoomDevPrepBatch(drawvars.screen, -1, dcvars->texture_fd, -1, -1, colormap_fd[boom_cm]);
    cmds = batch_cmds;
    size = &batch_size;
  }
  cmds[*size].draw_column = ((struct doomdev2_cmd_draw_column){
	.type = DOOMDEV2_CMD_TYPE_DRAW_COLUMN,
	.flags = draw_flags,
	.texture_offset = dcvars->source - dcvars->texture_base,
	.ustart = I_DoomDevComputeColumnStart(dcvars),
	.ustep = dcvars->iscale,
	.pos_a_y = dcvars->yl + drawvars.yoff,
	.pos_b_y = dcvars->yh + drawvars.yoff,
	.pos_x = dcvars->x + drawvars.xoff,
	.texture_height = dcvars->texheight,
	.colormap_idx = (dcvars->colormap - colormaps[boom_cm]) >> 8,
  });
  (*size)++;
}

void I_DoomDevDrawFuzzColumn(pdraw_column_vars_s dcvars)
{
  static int fuzzpos;
  if (dcvars->yl > dcvars->yh)
    return;
  I_DoomDevPrepBatch(drawvars.screen, -1, -1, -1, -1, colormap_fd[boom_cm]);
  batch_cmds[batch_size].draw_fuzz = ((struct doomdev2_cmd_draw_fuzz){
	.type = DOOMDEV2_CMD_TYPE_DRAW_FUZZ,
	.pos_a_y = dcvars->yl + drawvars.yoff,
	.pos_b_y = dcvars->yh + drawvars.yoff,
	.pos_x = dcvars->x + drawvars.xoff,
	.fuzz_pos = fuzzpos,
	.colormap_idx = 6,
	.fuzz_start = 0,
	.fuzz_end = doomdev2_surf_height-1,
  });
  fuzzpos += dcvars->yh - dcvars->yl + 1;
  fuzzpos %= 50;
  batch_size++;
}

void I_DoomDevDrawTranslatedColumn(pdraw_column_vars_s dcvars)
{
  int i;
  int xlat = 0;
  int draw_flags = DOOMDEV2_CMD_FLAGS_COLORMAP | DOOMDEV2_CMD_FLAGS_TRANSLATE;
  if (dcvars->yl > dcvars->yh)
    return;
  for (i = 0; i < 3; i++)
    if (dcvars->translation == translationtables + i * 256)
      xlat = CR_LIMIT + i;
  for (i = 0; i < CR_LIMIT; i++)
    if (dcvars->translation == colrngs[i])
      xlat = i;
  if (dcvars->flags & DRAW_COLUMN_ISPATCH)
    draw_flags = DOOMDEV2_CMD_FLAGS_TRANSLATE;
  I_DoomDevPrepBatch(drawvars.screen, -1, dcvars->texture_fd, -1, -1, colormap_fd[boom_cm]);
  batch_cmds[batch_size].draw_column = ((struct doomdev2_cmd_draw_column){
	.type = DOOMDEV2_CMD_TYPE_DRAW_COLUMN,
	.flags = draw_flags,
	.pos_x = dcvars->x + drawvars.xoff,
	.pos_a_y = dcvars->yl + drawvars.yoff,
	.pos_b_y = dcvars->yh + drawvars.yoff,
	.colormap_idx = (dcvars->colormap - colormaps[boom_cm]) >> 8,
	.translation_idx = xlat,
	.texture_height = dcvars->texheight,
	.texture_offset = dcvars->source - dcvars->texture_base,
	.ustart = I_DoomDevComputeColumnStart(dcvars),
	.ustep = dcvars->iscale,
  });
  batch_size++;
}

void I_DoomDevDrawSpan(draw_span_vars_t *dsvars)
{
  I_DoomDevPrepBatch(drawvars.screen, -1, -1, -1, dsvars->flat_fd, colormap_fd[boom_cm]);
  batch_cmds[batch_size].draw_span = ((struct doomdev2_cmd_draw_span){
	  .type = DOOMDEV2_CMD_TYPE_DRAW_SPAN,
	  .flags = DOOMDEV2_CMD_FLAGS_COLORMAP,
	  .ustart = dsvars->xfrac,
	  .ustep = dsvars->xstep,
	  .vstart = dsvars->yfrac,
	  .vstep = dsvars->ystep,
	  .pos_a_x = dsvars->x1 + drawvars.xoff,
	  .pos_b_x = dsvars->x2 + drawvars.xoff,
	  .pos_y = dsvars->y + drawvars.yoff,
	  .colormap_idx = (dsvars->colormap - colormaps[boom_cm]) >> 8,
	  .flat_idx = 0,
	  .translation_idx = 0,
  });
  batch_size++;
}

void I_DoomDevMeltColumn(int srcscrn, int destscrn, int x, int yoff, int height)
{
  I_DoomDevPrepBatch(destscrn, srcscrn, -1, -1, -1, -1);
  batch_cmds[batch_size].copy_rect = ((struct doomdev2_cmd_copy_rect){
	.type = DOOMDEV2_CMD_TYPE_COPY_RECT,
	.width = 1,
	.height = height,
	.pos_dst_x = x,
	.pos_dst_y = yoff,
	.pos_src_x = x,
	.pos_src_y = 0,
  });
  batch_size++;
}
