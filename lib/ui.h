#ifndef WALLY_UI_H
#define WALLY_UI_H

#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <stdio.h>

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
    SDL_Color *c;
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

extern int showTexture(void *);
extern int hideTexture(void *);
extern void renderActive(const char *);
extern int renderActiveEx(void *);
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
int createTexture(void *);
int createVideoTexture(void *);
int destroyTexture(void *);
int showTextureTestScreen(void *);
int loadFont(void *);
int addToTexture(texInfo *TI, SDL_Texture *src, SDL_Rect *origRect);
void hexToColor(int color, SDL_Color *c);
texInfo *getTexture(const char *name);
void **getTextureNamesByPrio(unsigned int *);
int resetScreen(void *);

#endif
