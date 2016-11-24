#include "sdlimage.h"

#define PLUGIN_SCOPE "screen"

#define renderTex(i) setTexture(MAINTEXTURE,ph->textures[i],NULL);

extern pluginHandler *ph;
char *logFile=NULL;
SDL_Color textColor = LOGFONT_COLOR;
SDL_Color stampColor = STAMPFONT_COLOR;
SDL_Color backgroundColor = { 0, 0, 0, 255 }; // black 

#define TEXT_ANSI 0
#define TEXT_UNICODE 1
#define TEXT_UTF8 2
#define TEXT_GLYPH 3

// See : https://www.libsdl.org/projects/SDL_ttf/docs/SDL_ttf_42.html
#define RENDERUTF8      TTF_RenderUTF8_Blended
#define RENDERUNICODE   TTF_RenderUNICODE_Blended
#define RENDERGLYPH     TTF_RenderGlyph_Blended
#define RENDERTEXT      TTF_RenderText_Blended

int setTextUTF8(void *strTmp){
    char *str=strdup(strTmp);

    int x,y;
    char *name = strtok(str, " ");

    texInfo *TI = getTexture(name);
    if(!TI) { return false; }

    char *colorName = strtok(NULL, " ");
    char *fontName = strtok(NULL, " ");
    if(!getNumOrPercent(strtok(NULL, " "), TI->rect->w, &x)){
        slog(ERROR,LOG_SDL,"Wrong parameters for setText(n, col, font, x, y, text) : (%s)",str);
        return false;
    }
    if(!getNumOrPercent(strtok(NULL, " "), TI->rect->h, &y)){
        slog(ERROR,LOG_SDL,"Wrong parameters for setText(n, col, font, x, y, text) : (%s)",str);
        return false;
    }
    char *text = strtok(NULL, "");
    if(!text || !name || !colorName || !fontName){
        slog(ERROR,LOG_SDL,"Wrong parameters for setText(n, col, font, x, y, text) : (%s)",str);
        return false;
    }
    slog(DEBUG,LOG_SDL,"Set text to texture %s at (%d,%d) : %s",name, x, y, text);
    setTextEx(name, x, y, 0, text, fontName, colorName, TEXT_UTF8);
    free(str);
    return true;
}


int setText(void *strTmp){
    char *str=strdup(strTmp);

    int x,y;
    char *name = strtok(str, " ");

    texInfo *TI = getTexture(name);
    if(!TI) { return false; }

    char *colorName = strtok(NULL, " ");
    char *fontName = strtok(NULL, " ");
    if(!getNumOrPercent(strtok(NULL, " "), TI->rect->w, &x)){
        slog(ERROR,LOG_SDL,"Wrong parameters for setText(n, col, font, x, y, text) : (%s)",str);
        return false;
    }
    if(!getNumOrPercent(strtok(NULL, " "), TI->rect->h, &y)){
        slog(ERROR,LOG_SDL,"Wrong parameters for setText(n, col, font, x, y, text) : (%s)",str);
        return false;
    }
    char *text = strtok(NULL, "");
    if(!text || !name || !colorName || !fontName){
        slog(ERROR,LOG_SDL,"Wrong parameters for setText(n, col, font, x, y, text) : (%s)",str);
        return false;
    }
    slog(DEBUG,LOG_SDL,"Set text to texture %s at (%d,%d) : %s",name, x, y, text);
    setTextEx(name, x, y, 0, text, fontName, colorName, TEXT_ANSI);
    free(str);
    return true;
}

//int setTextRotate(char *str){
//   char *name;
//   char *xS = strtok(str, " ");
//   char *yS = strtok(NULL, " ");
//   char *rotS = strtok(NULL, " ");
//   char *text = strtok(NULL, "");
//   int x=strtol(xS,NULL,10);
//   int y=strtol(yS,NULL,10);
//   int rot=strtol(rotS,NULL,10);
//   slog(DEBUG,LOG_SDL,"Set text at (%d,%d) rot %d : %s",x,y,rot,text);
//   TTF_Font *font = ht_get_simple(ph->fonts,"font");
//   if(!font){
//      slog(ERROR,LOG_SDL,"Font named '%s' not loaded","font");
//      return false;
//   }
//   return setTextEx(name, x, y, rot, text, font, &stampColor );
//}

