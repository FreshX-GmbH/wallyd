#include "wallystart.h"

char *repl_str(const char *str, const char *from, const char *to);

SDL_Texture *t1 = NULL;
SDL_Texture *t2 = NULL;

int main( int argc, char* argv[] )
{
    SDL_Texture *t3 = NULL;
    int argadd = 0;
    char *start;
    logStr = strdup("Hello Wally!");

    slog_init(NULL, WALLYD_CONFDIR"/wallyd.conf", DEFAULT_LOG_LEVEL, 0, LOG_ALL, LOG_ALL , true);

    if(argc > 1){
        start = argv[1]; 
    } else {
        start = START;
    }

    slog(INFO,LOG_CORE,"%s (V"VERSION")" ,argv[0]);
    rot = 0;

    loadSDL();
    if(!dumpModes()){
        exit(1);
    }
    if(!loadFont(BASE""FONT,16)){
          exit(1);
    }
    if(pthread_create(&log_thr, NULL, &logListener, NULL) != 0){
       slog(ERROR,LOG_CORE,"Failed to create listener thread!");
       exit(1);
    }
    slog(INFO,LOG_CORE,"Screen size : %dx%d",w,h);

    processStartupScript(START);

    while(!quit)
    {
       while(SDL_PollEvent(&event) != 0)
       {
           if(event.type == SDL_MOUSEBUTTONDOWN || 
               event.type == SDL_APP_TERMINATING || 
           //    event.type == SDL_KEYDOWN || 
               event.type == SDL_QUIT)
           {
                quit = true;
           }
       //sleep(1);
       showTexture(t1, rot);
       //slog("%p %p",text,renderer);
       }
    }
    SDL_DestroyTexture(t1);
    if(t2) {
        SDL_DestroyTexture(t2);
    }
    closeSDL();
    return 0;
}

bool dumpModes()
{
    SDL_DisplayMode mode;
    SDL_Rect r;
    int j,i,display_count;
    Uint32 f;
    if ((display_count = SDL_GetNumVideoDisplays()) < 1) {
        slog(ERROR,LOG_CORE,"No VideoDisplays found.");
        return false;
    }
    slog(INFO,LOG_CORE,"VideoDisplays: %i", display_count);

    for (j=0; j<display_count; j++){
        SDL_GetDisplayBounds(j,&r);
        slog(DEBUG,LOG_CORE,"Display %d boundaries : %d x %d",j,r.w,r.h);
        // Store size of first display
        if(j == 0){
            w = r.w;
            h = r.h;
        }
        for (i=0; i<SDL_GetNumDisplayModes(j); i++){
          SDL_GetDisplayMode(j,i,&mode);
          f = mode.format;
          slog(DEBUG,LOG_CORE,"Display %d / Mode %d : %d x %d x %d bpp (%s) @ %d Hz", j, i, mode.w, mode.h, SDL_BITSPERPIXEL(f), SDL_GetPixelFormatName(f), mode.refresh_rate);
        }
    }
    return true;
}
bool loadSDL()
{
    bool mode2d = false;
    if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER) != 0) {
        slog(ERROR, LOG_CORE, "SDL_image could not initialize! SDL_image Error: %s", IMG_GetError() );
	return false;
    }
    int flags=IMG_INIT_JPG|IMG_INIT_PNG;
    int initted=IMG_Init(flags);
    if((initted&flags) != flags)
    {
        slog(ERROR,LOG_CORE, "SDL_image could not initialize PNG and JPG! SDL_image Error: %s", IMG_GetError() );
	return false;
    }
    if ( TTF_Init() == -1 ) {
        slog(ERROR,LOG_CORE, "SDL_TTF could not initialize! SDL_ttf Error: %s", TTF_GetError() );
	return false;
    }
    window = SDL_CreateWindow("wallyd", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 0,0, SDL_WINDOW_OPENGL | SDL_WINDOW_FULLSCREEN_DESKTOP | SDL_WINDOW_SHOWN);
    
    if(mode2d){
           screenSurface = SDL_GetWindowSurface( window );
    } else {
       //renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED| SDL_RENDERER_TARGETTEXTURE | SDL_RENDERER_PRESENTVSYNC );
       renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED| SDL_RENDERER_TARGETTEXTURE );
       SDL_ShowCursor( 0 );

       if(renderer == NULL){
            slog(ERROR, LOG_CORE, "Renderer could not initialize : %s", IMG_GetError() );
            return false;
       }
    }
    return true;
}

