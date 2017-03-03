#include <SDL.h>
#include <SDL_image.h>
#include <stdbool.h>
#include <time.h>

#define VERSION "0.11"

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
SDL_Surface* screenSurface = NULL;
SDL_Event event;
bool quit = false;
int rot = 0;

bool loadSDL();
bool fadeImage(SDL_Texture *text, int rot, bool reverse);
bool showTexture(SDL_Texture *text, int rot);
SDL_Texture* loadImage(char *name);
void closeSDL();
bool dumpModes(void);


int main( int argc, char* args[] )
{
    SDL_Texture *t1 = NULL;
    SDL_Texture *t2 = NULL;
    SDL_Texture *t3 = NULL;
    int argadd = 0;

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
    t1 = loadImage(args[2]);
    if(argc >3){
       t2 = loadImage(args[3]);
    }
    if(argc > 4){
       t3 = loadImage(args[4]);
    }
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
               event.type == SDL_KEYDOWN || 
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

bool showTexture(SDL_Texture *text, int rot){
         if(rot == 0){
       	        SDL_RenderCopy( renderer, text, NULL, NULL);
         } else {
    	        SDL_RenderCopyEx( renderer, text, NULL, NULL,rot, NULL,SDL_FLIP_NONE);
         }
         SDL_RenderPresent( renderer );
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


void closeSDL()
{
    SDL_DestroyRenderer( renderer );
    SDL_DestroyWindow( window );
    window = NULL;
    renderer = NULL;
    SDL_Quit();
}