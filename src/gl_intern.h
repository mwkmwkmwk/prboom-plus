/* Emacs style mode select   -*- C++ -*-
 *-----------------------------------------------------------------------------
 *
 *
 *  PrBoom: a Doom port merged with LxDoom and LSDLDoom
 *  based on BOOM, a modified and improved DOOM engine
 *  Copyright (C) 1999 by
 *  id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
 *  Copyright (C) 1999-2000 by
 *  Jess Haas, Nicolas Kalkhof, Colin Phipps, Florian Schulze
 *  Copyright 2005, 2006 by
 *  Florian Schulze, Colin Phipps, Neil Stevens, Andrey Budko
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 *  02111-1307, USA.
 *
 * DESCRIPTION:
 *
 *---------------------------------------------------------------------
 */

#ifndef _GL_INTERN_H
#define _GL_INTERN_H

#define MAP_COEFF 128.0f
#define MAP_SCALE (MAP_COEFF*(float)FRACUNIT)

#define SMALLDELTA 0.001f

typedef enum
{
  GLDT_UNREGISTERED,
  GLDT_BROKEN,
  GLDT_PATCH,
  GLDT_TEXTURE,
  GLDT_FLAT
} GLTexType;

typedef enum
{
  GLTEXTURE_SPRITE    = 0x00000002,
  GLTEXTURE_HASHOLES  = 0x00000004,
  GLTEXTURE_SKY       = 0x00000008,
  GLTEXTURE_HIRES     = 0x00000010,
  GLTEXTURE_CLAMPY    = 0x00000020,
} GLTexture_flag_t;

typedef struct gl_strip_coords_s
{
  GLfloat v[4][3];

  GLfloat t[4][2];
} gl_strip_coords_t;

typedef struct
{
  int index;
  int width,height;
  int leftoffset,topoffset;
  int tex_width,tex_height;
  int realtexwidth, realtexheight;
  int buffer_width,buffer_height;
  int buffer_size;
  int glTexID[CR_LIMIT+MAXPLAYERS];
  
  //e6y: support for Boom colormaps
  int ***glTexExID;

  GLTexType textype;
  boolean mipmap;
  GLint wrap_mode;//e6y
  unsigned int flags;//e6y
  float scalexfac, scaleyfac; //e6y: right/bottom UV coordinates for patch drawing
} GLTexture;

typedef struct
{
  float x1,x2;
  float z1,z2;
  boolean fracleft, fracright; //e6y
} GLSeg;

typedef struct
{
  GLSeg *glseg;
  float ytop,ybottom;
  float ul,ur,vt,vb;
  float light;
  float fogdensity;
  float alpha;
  float skyymid;
  float skyyaw;
  GLTexture *gltexture;
  byte flag;
  seg_t *seg;
} GLWall;

typedef struct
{
  int sectornum;
  float light; // the lightlevel of the flat
  float fogdensity;
  float uoffs,voffs; // the texture coordinates
  float z; // the z position of the flat (height)
  GLTexture *gltexture;
  boolean ceiling;
} GLFlat;

/* GLLoopDef is the struct for one loop. A loop is a list of vertexes
 * for triangles, which is calculated by the gluTesselator in gld_PrecalculateSector
 * and in gld_PreprocessCarvedFlat
 */
typedef struct
{
  GLenum mode; // GL_TRIANGLES, GL_TRIANGLE_STRIP or GL_TRIANGLE_FAN
  int vertexcount; // number of vertexes in this loop
  int vertexindex; // index into vertex list
} GLLoopDef;

// GLSector is the struct for a sector with a list of loops.

typedef struct
{
  int loopcount; // number of loops for this sector
  GLLoopDef *loops; // the loops itself
} GLSector;

typedef struct
{
  GLfloat x;
  GLfloat y;
  GLfloat z;
} GLVertex;

typedef struct
{
  GLfloat u;
  GLfloat v;
} GLTexcoord;

typedef struct
{
  GLLoopDef loop; // the loops itself
} GLSubSector;

extern GLSeg *gl_segs;
extern GLSeg *gl_lines;

#define GLDWF_TOP 1
#define GLDWF_M1S 2
#define GLDWF_M2S 3
#define GLDWF_BOT 4
#define GLDWF_TOPFLUD 5 //e6y: project the ceiling plane into the gap
#define GLDWF_BOTFLUD 6 //e6y: project the floor plane into the gap
#define GLDWF_SKY 7
#define GLDWF_SKYFLIP 8

