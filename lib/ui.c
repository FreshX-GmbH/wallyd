#include "ui.h"

//  This file contains all the basic sdl functions we use
//  They run in its own thread which forces us to notify 
//  this thread once we want to render a new frame since
//  OpenGL is single threaded

#define dumpRect(x) slog(DEBUG,DEBUG,"Rect : {%d,%d,%d,%d}",x->w, x->h,x->w,x->h);

extern pluginHandler *ph;

bool sdlStatus(void *ptr){
   slog(DEBUG,LOG_SDL,"not implemented");
   return true;
}

texInfo *getTexture(const char *name){
   texInfo *t = ht_get_simple(ph->baseTextures,(void*)name);
   slog(TRACE,LOG_TEXTURE,"Texture %s is at 0x%x",name,t);
   return t;
}

int eventFilter(void *userdata, SDL_Event *event){
    if(event->type == SDL_QUIT){
        slog(DEBUG,LOG_SDL,"Received a quit event (%d)",event->type);
        ph->quit = true;
        return 0; 
    }
    if(event->type == SDL_KEYDOWN){
        if(event->key.keysym.sym == 27){
            slog(DEBUG,LOG_SDL,"Received ESC event. Quit.");
            ph->quit = true;
        } else {
           slog(DEBUG,LOG_SDL,"Received a keydown event (%d)",event->key.keysym.sym);
        }
        return 0; 
    }
    if(event->type < SDL_USEREVENT ){
        slog(DEBUG,LOG_SDL,"Ignoring event %d",event->type);
        return 0; 
    }
    if(event->type == SDL_USEREVENT+2 ){
        slog(DEBUG,LOG_SDL,"Caught a ff video quit event.");
        return 0;
    }
//    if(event->user.data1 == NULL){
//        slog(INFO,LOG_SDL,"Ignoring event, no function given");
//        return 0; 
//    }
    if(strcmp(event->user.data1,"ffvideo::refresh_timer") == 0 && ph->playVideo == false){
        slog(INFO,WARN,"Intercepted an orphaned refresh timer event");
       return 0; 
    }
    return 1;
}

bool uiLoop(void){
    slog(DEBUG,LOG_SDL,"Setting up SDL event filter");
    SDL_SetEventFilter(eventFilter,NULL);
    //int delay=0;
//    int id;
    char *funcName;
    const char *param;
    wally_call_ctx *wtx;
    SDL_Event event;
    slog(DEBUG,LOG_SDL,"UI Loop started and waiting for events (%p)",&event);
    while(!ph->quit) {
	SDL_zero(event);
        param = NULL;
#ifdef WAIT_EV
        int ret = SDL_WaitEventTimeout(&event,3000);
        ph->uiAllCount++;
        if(ret == 0){
            slog(ERROR,LOG_SDL,"Error while wating for events : %s",SDL_GetError());
            slog(INFO,LOG_SDL,"Current Thread : %p",pthread_self());
            ph->uiEventTimeout++;
            continue;
        }
#else
	// agressive polling, increasing timeout up to 100 ms if not used
        int ret = SDL_PollEvent(&event);
        ph->uiAllCount++;
        if(ret == 0){
            SDL_Delay(ph->eventDelay);
            ph->uiEventTimeout++;
            if(ph->eventDelay < 100){
               ph->eventDelay++;
            }
            continue;
        }
        ph->eventDelay=5;
#endif
        ph->uiOwnCount++;
        funcName = strdup(event.user.data1);
	free(event.user.data1);
        slog(DEBUG,LOG_PLUGIN,"New event request for %s",funcName);
        // Pop a request from prio queue
        Node *n = priqueue_pop(ph->queue);
	void *param = NULL;
        if(n){
            param = n->data->data;
            slog(DEBUG,LOG_PLUGIN,"Popped 0x%x from queue with prio %d (num %d, type %d)",n->data->data, n->priority, n->index, n->data->type);
            free(n);
        } else {
            slog(ERROR,LOG_PLUGIN,"Nothing popped.");
            continue;
        }

        if(event.type == WALLY_CALL_WTX){
	       wtx = param;
               slog(DEBUG,LOG_PLUGIN,"Threaded WTX loop call %d elements. wtx at 0x%x", wtx->elements,wtx);
           for(int i = 0; i < wtx->elements; i++){
                  slog(TRACE,LOG_PLUGIN,"WTX(0x%x) Call%d : %s(%s)",wtx, i, wtx->name[i], wtx->param[i]);
                  //thr_func(event.user.data2);
                  void *(*thr_func)(void *) = ht_get_simple(ph->thr_functions,wtx->name[i]);
                  if(!thr_func){
                      void *(*thr_func)(void *) = ht_get_simple(ph->functions,wtx->name[i]);
                      if(!thr_func){
                      	  slog(WARN,LOG_PLUGIN,"Function %s not defined (%d).",wtx->name[i],event.type);
	                  free(funcName);
                          continue;
		      }
                  } 
                  thr_func((void*)wtx->param[i]);
           }
           if(strcmp(funcName, "video::video_refresh_timer") != 0){
                SDL_CondSignal(ht_get_simple(ph->functionWaitConditions,funcName));
	   }
//           if(ph->transaction){
//              slog(WARN,LOG_PLUGIN,"Freeing wtx for id %d at 0x%x",id,ph->transactions[id]);
//              freeWtxElements(ph->transactions[id]);
//              ph->transaction = 0;
//          } else {
           freeWtx(wtx);
//           }
	   free(funcName);
           pthread_mutex_unlock(&ph->taMutex);
           continue;
        }
        void *(*thr_func)(void *) = ht_get_simple(ph->thr_functions,funcName);
        if(!thr_func){
            slog(WARN,LOG_PLUGIN,"Threaded function %s not defined (%d).",funcName,event.type);
            continue;
        } 
 
        switch(event.type){
            case WALLY_CALL_PTR:
                  slog(DEBUG,LOG_SDL,"Threaded PTR call to %s(0x%x)", funcName, event.user.data2);
                  thr_func(event.user.data2);
                  break;
            case WALLY_CALL_STR:
                  param = strdup(event.user.data2);
                  slog(DEBUG,LOG_SDL,"Threaded STR call to %s(%s)", funcName, param);
                  // ??
                  free(event.user.data2);
                  thr_func((void*)param);
                  break;
            case WALLY_CALL_NULL:
                  slog(DEBUG,LOG_SDL,"Threaded NULL call to %s()", funcName);
                  thr_func(NULL);
                  break;
            case WALLY_CALL_PS:
                  slog(DEBUG,LOG_SDL,"Threaded PS call to %s(0x%x)", funcName, event.user.data2);
                  //thr_func(event.user.data2);
                  break;
            case WALLY_CALL_CTX:
                  slog(DEBUG,LOG_SDL,"Threaded CTX call to %s(0x%x)", funcName, event.user.data2);
                  //thr_func(event.user.data2);
                  break;
            default:
                  slog(ERROR,LOG_SDL,"Unknown threaded call event");
        }
        if(strcmp(funcName, "video::video_refresh_timer") != 0){
             SDL_CondSignal(ht_get_simple(ph->functionWaitConditions,funcName));
        }
        free(funcName);
    }
    slog(DEBUG,LOG_SDL,"UI Loop quit. Waiting for threads to finish");
    return true;
}

