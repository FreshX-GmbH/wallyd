#include "sdlvideo.h"
#include <duktape.h>

#define PLUGIN_SCOPE "video"

const duk_function_list_entry videoMethods[];

int renderVideoEx(char * screen, char *file)
{
    int ret = 0;
    slog(DEBUG,DEBUG, "Going to play on screen %s : %s\n",screen, file);

    ph->playVideo = true;

    is->filename = av_malloc(strlen(file)+1);
    slog(DEBUG,DEBUG,"Screen name : 0x%x",is->screen);
    is->screen = av_malloc(strlen(screen)+1);

    av_strlcpy(is->filename, file, strlen(file)+1);
    av_strlcpy(is->screen, screen, strlen(screen)+1);
    slog(DEBUG,DEBUG,"Screen name : 0x%x",is->screen);

    is->pictq_mutex = SDL_CreateMutex();
    is->pictq_cond = SDL_CreateCond();

    schedule_refresh(is, 40);

    is->av_sync_type = DEFAULT_AV_SYNC_TYPE;
    is->parse_tid = SDL_CreateThread(decode_thread, "parse_tid", is);
    if(!is->parse_tid) {
        slog(LVL_QUIET,ERROR,"Could not create video thread.");
        ret = -1;
        is->quit = true;
        ph->playVideo = false;
        av_free(is->screen);
        av_free(is->filename);
    }
    //av_free(is);

    return ret;
}

int renderVideo(char *str)
{
   char *brk;
   slog(DEBUG,DEBUG,"Video(%s)",str);
   char *screen = strtok_r(str, " ", &brk);

   texInfo *TI = ht_get_simple(ph->baseTextures,screen);
   if(!TI) {
     slog(LVL_QUIET,ERROR,"Texture %s not found.",screen);
     return -1;
   }
   is->TI = TI;

   char *filename = strtok_r(NULL, " ",&brk);
   if(!filename){
       slog(LVL_QUIET,ERROR,"Wrong parameters for play(name,url) : (%s)",str);
      return -1;
   }
   is->scaled = false;
   is->videoPosX = 0;
   is->videoPosY = 0;
   slog(DEBUG,DEBUG,"Playing to fullscreen at %s name %s",screen, filename);
   return renderVideoEx(screen,filename);
}

int renderVideoUnScaled(char *str)
{
   is->scaled = false;
   is->videoPosX = 0;
   is->videoPosY = 0;
   return renderVideo(str);
}

int renderVideoFullscreen(char *str)
{
   is->directRender = true;
   return renderVideo(str);
}

int renderVideoScaled(char *str)
{
   slog(DEBUG,DEBUG,"VideoScaled(%s)",str);
   int x=0, y=0, w=0, h=0;
   char *brk;
   char *screen = strtok_r(str, " ", &brk);

   texInfo *TI = ht_get_simple(ph->baseTextures,screen);
   if(!TI) {
     slog(LVL_QUIET,ERROR,"Texture %s not found.",screen);
     return -1;
   }
   is->TI = TI;
 
   if(!getNumOrPercent(strtok_r(NULL, " ",&brk), TI->rect->w, &x)){
       slog(LVL_QUIET,ERROR,"Wrong parameters for playVideoScaled(name x y w h location) : (%s)",str);
       return false;
   }
   if(!getNumOrPercent(strtok_r(NULL, " ",&brk), TI->rect->h, &y)){
       slog(LVL_QUIET,ERROR,"Wrong parameters for playVideoScaled(name x y w h location) : (%s)",str);
       return false;
   }
   if(!getNumOrPercent(strtok_r(NULL, " ",&brk), TI->rect->w, &w)){
       slog(LVL_QUIET,ERROR,"Wrong parameters for playVideoScaled(name x y w h location) : (%s)",str);
       return false;
   }
   if(!getNumOrPercent(strtok_r(NULL, " ",&brk), TI->rect->h, &h)){
       slog(LVL_QUIET,ERROR,"Wrong parameters for playVideoScaled(name x y w h location) : (%s)",str);
       return false;
   }
   char *filename = strtok_r(NULL, " ",&brk);
   if(!w || !h || !filename){
       slog(LVL_QUIET,ERROR,"Wrong parameters for playVideoScaled(name x y w h location) : (%s)",str);
      return -1;
   }
   slog(DEBUG,DEBUG,"Playing scaled to screen %s pos %d,%d size %dx%d name %s",screen, x,y,w,h,filename);
   is->scaled = true;
   is->scaledWidth = w;
   is->scaledHeight = h;
   is->videoPosX = x;
   is->videoPosY = y;
   //is->TI = TI;
   return renderVideoEx(screen,filename);
}

int renderVideoScaledDirect(char *str){
   is->directRender = true;
   return renderVideoScaled(str);
}

int pauseVideo(char *empty)
{
   return is->pause=true;
}

int unpauseVideo(char *empty)
{
   return is->pause=false;
}

