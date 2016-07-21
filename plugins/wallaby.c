#include "sdlimage.h"

#define PLUGIN_SCOPE "gui"

pluginHandler *ph;

void drawLine(char *textureName, int x1, int y1, int x2, int y2, SDL_Color col) {
    SDL_SetRenderDrawColor(ph->renderer,col.r,col.g,col.b,0xff);
    SDL_RenderDrawLine(ph->renderer, x1, y1, x2, y2 );
}

void hLine(char *textureName,int x, int y, int w, SDL_Color c)
{
	drawLine(textureName, x, y, x+w, y, c);
}
void vLine(char *textureName,int x, int y, int h, SDL_Color c)
{
	drawLine(textureName, x, y, x, y+h, c);
}

void drawBox(char *textureName,int x, int y, int h, int s, SDL_Color c)
{
    SDL_Rect r= { x, y, h, s };
    SDL_SetRenderDrawColor(ph->renderer,c.r,c.g,c.b,0xff);
    SDL_RenderFillRect(ph->renderer,&r);
}

void drawBoxInBox(char *textureName, SDL_Rect *rect, int stroke, SDL_Color col, SDL_Color strokeCol, int alpha) {
    //SDL_SetRenderDrawColor(ph->renderer,strokeCol.r,strokeCol.g,strokeCol.b,0xff);
    SDL_Rect r= { rect->x+stroke, rect->y+stroke, rect->w-2*stroke, rect->h-2*stroke };
    drawBox(textureName, rect->x, rect->y, rect->w, stroke, strokeCol); 		// top
    drawBox(textureName, rect->x, rect->y, stroke, rect->h, strokeCol);	 	// left
    drawBox(textureName, rect->x+rect->w-stroke, rect->y, stroke, rect->h, strokeCol); 	// right
    drawBox(textureName, rect->x, rect->y+rect->h-stroke, rect->w, stroke, strokeCol); 	// bottom
    SDL_SetRenderDrawColor(ph->renderer,col.r,col.g,col.b,alpha);
    SDL_RenderFillRect(ph->renderer,&r);
}

void drawRect(char *textureName, SDL_Rect *rect, SDL_Color col) {
    SDL_SetRenderDrawColor(ph->renderer,col.r,col.g,col.b,0xff);
    SDL_RenderDrawRect(ph->renderer,rect);
}

void fillRect(char * textureName, SDL_Rect *rect, SDL_Color col) {
    SDL_SetRenderDrawColor(ph->renderer,col.r,col.g,col.b,0xff);
    SDL_RenderFillRect(ph->renderer,rect);
}

SDL_Color white = { 255,255,255,255 };
SDL_Color black = { 0,0,0,255 };
SDL_Color grey0 = { 200,200,200,255 };
SDL_Color grey1 = { 60,60,60,255 };
SDL_Color red = { 255,100,100,255 };

void drawButton(char *textureName, SDL_Rect *rect, SDL_Color col) {
    fillRect(textureName,rect,col);
//  if (up) {
    hLine(textureName,rect->x,rect->y,rect->w-1,black); // upper
    hLine(textureName,rect->x,rect->y+rect->h-1,rect->w-1,grey1); // bottom
    vLine(textureName,rect->x,rect->y,rect->h-1,black);  // left
    vLine(textureName,rect->x+rect->w-1,rect->y,rect->h-1,grey1); // right
//  }
//  else {
//    hline(textureName,rect->x,rect->x+rect->w-1,rect->y,grey0);  // upper
//    hline(textureName,rect->x,rect->x+rect->w-1,rect->y+rect->h-1,white); // bottom
//    vline(textureName,rect->x,rect->y,rect->y+rect->h-1,grey0);  // left
//    vline(textureName,rect->x+rect->w-1,rect->y,rect->y+rect->h-1,white); // right
//  }
}


void drawGradient(char *textureName, SDL_Rect rect,const SDL_Color *col,bool vertical,bool hollow) {
  const int *x, *y;
  x = malloc(rect.w);
  y = malloc(rect.h);
  texInfo *TI = getTexture(textureName);
  SDL_SetRenderTarget(ph->renderer,TI->texture);
  SDL_Rect trect = {0,0,0,0};
  int run = vertical ? rect.w : rect.h;
  for (int i=0; i < run;++i) {
    int cstep = 255/run;
    int j = hollow ? 255-i*cstep : i*cstep;
    SDL_Color c = { j, 0,0,255};
    SDL_SetRenderDrawColor(ph->renderer,c.r,c.g,c.b,0xff);
    if (vertical){
      trect.x = rect.x+i; trect.y = rect.y; trect.w = 1; trect.h = rect.h;
      SDL_RenderFillRect(ph->renderer,&trect);
    } else {
      trect.x = rect.x; trect.y = rect.y+i; trect.w = rect.w; trect.h =1;
      SDL_RenderFillRect(ph->renderer,&trect);
    }
  }
  SDL_Rect brect= {rect.x,rect.y,rect.w-1,rect.h-1};
  drawRect(textureName, &brect, grey1);
}

void drawButtonTest(char *textureName){
   slog(LVL_NOISY,DEBUG,"Texture : %s", textureName);
   SDL_Rect r =  { 50,10,100,30 };
   SDL_Rect r2 =  { 300,10,100,100 };
   SDL_Rect r3 =  { 500,300,100,100 };
   texInfo *TI = getTexture(textureName);
   SDL_SetRenderTarget(ph->renderer,TI->texture);
   drawButton(textureName, &r , red);
   drawGradient(textureName, r2 , &red,true,true);
   drawBoxInBox(textureName, &r3, 8, white, black, 200);
   SDL_SetRenderTarget(ph->renderer,NULL);
   renderActive(textureName);
}

char *cleanupPlugin(void *p){
   slog(LVL_NOISY,DEBUG,"Cleaning up plugin "PLUGIN_SCOPE);
   return NULL;
}

// TODO : set correct number of args
const function_list_entry c_SDLMethods[] = {
   {  PLUGIN_SCOPE"::drawBox"        ,WFUNC_THRD, drawButtonTest  ,0},
//   {  PLUGIN_SCOPE"::drawGradient" ,WFUNC_THRD, draw_gradient   ,1},
   {  NULL, 0, NULL, 0}
}; 

char *initPlugin(pluginHandler *_ph){
    ph=_ph;
    slog(LVL_NOISY,FULLDEBUG, "Plugin "PLUGIN_SCOPE" initializing");
    wally_put_function_list(c_SDLMethods);
    slog(LVL_NOISY,FULLDEBUG,"Plugin "PLUGIN_SCOPE" initialized. PH is at 0x%x",ph);
    return PLUGIN_SCOPE;
}