bool updateTextureNamesByPrio(unsigned int *items){
   //ret = malloc(ph->baseTextures->key_count * sizeof(void *));
   slog(DEBUG,LOG_TEXTURE,"Resorting texture names - entries : %d", ph->baseTextures->count);
   if(ph->texturePrio){
      free(ph->texturePrio);
   }
   ph->texturePrio = malloc(ph->baseTextures->count*sizeof(char*));
   void **keys = malloc(ph->baseTextures->count*sizeof(char*));
   slog(DEBUG,LOG_UTIL,"keys located at : 0x%x",keys);
   *items = ht_keys(ph->baseTextures,keys);
   slog(DEBUG,LOG_UTIL,"keys located at : 0x%x",keys);
   int i, j;
   // simple bubble sort
   for (i = 0; i < ph->baseTextures->count - 1; ++i) {
     for (j = 0; j < ph->baseTextures->count - i - 1; ++j) {
        texInfo *TIa = getTexture(keys[j]);
        texInfo *TIb = getTexture(keys[j+1]);
        if(!TIa || !TIb) { 
            slog(ERROR,LOG_TEXTURE,"Unexpected error in sorting texture priority"); 
            continue; 
        }
        slog(TRACE,LOG_TEXTURE,"TI: 0x%x / 0x%x",TIa, TIb);
        if (TIa->z > TIb->z) {
              void *tmp = keys[j];
              keys[j] = keys[j + 1];
              keys[j + 1] = tmp;
           }
      }
   }
   for (i = 0; i < ph->baseTextures->count; ++i) {
      ph->texturePrio[i] = keys[i];
      slog(DEBUG,LOG_UTIL,"Key prio %d : %s",i,keys[i]);
      //slog(DEBUG,DEBUG,"%s : 0x%x / 0x%x / 0x%x / 0x%x",keys[i], TI, TI->texture, TI->rect, TI->name);
   }
   return true;
}