typedef struct
{
  int cm;
  float x,y,z;
  float vt,vb;
  float ul,ur;
  float x1,y1;
  float x2,y2;
  float light;
  fixed_t scale;
  GLTexture *gltexture;
  boolean shadow;
  boolean trans;
  mobj_t *thing;//e6y
} GLSprite;

typedef enum
{
  GLDIT_NONE,

  GLDIT_WALL,    // opaque wall
  GLDIT_FWALL,   // projected wall
  GLDIT_TWALL,   // transparent walls
  GLDIT_SWALL,   // sky walls
  
  GLDIT_CEILING, // ceiling
  GLDIT_FLOOR,   // floor

  GLDIT_SPRITE,  // opaque sprite
  GLDIT_TSPRITE, // transparent sprites

  GLDIT_TYPES
} GLDrawItemType;

typedef struct GLDrawItem_s;

typedef struct GLDrawItem_s
{
  union
  {
    void *item;
    GLWall *wall;
    GLFlat *flat;
    GLSprite *sprite;
  } item;
} GLDrawItem;

typedef struct GLDrawDataItem_s
{
  byte *data;
  int maxsize;
  int size;
} GLDrawDataItem_t;

typedef struct
{
  GLDrawDataItem_t *data;
  int maxsize;
  int size;

  GLDrawItem *items[GLDIT_TYPES];
  int num_items[GLDIT_TYPES];
  int max_items[GLDIT_TYPES];
} GLDrawInfo;

void gld_AddDrawItem(GLDrawItemType itemtype, void *itemdata);

void gld_DrawTriangleStrip(GLWall *wall, gl_strip_coords_t *c);
void gld_DrawTriangleStripARB(GLWall *wall, gl_strip_coords_t *c1, gl_strip_coords_t *c2);

extern float roll;
extern float yaw;
extern float inv_yaw;
extern float pitch;

extern int gl_preprocessed; //e6y

extern GLDrawInfo gld_drawinfo;
extern GLSector *sectorloops;
extern GLVertex *gld_vertexes;
extern GLTexcoord *gld_texcoords;

extern int gld_max_texturesize;
extern char *gl_tex_format_string;
extern int gl_tex_format;
extern int gl_tex_filter;
extern int gl_mipmap_filter;
extern int gl_texture_filter_anisotropic;
extern int gl_use_texture_filter_anisotropic;
extern int gl_paletted_texture;
extern int gl_shared_texture_palette;
extern boolean use_mipmapping;
extern int transparent_pal_index;
extern unsigned char gld_palmap[256];
extern GLTexture *last_gltexture;
extern int last_cm;

//e6y
#define DETAIL_DISTANCE 9
extern float xCamera,yCamera,zCamera;
float distance2piece(float x0, float y0, float x1, float y1, float x2, float y2);
void gld_InitDetail(void);

//e6y: OpenGL version
typedef enum {
  OPENGL_VERSION_1_0,
  OPENGL_VERSION_1_1,
  OPENGL_VERSION_1_2,
  OPENGL_VERSION_1_3,
  OPENGL_VERSION_1_4,
  OPENGL_VERSION_1_5,
  OPENGL_VERSION_2_0,
  OPENGL_VERSION_2_1,
} glversion_t;
extern int glversion;

//e6y
extern PFNGLACTIVETEXTUREARBPROC        GLEXT_glActiveTextureARB;
extern PFNGLCLIENTACTIVETEXTUREARBPROC  GLEXT_glClientActiveTextureARB;
extern PFNGLMULTITEXCOORD2FARBPROC      GLEXT_glMultiTexCoord2fARB;
extern PFNGLMULTITEXCOORD2FVARBPROC     GLEXT_glMultiTexCoord2fvARB;
extern PFNGLCOMPRESSEDTEXIMAGE2DARBPROC GLEXT_glCompressedTexImage2DARB;

//e6y: in some cases textures with a zero index (NO_TEXTURE) should be registered
GLTexture *gld_RegisterTexture(int texture_num, boolean mipmap, boolean force);
void gld_BindTexture(GLTexture *gltexture);
GLTexture *gld_RegisterPatch(int lump, int cm);
void gld_BindPatch(GLTexture *gltexture, int cm);
GLTexture *gld_RegisterFlat(int lump, boolean mipmap);
void gld_BindFlat(GLTexture *gltexture);
void gld_InitPalettedTextures(void);
int gld_GetTexDimension(int value);
void gld_SetTexturePalette(GLenum target);
void gld_Precache(void);