int setAutoRender(void *t){
   if(strncmp(t,"true",4) == 0){
      ph->autorender = true;
      slog(DEBUG,LOG_SDL,"autorender is now on");
      return 0;
   }
   else if(strncmp(t,"false",5) == 0)
   {
      slog(DEBUG,LOG_SDL,"autorender is now off");
      ph->autorender = false;
      return 0;
   }
   slog(WARN,LOG_SDL,"Usage : setAutoRender(true|false)");
   return 1;
}
int renderLog(void *strTmp){
   char *str=strdup(strTmp);
   clearTextureNoPaint("log");
   setTextEx("log", 1, 0, 0, str, "logfont" ,"log", TEXT_ANSI);
   free(str);
   return true;
}


bool setTextEx(char *name, int x, int y,int rotation, const char *text, char *fontName, char *colorName, int textType)
{
   texInfo *TI = getTexture(name);
   if(!TI) { return false; }
   errno = 0;
   SDL_Color *c = ht_get_simple(ph->colors,colorName);
   if(!c && errno) { 
      slog(ERROR,LOG_SDL,"Color %s not defined",colorName);
      return false; 
   }

   // DEBUG memory leak
   //if(strncmp(name,"log",3) != 0) 
   //return true;

   TTF_Font *font = ht_get_simple(ph->fonts,fontName);
   if(!font){
      slog(ERROR,LOG_SDL,"Font named '%s' not loaded",fontName);
      return false;
   }
   SDL_Rect dest = {x,y,0,0};
   // TODO : Implement roto globally
   SDL_Surface *rsurf,*surf;

   slog(DEBUG,LOG_SDL,"Rendering text into surface with rot %d",rotation);
   switch(textType){
      case TEXT_UTF8:
         surf = RENDERUTF8( font, text, *c );
         break;
//      case TEXT_UNICODE:
//         surf = RENDERUNICODE( font, text, *c );
//         break;
//      case TEXT_GLYPH:
//         surf = RENDERGLYPH( font, text, *c );
//         break;
      default: 
         surf = RENDERTEXT( font, text, *c );
         break;
   }
   if(!surf) {
      slog(ERROR,LOG_SDL,"Could not create FontSurface : %s",SDL_GetError());
      return 1;
   }
   if(rotation != 0){
      rsurf = rotozoomSurface(surf, (double)rotation, 1.0, 1); 
      if(!rsurf) {
	 slog(ERROR,LOG_SDL,"Could not create rotated FontSurface : %s",SDL_GetError());
	 SDL_FreeSurface( surf );
	 return 1;
      }
      SDL_FreeSurface( surf );
      surf = rsurf;
   }

   SDL_Texture *texture = SDL_CreateTextureFromSurface( ph->renderer, surf );
   ph->textureCount++;

   SDL_QueryTexture( texture, NULL, NULL, &dest.w, &dest.h );

   slog(TRACE,LOG_SDL,"setTexture(%s,{%d,%d,%d,%d)",name,x,y,dest.w,dest.h);
   addToTexture(TI, texture, &dest);
   renderActive(NULL);
   SDL_FreeSurface( surf );
   SDL_DestroyTexture(texture);
   ph->textureCount--;
   return true;
}

int fillTexture(texInfo *TI, bool refresh)
{
   // TODO : checks
   if(!TI) return false;
   SDL_SetRenderTarget(ph->renderer,TI->texture);
   SDL_SetRenderDrawColor(ph->renderer, TI->c->r,TI->c->g,TI->c->b,255);
   SDL_RenderClear( ph->renderer );
   SDL_SetRenderTarget(ph->renderer,NULL);
   if(refresh){
      renderActive(TI->name);
   }
   return true;
}