// Displays all textures in different colors
int showTextureTestScreen(void *p){
//   unsigned int i=0;
//   int ret;
   texInfo *TI = ph->tempTexture;
   SDL_SetRenderTarget(ph->renderer,TI->texture);
   SDL_RenderClear( ph->renderer );

//   SDL_Rect *mr = TI->rect;
   SDL_Color color = STAMPFONT_COLOR;

   TTF_Font *font = ht_get_simple(ph->fonts,"logfont");
   char *tName=NULL;
   unsigned int items = 0;
   updateTextureNamesByPrio(&items);
   for(int i = 0; i < items; i++ ){
      texInfo *TI = getTexture( ph->texturePrio[i] );
      SDL_Texture *t = TI->texture;
      SDL_Rect *src = TI->rect;
      slog(DEBUG,LOG_TEXTURE,"Presenting texture(0x%x) %s size %dx%d at pos %d,%d", TI,
            ph->texturePrio, src->w,src->h,src->x,src->y);
      asprintf(&tName,"%s(%d,%d) size %dx%d",ph->texturePrio[i],src->x,src->y,src->w,src->h);
      slog(DEBUG,LOG_TEXTURE,"Name : %s",tName);
      if(font){
         SDL_Surface *surf = TTF_RenderText_Solid( font, tName, color );
         if(!surf) {
            slog(ERROR,LOG_TEXTURE,"Could not create FontSurface : %s",SDL_GetError());
            return 1;
         }
         SDL_Texture *ft = SDL_CreateTextureFromSurface( ph->renderer, surf );
         ph->textureCount++;
         SDL_Rect d;
         SDL_QueryTexture( ft, NULL, NULL, &d.w, &d.h );
         d.x = src->x+src->w/2 - d.w/2;
         d.y = src->y+src->h/2 - d.h/2;
         texInfo *TI = ph->tempTexture;
         SDL_SetRenderTarget(ph->renderer, TI->texture);
         SDL_RenderCopy( ph->renderer, t, NULL, src);
         SDL_RenderCopy( ph->renderer, ft, NULL, &d);
         SDL_FreeSurface(surf);
         SDL_DestroyTexture(ft);
         ph->textureCount--;
      } else {
         slog(ERROR,LOG_SDL,"Font 'logfont' not loaded");
      }
      free(tName);
   }
   SDL_SetRenderTarget(ph->renderer, NULL);
   SDL_RenderCopy(ph->renderer, TI->texture, NULL, NULL);
   SDL_RenderPresent( ph->renderer );
   slog(DEBUG,LOG_TEXTURE,"All textures displayed.");
   return true;
}

int renderActiveEx(void *startTex)
{
   unsigned int i=0;
   texInfo *TI,*TempTI;
   bool start = false;
   if(startTex == NULL){
      start = true;
   }
   TempTI = ph->tempTexture;
   SDL_SetRenderTarget(ph->renderer,TempTI->texture);
//   SDL_RenderClear( ph->renderer );

   for (i = 0; i < ph->baseTextures->count; ++i) {
      char *name = ph->texturePrio[i];
      if(start == false && strncmp(name,startTex,strlen(name)) == 0){
         start = true;
      }
      if(start == false) continue;
      slog(TRACE,LOG_TEXTURE,"Key %d : %s",i,name);
      TI = getTexture( name );
      if(TI->active == true && TI->autorender == true){
          SDL_Rect *mr = TI->rect;
          if(mr == NULL || mr->w ==0 || mr->h == 0){
	       slog(INFO,LOG_TEXTURE,"Refusing to place texture %s with invalid size.",name);
	       continue;
          }
          slog(TRACE,LOG_TEXTURE,"RenderActive(%s,{%d,%d,%d,%d});",name,mr->x, mr->y, mr->w, mr->h);
          SDL_RenderCopyEx( ph->renderer, TI->texture, NULL, mr, 0, NULL, SDL_FLIP_NONE);
       }
   }
   SDL_SetRenderTarget(ph->renderer, NULL);
   SDL_RenderCopyEx(ph->renderer, TempTI->texture, NULL, NULL,0,NULL,SDL_FLIP_NONE);
   SDL_RenderPresent( ph->renderer );
   return true;
}

void renderActive(const char *startTex)
{
   if(ph->autorender == false) 
         return;
   renderActiveEx((void*)startTex);
}

//   renders and displays a texture on the Main Renderer
void renderTexture(SDL_Texture *t, SDL_Rect *mr){
    slog(DEBUG,LOG_TEXTURE,"Display texture(0x%x,{%d,%d,%d,%d})",t, mr->x, mr->y, mr->w, mr->h);
    if(mr == NULL || mr->w == 0 || mr->h == 0 || t == NULL){
       slog(DEBUG,LOG_TEXTURE,"Refusing to place invalid texture or texture has an invalid size.");
    } else {
       SDL_RenderClear(ph->renderer);
       SDL_RenderCopy(ph->renderer, t, mr, mr);
       SDL_RenderPresent(ph->renderer);
    }
}