int setVideoTexture(char *str)
{
   char *name = strtok(str, " ");
   if(!name){
      slog(LVL_QUIET,ERROR,"Wrong paramters for setVideoTexture(name) : %s",str);
      return false;
   }
   slog(DEBUG,DEBUG,"Video output points now to texture %s and is set active",name);
   return 0;
}

int stopVideo(char *empty)
{
   int i=0;
   SDL_CondSignal(is->audioq.cond);
   SDL_CondSignal(is->videoq.cond);
   slog(DEBUG,DEBUG,"Stop Video");
   ph->playVideo = false;
   is->quit = true;
   SDL_PauseAudio(1);
   SDL_CloseAudioDevice(dev);
   slog(DEBUG,DEBUG,"Stop Audio");
   // TODO : reset orig texture
   //if(ph->disableVideoAfterFinish){
   //   is->TI->active = false;
   //   !!! call does not exist / work anymore at this point
   //   call("screen::render",&i,NULL);
   //}
   return true;
}

char *cleanupPlugin(void *p){
   int status;

   stopVideo(NULL);

   slog(LVL_INFO,INFO,"Cleaning up video plugin. Waiting for threads to finish.");
   SDL_WaitThread(is->parse_tid,&status);
   slog(DEBUG,DEBUG,"Thread 1/2 finished.");
   SDL_CondSignal(is->pictq_cond);
   SDL_WaitThread(is->video_tid,&status);
   slog(DEBUG,DEBUG,"Thread 2/2 finished.");
   if(is) free(is);
   return NULL;
}

bool initVariables(){
    // allocate Videostate struct
    is = av_mallocz(sizeof(VideoState));
    if(!is) {
      slog(LVL_QUIET,ERROR,"Could not allocate video struct");
      return false;
    }
    is->hasAudio = true;
    is->scaled = false;
    is->newTexture=true;

    is->scaledWidth = ph->width;
    is->scaledHeight = ph->height;
    is->videoPosX = 0;
    is->videoPosY = 0;
    // render directly ignoring other textures (for fullscreen only)
    is->directRender = false;

    // Register all formats and codecs
    // TODO : register only needed stuff (for memory)
    av_register_all();
    avformat_network_init();

    return true;
}

char *disableTextureAfterFinish(void *p){
   if(strncmp(p,"true",4) == 0){
      slog(LVL_INFO,INFO,"disabling Video Texture when video has finished"); 
      ph->disableVideoAfterFinish = true;
      return NULL;
   }
   if(strncmp(p,"false",5) == 0){
      slog(LVL_INFO,INFO,"Leaving Video Texture when video has finished"); 
      ph->disableVideoAfterFinish = false;
      return NULL;
   }
   slog(LVL_QUIET,ERROR,"Usage : disableTextureAfterFinish(true|false)");
   return NULL;
}
char *resetVideo(void *p){
   cleanupPlugin(p);
   if(initVariables() == false){
      slog(LVL_INFO,WARN,"Could not (re)init "PLUGIN_SCOPE);
      return NULL;
   }
   return "true";
}

int js_playDirect(duk_context *ctx)
{
    //video::playScaled main 0 0 100% 100% http://VIDEOURL
    int ret;
    char *cs;
    int n = duk_get_top(ctx);
    const char *name = duk_to_string(ctx,0);
    const char *x   = duk_to_string(ctx,1);
    const char *y   = duk_to_string(ctx,2);
    const char *w   = duk_to_string(ctx,3);
    const char *h   = duk_to_string(ctx,4);
    const char *url = duk_to_string(ctx,5);
    asprintf(&cs,"%s %s %s %s %s %s",name,x,y,w,h,url);
    call("video::playScaledDirect",&ret,cs);
    // TODO : make copy of the string IN the function utilizing it, not here
    //free(cs);
    return 1;
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
    call("video::play",&ret,cs);
    // TODO : make copy of the string IN the function utilizing it, not here
    //free(cs);
    return 1;
}

int js_playScaled(duk_context *ctx)
{
    //video::playScaled main 0 0 100% 100% http://VIDEOURL
    int ret;
    char *cs;
    int n = duk_get_top(ctx);
    const char *name = duk_to_string(ctx,0);
    const char *x   = duk_to_string(ctx,1);
    const char *y   = duk_to_string(ctx,2);
    const char *w   = duk_to_string(ctx,3);
    const char *h   = duk_to_string(ctx,4);
    const char *url = duk_to_string(ctx,5);
    asprintf(&cs,"%s %s %s %s %s %s",name,x,y,w,h,url);
    call("video::playScaled",&ret,cs);
    // TODO : make copy of the string IN the function utilizing it, not here
    //free(cs);
    return 1;
}

duk_ret_t js_stop(duk_context *ctx)
{
   return stopVideo(NULL);
}

duk_ret_t js_pause(duk_context *ctx)
{
   return pauseVideo(NULL);
}

duk_ret_t js_unpause(duk_context *ctx)
{
   return unpauseVideo(NULL);
}

duk_ret_t js_video_dtor(duk_context *ctx)
{
   slog(DEBUG,DEBUG, "Video object destroyed.");
   return 0;
}

