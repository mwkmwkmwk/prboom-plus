#ifndef __I_DOOMDEV__
#define __I_DOOMDEV__

#include "doomtype.h"
#include "r_draw.h"
#include "r_patch.h"
#include "v_video.h"

void I_DoomDevRead(int idx);
void I_DoomDevAllocScreens(void);
void I_DoomDevFreeScreens(void);
void I_DoomDevUploadFlat(int lump, const byte *data);
void I_DoomDevUploadColormap(int idx);
void I_DoomDevUploadPatch(rpatch_t *patch);
void I_DoomDevClosePatch(rpatch_t *patch);

void I_DoomDevPlotPixel(int scrn, int x, int y, byte color);
void I_DoomDevPlotPixelWu(int scrn, int x, int y, byte color, int weight);
void I_DoomDevDrawLine(fline_t* fl, int color);
void I_DoomDevDrawBackground(const char *flatname, int n);
void I_DoomDevCopyRect(int srcscrn, int destscrn, int x, int y, int width, int height, enum patch_translation_e flags);
void I_DoomDevFillRect(int scrn, int x, int y, int width, int height, byte colour);
void I_DoomDevDrawColumn(pdraw_column_vars_s dcvars);
void I_DoomDevDrawSpan(draw_span_vars_t *dsvars);

#endif