int createTextureEx(void *strTmp,bool isVideo){
   char *str = strdup(strTmp);
   int z=-1,x=-1, y=-1, w=-1, h=-1;
   unsigned int color = 0;
   char *r;
   texInfo *TI = NULL;

   char *textureName = strtok_r(str, " ",&r);
   char *zS = strtok_r(NULL, " ",&r);
   char *xS = strtok_r(NULL, " ",&r);
   char *yS = strtok_r(NULL, " ",&r);
   char *wS = strtok_r(NULL, " ",&r);
   char *hS = strtok_r(NULL, " ",&r);
   char *cS = strtok_r(NULL, " ",&r);

   if(getTexture(textureName) != NULL){
	slog(INFO,LOG_TEXTURE,"Texture %s already existing, replacing it",textureName);
	destroyTexture(textureName);
   }
   hashtable_print(ph->baseTextures,"Textures : ");

   if(!getNumOrPercent(zS, 0, &z)){
      slog(INFO,LOG_TEXTURE,"Wrong parameters for createTexture(name, Z, x, y, w, h [,color hex]) : (%s)",str);
      goto fail;
   } else {
      slog(DEBUG,LOG_TEXTURE,"Z-Value : %d",z,&r);
   }

   if(!getNumOrPercent(xS, ph->width, &x)){
      slog(INFO,LOG_TEXTURE,"Wrong parameters for createTexture(name, Z, x, y, w, h [,color hex]) : (%s)",str);
      goto fail;
   }
   if(!getNumOrPercent(yS, ph->height, &y)){
      slog(INFO,LOG_TEXTURE,"Wrong parameters for createTexture(name, Z, x, y, w, h [,color hex]) : (%s)",str);
      goto fail;
   }
   if(!getNumOrPercent(wS, ph->width, &w)){
      slog(INFO,LOG_TEXTURE,"Wrong parameters for createTexture(name, Z, x, y, w, h [,color hex]) : (%s)",str);
      goto fail;
   }
   if(!getNumOrPercent(hS, ph->height, &h)){
      slog(INFO,LOG_TEXTURE,"Wrong parameters for createTexture(name, Z, x, y, w, h [,color hex]) : (%s)",str);
      goto fail;
   }
   if(!getNumHex(cS,&color)){
      slog(INFO,LOG_TEXTURE,"Texture color not given, using black.");
   }

   // No parameters == FULLSCREEN, BLACK
   if(x==-1 && y==-1 && w==-1 && h==-1){
      x=0, y=0;
      w=ph->width;
      h=ph->height;
   }
   if(w == 0 || h == 0){
      slog(ERROR,LOG_TEXTURE,"Refusing to create texture %s with size %dx%d",textureName,w,h);
      goto fail;
   }

   // TODO : Free!
   TI = malloc(sizeof(texInfo));
   memset(TI,0,sizeof(texInfo));

   if(isVideo == true){
      TI->video = true;
   }
   TI->z = z;
   TI->active = true;
   TI->autorender = true;
   TI->name = strdup(textureName);

   if(x!=-1 && y!=-1 && w!=-1 && h!=-1 && w != 0 && h != 0){

      TI->c = malloc(sizeof(SDL_Color));
      memset(TI->c,255,sizeof(SDL_Color));
      hexToColor(color, TI->c);
      TI->c->a=255;

      if(isVideo == true) {
         slog(DEBUG,LOG_VIDEO,"Creating video texture named %s at (%d,%d) size %dx%d prio %d, color(%d,%d,%d,%d)",textureName,x,y,w,h,TI->z,TI->c->r,TI->c->g,TI->c->b,TI->c->a);
         TI->texture = SDL_CreateTexture(ph->renderer, 
	  	 SDL_PIXELFORMAT_RGBA8888,
                 SDL_TEXTUREACCESS_STREAMING | 
                 SDL_TEXTUREACCESS_TARGET, 
		 w, h);
      } else {
         slog(DEBUG,LOG_TEXTURE,"Creating texture named %s at (%d,%d) size %dx%d prio %d, color(%d,%d,%d,%d)",textureName,x,y,w,h,TI->z,TI->c->r,TI->c->g,TI->c->b,TI->c->a);
         SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
         TI->texture = SDL_CreateTexture(ph->renderer, 
	  	 //SDL_PIXELFORMAT_RGBA8888,
                 SDL_PIXELFORMAT_ARGB8888,
                 //SDL_TEXTUREACCESS_STREAMING | 
                 SDL_TEXTUREACCESS_TARGET, 
		 w, h);
      }
      if(!TI->texture){
         slog(ERROR,LOG_TEXTURE,"Failed to create a texture : %s",SDL_GetError());
         goto fail;
      }
      ph->textureCount++;
      // Clear the texture
      SDL_SetRenderTarget(ph->renderer,TI->texture);
      SDL_SetRenderDrawColor(ph->renderer, TI->c->r, TI->c->g, TI->c->b ,255);
      SDL_RenderClear( ph->renderer );
      SDL_SetRenderTarget(ph->renderer,NULL);

      SDL_Rect *p = malloc(sizeof(SDL_Rect));
      memset(p,0,sizeof(SDL_Rect));
      p->x = x; p->y = y;
      p->w = w; p->h = h;
      TI->rect = p;
      TI->z = z;
      ht_insert_simple(ph->baseTextures,TI->name,TI);

      unsigned int items = 0;
      updateTextureNamesByPrio(&items);
   } else {
      slog(INFO,LOG_TEXTURE,"Wrong parameters for createTexture(n,x,y,w,h) : (%s)",str);
      goto fail;
   }
   free(str);
   return true;
fail:
   free(str);
   return false;
}

