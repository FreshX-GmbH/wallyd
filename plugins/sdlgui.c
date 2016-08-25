#include "sdlimage.h"

#define PLUGIN_SCOPE "gui"

pluginHandler *ph;

void drawLine(char *textureName, int x1, int y1, int x2, int y2, SDL_Color col);
void drawGradient(char *textureName, SDL_Rect rect,const SDL_Color from, SDL_Color to, bool vertical,bool hollow);
void drawBox(char *textureName, int x, int y, int w, int h, SDL_Color col);
void drawFilledBox(char *textureName, SDL_Rect rect, int stroke, SDL_Color col, SDL_Color strokeCol, int alpha);

int js_setTargetTexture(duk_context *ctx){
    int ret;
    dschema_check(ctx, (const duv_schema_entry[]) {
      {"texture", duk_is_string},
      {0,0}});
    const char *texName  = duk_require_string(ctx, 0);
    callWithString("gui::setTargetTexture",&ret,texName);
    return 0;
}

duk_ret_t *js_resetTargetTexture(duk_context *ctx){
    int ret;
    callWithString("gui::setTargetTexture",&ret,NULL);
    return 0;
}

duk_ret_t *js_clearTexture(duk_context *ctx){
    int ret;
    dschema_check(ctx, (const duv_schema_entry[]) {
      {"texture", duk_is_string},
      {0,0} });
    const char *texName  = duk_require_string(ctx, 0);
    callWithString("screen::clearTexture",&ret,texName);
    return 0;
}

duk_ret_t *js_clearTextureNoPaint(duk_context *ctx){
    int ret;
    dschema_check(ctx, (const duv_schema_entry[]) {
      {"texture", duk_is_string},
      {0,0} });
    const char *texName  = duk_require_string(ctx, 0);
    callWithString("screen::clearTextureNoPaint",&ret,texName);
    return 0;
}

duk_ret_t js_putImage(duk_context *ctx){
    dschema_check(ctx, (const duv_schema_entry[]) {
      {"x1", duk_is_number},
      {"y1", duk_is_number},
      {"x2", duk_is_number},
      {"y2", duk_is_number},
      {"alpha", duk_is_number},
      {0,0} });
    SDL_Color c;
    int ret;
    char *callStr;
    void *args[3];
    duk_size_t sz=0;
    //args[0] = duk_require_string(ctx, 0);
    //args[1] = duk_require_buffer(ctx, 1, &sz);
    const int x1 = duk_require_int(ctx, 2);
    const int y1 = duk_require_int(ctx, 3);
    const int x2 = duk_require_int(ctx, 4);
    const int y2 = duk_require_int(ctx, 5);
    const int alpha = duk_require_int(ctx, 6);
    SDL_Rect r = {x1,y1,x2,y2};
    //args[2] = &r;
    //args[3] = &alpha;
    //asprintf(&callStr,"%s %p %d %d %d %d %d",args[0],args[1],
	//((SDL_Rect *)args[2])->x,((SDL_Rect *)args[2])->y,((SDL_Rect *)args[2])->w,((SDL_Rect *)args[2])->h,args[3]);
    //slog(LVL_INFO,INFO,"SZ : %d / STR : %s",sz,callStr);
    //call("gui::loadImageFile",&ret,callStr);
    //free(callStr);
    return 0;
}


int js_loadImageFile(duk_context *ctx){
    dschema_check(ctx, (const duv_schema_entry[]) {
       {"texture", duk_is_string},
       {"file ", duk_is_string},
       {"x1", duk_is_number},
       {"y1", duk_is_number},
       {"x2", duk_is_number},
       {"y2", duk_is_number},
       {"alpha", duk_is_number},
       {0,0} });
    SDL_Color c;
    int ret;
    char *callStr;
    const char *texName  = duk_require_string(ctx, 0);
    const char *fileName = duk_require_string(ctx, 1);
    const int x1 = duk_require_int(ctx, 2);
    const int y1 = duk_require_int(ctx, 3);
    const int x2 = duk_require_int(ctx, 4);
    const int y2 = duk_require_int(ctx, 5);
    const int alpha = duk_require_int(ctx, 6);
    SDL_Rect r = {x1,y1,x2,y2};
    asprintf(&callStr,"%s %s %d %d %d %d %d",texName,fileName, x1,y1,x2,y2,alpha);
    call("gui::loadImageFile",&ret,callStr);
    free(callStr);
    return 0;
}

