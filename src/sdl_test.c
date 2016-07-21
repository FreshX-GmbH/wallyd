#include <SDL.h>
#include <stdbool.h>

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
SDL_Event event;
bool quit = false;

void loadSDL();
void closeSDL();

int main( int argc, char* args[] )
{
    loadSDL();
    while(!quit)
    {
        while(SDL_PollEvent(&event) != 0)
        {
            if(event.type == SDL_KEYDOWN)
            {
                quit = true;
            }
        }

        SDL_RenderClear( renderer );
        SDL_RenderPresent( renderer );
    }
    closeSDL();
    return 0;
}

void loadSDL()
{
    SDL_Init( SDL_INIT_VIDEO );
    window = SDL_CreateWindow("wallyd", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 0,0, SDL_WINDOW_OPENGL | SDL_WINDOW_FULLSCREEN_DESKTOP | SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_SetRenderDrawColor(renderer, 0x0, 0x0, 0x0, 0xFF);
}

void closeSDL()
{
    SDL_DestroyRenderer( renderer );
    SDL_DestroyWindow( window );
    window = NULL;
    renderer = NULL;
    SDL_Quit();
}