bool showTexture(SDL_Texture *tex1, int rot){
      //SDL_Rect r = { h-16, 0, h, w };
      SDL_Rect r = {0, 0, 32, w };
      int tw,th;
      SDL_Texture *tex2;
      if(logStr) {
         tex2 = renderLog(logStr,&r.w, &r.h);
         r.x = 0;
         r.y = h - r.h;
      }
      if(rot == 0){
       	        SDL_RenderCopy( renderer, tex1, NULL, NULL);
                //if(tex2) {
       	            SDL_RenderCopy( renderer, tex2, NULL, &r);
                //}
      } else {
    	        SDL_RenderCopyEx( renderer, tex1, NULL, NULL,rot, NULL,SDL_FLIP_NONE);
                //if(tex2){
    	            SDL_RenderCopyEx( renderer, tex2, NULL, &r,rot, NULL,SDL_FLIP_NONE);
                //}
      }
      SDL_RenderPresent( renderer );
      SDL_DestroyTexture( tex2 );
      return true;
}

bool fadeOver(SDL_Texture *t1, SDL_Texture *t2,int rot, long delay){
    struct timespec t = { 0, delay};
    int v = 0;
    SDL_Rect size;
    SDL_Texture *temp = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, w, h);
    SDL_SetTextureBlendMode(t1, SDL_BLENDMODE_BLEND);
    SDL_SetTextureBlendMode(t2, SDL_BLENDMODE_BLEND);
    for(int i = 255; i >= 0;i--){
         slog(DEBUG,LOG_CORE,"Fade over step %d",i);
         SDL_SetRenderTarget(renderer, temp);
    	 SDL_RenderCopyEx( renderer, t2, NULL, NULL,rot, NULL,SDL_FLIP_NONE);
         SDL_SetTextureAlphaMod(t1,i);
    	 SDL_RenderCopyEx( renderer, t1, NULL, NULL,rot, NULL,SDL_FLIP_NONE);
         SDL_SetRenderTarget(renderer, NULL);
         showTexture(temp, rot);
         nanosleep(&t,NULL);
    }
    SDL_DestroyTexture(temp);
    return true;
}


bool fadeImage(SDL_Texture *text, int rot, bool reverse, long delay){
    struct timespec t = { 0, delay};
    //SDL_SetRenderDrawColor(renderer, 0,0,0, 0xFF);
    //SDL_RenderClear(renderer);
    int v = 0;
    for(int i = 0; i < 255;i++){
       if(reverse){
          v = 255-i; 
       } else {
          v = i;
       }
     //    SDL_SetRenderDrawColor(renderer, i,i,i, 0xFF);
    //     SDL_RenderClear(renderer);
   //      SDL_RenderPresent( renderer );
   //      nanosleep(&t,NULL);
   //rot=i*360/255;
         //SDL_SetTextureBlendMode(text, SDL_BLENDMODE_BLEND);
         //SDL_SetTextureAlphaMod(text,i);
         SDL_SetTextureColorMod(text, v, v, v);
         showTexture(text, rot);
         nanosleep(&t,NULL);
    }
    return true;
}