int js_loadImageMemory(duk_context *ctx){
    dschema_check(ctx, (const duv_schema_entry[]) {
       {"texture", duk_is_string},
       {"file ", duk_is_string},
       {"x1", duk_is_number},
       {"y1", duk_is_number},
       {"x2", duk_is_number},
       {"y2", duk_is_number},
       {"alpha", duk_is_number},
       {0,0} });
    SDL_Color c;
    int ret, size;
    char *callStr;
    const char *texName= duk_require_string(ctx, 0);
    const char *imgPtr = duk_require_buffer(ctx, 1, &size);
    const int x1       = duk_require_int(ctx, 2);
    const int y1       = duk_require_int(ctx, 3);
    const int x2       = duk_require_int(ctx, 4);
    const int y2       = duk_require_int(ctx, 5);
    const int alpha    = duk_require_int(ctx, 6);
    SDL_Rect r = { x1,y1,x2,y2 };
    // TODO : a clean function passing interface is needed here!!
//    asprintf(&callStr,"%s %s %d %d %d %d %d",texName,fileName, x1,y1,x2,y2,alpha);
//    call("gui::loadImageMemory",&ret,callStr);
//    free(callStr);
    return 0;
}

int js_drawGradient(duk_context *ctx){
    SDL_Color from,to;
    int ret;
    char *callStr;
    dschema_check(ctx, (const duv_schema_entry[]) {
       {"texture", duk_is_string},
       {"x1", duk_is_number},
       {"y1", duk_is_number},
       {"x2", duk_is_number},
       {"y2", duk_is_number},
       {"fromColor", duk_is_number},
       {"toColor", duk_is_number},
       {"horizontal", duk_is_boolean},
       {"hollow", duk_is_boolean},
       {0,0} });
    const char *texName = duk_require_string(ctx, 0);
    const int x1 = duk_require_int(ctx, 1);
    const int y1 = duk_require_int(ctx, 2);
    const int x2 = duk_require_int(ctx, 3);
    const int y2 = duk_require_int(ctx, 4);
    const int fromcolor = duk_require_int(ctx, 5);
    const int tocolor = duk_require_int(ctx, 6);
    const int horizontal = duk_require_boolean(ctx, 7);
    const int hollow = duk_require_boolean(ctx, 8);
    SDL_Rect r = {x1,y1,x2,y2};
    hexToColor(fromcolor, &from);
    hexToColor(tocolor, &to);
    asprintf(&callStr,"%s %d %d %d %d %x %x %d %d",texName,x1,y1,x2,y2,fromcolor,tocolor,horizontal, hollow);
    call("gui::drawGradient",&ret,callStr);
    free(callStr);
    return 0;
}

int js_drawLine(duk_context *ctx)
{
    int ret;
    SDL_Color c;
    char *callStr;
    dschema_check(ctx, (const duv_schema_entry[]) {
       {"texture", duk_is_string},
       {"x1", duk_is_number},
       {"y1", duk_is_number},
       {"x2", duk_is_number},
       {"y2", duk_is_number},
       {"color", duk_is_number},
       {0,0} });
    const char *texName = duk_require_string(ctx, 0);
    const int x1 = duk_require_int(ctx, 1);
    const int y1 = duk_require_int(ctx, 2);
    const int x2 = duk_require_int(ctx, 3);
    const int y2 = duk_require_int(ctx, 4);
    const int color = duk_require_int(ctx, 5);
    asprintf(&callStr, "%s %d %d %d %d %x",texName,x1,y1,x2,y2,color);
    slog(TRACE,DEBUG,"gui::drawLine %s",callStr);
    callWithString("gui::drawLine",&ret,callStr);
    free(callStr);
    //drawLine(texName, x1,y1,x2,y2,c);
    return 0;
}