int createVideoTexture(void *a){
   return createTextureEx(a,true);
}

int createTexture(void *a){
   return createTextureEx(a,false);
}

int setTextureActive(void *s,bool active){
   char *name = strtok(s," ");
   if(!name){
      slog(INFO,LOG_TEXTURE,"Wrong parameters for hideTexture(name) : (%s)",s);
      return false;
   }
   texInfo *TI = getTexture(name);
   if(!TI) {
      slog(ERROR,LOG_TEXTURE,"Texture %s not found.",name);
      return false;
   }
   TI->active=active;
   return true;
}

int hideTexture(void *a){
   return setTextureActive(a,false);
}

int showTexture(void *a){
   return setTextureActive(a,true);
}

int replaceTexture(void *name,texInfo *_TI){
   texInfo *TI; // = (texInfo *)_TI;
   TI = getTexture(name);
   if(!TI) {
      slog(WARN,LOG_TEXTURE,"Texture %s not found.",name);
      return false;
   }
   SDL_DestroyTexture(TI->texture);
   TI->texture = _TI->texture;
   slog(DEBUG,LOG_TEXTURE,"Destroyed texture %s",name);
   return true;
}

int destroyTexture(void *s){
   unsigned int items = 0;
   texInfo *TI;
   if(!s){
      slog(INFO,LOG_TEXTURE,"Wrong parameters for destroyTexture(name) : (%s)",s);
      return false;
   }
   char *name = strtok(s, " ");
   TI = getTexture(name);
   if(!TI) {
      slog(WARN,LOG_TEXTURE,"Texture %s not found.",name);
      return false;
   }
   ht_remove_simple(ph->baseTextures, name);
   SDL_DestroyTexture(TI->texture);
   ph->textureCount--;
   free(TI->rect);
   free(TI->name);
   free(TI->c);
   // remove item and reorder prio list
   free(TI);
   updateTextureNamesByPrio(&items);
   slog(DEBUG,LOG_TEXTURE,"Destroyed texture %s",s);
   return true;
}

//   Simply renders texture[i] to the Main Renderer
bool renderTexName(void *s){
   texInfo *TI;
   if(!s){
      slog(INFO,LOG_TEXTURE,"Wrong parameters for destroyTexture(name) : (%s)",s);
      return false;
   }
   char *name = strtok(s, " ");
   TI = getTexture(name);
 
   SDL_Rect *mr = TI->rect;
   slog(DEBUG,LOG_TEXTURE,"Render tex[%s] pos %dx%d size : %dx%d at 0x%x",name,mr->x, mr->y, mr->w, mr->h, TI->texture);
   if(mr == NULL || mr->w ==0 || mr->h == 0 || TI->texture == NULL){
      slog(DEBUG,LOG_TEXTURE,"Refusing to place texture %d with invalid size.",name);
   } else {
      SDL_RenderClear(ph->renderer);
      SDL_RenderCopy(ph->renderer, TI->texture, mr, mr);
      SDL_RenderPresent(ph->renderer);
   }
   return true;
}

//      This should not be used
int setTexture(texInfo *TI, SDL_Texture *texture, SDL_Rect *origRect)
{
   // This is fast
   slog(DEBUG,LOG_TEXTURE,"Updating Texture[%s]",TI->name);
   TI->rect->x = origRect->x;
   TI->rect->y = origRect->y;
   TI->rect->h = origRect->h;
   TI->rect->w = origRect->w;
   SDL_DestroyTexture(TI->texture);
   ph->textureCount--;
   TI->texture=texture;
   return true;
}

int addToTexture(texInfo *TI, SDL_Texture *src, SDL_Rect *origRect)
{
   slog(DEBUG,LOG_TEXTURE,"Adding a texture({%d,%d,%d,%d}) to %s",
          origRect->x, origRect->y,
          origRect->w, origRect->h, TI->name);
   SDL_SetTextureBlendMode(TI->texture, SDL_BLENDMODE_BLEND);
   SDL_SetRenderTarget(ph->renderer,TI->texture);
   SDL_RenderCopy( ph->renderer, src, NULL, origRect );
   SDL_SetRenderTarget(ph->renderer,NULL);
   return true;
}