SDL_Texture *loadImage(char *name)
{
    bool mode2d = false;
    bool success = true;
    SDL_Surface* image = NULL;
    SDL_Texture* text = NULL;

    if(mode2d == true){
        image = IMG_Load( name );
        if( image == NULL )
        {
            slog(ERROR, LOG_CORE, "Unable to load image %s! SDL Error: %s", name, SDL_GetError() );
            return false;
        }
        SDL_Surface *optimizedSurface = SDL_ConvertSurface( image, screenSurface->format, 0 );

        if(SDL_BlitScaled( optimizedSurface, NULL, screenSurface, NULL )){
            slog(ERROR, LOG_CORE, "Unable to blit image %s! SDL Error: %s", name, SDL_GetError() );
            return false;
        }
        SDL_UpdateWindowSurface( window );
    } else {
    
       SDL_Rect rect={0,0,0,0};
       text = IMG_LoadTexture(renderer,name);
       if(text == NULL){
           slog(ERROR, LOG_CORE, "Error loading image : %s",IMG_GetError());
           return false;
       }
       SDL_SetTextureBlendMode(text, SDL_BLENDMODE_BLEND);
       //if(rot == 0){
       //		SDL_RenderCopy( renderer, text, NULL, NULL);
       //} else {
       //		SDL_RenderCopyEx( renderer, text, NULL, NULL,rot, NULL,SDL_FLIP_NONE);
       //}
       //SDL_DestroyTexture(text);
       //SDL_RenderPresent( renderer );
    }
    return text;
}

bool loadFont(char *file, int size){
   font = TTF_OpenFont( file, size );
   if ( font == NULL ) {
      slog(ERROR, LOG_CORE, "Failed to load font : %s ",TTF_GetError());
      return false;
   } else {
      return true;
   }
}


void closeSDL()
{
    SDL_DestroyRenderer( renderer );
    SDL_DestroyWindow( window );
    window = NULL;
    renderer = NULL;
    SDL_Quit();
}

SDL_Texture* renderLog(char *str,int *w, int *h)
{
   SDL_Rect dest;
   SDL_Surface *rsurf,*surf;
   SDL_Texture *text;

   surf = TTF_RenderUTF8_Blended( font, str, black );

   text = SDL_CreateTextureFromSurface( renderer, surf );

   SDL_QueryTexture( text, NULL, NULL, w, h );
   //*w = dest.w;
   //*h = dest.h;
//   slog("Font text : %dx%d",dest.w, dest.h);
   //memcpy(d, &dest, sizeof(dest));
   SDL_FreeSurface( surf );
   return text;
}

void* logListener(void *ptr){
   int i = 0;
   sleep(4);
   int sockfd, newsockfd, portno;
   socklen_t clilen;
   struct sockaddr_in serv_addr, cli_addr;
   int  n;
   //char buffer[1024];
   char *buffer;
   char *oldStr = NULL;
   SDL_Event sdlevent;
   
   /* First call to socket() function */
   sockfd = socket(AF_INET, SOCK_STREAM, 0);
   
   if (sockfd < 0) {
      perror("ERROR opening socket");
      exit(1);
   }
   
   /* Initialize socket structure */
   bzero((char *) &serv_addr, sizeof(serv_addr));
   portno = 1109;
   
   serv_addr.sin_family = AF_INET;
   serv_addr.sin_addr.s_addr = INADDR_ANY;
   serv_addr.sin_port = htons(portno);
   
   /* Now bind the host address using bind() call.*/
   if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
      perror("ERROR on binding");
      exit(1);
   }
      
   listen(sockfd,5);
   clilen = sizeof(cli_addr);
   
   while (1) {
      newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
      if (newsockfd < 0) {
         perror("ERROR on accept");
         return NULL;
      }
  
      while ( (n = sgetline(newsockfd, &buffer))){
//      bzero(buffer,256);
//      n = read( newsockfd,buffer,255 );
      
         if (n < 0) {
            close(newsockfd);
            break;
         }
         n = write(newsockfd,buffer,n);
      
         if (n < 0) {
            close(newsockfd);
            break;
         }
         oldStr = logStr;
         logStr = strndup(buffer,n-1);

         sdlevent.type = SDL_USEREVENT;
         SDL_PushEvent(&sdlevent);

         free(buffer);
         free(oldStr);
      }
   }
   return NULL;
}