int js_drawText(duk_context *ctx){
    dschema_check(ctx, (const duv_schema_entry[]) {
       {"texture", duk_is_string},
       {"x1", duk_is_number},
       {"y1", duk_is_number},
       {"font", duk_is_string},
       {"color", duk_is_number},
       {"text", duk_is_string},
       {0,0}});
    int ret;
    char *callStr;
    const char *texName = duk_require_string(ctx, 0);
    const int x1 = duk_require_int(ctx, 1);
    const int y1 = duk_require_int(ctx, 2);
    const char *font = duk_require_string(ctx, 3);
    const char *color = duk_require_int(ctx, 4);
    const char *text = duk_require_string(ctx, 5);
    asprintf(&callStr, "%s %d %d %s %x %s",texName,x1,y1,font,color,text);
    call("gui::drawText",&ret,callStr);
    free(callStr);
    return 0;
}

int js_drawBox(duk_context *ctx){
    int ret;
    char *callStr;
    dschema_check(ctx, (const duv_schema_entry[]) {
       {"texture", duk_is_string},
       {"x", duk_is_number},
       {"y", duk_is_number},
       {"w", duk_is_number},
       {"h", duk_is_number},
       {"color", duk_is_number},
       {0,0}});
    const char *texName = duk_require_string(ctx, 0);
    const int x = duk_require_int(ctx, 1);
    const int y = duk_require_int(ctx, 2);
    const int w = duk_require_int(ctx, 3);
    const int h = duk_require_int(ctx, 4);
    const char *color = duk_require_int(ctx, 5);
    asprintf(&callStr, "%s %d %d %d %d %x",texName,x,y,w,h,color);
    call("gui::drawBox",&ret,callStr);
    free(callStr);
    return 0;
}

int js_drawFilledBox(duk_context *ctx){
    int ret;
    char *callStr;
    dschema_check(ctx, (const duv_schema_entry[]) {
       {"texture", duk_is_string},
       {"x", duk_is_number},
       {"y", duk_is_number},
       {"w", duk_is_number},
       {"h", duk_is_number},
       {"stroke", duk_is_number},
       {"color", duk_is_number},
       {"strokeColor", duk_is_number},
       {"alpha", duk_is_number},
       {0,0}});
    const char *texName = duk_require_string(ctx, 0);
    const int x = duk_require_int(ctx, 1);
    const int y = duk_require_int(ctx, 2);
    const int w = duk_require_int(ctx, 3);
    const int h = duk_require_int(ctx, 4);
    const int stroke = duk_require_int(ctx, 5);
    const int color = duk_require_int(ctx, 6);
    const int strokeCol = duk_require_int(ctx, 7);
    const int alpha = duk_require_int(ctx, 8);
    asprintf(&callStr, "%s %d %d %d %d %d %x %x %d",texName,x,y,w,h,stroke,color, strokeCol, alpha);
    call("gui::drawFilledBox",&ret,callStr);
    free(callStr);
    return 0;
}

void c_drawLine(char *str)
{
    SDL_Color col;
//    dschema_check(ctx, (const duv_schema_entry[]) {
//       {"texture", duk_is_string},
//       {"x1", duk_is_number},
//       {"y1", duk_is_number},
//       {"x2", duk_is_number},
//       {"y2", duk_is_number},
//       {"color", duk_is_number},
//       {0,0}});
    const char *texName= strsep(&str, " \t");
    const char *x1Str  = strsep(&str, " \t");
    const char *y1Str  = strsep(&str, " \t");
    const char *x2Str  = strsep(&str, " \t");
    const char *y2Str  = strsep(&str, " \t");
    const char *cStr  = strsep(&str, " \t");
    int x1=atoi(x1Str);
    int x2=atoi(x2Str);
    int y1=atoi(y1Str);
    int y2=atoi(y2Str);
    int color=strtol(cStr,0,16);
    hexToColor(color, &col);
    slog(TRACE,DEBUG, "c_drawLine %s %d %d %d %d {%d,%d,%d}",texName,x1,y1,x2,y2,col.r, col.g,col.b);
    drawLine(texName, x1, y1, x2, y2, col);
}

