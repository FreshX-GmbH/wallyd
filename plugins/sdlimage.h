#ifndef WALLY_SDL_H
#define WALLY_SDL_H

#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>
#include <SDL2_rotozoom.h>
#ifdef RASPBERRY
#include <GLES2/gl2.h>
#endif

#include "../lib/plugins.h"
#include "../lib/ui.h"
#include "../lib/util.h"
#include "duv.h"

#define LOGFONT_SIZE 20
#define LOGFONT_COLOR { 0,0,0,0 }
#define STAMPFONT_SIZE 144
// Dark red
#define STAMPFONT_COLOR { 175,0,0,0 }

bool setTextEx(char *name,int x,int y,int rotation, const char *txt,char *fontName, char *colorName, int type);
SDL_Surface *createLogText(const char *text);
SDL_Texture* surfaceToTexture( SDL_Surface* );
int clearTexture(void *);
int clearTextureNoPaint(void *str);

#endif
