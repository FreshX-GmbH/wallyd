#ifndef WALLY_UI_H
#define WALLY_UI_H

#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>
#include <SDL2_rotozoom.h>
#include <errno.h>
#include <time.h>
#ifdef RASPBERRY
#include <GLES2/gl2.h>
#endif

#include "plugins.h"
#include "util.h"

typedef struct Color
{
    Uint8 r;
    Uint8 g;
    Uint8 b;
    Uint8 a;
} Color;


typedef struct {
    SDL_Texture *texture;
    SDL_Rect *rect;
    char *name;
    Color *c;
    bool video;
    bool active;
    int z;
    int opaque;
    int rotation;
    bool autorender;
} texInfo;

#define LOGFONT_SIZE 20
#define LOGFONT_COLOR { 0,0,0,0 }
#define STAMPFONT_SIZE 144
// Dark red
#define STAMPFONT_COLOR { 175,0,0,0 }

#define LOGFONT 0
#define STAMPFONT 1

#define VIDEOTEXTURE 0
#define MAINTEXTURE 1
#define TEXTTEXTURE 2
#define LOGTEXTURE 3 
#define TEMPTEXTURE MAXTEXTURES-1

extern int showTexture(char *textureNum);
extern int hideTexture(char *textureNum);
extern void renderActive(char *);
extern int renderActiveEx(char *);
extern void renderToTex(int textureNum);
extern void render(SDL_Texture *texture, SDL_Rect *rect);
extern int setTexture(texInfo *TI, SDL_Texture *texture, SDL_Rect *origRect);
extern int copyToTexture(texInfo *TI, SDL_Texture *texture, SDL_Rect *origRect);
extern int scaleToTexture(texInfo *TI, SDL_Texture *texture, SDL_Rect *origRect);
void renderActive();
bool sdlInit(void);
bool uiLoop(void);
void renderTexture(SDL_Texture *t, SDL_Rect *mr);
void renderTexNum(int i);
int createTexture(char *);
int createVideoTexture(char *);
int destroyTexture(char *);
int showTextureTestScreen(char *);
int loadFont(char *);
int addToTexture(texInfo *TI, SDL_Texture *src, SDL_Rect *origRect);
void hexToColor(int color, Color *c);
texInfo *getTexture(char *name);
void **getTextureNamesByPrio(unsigned int *);
int resetScreen(char *);

//static const Uint32
//red       = 0xff0000ff,
//green     = 0x00ff00ff,
//dark      = 0x303030ff,
//grey0     = 0x606060ff,
//grey1     = 0x707070ff,
//grey2     = 0x909090ff,
//grey3     = 0xa0a0a0ff,
//grey4     = 0xb0b0b0ff,
//grey8     = 0xe9e9e9ff,
//white     = 0xffffffff;
//
//const SDL_Color
//cGrey0       = sdl_color(grey0),
//cGrey1       = sdl_color(0x808080ff),
//cGrey2       = sdl_color(grey2),
//cGrey3       = sdl_color(grey3),
//cGrey4       = sdl_color(grey4),
//cGrey5       = sdl_color(0xc0c0c0ff),
//cGrey6       = sdl_color(0xd0d0d0ff),
//cGrey7       = sdl_color(0xe0e0e0ff),
//cGrey8       = sdl_color(grey8);

#endif
