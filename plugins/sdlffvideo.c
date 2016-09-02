#include <duktape.h>
#include <duv.h>
#include "sdlffvideo.h"

#define PLUGIN_SCOPE "ffvideo"

bool rendering = false;
int loop;
int finishCallback;

#define KEYLEN sizeof(void*)*2+1

const duk_function_list_entry videoMethods[];

int video_refresh_timer(VideoState *is){
    if(!is) return 0;
    video_refresh(is);
    is->refresh = 0;
    is->videoRefreshTimer--;
    return 0;
}

int video_open(VideoState *is, int force_set_video_mode)
{
    int flags = 0;
    int w,h;
    VideoPicture *vp = &is->pictq[is->pictq_rindex];

    is->width  = is->video_st->codec->width;
    is->height = is->video_st->codec->height;
    is->refresh = 0;
    loop = 100;

    return 0;
}

void *videoFinishCallback(VideoState *is){
   slog(TRACE,ERROR,"Video finished");

   duk_context *ctx = ph->ctx;
   ph->playVideo = false;

   duk_push_heap_stash(ctx);
   duk_get_prop_string(ctx, -1, "\xffon-finish");
   duk_remove(ctx, 0);
   duk_remove(ctx, 0);
   duk_remove(ctx, 0);
   slog(TRACE,DEBUG,"Emitting on-finish callback : %d\n",duk_get_top(ctx));
   if (!duk_is_function(ctx, 0)) {
     slog(TRACE,ERROR,"FFVideo : no valid callback found.\n");
     return NULL;
   }
   duk_call(ctx, 0);
   duk_pop(ctx);
   return NULL;
}

void *createTextureCallback(VideoState *is){
    slog(TRACE,ERROR,"Creating Video texture with is : 0x%x",is);
    
    texInfo *TI = is->TI;
    //if(TI && TI->texture){
    //    SDL_DestroyTexture(TI->texture);
    //    ph->textureCount--;
    //}
    //slog(TRACE,ERROR,"Creating video texture for %s, size %dx%d",TI->name, is->width, is->height);
    TI->texture = SDL_CreateTexture( ph->renderer, SDL_PIXELFORMAT_YV12, SDL_TEXTUREACCESS_STREAMING, is->width, is->height );
    is->TI = TI;
    ph->textureCount++;
    return TI->texture;
}