// This is slower than setTexture but allows to copy one texture 
// into antoher at a given position 
int copyToTexture(texInfo *TI, SDL_Texture *src, SDL_Rect *origRect)
{
   slog(DEBUG,LOG_TEXTURE,"not functional");
   return false;
// SDL_Rect *mr = ph->textureSize[i];
//   SDL_Rect src = { 0, 0, origRect->w, origRect->h};
//   slog(DEBUG,DEBUG,"Copying texture(0x%x,{0,0,%d,%d}) to texture(%d,{%d,%d,%d,%d})",
//          texture, origRect->w, origRect->h, 
//          i, mr->x, mr->y, mr->w, mr->h);
//   SDL_SetRenderTarget(ph->renderer,TI->texture);
//   SDL_RenderCopy( ph->renderer, src, origRect, origRect );
//   SDL_SetRenderTarget(ph->renderer,NULL);
//   return true;
}

// This is slower than setTexture but allows to stretch one texture 
// into antoher at a given position 
int scaleToTexture(texInfo *TI, SDL_Texture *src, SDL_Rect *origRect)
{
   // This is slow (speedup by just copying the texture pointer)
   SDL_Rect mr = {0, 0, TI->rect->w, TI->rect->h};
   slog(DEBUG,LOG_TEXTURE,"Scaling texture({0,0,%d,%d}) to texture %s {%d,%d,%d,%d}",origRect->w, origRect->h,TI->name, mr.x,mr.y,mr.w,mr.h);

   SDL_SetTextureBlendMode(TI->texture, SDL_BLENDMODE_BLEND);
   SDL_SetRenderTarget( ph->renderer, TI->texture );
   SDL_RenderCopy( ph->renderer, src, origRect, &mr);
   SDL_SetRenderTarget( ph->renderer,NULL );
   return true;
}

int copyTexture(texInfo *TI, SDL_Texture *src, SDL_Rect *origRect){
   SDL_Rect dst = {0,0,origRect->w, origRect->h};
   scaleToTexture(TI, src, &dst); 
   return true;
}

