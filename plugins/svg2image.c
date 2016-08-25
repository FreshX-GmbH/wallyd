//	Simple SVG plugin, based on nanosvg
#include "svg2image.h"

#define PLUGIN_SCOPE "svg"

NSVGimage *image = NULL;
NSVGrasterizer *rast = NULL;
void *svgToImage(char *,long *);

int js_svgToImage(duk_context *ctx){
    SDL_Color c;
    long size = 0;
    char *callStr;
    const char *str = duk_require_string(ctx, 0);
    char *svg;
    svg = strdup(str);
    void *image = svgToImage(svg,&size);
    free(svg);
    if(image == NULL){
      slog(LVL_QUIET,ERROR,"Failed to parse svg to IMG");
      return 0;
    } else {
      slog(DEBUG,DEBUG,"Parsed svg to IMG, size %d",size);
      void *p = duk_push_buffer(ctx, size, 0);
      memcpy(p,image,size);
      nsvgDelete(image);
      return 1;
    }
}

int js_svgToTex(duk_context *ctx){
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
    call("svg::svgToTex",&ret,callStr);
    free(callStr);
    return 0;
}

duk_ret_t js_svgToPng(duk_context *ctx){
    SDL_Color c;
    int ret;
    char *callStr;
    const char *fileName = duk_require_string(ctx, 0);
    const char *pngName  = duk_require_string(ctx, 1);
    asprintf(&callStr, "%s %s",fileName,pngName);
    call("svg::svgToPng",&ret,callStr);
    free(callStr);
    return 0;
}

int js_freeImage(duk_context *ctx){
    if(image) {
        nsvgDelete(image);
    }
    return 0;
}

int freeImage(char *empty){
    if(image) {
        nsvgDelete(image);
    }
    return 0;
}

//  TODO : in memory images system
int svgToTex(char *str)
{
    char *imgPtr;
    char *texName= strsep(&str, " \t");
    const char *fileName= strsep(&str, " \t");
    const char *xStr  = strsep(&str, " \t");
    const char *yStr  = strsep(&str, " \t");
    const char *wStr  = strsep(&str, " \t");
    const char *hStr  = strsep(&str, " \t");
    const char *aStr  = strsep(&str, " \t");
    int x=atoi(xStr);
    int y=atoi(yStr);
    int dw=atoi(wStr);
    int dh=atoi(hStr);
    int a=atoi(aStr);

    texInfo *TI = getTexture(texName);
    if(!TI) { return false; }

    unsigned char* img = NULL;
    int w, h;
    
    slog(DEBUG,DEBUG,"parsing %s", fileName);
    image = nsvgParseFromFile(fileName, "px", 96.0f);
    if (image == NULL) {
    	slog(DEBUG,DEBUG,"Could not open SVG image.");
    	goto error;
    }
    w = (int)image->width;
    h = (int)image->height;
    
    rast = nsvgCreateRasterizer();
    if (rast == NULL) {
    	slog(DEBUG,DEBUG,"Could not init rasterizer.");
    	goto error;
    }
    
    img = malloc(w*h*4);
    if (img == NULL) {
    	slog(DEBUG,DEBUG,"Could not alloc image buffer.");
    	goto error;
    }
    
    slog(DEBUG,DEBUG,"rasterizing image %d x %d\n", w, h);
    nsvgRasterize(rast, image, 0,0,1, img, w, h, w*4);
    SDL_Surface *surf = SDL_CreateRGBSurfaceFrom(img, //pointer to the pixels
            w, //Width
            h, //Height
            32, //Depth (bits per pixel)
            w * 4, //Pitch (width*depth_in_bytes, in this case)
            0x000000FF, //Red mask
            0x0000FF00, //Green mask
            0x00FF0000, //Blue mask
            0xFF000000); //Alpha mask (alpha in this format)

    SDL_Texture *texture = SDL_CreateTextureFromSurface( ph->renderer, surf );
    ph->textureCount++;

    SDL_Rect dest =  {x,y,dw,dh};
    SDL_Rect srect = {0,0,0,0};
    SDL_QueryTexture( texture, NULL, NULL, &srect.w, &srect.h );
    SDL_RenderCopy( ph->renderer, texture, &srect, &dest );
    SDL_FreeSurface( surf );
    SDL_DestroyTexture(texture);
    ph->textureCount--;
    
error:
    nsvgDeleteRasterizer(rast);
    nsvgDelete(image);

    return 0;
}