duk_ret_t js_video_ctor(duk_context *ctx)
{
    slog(DEBUG,DEBUG, "New video object created.");

    duk_push_this(ctx);
    duk_dup(ctx, 0);  /* -> stack: [ name this name ] */
    duk_put_prop_string(ctx, -2, "name");  /* -> stack: [ name this ] */

    return 0;
}

duk_ret_t js_video_print(duk_context *ctx) {
   duk_push_this(ctx);  /* -> stack: [ this ] */
   duk_get_prop_string(ctx, -1, "name");
   printf("Video object info: %s\n", duk_safe_to_string(ctx, -1));
   return 0;  /* no return value (= undefined) */
}

/* Initialize Video object into global object. */
void js_video_init(duk_context *ctx) {
   slog(DEBUG,DEBUG,"Constructing video object");

   duk_push_c_function(ctx, js_video_ctor, 5 );

   /* Push Video.prototype object. */
   duk_push_object(ctx);  

   duk_put_function_list(ctx, -1, videoMethods);
   duk_put_prop_string(ctx, -2, "prototype");
   /* Finally, register Video to the global object */
   duk_put_global_string(ctx, "Video");  /* -> stack: [ ] */
}

// Video object methods
const duk_function_list_entry videoMethods[] = {
    { "play",         js_play, 2 },
    { "playScaled",   js_playScaled,  6   },
    { "playDirect",   js_playDirect,  6   },
    { "pause",        js_pause,  0   },
    { "unpause",      js_unpause, 0},
    { "stop",         js_stop,  0     },
    { "print",        js_video_print,  0     },
    { NULL,           NULL,        0 }
};

const wally_function_list_entry c_videoMethods[] = {
   { PLUGIN_SCOPE"::alloc_picture"      ,WFUNC_THRD,alloc_picture       ,1},
   { PLUGIN_SCOPE"::video_refresh_timer",WFUNC_THRD,video_refresh_timer ,1},
   { PLUGIN_SCOPE"::playScaled"         ,WFUNC_SYNC,renderVideoScaled,1},
   { PLUGIN_SCOPE"::play"               ,WFUNC_SYNC,renderVideo     ,1},
   { PLUGIN_SCOPE"::pause"              ,WFUNC_SYNC,pauseVideo      ,1},
   { PLUGIN_SCOPE"::unpause"            ,WFUNC_SYNC,unpauseVideo    ,1},
   { PLUGIN_SCOPE"::stop"               ,WFUNC_SYNC,stopVideo       ,1},
   { PLUGIN_SCOPE"::reset"              ,WFUNC_SYNC,resetVideo      ,1},
   { PLUGIN_SCOPE"::playFullscreen"     ,WFUNC_SYNC,renderVideoFullscreen,1},
   { PLUGIN_SCOPE"::playScaledDirect"   ,WFUNC_SYNC,renderVideoScaledDirect,1},
   { PLUGIN_SCOPE"::disableTextureAfterFinish",WFUNC_SYNC,disableTextureAfterFinish,1},
 //  PLUGIN_SCOPE"::setDestinationTexture",&setVideoTexture},
   { NULL, 0, NULL, 0}
};
 

char *initPlugin(pluginHandler *_ph){
    ph = _ph;
    pluginMode = true;
    ph->disableVideoAfterFinish = true;
    duk_context *ctx = ph->ctx;

    if(initVariables() == false){
      slog(LVL_INFO,WARN,"Could not init "PLUGIN_SCOPE);
      return NULL;
    }

    // Create Video Object
    js_video_init(ctx);

    //wally_put_function_list(ph,c_videoMethods);

    wally_put_function(PLUGIN_SCOPE"::alloc_picture"      ,WFUNC_THRD,alloc_picture       ,1   );
    wally_put_function(PLUGIN_SCOPE"::video_refresh_timer",WFUNC_THRD,video_refresh_timer ,1   );
    wally_put_function(PLUGIN_SCOPE"::playScaled"         ,WFUNC_SYNC,renderVideoScaled,1      );
    wally_put_function(PLUGIN_SCOPE"::play"               ,WFUNC_SYNC,renderVideo     ,1       );
    wally_put_function(PLUGIN_SCOPE"::pause"              ,WFUNC_SYNC,pauseVideo      ,1       );
    wally_put_function(PLUGIN_SCOPE"::unpause"            ,WFUNC_SYNC,unpauseVideo    ,1       );
    wally_put_function(PLUGIN_SCOPE"::stop"               ,WFUNC_SYNC,stopVideo       ,1       );
    wally_put_function(PLUGIN_SCOPE"::reset"              ,WFUNC_SYNC,resetVideo      ,1       );
    wally_put_function(PLUGIN_SCOPE"::playFullscreen"     ,WFUNC_SYNC,renderVideoFullscreen,  1);
    wally_put_function(PLUGIN_SCOPE"::playScaledDirect"   ,WFUNC_SYNC,renderVideoScaledDirect,1);

    slog(DEBUG,DEBUG,"Plugin video initialized. PH is at 0x%x",ph);

    return PLUGIN_SCOPE;
}