int clearTexture(void *str){
   if(!str) return false;
   slog(DEBUG,LOG_SDL,"Clearing texture %s.",str);
   return fillTexture(getTexture(strtok(str, " ")),true);
}

int clearTextureNoPaint(void *name){
   if(!name) return false;
   slog(DEBUG,LOG_SDL,"Clearing texture %s.",name);
   return fillTexture(getTexture(name),false);
}

int waitForTexture(void *str){
   slog(DEBUG,LOG_SDL,"Waiting for texture %s",str);
   char *name = strtok(str, " ");
   int timeout;
   if(!name){
      slog(ERROR,LOG_SDL,"Usage : waitForTexture(name, [timeout | 60])");
      return false;
   }
   char *tS = strtok(NULL, " ");
   if(!tS || !getNumOrPercent(strtok(NULL, " "),0 ,&timeout)){
      slog(WARN,LOG_SDL,"Usage : waitForTexture(name, [timeout | 60])");
      timeout = 60;
   }
   while( timeout > 0 ){
      void *t = ht_get_simple(ph->baseTextures,name);
      if(t) break;
      slog(DEBUG,LOG_SDL,"Waiting for texture %s another %d seconds",name,timeout);
      timeout--;
      sleep(1);
   }
   if(timeout >0) return false;
   return true;
}

int setTexturePrio(void *str){
   int prio;
   texInfo *TI = getTexture(strtok(str, " "));
   if(!TI) { return false; }
   char *aS = strtok(NULL, " ");
   if(aS == NULL){
      slog(ERROR,LOG_SDL,"Wrong parameters setTextureAlpha(screen,hex) (%s).",str);
      return false;
   }
   getNumHex(aS,&prio);
   TI->z = prio;
   unsigned int items = 0;
   if(ph->texturePrio) {
	free(ph->texturePrio);
   }
   updateTextureNamesByPrio(&items);
   return true;
}

int setTextureAlpha(void *str){
   int alpha;
   texInfo *TI = getTexture(strtok(str, " "));
   if(!TI) { return false; }
   char *aS = strtok(NULL, " ");
   if(aS == NULL){
      slog(ERROR,LOG_SDL,"Wrong parameters setTextureAlpha(screen,hex) (%s).",str);
      return false;
   }
   getNumHex(aS,&alpha);
   TI->c->a = alpha;
   SDL_SetTextureBlendMode(TI->texture,SDL_BLENDMODE_BLEND);
   SDL_SetRenderDrawBlendMode(ph->renderer, SDL_BLENDMODE_BLEND);
   return fillTexture(TI,true);
}


int setTextureColor(void *strTmp){
   char *str = strdup(strTmp);
   int colInt;
   texInfo *TI = getTexture(strtok(str, " "));
   if(!TI) { free(str);
	return false; 
   }
   char *color = strtok(NULL, " ");
   if(color == NULL){
      slog(ERROR,LOG_SDL,"Wrong parameters setTextureColor(screen,hex) (%s).",str);
      free(str);
      return false;
   }
   if(!getNumOrPercent(color,0,&colInt)){
      slog(ERROR,LOG_SDL,"Wrong parameters setTextureColor(screen,hex) (%s).",str);
      free(str);
      return false;
   }
   hexToColor(colInt,TI->c);
   free(str);
   return fillTexture(TI,true);
}