int sgetline(int fd, char ** out)
{
    int buf_size = 1024;
    int bytesloaded = 0;
    int ret;
    char buf;
    char * buffer = malloc(buf_size);
    char * newbuf;

    if (NULL == buffer)
        return -1;

    while ( 1 )
    {
        // read a single byte
        ret = read(fd, &buf, 1);
        if (ret < 1)
        {
            // error or disconnect
            free(buffer);
            return -1;
        }

        buffer[bytesloaded] = buf;
        bytesloaded++;

        // has end of line been reached?
        if (buf == '\n')
            break; // yes

        // is more memory needed?
        if (bytesloaded >= buf_size)
        {
            buf_size += 128;
            newbuf = realloc(buffer, buf_size);

            if (NULL == newbuf)
            {
                free(buffer);
                return -1;
            }

            buffer = newbuf;
        }
    }

    // if the line was terminated by "\r\n", ignore the
    // "\r". the "\n" is not in the buffer
    if ((bytesloaded) && (buffer[bytesloaded-1] == '\r'))
        bytesloaded--;

    *out = buffer; // complete line
    return bytesloaded; // number of bytes in the line, not counting the line break
}

void processStartupScript(char *file){
  slog(DEBUG,LOG_CORE,"Reading wallystart config : %s",file);
  long fsize=0;
  char *cmds=NULL;

  FILE *f = fopen(file, "rb");
  if(!f){
      slog(DEBUG,LOG_CORE,"File not found. Not running any startup commands");
      return;
  }

  fseek(f, 0, SEEK_END);
  fsize = ftell(f);
  fseek(f, 0, SEEK_SET);

  cmds = malloc(fsize + 1);
  fread(cmds, fsize, 1, f);
  fclose(f);

  cmds[fsize] = 0;
  slog(DEBUG,LOG_CORE,"Processing %d bytes from startupScript",fsize);
  processCommand(cmds);
  free(cmds);
}

bool processCommand(char *buf)
{
    int ret;
    int validCmd = 0;
    bool nextLine = true;
    char *lineBreak, *spaceBreak;
    char *lineCopy = NULL;
    char *cmd = strtok_r(buf,"\n",&lineBreak);
    while( nextLine ){
        // TODO : Keep track of this and clean it up!
        unsigned long cmdLen = strlen(cmd);
        lineCopy = repl_str(cmd, "$CONF", WALLYD_CONFDIR);
        void *linePtr = lineCopy;
        slog(DEBUG,LOG_CORE,"Processing line (%d) : %s",cmdLen,lineCopy);
        if(cmd[0] != '#') {
            validCmd++;
            char *myCmd = strsep(&lineCopy, " ");
            if(strcmp(myCmd,"fadein") == 0){
                char *delayStr = strsep(&lineCopy, " ");
                char *file  = strsep(&lineCopy, " ");
                long delay = atol(delayStr);
                slog(DEBUG,LOG_CORE,"Fadein %s with delay %u",file, delay);
                if(file && delay) {
                    t1 = loadImage(file);
                    fadeImage(t1, rot, false, 9000000);
                } else {
                    slog(DEBUG,LOG_CORE,"fadein <delay> <file>");
                }
            }
            else if(strcmp(myCmd,"fadeover") == 0){
                char *delayStr = strsep(&lineCopy, " ");
                char *file  = strsep(&lineCopy, " ");
                long delay = atol(delayStr);
                slog(DEBUG,LOG_CORE,"Fadeover %s with delay %u",file, delay);
                if(file && delay) {
                    t2 = loadImage(file);
                    fadeOver(t1, t2, rot, 9000000);
                    SDL_DestroyTexture(t1);
                    t1 = t2;
                } else {
                    slog(DEBUG,LOG_CORE,"fadeover <delay> <file>");
                }
            }
            else if(strcmp(myCmd,"fadeout") == 0){
                char *delayStr = strsep(&lineCopy, " ");
                long delay = 4500000;
                if(delayStr != NULL) {
                    delay = atol(delayStr);
                }
                slog(DEBUG,LOG_CORE,"Fadeout with delay %u",delay);
                if(delay) {
                    fadeImage(t1, rot, true, delay);
                } else {
                    slog(DEBUG,LOG_CORE,"fadeout <delay>");
                }
                SDL_DestroyTexture(t1);
            }
            else if(strcmp(myCmd,"rot") == 0){
                char *rotStr = strsep(&lineCopy, " ");
                rot = atoi(rotStr);
                slog(DEBUG,LOG_CORE,"Set rotation to %u", rot);
            }
            else if(strcmp(myCmd,"sleep") == 0){
                char *sleepStr = strsep(&lineCopy, " ");
                int sl = atoi(sleepStr);
                slog(DEBUG,LOG_CORE,"Sleeping %u sec", sl);
                sleep(sl);
            }
            else {
                slog(WARN,LOG_CORE,"Command not valid : %s", cmd);
                validCmd--;
            }
        } else {
            slog(DEBUG,LOG_CORE,"Ignoring comment line");
        }
        free(linePtr);
        cmd = strtok_r(NULL,"\n",&lineBreak);
        if(cmd == NULL) nextLine=false;
    }
    slog(DEBUG,LOG_CORE,"Command stack executed.");
    return validCmd;
}