//	Init SDL, Fonts, EGL (if raspberry) and structures
bool sdlInit(void)
{
   int w=ph->width;
   int h=ph->height;
#ifdef RASPBERRY
   if(ph->broadcomInit == true){
      bcm_host_init();
      slog(INFO,LOG_TEXTURE,"Initializing broadcom hardware");
   }
#else
   ph->broadcomInit = false;
#endif

   if(ph->sdldebug){
   	slog(INFO,LOG_SDL,"Setting SDL debug level to VERBOSE");
   	SDL_LogSetAllPriority(SDL_LOG_PRIORITY_VERBOSE);
   } else {
      SDL_LogSetPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO);
   }

   if ( SDL_Init( SDL_INIT_TIMER | SDL_INIT_VIDEO | SDL_INIT_EVENTS ) == -1 ) {
      slog(ERROR,LOG_SDL, " Failed to initialize SDL : %s", SDL_GetError());
      return false;
   }
   if(ph->disableAudio == false){
	if(SDL_InitSubSystem(SDL_INIT_AUDIO)){
            slog(ERROR,LOG_SDL, " Failed to initialize SDL Audio (set disableAudio=false in wallyd.conf) : %s", SDL_GetError());
            return false;
        }
   }

   if(ph->broadcomInit == true || w==0 || h==0 ){
      slog(DEBUG,LOG_SDL,"Starting in full screen with current resolution : %d==%d / %d / %d",true,ph->broadcomInit, w, h);
      ph->window = SDL_CreateWindow("wallyd", 
	     SDL_WINDOWPOS_UNDEFINED, 
	     SDL_WINDOWPOS_UNDEFINED, 
	     0,0, 
	SDL_WINDOW_OPENGL | SDL_WINDOW_FULLSCREEN_DESKTOP | SDL_WINDOW_SHOWN);
      SDL_GetWindowSize(ph->window, &w, &h);
      slog(DEBUG,LOG_SDL,"Autodetermined display resolution : %dx%d",w, h);
      ph->width=w;
      ph->height=h;
   } else {
      if(w == 0 || h == 0){
         slog(ERROR,LOG_SDL,"Refusing to open a window with invalid size (%dx%d).",w,h);
         return false;
      }
      slog(DEBUG,LOG_SDL,"Starting in windowed with current resolution : %d==%d / %d / %d",true,ph->broadcomInit, w, h);
      ph->window = SDL_CreateWindow( "Server", 
	    SDL_WINDOWPOS_UNDEFINED,
	    SDL_WINDOWPOS_UNDEFINED,
	    w, h, 
	    SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
      SDL_GetWindowSize(ph->window, &w, &h);
      slog(DEBUG,LOG_SDL,"Redetermined display resolution : %dx%d",w, h);
      ph->width=w;
      ph->height=h;
      slog(DEBUG,LOG_SDL,"Window is at 0x%x.",ph->window);
      if ( ph->window == NULL || ph->window == 0x0 ) {
	 slog(ERROR,LOG_SDL, "Failed to get/create window : %s ", SDL_GetError());
	 return false;
      }
   }

   //ph->glcontext = SDL_GL_CreateContext(ph->window);
   // Show a cursor in Mouse driven OSes
   SDL_ShowCursor( 0 ); 

   int numdrivers = SDL_GetNumRenderDrivers (); 
   slog(INFO,LOG_SDL,"Number of Render Drivers : %d",numdrivers);
   for(int i = 0; i < numdrivers; i++){
   	SDL_RendererInfo drinfo; 
   	SDL_GetRenderDriverInfo (i, &drinfo); 
   	slog(INFO,LOG_SDL,"Driver %d : %s",i,drinfo.name );
   }

   if(ph->vsync == true){
       ph->renderer = SDL_CreateRenderer( ph->window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE | SDL_RENDERER_PRESENTVSYNC );
   } else {
       ph->renderer = SDL_CreateRenderer( ph->window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE );
   }

   if ( ph->renderer == NULL ) {
      slog(ERROR,LOG_SDL,"Failed to create renderer %s ", SDL_GetError());
	ph->renderer = SDL_CreateRenderer(ph->window, -1, SDL_RENDERER_SOFTWARE);
        if ( ph->renderer == NULL ) {
      		slog(ERROR,LOG_SDL,"Failed to create SW renderer %s ", SDL_GetError());
      		return false;
	}
   }
   SDL_SetRenderDrawBlendMode(ph->renderer, SDL_BLENDMODE_BLEND);

   // Unnice effekt on wallyTV
#ifndef RASPBERRY
   slog(DEBUG,LOG_SDL,"Clearing renderer");
   SDL_SetRenderDrawColor(ph->renderer, 0,0,0,255);
   SDL_RenderClear( ph->renderer );
   SDL_RenderPresent( ph->renderer );
#endif
   SDL_RenderSetLogicalSize( ph->renderer, w, h );
   ph->SDL = true;

   //SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION,"Info","Hallo Welt",ph->window);

   // Initialize Textures / Renderers
   ph->tempTexture = malloc(sizeof(texInfo));
   memset(ph->tempTexture,0,sizeof(texInfo));
   texInfo *TI = ph->tempTexture;
    
   TI->active = true;
   TI->autorender = true;
   //SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
   TI->texture = SDL_CreateTexture(ph->renderer, 
	  	 SDL_PIXELFORMAT_ARGB8888,
         //        SDL_TEXTUREACCESS_STREAMING | 
                 SDL_TEXTUREACCESS_TARGET, 
	  	 w, h);
   ph->textureCount++;
   SDL_SetTextureBlendMode(TI->texture,SDL_BLENDMODE_BLEND);
   SDL_Rect *mr = malloc(sizeof(SDL_Rect));
   TI->rect = mr;
   mr->x = 0; mr->y = 0;
   mr->w = w; mr->h = h;
   //SDL_QueryTexture( ph->textures[i], NULL, NULL, &mr->w, &mr->h );
   // TODO : what is it doing? Do we need it?
   // SDL_SetTextureBlendMode(ph->textures[i], SDL_BLENDMODE_BLEND);
   slog(DEBUG,LOG_SDL,"Created temptexture at 0x%x size %dx%d", TI->texture, mr->w, mr->h);

   ph->tempTexture = TI;

   if ( IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG) != (IMG_INIT_PNG | IMG_INIT_JPG)){
      slog(ERROR,LOG_SDL, " Failed to initialize SDL_Image : %s ",IMG_GetError());
      return false;
   }

   if ( TTF_Init() == -1 ) {
      slog(ERROR,LOG_SDL, " Failed to initialize SDL_TTF : %s ",TTF_GetError());
      return false;
   }

   return true;
}

texInfo *getTexInfo(void *name)
{
   if(name == NULL){
      slog(ERROR,LOG_SDL,"Texture %s not found.",name);
      return false;
   }
   texInfo *TI = getTexture(name);
   if(!TI) {
      slog(ERROR,LOG_SDL,"Texture %s not found.",name);
      return false;
   }
   return TI;
}

bool closeFont(void *name){
   TTF_Font *f = ht_get_simple(ph->fonts,name);
   TTF_CloseFont(f);
   return true;
}