// Convert SVG into Image Buffer. The caller needs to 
// free the image after usage by calling freeImage
// returns pointer to the image buffer
void *svgToImage(char *str,long *size)
{
    unsigned char* img = NULL;
    int w, h;
    
    image = nsvgParse(str, "px", 96.0f);
    if (image == NULL) {
    	slog(LVL_QUIET,ERROR,"Could not parse SVG image.");
    	goto error;
    }
    w = (int)image->width;
    h = (int)image->height;
    
    rast = nsvgCreateRasterizer();
    if (rast == NULL) {
    	slog(LVL_QUIET,ERROR,"Could not init rasterizer.");
    	goto error;
    }
    
    img = malloc(w*h*4);
    if (img == NULL) {
    	slog(LVL_QUIET,ERROR,"Could not alloc image buffer.");
    	goto error;
    }
    
    *size = w*h*4;
    slog(LVL_QUIET,ERROR,"rasterizing image %d x %d", w, h);
    nsvgRasterize(rast, image, 0,0,1, img, w, h, w*4);
    
error:
    nsvgDeleteRasterizer(rast);
    return img;
}


int svgToPng(char *str)
{
    const char *fileName= strsep(&str, " \t");
    const char *pngName= strsep(&str, " \t");
    
    unsigned char* img = NULL;
    int w, h;
    
    slog(DEBUG,DEBUG,"parsing %s", fileName);
    image = nsvgParseFromFile(fileName, "px", 96.0f);
    if (image == NULL) {
    	slog(DEBUG,DEBUG,"Could not open SVG image.");
    	goto error;
    }
    w = (int)image->width;
    h = (int)image->height;
    
    rast = nsvgCreateRasterizer();
    if (rast == NULL) {
    	slog(DEBUG,DEBUG,"Could not init rasterizer.");
    	goto error;
    }
    
    img = malloc(w*h*4);
    if (img == NULL) {
        nsvgDeleteRasterizer(rast);
    	slog(DEBUG,DEBUG,"Could not alloc image buffer.");
    	goto error;
    }
    
    slog(DEBUG,DEBUG,"rasterizing image %d x %d", w, h);
    nsvgRasterize(rast, image, 0,0,1, img, w, h, w*4);
    
    slog(DEBUG,DEBUG,"writing %s",pngName);
    stbi_write_png(pngName, w, h, 4, img, w*4);
    
error:
    nsvgDeleteRasterizer(rast);
    nsvgDelete(image);
    
    return 0;
}

// TODO : set correct number of args
const function_list_entry c_SDLMethods[] = {
   {  PLUGIN_SCOPE"::freeImage"  ,WFUNC_THRD, freeImage   ,0},
   {  PLUGIN_SCOPE"::svgToTex"   ,WFUNC_THRD, svgToTex    ,7},
   {  PLUGIN_SCOPE"::svgToPng"   ,WFUNC_THRD, svgToPng    ,2},
   {  NULL, 0, NULL, 0}
};

const duk_function_list_entry js_svgMethods[] = {
   {  "freeImage"  , js_freeImage   ,0},
   {  "svgToTex"   , js_svgToTex    ,7},
   {  "svgToImage" , js_svgToImage  ,1},
   {  "svgToPng"   , js_svgToPng    ,2},
   { NULL,    NULL,            0 }
};

char *cleanupPlugin(void *p){
   slog(DEBUG,DEBUG,"Cleaning up plugin "PLUGIN_SCOPE);
   return NULL;
}

duk_ret_t js_svg_ctor(duk_context *ctx)
{
    slog(DEBUG,DEBUG, "Getting access to SVG object.");

    duk_push_this(ctx);
    duk_dup(ctx, 0);  /* -> stack: [ name this name ] */
    duk_put_prop_string(ctx, -2, "name");  /* -> stack: [ name this ] */
    return 1;
}

char *initPlugin(pluginHandler *_ph){
   ph=_ph;
   slog(DEBUG,DEBUG, "Plugin "PLUGIN_SCOPE" initializing, ph is at 0x%x, renderer at 0x%x",ph, ph->renderer);
   wally_put_function_list(ph,c_SDLMethods);

   duk_push_c_function(ph->ctx, js_svg_ctor, 0 );
   duk_push_object(ph->ctx);
   duk_put_function_list(ph->ctx, -1, js_svgMethods);
   duk_put_prop_string(ph->ctx, -2, "prototype");
   duk_put_global_string(ph->ctx, "SVG");  /* -> stack: [ ] */

   slog(DEBUG,DEBUG,"Plugin "PLUGIN_SCOPE" initialized. PH is at 0x%x",ph);
   return PLUGIN_SCOPE;
}


