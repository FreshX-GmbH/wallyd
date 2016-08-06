#include <duktape.h>
#include "sdlffvideo.h"

#define PLUGIN_SCOPE "ffvideo"

bool rendering = false;
int loop;

const duk_function_list_entry videoMethods[];

int video_refresh_timer(VideoState *is){
    video_refresh(is);
    is->refresh = 0;
    return 0;
}

int video_open(VideoState *is, int force_set_video_mode)
{
    int flags = 0;
    int w,h;
    VideoPicture *vp = &is->pictq[is->pictq_rindex];

    is->width  = is->video_st->codec->width;
    is->height = is->video_st->codec->height;
    loop = 100;

    return 0;
}

void *createTextureCallback(VideoState *is){
    slog(LVL_ALL,DEBUG,"Creating Video texture with is : 0x%x",is);
    
    texInfo *TI = is->TI;
    if(TI->texture){
        SDL_DestroyTexture(TI->texture);
        ph->textureCount--;
    }
    slog(LVL_ALL,DEBUG,"Creating video texture for %s, size %dx%d",TI->name, is->width, is->height);
    TI->texture = SDL_CreateTexture( ph->renderer, SDL_PIXELFORMAT_YV12, SDL_TEXTUREACCESS_STREAMING, is->width, is->height );
    ph->textureCount++;
    return TI->texture;
}

void displayTextureCallback(VideoState *is, void *p){
    slog(LVL_NOISY,DEBUG,"Render IImage");
    if(rendering == true) return;
    rendering = true;
    texInfo *TI = is->TI;
    TI->texture = p;
    renderActive(TI->name);
    rendering = false;
}

int renderVideo(char *str)
{
    int ret = 0;
    char *brk;
    char *screen = strtok_r(str, " ", &brk);
    char *file = strtok_r(NULL, " ",&brk);
    if(!file){
       slog(LVL_QUIET,ERROR,"Wrong parameters for play(name,url) : (%s)",str);
      return -1;
    }
    slog(LVL_NOISY,DEBUG, "FFPlugin going to play on screen %s : %s\n",screen, file);

    VideoObject *vo = getVideoObject(ph->ctx);

    texInfo *TI = ht_get_simple(ph->baseTextures,screen);
    if(!TI) {
      slog(LVL_QUIET,ERROR,"Taget texture %s not found.",screen);
      return -1;
    }
    vo->TI = ht_get_simple(ph->baseTextures,screen);

    ph->playVideo = true;

    setRenderer(ph->renderer);
    setDisplayTextureCallback(displayTextureCallback);
    setCreateTextureCallback(createTextureCallback);

    vo->is = renderFFVideo(file);
    vo->is->TI = TI;
    vo->is->VO = vo;
    slog(LVL_NOISY,DEBUG, "FFVideo Object : 0x%x, is : 0x%x",vo,vo->is);
    return ret;
}

int setVideoTexture(char *str)
{
   char *name = strtok(str, " ");
   if(!name){
      slog(LVL_QUIET,ERROR,"Wrong paramters for setVideoTexture(name) : %s",str);
      return false;
   }
   slog(LVL_NOISY,DEBUG,"Video output points now to texture %s and is set active",name);
   return 0;
}

char *cleanupPlugin(void *p){
   int status;
   // TODO : Cleanup FFPlayer and kill threads by signal
 //  stopVideo(NULL);

 //  slog(LVL_INFO,INFO,"Cleaning up video plugin. Waiting for threads to finish.");
 //  SDL_WaitThread(is->parse_tid,&status);
  // slog(LVL_NOISY,DEBUG,"Thread 1/2 finished.");
  // SDL_CondSignal(is->pictq_cond);
  // SDL_WaitThread(is->video_tid,&status);
 //  slog(LVL_NOISY,DEBUG,"Thread 2/2 finished.");
 //  if(is) free(is);
   return NULL;
}

char *resetVideo(char *p){
   cleanupPlugin(p);
   //if(initVariables() == false){
   //   slog(LVL_INFO,WARN,"Could not (re)init "PLUGIN_SCOPE);
   //  return NULL;
   //}
   return "true";
}