int setImageScaleSW(void *str)
{
   SDL_Texture *text;
   SDL_Rect rect={0,0,0,0};
   Uint32 rmask, gmask, bmask, amask;

   texInfo *TI = getTexture(strtok(str, " "));
   if(!TI) { return false; }
   char *file = strtok(NULL, " ");

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    rmask = 0xff000000;
    gmask = 0x00ff0000;
    bmask = 0x0000ff00;
    amask = 0x000000ff;
#else
    rmask = 0x000000ff;
    gmask = 0x0000ff00;
    bmask = 0x00ff0000;
    amask = 0xff000000;
#endif

   SDL_Surface *dst = SDL_CreateRGBSurface(0, TI->rect->w, TI->rect->h, 32, rmask, gmask, bmask, amask);
   if(dst == NULL) {
      fprintf(stderr, "CreateRGBSurface failed: %s\n", SDL_GetError());
      return false;
   }
   slog(TRACE,LOG_SDL,"File : %s",file);
   if(file == NULL){
      slog(ERROR,LOG_SDL,"Wrong parameters setImageScale(name,file) (%s,%s).",TI->name,file);
      return false;
   }
 
   slog(DEBUG,LOG_SDL,"Loading image %s",file);
   SDL_Surface *src = IMG_Load(file);
   if(src == NULL){
      slog(ERROR,LOG_SDL, "Error loading image : %s",IMG_GetError()); 
      return false;
   }
   SDL_QueryTexture( TI->texture, NULL, NULL, &rect.w, &rect.h );
   rect.x = 0;
   rect.y = 0;
   SDL_Rect srect={0,0,src->w,src->h};
   slog(DEBUG,LOG_SDL,"Rescaling surface to Rect({%d,%d,%d,%d}), lock : %d )",rect.x,rect.y,rect.w,rect.h,src->locked);
   //SDL_UnlockSurface( src );
   if(SDL_BlitScaled(src, &srect, dst, &rect)){
      slog(ERROR,LOG_SDL, "Error blitting image : %s",SDL_GetError()); 
      return false;
   }
   text = SDL_CreateTextureFromSurface(ph->renderer, dst);
   ph->textureCount++;
   scaleToTexture(TI,text,&rect);
   renderActive(TI->name);
   SDL_FreeSurface(src);
   SDL_FreeSurface(dst);
   SDL_DestroyTexture(text);
   ph->textureCount--;
   return true;
}


int setImageScale(void *str)
{
   texInfo *TI = getTexture(strtok(str, " "));
   if(!TI) { return false; }
   char *file = strtok(NULL, " ");

   slog(TRACE,LOG_SDL,"File : %s",file);
   if(file == NULL){
      slog(ERROR,LOG_SDL,"Wrong parameters setImageScale(name,file) (%s,%s).",TI->name,file);
      return false;
   }
 
   SDL_Rect rect={0,0,0,0};
   slog(DEBUG,LOG_SDL,"Loading image %s",file);
   SDL_Texture *text = IMG_LoadTexture(ph->renderer,file);
   ph->textureCount++;
   if(text == NULL){
      slog(ERROR,LOG_SDL, "Error loading image : %s",IMG_GetError()); 
      return false;
   }
   SDL_QueryTexture( text, NULL, NULL, &rect.w, &rect.h );
   rect.x = 0;
   rect.y = 0;
   slog(DEBUG,LOG_SDL,"Calling scaleToTexture(%s,Rect({%d,%d,%d,%d}) )",TI->name,rect.x,rect.y,rect.w,rect.h);
   scaleToTexture(TI,text,&rect);
   renderActive(TI->name);
   SDL_DestroyTexture(text);
   ph->textureCount--;
   return true;
}

//int setImageStretch(char *str)
//{
//   texInfo *TI = getTexture(strtok(str, " "));
//   if(!TI) { return false; }
//   char *file = strtok(NULL, " ");
//
//   slog(TRACE,LOG_SDL,"File : %s",file);
//   if(file == NULL){
//      slog(ERROR,LOG_SDL,"Wrong parameters setImageStretch(name,file,x,y,w,h,alpha).",TI->name,file);
//      return false;
//   }
// 
//   SDL_Rect rect={0,0,0,0};
//   slog(DEBUG,LOG_SDL,"Loading image %s",file);
//   SDL_Texture *text = IMG_LoadTexture(ph->renderer,file);
//   ph->textureCount++;
//   if(text == NULL){
//      slog(ERROR,LOG_SDL, "Error loading image : %s",IMG_GetError()); 
//      return false;
//   }
//   SDL_QueryTexture( text, NULL, NULL, &rect.w, &rect.h );
//   rect.x = 0;
//   rect.y = 0;
//   slog(DEBUG,LOG_SDL,"Calling scaleToTexture(%s,Rect({%d,%d,%d,%d}) )",TI->name,rect.x,rect.y,rect.w,rect.h);
//   scaleToTexture(TI,text,&rect);
//   renderActive(TI->name);
//   SDL_DestroyTexture(text);
//   return true;
//}

