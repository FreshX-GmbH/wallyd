#include <stdio.h>
#include <SDL.h>

extern int renderVideo(char *filename);
extern char *initPlugin(void *params);

int main(int argc, char *argv[]){
   if(argc < 2){
      printf("Usage : video_test <file | url>\n");
      return 1;
   }
   SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER);
   SDL_Window *window = SDL_CreateWindow("wallyd", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
               0,0, SDL_WINDOW_OPENGL | SDL_WINDOW_FULLSCREEN_DESKTOP);

   printf("Initialized plugin : %s\n", initPlugin((void*)window));
   renderVideo(argv[1]);
}