int loadFont(void *strTmp){
   char *str=strdup(strTmp);

   char *name = strtok(str, " ");
   char *file = strtok(NULL, " ");
   char *sStr = strtok(NULL, " ");
   int size;
   if(ht_contains_simple(ph->fonts,name)){
      slog(DEBUG,LOG_SDL,"Font %s already loaded",name);
      free(str);
      return true;
   }
   bool ret = getNumOrPercent(sStr,0,&size);
   if(!name || !file || !ret) {
      slog(ERROR,LOG_SDL,"Wrong parameters loadFont(name, file) : %s",str);
      free(str);
      return false;
   }
   TTF_Font *f = TTF_OpenFont( file, size );
   if ( f == NULL ) {
      slog(ERROR, LOG_SDL, "Failed to load font : %s ",TTF_GetError());
      free(str);
      return false;
   } else {
      slog(DEBUG, LOG_SDL, "Font: %s loaded",file);
      ht_insert_simple(ph->fonts, name, f);
      free(str);
   }
   return true;
}

int createColor(void *strTmp){
   char *str = strdup(strTmp);
   char *name = strtok(str, " ");
   char *cS = strtok(NULL, " ");
   char *aS = strtok(NULL, " ");
   int col, alpha;
   if(!name || !getNumHex(cS,&col) || !getNumHex(aS,&alpha)){
      slog(DEBUG,LOG_SDL,"Wrong parameter createColor(name FFFFFF FF) : %s",str);
      return false;
   }
   if(ht_contains_simple(ph->colors,name)){
      slog(DEBUG,LOG_SDL,"Color %s already defined",name);
      free(str);
      return true;
   }
   SDL_Color *c=malloc(sizeof(Color));
   hexToColor(col,c);
   c->a = alpha;
   ht_insert_simple(ph->colors,name,c);
   hashtable_print(ph->colors,NULL);
   slog(DEBUG,LOG_SDL,"Created new color %s={%d,%d,%d} alpha = %d at 0x%x / PH : 0x%x",name,c->r,c->g,c->b,c->a,c,ph);
   free(str);
   return true;
}

void hexToColor(int color, SDL_Color *c)
{
   c->r = (color >> 16) & 0xff;
   c->g = (color >> 8)  & 0xff;
   c->b = color & 0xff;
   c->a = 0;
}

// Create a new window and a new renderer and switch over
int resetScreen(void *p){
   int w=0,h=0;
   SDL_Window *window;
   if(ph->broadcomInit == true){
      slog(DEBUG,LOG_SDL,"Starting in full screen with current resolution : %d==%d / %d / %d",true,ph->broadcomInit, w, h);
      window = SDL_CreateWindow("wallyd", 
	     SDL_WINDOWPOS_UNDEFINED, 
	     SDL_WINDOWPOS_UNDEFINED, 
	     0,0, 
	     SDL_WINDOW_OPENGL | SDL_WINDOW_FULLSCREEN_DESKTOP | SDL_WINDOW_SHOWN);
   } else {
      window = SDL_CreateWindow( "Server", 
	    SDL_WINDOWPOS_UNDEFINED,
	    SDL_WINDOWPOS_UNDEFINED,
	    ph->width, ph->height, 
	    SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_INPUT_FOCUS  );
   }
   if ( window == NULL ){
	 slog(ERROR,LOG_SDL, "Failed to get/create window : %s ", SDL_GetError());
	 return false;
   }
   SDL_GetWindowSize(window, &w, &h);
   slog(DEBUG,LOG_SDL,"Autodetermined new display resolution : %dx%d",w, h);
   SDL_Renderer *renderer = SDL_CreateRenderer( window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE | SDL_RENDERER_PRESENTVSYNC );
   if ( renderer == NULL ){
	 slog(ERROR,LOG_SDL, "Failed to get/create renderer : %s ", SDL_GetError());
	 return false;
   }
   SDL_Window *oldw = ph->window;
   SDL_Renderer *oldr = ph->renderer;
   ph->renderer = renderer;
   ph->window = window;
   ph->glcontext = SDL_GL_CreateContext(window);
   ph->width=w;
   ph->height=h;
   renderActive(NULL);
   SDL_DestroyRenderer(oldr);
   SDL_DestroyWindow(oldw);
   return true;
}

void cleanupSDL(void){
   slog(INFO,LOG_SDL,"Cleanup sdl");
   TTF_Font *f = ht_get_simple(ph->fonts,"font");
   if(f) TTF_CloseFont(f);
   f = ht_get_simple(ph->fonts,"stampfont");
   if(f) TTF_CloseFont(f);
   TTF_Quit();
   // TODO : Clean all textures etc
   texInfo *TI = ph->tempTexture;
   SDL_DestroyTexture(TI->texture);
   ph->textureCount--;
   free(TI->rect);
   free(TI);
}