int setImage(char *name,int x, int y,const char *fileName){
   if(!name || !fileName){
      slog(ERROR,LOG_SDL,"Wrong parameters setImage(name,x,y,file).");
      return false;
   }

   texInfo *TI = getTexture(strtok(name, " "));
   if(!TI) { return false; }
 
   SDL_Rect rect;
   ph->playVideo=false;
   SDL_Texture *text = IMG_LoadTexture(ph->renderer, fileName);
   ph->textureCount++;
   SDL_QueryTexture(text, NULL, NULL, &rect.w, &rect.h );
   rect.x = x;
   rect.y = y;
   slog(DEBUG,LOG_SDL,"Texture(%d,%d)",rect.w,rect.h);
   copyToTexture(TI,text,&rect);
   SDL_DestroyTexture(text);
   ph->textureCount--;
   renderActive(NULL);
   return true;
}

char *cleanupPlugin(void *p){
   slog(DEBUG,LOG_SDL,"Cleaning up plugin "PLUGIN_SCOPE);
   return NULL;
}

// TODO : set correct number of args
//const function_list_entry c_SDLMethods[] = {
//   {  PLUGIN_SCOPE"::waitForTexture"    ,WFUNC_SYNC, waitForTexture     ,0},
//   {  PLUGIN_SCOPE"::createTexture"     ,WFUNC_THRD, createTexture      ,1},
//   {  PLUGIN_SCOPE"::createVideoTexture",WFUNC_THRD, createVideoTexture ,1},
//   {  PLUGIN_SCOPE"::destroyTexture"    ,WFUNC_THRD, destroyTexture     ,1},
//   {  PLUGIN_SCOPE"::render"            ,WFUNC_THRD, renderActiveEx     ,1},
//   {  PLUGIN_SCOPE"::resetScreen"       ,WFUNC_THRD, resetScreen        ,1},
//   {  PLUGIN_SCOPE"::setAutoRender"     ,WFUNC_THRD, setAutoRender      ,1},
//   {  PLUGIN_SCOPE"::clearTexture"      ,WFUNC_THRD, clearTexture       ,1},
//   {  PLUGIN_SCOPE"::setTextureColor"   ,WFUNC_THRD, setTextureColor    ,1},
//   {  PLUGIN_SCOPE"::setTextureAlpha"   ,WFUNC_THRD, setTextureAlpha    ,1},
//   {  PLUGIN_SCOPE"::setTexturePrio"    ,WFUNC_THRD, setTexturePrio     ,1},
//   {  PLUGIN_SCOPE"::setText"           ,WFUNC_THRD, setText            ,1},
//   {  PLUGIN_SCOPE"::setTextUTF8"       ,WFUNC_THRD, setTextUTF8        ,1},
//   {  PLUGIN_SCOPE"::log"               ,WFUNC_THRD, renderLog          ,1},
//   {  PLUGIN_SCOPE"::setImageScaled"    ,WFUNC_THRD, setImageScale      ,1},
//   {  PLUGIN_SCOPE"::setImageScaledSW"  ,WFUNC_THRD, setImageScaleSW    ,1},
//   {  PLUGIN_SCOPE"::setPngScaled"      ,WFUNC_THRD, setImageScale      ,1},
//   {  PLUGIN_SCOPE"::showTexture"       ,WFUNC_THRD, showTexture        ,1},
//   {  PLUGIN_SCOPE"::hideTexture"       ,WFUNC_THRD, hideTexture        ,1},
//   {  PLUGIN_SCOPE"::loadFont"          ,WFUNC_THRD, loadFont           ,1},
//   {  PLUGIN_SCOPE"::createColor"       ,WFUNC_THRD, createColor        ,1},
//   {  PLUGIN_SCOPE"::clearTextureNoPaint",WFUNC_THRD, clearTextureNoPaint,1},
//   {  PLUGIN_SCOPE"::showTextureTestScreen",WFUNC_THRD, showTextureTestScreen,1},
////      PLUGIN_SCOPE"::setTextrotate",(*setTextRotate)
////      PLUGIN_SCOPE"::setPng",(*setImage)
////      PLUGIN_SCOPE"::setImage",(*setImage)
//   {  NULL, 0, NULL, 0}
//}; 