char *repl_str(const char *str, const char *from, const char *to) {

	/* Adjust each of the below values to suit your needs. */

	/* Increment positions cache size initially by this number. */
	size_t cache_sz_inc = 16;
	/* Thereafter, each time capacity needs to be increased,
	 * multiply the increment by this factor. */
	const size_t cache_sz_inc_factor = 3;
	/* But never increment capacity by more than this number. */
	const size_t cache_sz_inc_max = 1048576;

	char *pret, *ret = NULL;
	const char *pstr2, *pstr = str;
	size_t i, count = 0;
	#if (__STDC_VERSION__ >= 199901L)
	uintptr_t *pos_cache_tmp, *pos_cache = NULL;
	#else
	ptrdiff_t *pos_cache_tmp, *pos_cache = NULL;
	#endif
	size_t cache_sz = 0;
	size_t cpylen, orglen, retlen, tolen, fromlen = strlen(from);

	/* Find all matches and cache their positions. */
	while ((pstr2 = strstr(pstr, from)) != NULL) {
		count++;

		/* Increase the cache size when necessary. */
		if (cache_sz < count) {
			cache_sz += cache_sz_inc;
			pos_cache_tmp = realloc(pos_cache, sizeof(*pos_cache) * cache_sz);
			if (pos_cache_tmp == NULL) {
				goto end_repl_str;
			} else pos_cache = pos_cache_tmp;
			cache_sz_inc *= cache_sz_inc_factor;
			if (cache_sz_inc > cache_sz_inc_max) {
				cache_sz_inc = cache_sz_inc_max;
			}
		}

		pos_cache[count-1] = pstr2 - str;
		pstr = pstr2 + fromlen;
	}

	orglen = pstr - str + strlen(pstr);

	/* Allocate memory for the post-replacement string. */
	if (count > 0) {
		tolen = strlen(to);
		retlen = orglen + (tolen - fromlen) * count;
	} else	retlen = orglen;
	ret = malloc(retlen + 1);
	if (ret == NULL) {
		goto end_repl_str;
	}

	if (count == 0) {
		/* If no matches, then just duplicate the string. */
		strcpy(ret, str);
	} else {
		/* Otherwise, duplicate the string whilst performing
		 * the replacements using the position cache. */
		pret = ret;
		memcpy(pret, str, pos_cache[0]);
		pret += pos_cache[0];
		for (i = 0; i < count; i++) {
			memcpy(pret, to, tolen);
			pret += tolen;
			pstr = str + pos_cache[i] + fromlen;
			cpylen = (i == count-1 ? orglen : pos_cache[i+1]) - pos_cache[i] - fromlen;
			memcpy(pret, pstr, cpylen);
			pret += cpylen;
		}
		ret[retlen] = '\0';
	}

end_repl_str:
	/* Free the cache and return the post-replacement string,
	 * which will be NULL in the event of an error. */
	free(pos_cache);
	return ret;
}
