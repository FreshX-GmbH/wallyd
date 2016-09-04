#include <SDL.h>
#include <SDL_image.h>
#include <stdbool.h>

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
SDL_Surface* screenSurface = NULL;
SDL_Event event;
bool quit = false;

bool loadSDL(void);
bool loadImage(char *name);
void closeSDL();
bool dumpModes(void);

int main( int argc, char* args[] )
{
    if(argc < 2){
        printf("Usage : %s <imagefile>\n",args[0]);
        exit(1);
    }
    if(!loadSDL()){
        exit(1);
    }
    if(!dumpModes()){
        exit(1);
    }
    if(!loadImage(args[1])){
        exit(1);
    }
    SDL_UpdateWindowSurface( window );

    while(!quit)
    {
        while(SDL_PollEvent(&event) != 0)
        {
            if(event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_APP_TERMINATING || event.type == SDL_KEYDOWN || event.type == SDL_QUIT)
            {
                quit = true;
            }
        }
        //SDL_RenderClear( renderer );
        //SDL_RenderPresent( renderer );
    }
    closeSDL();
    return 0;
}

bool dumpModes(void)
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

    screenSurface = SDL_GetWindowSurface( window );
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_SetRenderDrawColor(renderer, 0x0, 0x0, 0x0, 0xFF);
    return true;
}

bool loadImage(char *name)
{
    bool success = true;
	SDL_Surface* image = NULL;

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