void c_drawFilledBox(char *tmpStr)
{
    SDL_Color col;
    SDL_Color scol;
    char *str = strdup(tmpStr); 
    char *texName= strsep(&str, " \t");
    char *x1Str  = strsep(&str, " \t");
    char *y1Str  = strsep(&str, " \t");
    char *wStr   = strsep(&str, " \t");
    char *hStr   = strsep(&str, " \t");
    char *strokeStr = strsep(&str, " \t");
    char *colStr    = strsep(&str, " \t");
    char *strColStr = strsep(&str, " \t");
    char *alphaStr  = strsep(&str, " \t");
    if(!texName || !x1Str || !y1Str || !wStr || !hStr || !strokeStr || !alphaStr){
    	slog(TRACE,DEBUG, "c_drawFilledBox parse error : %s",str);
	return;
    }
    int x1=atoi(x1Str);
    int y1=atoi(y1Str);
    int w=atoi(wStr);
    int h=atoi(hStr);
    SDL_Rect r = {x1, y1, w, h};
    int s=atoi(strokeStr);
    int a=atoi(alphaStr);
    hexToColor(strtol(colStr,NULL,16), &col);
    hexToColor(strtol(strColStr,NULL,16), &scol);
    slog(TRACE,DEBUG, "c_drawFilledBox %s {%d,%d,%d,%d} s:%d {%d,%d,%d} {%d,%d,%d} a:%d",texName,x1,y1,w,h,s,
	col.r,col.g,col.b, scol.r,scol.g,scol.b, a);
    drawFilledBox(texName, r, s, col, scol, a);
}


void c_drawBox(char *str)
{
    SDL_Color col;
    const char *texName= strsep(&str, " \t");
    const char *x1Str  = strsep(&str, " \t");
    const char *y1Str  = strsep(&str, " \t");
    const char *wStr   = strsep(&str, " \t");
    const char *hStr   = strsep(&str, " \t");
    const char *colStr = strsep(&str, " \t");
    int x1=atoi(x1Str);
    int y1=atoi(y1Str);
    int w=atoi(wStr);
    int h=atoi(hStr);
    hexToColor(strtol(colStr,NULL,16), &col);
    slog(TRACE,DEBUG, "c_drawBox %s %d %d %d %d {%d,%d,%d}",texName,x1,y1,w,h,col.r,col.g,col.b);
    drawBox(texName, x1, y1, w, h, col);
}

void c_drawGradient(char *str)
{
    SDL_Color fromcol,tocol;
    const char *texName= strsep(&str, " \t");
    const char *x1Str  = strsep(&str, " \t");
    const char *y1Str  = strsep(&str, " \t");
    const char *x2Str  = strsep(&str, " \t");
    const char *y2Str  = strsep(&str, " \t");
    const char *fcolStr = strsep(&str, " \t");
    const char *tcolStr = strsep(&str, " \t");
    const char *horStr = strsep(&str, " \t");
    const char *holStr = strsep(&str, " \t");
    int x1=atoi(x1Str);
    int y1=atoi(y1Str);
    int x2=atoi(x2Str);
    int y2=atoi(y2Str);
    SDL_Rect r = { x1, y1, x2, y2 };
    int hor=atoi(horStr);
    int hol=atoi(holStr);
    hexToColor(strtol(fcolStr,NULL,16), &fromcol);
    hexToColor(strtol(tcolStr,NULL,16), &tocol);
    slog(TRACE,DEBUG, "c_drawGradient %s {%d,%d,%d,%d} {...}{...} %d %d",texName,x1,y1,x2,y2,!hor, hol);
    drawGradient(texName, r, fromcol,tocol,!hor,hol);
    renderActive(texName);
}

bool c_drawText(char *str)
{
  SDL_Color col;
  const char *texName= strsep(&str, " \t");
  const char *x1Str  = strsep(&str, " \t");
  const char *y1Str  = strsep(&str, " \t");
  const char *fontStr  = strsep(&str, " \t");
  const char *colStr  = strsep(&str, " \t");
  int x=atoi(x1Str);
  int y=atoi(y1Str);
 
  texInfo *TI = getTexture(texName);
  TTF_Font *font = ht_get_simple(ph->fonts,fontStr);
  if(!font){
     slog(LVL_QUIET,ERROR,"Font named '%s' not loaded",fontStr);
     return false;
  }
  hexToColor(strtol(colStr,NULL,16), &col);
  SDL_Surface *surf = TTF_RenderUTF8_Blended( font, str, col );
  if(!surf) {
     slog(LVL_QUIET,ERROR,"Could not create FontSurface : %s",SDL_GetError());
     return 1;
  }
  SDL_Texture *texture = SDL_CreateTextureFromSurface( ph->renderer, surf );
  ph->textureCount++;

  SDL_Rect dest = {x,y,0,0};

  SDL_QueryTexture( texture, NULL, NULL, &dest.w, &dest.h );

//  addToTexture(TI, texture, &dest);
  SDL_RenderCopy( ph->renderer, texture, NULL, &dest );

  //renderActive(NULL);
  SDL_FreeSurface(surf);
  SDL_DestroyTexture(texture);
  ph->textureCount--;
  return 1;
}