int js_play(duk_context *ctx)
{
    //video::play main http://VIDEOURL
    int ret;
    char *cs;
    int n = duk_get_top(ctx);
    const char *name = duk_to_string(ctx,0);
    const char *url = duk_to_string(ctx,1);
    asprintf(&cs,"%s %s",name,url);
    call("ffvideo::play",&ret,cs);
    // TODO : make copy of the string IN the function utilizing it, not here
    free(cs);
    return 1;
}

duk_ret_t js_info(duk_context *ctx)
{
   duk_push_this(ctx);  /* -> stack: [ this ] */
   duk_get_prop_string(ctx, -1, "name");
   const char *name = duk_safe_to_string(ctx, -1);
   duk_pop(ctx);
   duk_push_sprintf(ctx, "{ id : %s, width: %d, height: %d }",name,10,10);
   return 1;
}

duk_ret_t js_video_dtor(duk_context *ctx)
{
   free(getVideoObject(ctx));
   slog(LVL_NOISY,DEBUG, "Video object destroyed.");
   return 0;
}

duk_ret_t js_video_ctor(duk_context *ctx)
{
    slog(LVL_NOISY,DEBUG, "New video object created.");

    duk_push_this(ctx);
    duk_dup(ctx, 0);  /* -> stack: [ name this name ] */
    duk_put_prop_string(ctx, -2, "name");  /* -> stack: [ name this ] */

    // Create a new VideoObject and bind it to the JS Object
    VideoObject *vo = malloc(sizeof(VideoObject));
    memset(vo,0,sizeof(VideoObject));

    slog(LVL_ALL,DEBUG,"VideoOject located at : 0x%x",vo);

    // This is a hidden property
    duk_push_pointer(ctx, vo);
    duk_put_prop_string(ctx, -2, "\xff""\xff""vo");

    duk_push_this(ctx);  /* -> stack: [ this ] */
    duk_get_prop_string(ctx, -1, "name");
    const char *name = duk_safe_to_string(ctx, -1);
    duk_pop(ctx);
    duk_push_sprintf(ctx, "{ id : %s, width: %d, height: %d }",name,10,10);
    return 1;
}

VideoObject *getVideoObject(duk_context *ctx) {
   duk_push_this(ctx);  /* -> stack: [ this ] */
   duk_get_prop_string(ctx, -1, "\xff""\xff""vo");
   return duk_to_pointer(ctx, -1);
}

/* Initialize Video object into global object. */
void js_video_init(duk_context *ctx) {
   slog(LVL_NOISY,FULLDEBUG,"Constructing video object");

   duk_push_c_function(ctx, js_video_ctor, 5 );
   duk_push_object(ctx);  
   duk_put_function_list(ctx, -1, videoMethods);
   duk_put_prop_string(ctx, -2, "prototype");
   duk_put_global_string(ctx, "FFVideo");  /* -> stack: [ ] */
}

// Video object methods
const duk_function_list_entry videoMethods[] = {
    { "play",         js_play, 2 },
    { "info",         js_info, 0 },
    { NULL,           NULL,        0 }
};

const function_list_entry c_videoMethods[] = {
    {  PLUGIN_SCOPE"::alloc_picture", WFUNC_THRD, alloc_picture, 0 },
    {  PLUGIN_SCOPE"::refresh_timer", WFUNC_THRD, video_refresh_timer,0 },
    {  PLUGIN_SCOPE"::play"         , WFUNC_SYNC, renderVideo, 0  },
    {  PLUGIN_SCOPE"::reset"        , WFUNC_SYNC, resetVideo,  0  },
    {  NULL, 0, NULL, 0 }
};
 
char *initPlugin(pluginHandler *_ph){
    ph = _ph;
    ph->disableVideoAfterFinish = true;
    duk_context *ctx = ph->ctx;

    wally_put_function_list(c_videoMethods);
    js_video_init(ctx);

    slog(LVL_NOISY,FULLDEBUG,"Plugin video initialized. PH is at 0x%x",ph);

    return PLUGIN_SCOPE;
}
