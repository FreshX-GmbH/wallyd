#include <SDL.h>
#include <SDL_image.h>
#include <stdbool.h>
#include <time.h>

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
SDL_Surface* screenSurface = NULL;
SDL_Event event;
bool quit = false;

bool loadSDL(bool);
bool loadImage(char *name,bool mode);
void closeSDL();
bool dumpModes(void);

int main( int argc, char* args[] )
{
    bool mode2d = false;
    int fileargnum = 1;

    if(argc < 2){
        printf("Usage : %s [-2] <imagefile>\n\t-2 for 2D surface mode only\n",args[0]);
        exit(1);
    }
    if(argc > 2){
        if(args[1]=='2'){
            printf("Using 2D/Surface mode only\n");
            mode2d = true;
            fileargnum=2;
        }
    }
    if(!loadSDL(mode2d)){
        exit(1);
    }
    if(!dumpModes()){
        exit(1);
    }
    if(!loadImage(args[fileargnum],mode2d)){
        exit(1);
    }

    while(!quit)
    {
        while(SDL_PollEvent(&event) != 0)
        {
            if(event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_APP_TERMINATING || event.type == SDL_KEYDOWN || event.type == SDL_QUIT)
            {
                quit = true;
            }
        }
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
bool loadSDL(bool mode2d)
{
    struct timespec t = { 0, 6000000};
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
       renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED| SDL_RENDERER_TARGETTEXTURE);
       if(renderer == NULL){
            printf( "Renderer could not initialize : %s\n", IMG_GetError() );
            return false;
       }
       for(int i = 0; i <255;i++){
            SDL_SetRenderDrawColor(renderer, i, i, i, 0xFF);
            SDL_RenderClear(renderer);
            SDL_RenderPresent( renderer );
            nanosleep(&t,NULL);
       }
    }
    return true;
}

bool loadImage(char *name,bool mode2d)
{
    bool success = true;
    SDL_Surface* image = NULL;

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
       SDL_Texture *text = IMG_LoadTexture(renderer,name);
       if(text == NULL){
           printf("Error loading image : %s\n",IMG_GetError());
           return false;
       }
//       SDL_QueryTexture( text, NULL, NULL, &rect.w, &rect.h );
//       rect.x = 0;
//       rect.y = 0;
//       SDL_Rect mr = {0, 0, TI->rect->w, TI->rect->h};
   
//       SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
       SDL_RenderCopy( renderer, text, NULL, NULL);
       SDL_DestroyTexture(text);
       SDL_RenderPresent( renderer );
    }
    return true;
}


void closeSDL()
{
    SDL_DestroyRenderer( renderer );
    SDL_DestroyWindow( window );
    window = NULL;
    renderer = NULL;
    SDL_Quit();
}