char *initPlugin(pluginHandler *_ph){
    ph=_ph;
    slog(DEBUG,LOG_SDL,
        "Plugin SDL2 initializing, PH is at 0x%x, window at 0x%x, renderer at 0x%x, winRenderer at 0x%x",
        ph,ph->window, ph->renderer, SDL_GetRenderer(ph->window));
   // wally_put_function_list(ph,c_SDLMethods);

   wally_put_function(PLUGIN_SCOPE"::waitForTexture"    ,WFUNC_SYNC, waitForTexture     ,0);
   wally_put_function(PLUGIN_SCOPE"::createTexture"     ,WFUNC_THRD, createTexture      ,1);
   wally_put_function(PLUGIN_SCOPE"::createVideoTexture",WFUNC_THRD, createVideoTexture ,1);
   wally_put_function(PLUGIN_SCOPE"::destroyTexture"    ,WFUNC_THRD, destroyTexture     ,1);
   wally_put_function(PLUGIN_SCOPE"::render"            ,WFUNC_THRD, renderActiveEx     ,1);
   wally_put_function(PLUGIN_SCOPE"::resetScreen"       ,WFUNC_THRD, resetScreen        ,1);
   wally_put_function(PLUGIN_SCOPE"::setAutoRender"     ,WFUNC_THRD, setAutoRender      ,1);
   wally_put_function(PLUGIN_SCOPE"::clearTexture"      ,WFUNC_THRD, clearTexture       ,1);
   wally_put_function(PLUGIN_SCOPE"::setTextureColor"   ,WFUNC_THRD, setTextureColor    ,1);
   wally_put_function(PLUGIN_SCOPE"::setTextureAlpha"   ,WFUNC_THRD, setTextureAlpha    ,1);
   wally_put_function(PLUGIN_SCOPE"::setTexturePrio"    ,WFUNC_THRD, setTexturePrio     ,1);
   wally_put_function(PLUGIN_SCOPE"::setText"           ,WFUNC_THRD, setText            ,1);
   wally_put_function(PLUGIN_SCOPE"::setTextUTF8"       ,WFUNC_THRD, setTextUTF8        ,1);
   wally_put_function(PLUGIN_SCOPE"::log"               ,WFUNC_THRD, renderLog          ,1);
   wally_put_function(PLUGIN_SCOPE"::setImageScaled"    ,WFUNC_THRD, setImageScale      ,1);
   wally_put_function(PLUGIN_SCOPE"::setImageScaledSW"  ,WFUNC_THRD, setImageScaleSW    ,1);
   wally_put_function(PLUGIN_SCOPE"::setPngScaled"      ,WFUNC_THRD, setImageScale      ,1);
   wally_put_function(PLUGIN_SCOPE"::showTexture"       ,WFUNC_THRD, showTexture        ,1);
   wally_put_function(PLUGIN_SCOPE"::hideTexture"       ,WFUNC_THRD, hideTexture        ,1);
   wally_put_function(PLUGIN_SCOPE"::loadFont"          ,WFUNC_THRD, loadFont           ,1);
   wally_put_function(PLUGIN_SCOPE"::createColor"       ,WFUNC_THRD, createColor        ,1);
   wally_put_function(PLUGIN_SCOPE"::clearTextureNoPaint",WFUNC_THRD, clearTextureNoPaint,1);

    slog(TRACE,LOG_SDL,"Plugin SDL2 initialized. PH is at 0x%x",ph);
    return PLUGIN_SCOPE;
}
