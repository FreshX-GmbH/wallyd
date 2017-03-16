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
#include "../lib/slog.h"
#include "../lib/autoversion.h"

#define VERSION "0.14"
#define BASE "."
#define FONT "/etc/wallyd.d/fonts/FreeMono.ttf"
#define START WALLYD_CONFDIR"/wallystart.conf"

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
char *startupScript = "/etc/wallyd.d";
void* globalSLG;

bool loadSDL();
bool loadFont(char *file, int size);
bool fadeImage(SDL_Texture *text, int rot, bool reverse, long delay);
bool showTexture(SDL_Texture *text, int rot);
SDL_Texture* loadImage(char *name);
void closeSDL();
bool dumpModes(void);
SDL_Texture* renderLog(char *strTmp,int *w, int *h);
void* logListener(void *);
int sgetline(int fd, char ** out);
void processStartupScript(char *file);
bool processCommand(char *buf);
char *repl_str(const char *str, const char *from, const char *to);

