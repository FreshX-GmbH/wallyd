//
//  ffplay.h
//  FFplay_Mac
//
//  Created by ??????????? 13-12-5.
//  Copyright (c) 2013年 Clov4r. All rights reserved.
//

#ifndef FFplay_Mac_ffplay_h
#define FFplay_Mac_ffplay_h

#define AVCODEC_MAX_AUDIO_FRAME_SIZE 192000


//#include "config.h"
#include <inttypes.h>
#include <math.h>
#include <limits.h>
#include <signal.h>
#include "libavutil/avstring.h"
//#include "libavutil/
#include "colorspace.h"
#include "libavutil/mathematics.h"
#include "libavutil/pixdesc.h"
#include "libavutil/imgutils.h"
#include "libavutil/dict.h"
#include "libavutil/parseutils.h"
#include "libavutil/samplefmt.h"
#include "libavutil/avassert.h"
#include "libavutil/time.h"
#include "libavformat/avformat.h"
#include "libavdevice/avdevice.h"
#include "libswscale/swscale.h"
#include "libavutil/opt.h"
#include "libavcodec/avfft.h"
#include "libswresample/swresample.h"

#if CONFIG_AVFILTER
# include "libavfilter/avcodec.h"
# include "libavfilter/avfilter.h"
# include "libavfilter/avfiltergraph.h"
# include "libavfilter/buffersink.h"
# include "libavfilter/buffersrc.h"
#endif

#include <SDL2/SDL.h>
#include <SDL2/SDL_thread.h>
#include <SDL2/SDL_events.h>
#include "SDL2/SDL_syswm.h"

#define SDL_main main

#include "cmdutils.h"

#include <assert.h>

int ffplay(int argc, char **argv);

//extern SDL_Window *sdl_window;
extern const void *data;
     
#endif