c_loadImageFile(char *str)
{
   const char *texName= strsep(&str, " \t");
   const char *fileName= strsep(&str, " \t");
   const char *xStr  = strsep(&str, " \t");
   const char *yStr  = strsep(&str, " \t");
   const char *wStr  = strsep(&str, " \t");
   const char *hStr  = strsep(&str, " \t");
   const char *aStr  = strsep(&str, " \t");
   int x=atoi(xStr);
   int y=atoi(yStr);
   int w=atoi(wStr);
   int h=atoi(hStr);
   int a=atoi(aStr);
 
   texInfo *TI = getTexture(texName);
   if(!TI) { return false; }
   slog(TRACE,FULLDEBUG,"File : %s",fileName);
   if(fileName == NULL){
      slog(LVL_QUIET,ERROR,"Wrong parameters setImageScale(name,file) (%s,%s).",TI->name,fileName);
      return false;
   }

   SDL_Rect srect = { 0,0,0,0 };
   slog(DEBUG,DEBUG,"Loading image %s",fileName);
   SDL_Texture *text = IMG_LoadTexture(ph->renderer,fileName);
   ph->textureCount++;
   if(text == NULL){
      slog(LVL_QUIET,ERROR, "Error loading image : %s",IMG_GetError());
      return false;
   }
   SDL_QueryTexture( text, NULL, NULL, &srect.w, &srect.h );
   SDL_Rect rect = {x,y,w,h};
   //SDL_SetTextureBlendMode(TI->texture, SDL_BLENDMODE_BLEND);
//   SDL_SetRenderTarget( ph->renderer, TI->texture );
   SDL_RenderCopy( ph->renderer, text, &srect, &rect);
//   SDL_SetRenderTarget( ph->renderer,NULL );
//   renderActive(TI->name);
   SDL_DestroyTexture(text);
   ph->textureCount--;
   return true;
}

void setTargetTexture(char *textureName){
    if(textureName){
      slog(TRACE,DEBUG,"setTarget %s",textureName);
      texInfo *TI = getTexture(textureName);
      SDL_SetRenderTarget(ph->renderer,TI->texture);
      SDL_SetTextureBlendMode(TI->texture, SDL_BLENDMODE_BLEND);
    } else {
      slog(TRACE,DEBUG,"setTarget NULL");
      SDL_SetRenderTarget(ph->renderer,NULL);
    }
}

void drawLine(char *textureName, int x1, int y1, int x2, int y2, SDL_Color col) {
//    slog(DEBUG,DEBUG,"drawLine %s",textureName);
//    texInfo *TI = getTexture(textureName);
//    if(!textureName || !TI){
//        slog(LVL_QUIET,ERROR,"Texture %s not found.",textureName);
//        return;
//   }
//    SDL_SetRenderTarget(ph->renderer,TI->texture);
    SDL_SetRenderDrawColor(ph->renderer,col.r,col.g,col.b,0xff);
    SDL_RenderDrawLine(ph->renderer, x1, y1, x2, y2 );
//    SDL_SetRenderTarget(ph->renderer,NULL);
//    renderActive(textureName);
}

void drawBox(char *textureName, int x, int y, int w, int h, SDL_Color c) {
   slog(TRACE,DEBUG,"drawBox %s",textureName);
//   texInfo *TI = getTexture(textureName);
//   SDL_SetRenderTarget(ph->renderer,TI->texture);
   SDL_Rect r= { x, y, w, h };
   SDL_SetRenderDrawColor(ph->renderer,c.r,c.g,c.b,0xff);
   SDL_RenderFillRect(ph->renderer,&r);
//   SDL_SetRenderTarget(ph->renderer,NULL);
//   renderActive(textureName);
}