//gamma
void gld_ResetGammaRamp(void);
void gld_GammaCorrect(unsigned char *buffer, int bufSize);

//e6y: from gl_vertex
//extern int render_segs;
#define render_segs 0
void gld_SplitLeftEdge(const GLWall *wall, boolean detail, float detail_w, float detail_h);
void gld_SplitRightEdge(const GLWall *wall, boolean detail, float detail_w, float detail_h);
void gld_RecalcVertexHeights(const vertex_t *v);

//e6y
void gld_InitGLVersion(void);
void gld_InitExtensionsEx(void);
void gld_PreprocessDetail(void);
void gld_DrawDetail_NoARB(void);
void gld_DrawWallWithDetail(GLWall *wall);
void gld_ResetLastTexture(void);

PFNGLCOLORTABLEEXTPROC gld_ColorTableEXT;

int gld_BuildTexture(GLTexture *gltexture, void *data, boolean readonly,
                     int pitch, int width, int height,
                     unsigned char **out_buf, int *out_bufsize,
                     int *out_width, int *out_height);

//hires
extern int gl_have_hires_textures;
extern int gl_have_hires_flats;
extern int gl_have_hires_patches;
void gld_PrecacheGLTexture(GLTexture *gltexture);
void gld_InitHiRes(void);
int gld_LoadHiresTex(GLTexture *gltexture, int *glTexID, int cm);
GLuint CaptureScreenAsTexID(void);
void gld_ProgressUpdate(char * text, int progress, int total);
int gld_ProgressStart(void);
int gld_ProgressEnd(void);

//FBO
#define INVUL_CM         0x00000001
#define INVUL_INV        0x00000002
#define INVUL_BW         0x00000004

extern unsigned int invul_method;
extern float bw_red;
extern float bw_green;
extern float bw_blue;
extern int SceneInTexture;
extern int gl_invul_bw_method;
void gld_InitFBO(void);

//motion bloor
extern int gl_motionblur;
extern char *gl_motionblur_minspeed;
extern char *gl_motionblur_linear_k;
extern char *gl_motionblur_linear_b;

extern int imageformats[];

//missing flats (fake floors and ceilings)

void gld_PreprocessFakeSectors(void);

void gld_SetupFloodStencil(GLWall *wall);
void gld_ClearFloodStencil(GLWall *wall);

void gld_SetupFloodedPlaneCoords(GLWall *wall, gl_strip_coords_t *c);
void gld_SetupFloodedPlaneLight(GLWall *wall);

//light
void gld_StaticLightAlpha(float light, float alpha);
#define gld_StaticLight(light) gld_StaticLightAlpha(light, 1.0f)
void gld_InitLightTable(void);
float gld_CalcLightLevel(int lightlevel);

//fog
extern int gl_fog;
extern int gl_use_fog;
void gl_EnableFog(int on);
float gld_CalcFogDensity(sector_t *sector, int lightlevel);
void gld_SetFog(float fogdensity);

//HQ resize
unsigned char* gld_HQResize(GLTexture *gltexture, unsigned char *inputBuffer, int inWidth, int inHeight, int *outWidth, int *outHeight);

// SkyBox
#define SKY_CEILING 1
#define SKY_FLOOR   2
typedef struct PalEntry_s
{
  float r, g, b, a;
} PalEntry_t;
typedef struct SkyBoxParams_s
{
  GLWall *wall;
  float sx, sy;
  PalEntry_t FloorSkyColor;
  PalEntry_t CeilingSkyColor;
} SkyBoxParams_t;
extern int gl_drawskys;
extern SkyBoxParams_t SkyBox;
void gld_AddSkyTexture(GLWall *wall, int sky1, int sky2, int skytype);
void gld_SetSkyCapColors(const unsigned char *buffer, int width, int height);
void gld_InitFrameSky(void);
void gld_DrawSkybox(void);
void gld_DrawScreenSkybox(void);
void gld_DrawDomeSkyBox(void);
void gld_SaveSkyCap(GLWall *wall, float sx, float sy);
void gld_DrawSkyCaps(void);

//clamp
extern int GLEXT_CLAMP_TO_EDGE;

#endif // _GL_INTERN_H
