#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>
#include <stdbool.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>

#define VERSION "0.11"

#define BASE "."
#define FONT "/etc/wallyd.d/fonts/FreeMono.ttf"

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
SDL_Surface* screenSurface = NULL;
TTF_Font *font = NULL;
SDL_Event event;
SDL_Color black = {0,0,0,255};
SDL_Color white = {255,255,255,255};
bool quit = false;
int rot = 0, h = 0, w = 0;
pthread_t log_thr;

char *logStr = NULL;

bool loadSDL();
bool loadFont(char *file, int size);
bool fadeImage(SDL_Texture *text, int rot, bool reverse);
bool showTexture(SDL_Texture *text, int rot);
SDL_Texture* loadImage(char *name);
void closeSDL();
bool dumpModes(void);
SDL_Texture* renderLog(char *strTmp,int *w, int *h);
void* logListener(void *);
int sgetline(int fd, char ** out);

int main( int argc, char* args[] )
{
    SDL_Texture *t1 = NULL;
    SDL_Texture *t2 = NULL;
    SDL_Texture *t3 = NULL;
    int argadd = 0;
    logStr = strdup("Hello Wally!");

    if(argc < 2){
        printf("Usage : %s (V"VERSION") [degree] <defaultimage> <fadeimage 1> [<fadeimage 2> <...>]\n",args[0]);
        exit(1);
    }
    printf("%s (V"VERSION")\n" ,args[0]);
    rot = strtol(args[1],NULL,10);
    if(!rot) {
        printf("Could not convert %s to a number\n",args[1]);
    //    exit(1);
    }
    loadSDL();
    if(!dumpModes()){
        exit(1);
    }
    if(!loadFont(BASE""FONT,16)){
          exit(1);
    }
    if(pthread_create(&log_thr, NULL, &logListener, NULL) != 0){
       printf("Failed to create listener thread!\n");
       exit(1);
    }
    t1 = loadImage(args[2]);
    if(argc >3){
       t2 = loadImage(args[3]);
    }
    if(argc > 4){
       t3 = loadImage(args[4]);
    }
    printf("Screen size : %dx%d\n",w,h);
    sleep(1);
    if(t2){
      fadeImage(t2, rot, false);
      fadeImage(t2, rot, true);
      if(t3){
          if(strcmp(args[2],args[argc-1]) != 0){
              fadeImage(t3, rot, false);
              fadeImage(t3, rot, true);
          }
      }
    }
    // Fade in final image
    fadeImage(t1,rot, false);

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
       //printf("%p %p\n",text,renderer);
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
        printf("No VideoDisplays found.");
        return false;
    }
    printf("VideoDisplays: %i\n", display_count);

    for (j=0; j<display_count; j++){
        SDL_GetDisplayBounds(j,&r);
        printf("Display %d boundaries : %d x %d\n",j,r.w,r.h);
        // Store size of first display
        if(j == 0){
            w = r.w;
            h = r.h;
        }
        for (i=0; i<SDL_GetNumDisplayModes(j); i++){
          SDL_GetDisplayMode(j,i,&mode);
          f = mode.format;
          printf("Display %d / Mode %d : %d x %d x %d bpp (%s) @ %d Hz\n", j, i, mode.w, mode.h, SDL_BITSPERPIXEL(f), SDL_GetPixelFormatName(f), mode.refresh_rate);
        }
    }
    return true;
}
bool loadSDL()
{
    bool mode2d = false;
    if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER) != 0) {
        printf( "SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError() );
	return false;
    }
    int flags=IMG_INIT_JPG|IMG_INIT_PNG;
    int initted=IMG_Init(flags);
    if((initted&flags) != flags)
    {
        printf( "SDL_image could not initialize PNG and JPG! SDL_image Error: %s\n", IMG_GetError() );
	return false;
    }
    if ( TTF_Init() == -1 ) {
        printf( "SDL_TTF could not initialize! SDL_ttf Error: %s\n", TTF_GetError() );
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
            printf( "Renderer could not initialize : %s\n", IMG_GetError() );
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

bool fadeImage(SDL_Texture *text, int rot, bool reverse){
    struct timespec t = { 0, 5000000};
    SDL_SetRenderDrawColor(renderer, 0,0,0, 0xFF);
    SDL_RenderClear(renderer);
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
            printf( "Unable to load image %s! SDL Error: %s\n", name, SDL_GetError() );
            return false;
        }
        SDL_Surface *optimizedSurface = SDL_ConvertSurface( image, screenSurface->format, 0 );

        if(SDL_BlitScaled( optimizedSurface, NULL, screenSurface, NULL )){
            printf( "Unable to blit image %s! SDL Error: %s\n", name, SDL_GetError() );
            return false;
        }
        SDL_UpdateWindowSurface( window );
    } else {
    
       SDL_Rect rect={0,0,0,0};
       text = IMG_LoadTexture(renderer,name);
       if(text == NULL){
           printf("Error loading image : %s\n",IMG_GetError());
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
      printf("Failed to load font : %s ",TTF_GetError());
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
//   printf("Font text : %dx%d\n",dest.w, dest.h);
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
  
      while ( n = sgetline(newsockfd, &buffer)){
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