void hLine(char *textureName,int x, int y, int w, SDL_Color c)
{
	drawLine(textureName, x, y, x+w, y, c);
}
void vLine(char *textureName,int x, int y, int h, SDL_Color c)
{
	drawLine(textureName, x, y, x, y+h, c);
}

void drawFilledBox(char *textureName, SDL_Rect rect, int stroke, SDL_Color col, SDL_Color strokeCol, int alpha) {
    //SDL_SetRenderDrawColor(ph->renderer,strokeCol.r,strokeCol.g,strokeCol.b,0xff);
    slog(TRACE,DEBUG,"drawFilledBox %s Alpha:%d",textureName,alpha);
//    texInfo *TI = getTexture(textureName);
//    SDL_SetRenderTarget(ph->renderer,TI->texture);
    drawBox(textureName, rect.x,rect.y, rect.w, stroke, strokeCol); 		// top
    drawBox(textureName, rect.x,rect.y, stroke, rect.h, strokeCol);	 	// left
    drawBox(textureName, rect.x+rect.w-stroke, rect.y, stroke, rect.h, strokeCol); 	// right
    drawBox(textureName, rect.x,rect.y+rect.h-stroke, rect.w, stroke, strokeCol); 	// bottom
    SDL_SetRenderDrawColor(ph->renderer,col.r,col.g,col.b,alpha);
    SDL_Rect r= { rect.x+stroke, rect.y+stroke, rect.w-2*stroke, rect.h-2*stroke };
    SDL_RenderFillRect(ph->renderer,&r);
//    SDL_SetRenderTarget(ph->renderer,NULL);
//    renderActive(textureName);
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


void drawGradient(char *textureName, SDL_Rect rect,const SDL_Color col1, SDL_Color col2,bool vertical,bool hollow) {
  SDL_Color col;
  SDL_Rect trect = {0,0,0,0};
  SDL_Color from = hollow ? col1 : col2;
  SDL_Color to   = hollow ? col2 : col1;
  float fr = (float)(from.r-to.r);
  float fg = (float)(from.g-to.g);
  float fb = (float)(from.b-to.b);
  float maxrun = fabs(fmax(fmax(fr,fg),fb));
  float minrun = fabs(fmin(fmin(fr,fg),fb));
  int run = (int)fmax(fabs(maxrun),fabs(minrun));
  float steps = vertical ? (float)(rect.w-rect.y)/run : (float)(rect.h-rect.x)/run;
  int isteps = (int)ceil(steps);
  //float rstep = from->r > to->r ? abs((float)(from->r-to->r)/run) : abs((float)(from->r-to->r)/run);
  //float gstep = from->g > to->g ? abs((float)(from->g-to->g)/run) : abs((float)(from->g-to->g)/run);
  //float bstep = from->b > to->b ? abs((float)(from->b-to->b)/run) : abs((float)(from->b-to->b)/run);
  int rbase = from.r;
  int gbase = from.g;
  int bbase = from.b;
  int rdir = from.r > to.r ? -1 : 1;
  int gdir = from.g > to.g ? -1 : 1;
  int bdir = from.b > to.b ? -1 : 1;
  float rstep = fabs(fr/run)*rdir;
  float gstep = fabs(fg/run)*gdir;
  float bstep = fabs(fb/run)*bdir;
  slog(TRACE,DEBUG,"Rect {%d,%d,%d,%d}",rect.x,rect.y,rect.w,rect.h);
  slog(TRACE,DEBUG,"Run %d, Steps %f, FMAX:%f, FMIN:%f r:%f,g:%f,b:%f",run,steps, maxrun,minrun,rstep, gstep, bstep);
  for (int i=0; i <= run; ++i) {
    int r = rbase+i*rstep;
    int g = gbase+i*gstep;
    int b = bbase+i*bstep;
    if (vertical){
      trect.x = rect.x+(int)ceil(i*steps); trect.y = rect.y; trect.w = isteps; trect.h = rect.h;
    } else {
      trect.x = rect.x; trect.y = rect.y+(int)ceil(i*steps); trect.w = rect.w; trect.h = isteps;
    }
    //slog(TRACE,DEBUG,"Step : {%d,%d,%d,%d} {%d,%d,%d}",trect.x,trect.y,trect.w,trect.h,r,g,b);
    SDL_SetRenderDrawColor(ph->renderer,r,g,b,0xff);
    SDL_RenderFillRect(ph->renderer,&trect);
  }
}

void drawButtonTest(char *textureName){
   SDL_Rect r =  { 50,10,100,30 };
   SDL_Rect r2 =  { 300,10,100,100 };
   SDL_Rect r3 =  { 500,300,100,100 };
   texInfo *TI = getTexture(textureName);
   SDL_SetRenderTarget(ph->renderer,TI->texture);
   drawButton(textureName, &r , red);
   drawGradient(textureName, r2 , red,white,true,true);
   drawFilledBox(textureName, r3, 8, white, black, 200);
   SDL_SetRenderTarget(ph->renderer,NULL);
   renderActive(textureName);
}

char *cleanupPlugin(void *p){
   slog(DEBUG,DEBUG,"Cleaning up plugin "PLUGIN_SCOPE);
   return NULL;
}

duk_ret_t js_gui_ctor(duk_context *ctx)
{
    slog(DEBUG,DEBUG, "Getting access to gui object.");

    duk_push_this(ctx);
    duk_dup(ctx, 0);  /* -> stack: [ name this name ] */
    duk_put_prop_string(ctx, -2, "name");  /* -> stack: [ name this ] */
    return 1;
}

// TODO : set correct number of args
const function_list_entry c_SDLMethods[] = {
   {  PLUGIN_SCOPE"::drawText"       ,WFUNC_THRD, c_drawText  ,6},
   {  PLUGIN_SCOPE"::drawBox"        ,WFUNC_THRD, c_drawBox   ,6},
   {  PLUGIN_SCOPE"::drawFilledBox"  ,WFUNC_THRD, c_drawFilledBox   ,9},
   {  PLUGIN_SCOPE"::drawLine"       ,WFUNC_THRD, c_drawLine  ,6},
   {  PLUGIN_SCOPE"::drawGradient"   ,WFUNC_THRD, c_drawGradient ,10},
   {  PLUGIN_SCOPE"::loadImageFile"  ,WFUNC_THRD, c_loadImageFile ,7},
   {  PLUGIN_SCOPE"::setTargetTexture",WFUNC_THRD, setTargetTexture ,1},
//   {  PLUGIN_SCOPE"::loadImageMemory",WFUNC_THRD, c_loadImageMemory ,7},
   {  NULL, 0, NULL, 0}
}; 

const duk_function_list_entry js_guiMethods[] = {
     //{  "drawBoxEx"       , js_drawBoxEx,7},
     {  "drawFilledBox"     , js_drawFilledBox,9},
     {  "drawBox"           , js_drawBox,6},
     {  "drawLine"          , js_drawLine,6},
     {  "loadImage"         , js_loadImageFile,7},
     {  "drawText"          , js_drawText,6},
     {  "putImage"          , js_putImage,7},
     {  "clearTexture"      , js_clearTexture,1},
     {  "clearTextureNoPaint" , js_clearTextureNoPaint,1},
     {  "drawGradient"      , js_drawGradient,9},
     {  "setTargetTexture"  , js_setTargetTexture,1},
     {  "resetTargetTexture", js_resetTargetTexture,0},
     { NULL,    NULL,            0 }
};

char *initPlugin(pluginHandler *_ph){
   ph=_ph;
   slog(DEBUG,DEBUG, "Plugin "PLUGIN_SCOPE" initializing, ph is at 0x%x, renderer at 0x%x",ph, ph->renderer);
   wally_put_function_list(c_SDLMethods);

   duk_push_c_function(ph->ctx, js_gui_ctor, 0 );
   duk_push_object(ph->ctx);
   duk_put_function_list(ph->ctx, -1, js_guiMethods);
   duk_put_prop_string(ph->ctx, -2, "prototype");
   duk_put_global_string(ph->ctx, "GUI");  /* -> stack: [ ] */

   slog(DEBUG,FULLDEBUG,"Plugin "PLUGIN_SCOPE" initialized. PH is at 0x%x",ph);
   return PLUGIN_SCOPE;
}