void displayTextureCallback(VideoState *is, void *p){
    slog(DEBUG,DEBUG,"Render IImage");
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
    slog(TRACE,INFO,"FFPlugin going to play on screen %s : %s\n",screen, file);

    VideoObject *vo = getVideoObject(ph->ctx);

    texInfo *TI = ht_get_simple(ph->baseTextures,screen);
    if(!TI) {
      slog(LVL_QUIET,ERROR,"Taget texture %s not found.",screen);
      return -1;
    }
    //vo->TI = TI;

    ph->playVideo = true;

    setRenderer(ph->renderer);
    setDisplayTextureCallback(displayTextureCallback);
    setCreateTextureCallback(createTextureCallback);
    setFinishCallback(videoFinishCallback);

    vo->is = renderFFVideo(file);
    //vo->is->TI = TI;
    //vo->is->VO = vo;
    slog(DEBUG,DEBUG, "FFVideo Object : 0x%x, is : 0x%x",vo,vo->is);
    return ret;
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

char *cleanupPlugin(void *p){
   int status;
   // TODO : Cleanup FFPlayer and kill threads by signal
 //  stopVideo(NULL);

 //  slog(LVL_INFO,INFO,"Cleaning up video plugin. Waiting for threads to finish.");
 //  SDL_WaitThread(is->parse_tid,&status);
  // slog(DEBUG,DEBUG,"Thread 1/2 finished.");
  // SDL_CondSignal(is->pictq_cond);
  // SDL_WaitThread(is->video_tid,&status);
 //  slog(DEBUG,DEBUG,"Thread 2/2 finished.");
 //  if(is) free(is);
   return NULL;
}

int resetVideo(char *p){
   cleanupPlugin(p);
   //if(initVariables() == false){
   //   slog(LVL_INFO,WARN,"Could not (re)init "PLUGIN_SCOPE);
   //  return NULL;
   //}
   return true;
}

int js_play(duk_context *ctx)
{
  char *cs;
  int ret;
  static char key[KEYLEN];
  dschema_check(ctx, (const duv_schema_entry[]) {
     {"texture", duk_is_string},
     {"url ", duk_is_string},
     {"callback", dschema_is_continuation},
     {0,0}
  });

  const char *texname = duk_to_string(ctx,0);
  const char *url = duk_to_string(ctx,1);
  asprintf(&cs,"%s %s",texname,url);

  // Push the callback
  duk_push_heap_stash(ctx);
  duk_dup(ctx, -2);
  duk_put_prop_string(ctx, -2, "\xffon-finish");
  duk_pop(ctx);

  call("ffvideo::play",&ret,cs);
  free(cs);
  return 0;
}

duk_ret_t js_info(duk_context *ctx)
{

   VideoObject *vo = getVideoObject(ph->ctx);
   duk_push_object(ctx);
   duk_push_int(ctx, vo->is->width);
   duk_put_prop_string(ctx, -2, "width");
   duk_push_int(ctx, vo->is->height);
   duk_put_prop_string(ctx, -2, "height");
   if(ph->playVideo == true){
      duk_push_string(ctx, "playing");
   } else {
      duk_push_string(ctx, "stopped");
   }
   duk_put_prop_string(ctx, -2, "status");
   return 1;
}

duk_ret_t js_video_dtor(duk_context *ctx)
{
   free(getVideoObject(ctx));
   slog(DEBUG,DEBUG, "Video object destroyed.");
   return 0;
}

duk_ret_t js_video_ctor(duk_context *ctx)
{
    slog(DEBUG,DEBUG, "New video object created.");

    //duk_push_this(ctx);
    uv_process_t *ff = duk_push_fixed_buffer(ctx, sizeof(uv_process_t));
    duv_setup_handle(ctx, (uv_handle_t*)ff, DUV_PROCESS);

    VideoObject *vo = malloc(sizeof(VideoObject));
    memset(vo,0,sizeof(VideoObject));

    slog(TRACE,DEBUG,"VideoOject located at : 0x%x",vo);

    // This is a hidden property
    duk_push_pointer(ctx, vo);
    duk_put_prop_string(ctx, -2, "\xffvo");

    duk_pop(ctx);
    return 1;
}

VideoObject *getVideoObject(duk_context *ctx) {
   duk_push_this(ctx);  /* -> stack: [ this ] */
   duk_get_prop_string(ctx, -1, "\xffvo");
   return duk_to_pointer(ctx, -1);
}

/* Initialize Video object into global object. */
void js_video_init(duk_context *ctx) {
   slog(DEBUG,FULLDEBUG,"Constructing video object");

   duk_push_c_function(ctx, js_video_ctor, 5 );
   duk_push_object(ctx);  
   duk_put_function_list(ctx, -1, videoMethods);
   duk_put_prop_string(ctx, -2, "prototype");
   duk_put_global_string(ctx, "FFVideo"); 
}

// Video object methods
const duk_function_list_entry videoMethods[] = {
    { "play",         js_play, 3 },
    { "info",         js_info, 0 },
    { NULL,           NULL,        0 }
};

//const function_list_entry c_videoMethods[] = {
//    {  PLUGIN_SCOPE"::alloc_picture", WFUNC_THRD, alloc_picture, 0 },
//    {  PLUGIN_SCOPE"::refresh_timer", WFUNC_THRD, video_refresh_timer, 0 },
//    {  PLUGIN_SCOPE"::play"         , WFUNC_SYNC, renderVideo, 0  },
//    {  PLUGIN_SCOPE"::reset"        , WFUNC_SYNC, resetVideo,  0  },
//    {  NULL, 0, NULL, 0 }
//};
 
char *initPlugin(pluginHandler *_ph){
    ph = _ph;
    ph->disableVideoAfterFinish = true;
    duk_context *ctx = ph->ctx;

    wally_put_function(PLUGIN_SCOPE"::alloc_picture", WFUNC_THRD, alloc_picture, 0);
    wally_put_function(PLUGIN_SCOPE"::refresh_timer", WFUNC_THRD, video_refresh_timer, 0);
    wally_put_function(PLUGIN_SCOPE"::play"         , WFUNC_SYNC, renderVideo, 0);
    wally_put_function(PLUGIN_SCOPE"::reset"        , WFUNC_SYNC, resetVideo,  0);
    js_video_init(ctx);

    slog(DEBUG,FULLDEBUG,"Plugin video initialized. PH is at 0x%x",ph);

    return PLUGIN_SCOPE;
}
