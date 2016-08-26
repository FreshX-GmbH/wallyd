#ifndef WALLY_PLUGIN_VIDEO
#define WALLY_PLUGIN_VIDEO

#include <SDL.h>
#include <SDL_thread.h>

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavformat/avio.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
#include <libavutil/avstring.h>
#include <libavutil/time.h>

#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <float.h>

#include "ui.h"
#include "plugins.h"

// Number of packets we can not read before we stop playing
#define VIDEO_FAIL_PACKETS  32

#define SDL_AUDIO_BUFFER_SIZE 1024
#define MAX_AUDIO_FRAME_SIZE 192000

#define MAX_AUDIOQ_SIZE (5 * 16 * 1024)
#define MAX_VIDEOQ_SIZE (5 * 256 * 1024)

#define MAX_AUDIO_SIZE (5 * 16 * 1024)
#define MAX_VIDEO_SIZE (5 * 256 * 1024)

#define AV_SYNC_THRESHOLD 0.01
#define AV_NOSYNC_THRESHOLD 10.0

#define SAMPLE_CORRECTION_PERCENT_MAX 10
#define AUDIO_DIFF_AVG_NB 20

#define VIDEO_PICTURE_QUEUE_SIZE 1
#define DEFAULT_AV_SYNC_TYPE AV_SYNC_VIDEO_MASTER

#define PRE_WIDTH 904
#define PRE_HEIGHT 600

#define VIDEO_PICTURE_QUEUE_SIZE 1
#define AVCODEC_MAX_AUDIO_FRAME_SIZE 192000 // 1 second of 48khz 32bit audio



typedef struct PacketQueue {
    AVPacketList *first_pkt, *last_pkt;
    int nb_packets;
    int size;
    SDL_mutex *mutex;
    SDL_cond *cond;
} PacketQueue;

typedef struct VideoPicture {
    AVFrame *rawdata;
    uint8_t *bufpoint;
    double pts;
    int width, height; /* source height & width */
    int allocated;
} VideoPicture;

typedef struct VideoState {
    AVFormatContext *ic;
    SwrContext *swr;
    int audio_freq;
    AVFormatContext *pFormatCtx;
    int videoStream, audioStream;
    int av_sync_type;
    double external_clock; /* external clock base */
    int64_t external_clock_time;
    double audio_clock;
    AVStream *audio_st;
    PacketQueue audioq;
    //uint8_t audio_buf[(MAX_AUDIO_FRAME_SIZE * 3) / 2];
    uint8_t *audio_buf;
    unsigned int audio_buf_size;
    unsigned int audio_buf_index;
    AVFrame *audio_frame;
    AVPacket audio_pkt;
    uint8_t *audio_pkt_data;
    int audio_pkt_size;
    int audio_hw_buf_size;
    double audio_diff_cum;                    /** used for AV difference average computation */
    double audio_diff_avg_coef;
    double audio_diff_threshold;
    int audio_diff_avg_count;
    enum AVSampleFormat audio_src_fmt;
    enum AVSampleFormat audio_tgt_fmt;
    int audio_src_channels;
    int audio_tgt_channels;
    int64_t audio_src_channel_layout;
    int64_t audio_tgt_channel_layout;
    int audio_src_freq;
    int audio_tgt_freq;
    AVIOContext *io_ctx;
    struct SwrContext *swr_ctx;
    DECLARE_ALIGNED(16,uint8_t,audio_buf2) [AVCODEC_MAX_AUDIO_FRAME_SIZE * 4];

    double frame_timer;
    double frame_last_pts;
    double frame_last_delay;

    double video_clock; ///<pts of last decoded frame / predicted pts of next decoded frame
    double video_current_pts; ///<current displayed pts (different from video_clock if frame fifos are used)
    int64_t video_current_pts_time; ///<time (av_gettime) at which we updated video_current_pts - used to have running video pts

    char texName;

    AVStream *video_st;
    PacketQueue videoq;
    VideoPicture pictq[VIDEO_PICTURE_QUEUE_SIZE];
    int pictq_size, pictq_rindex, pictq_windex;
    SDL_mutex *pictq_mutex;
    SDL_cond *pictq_cond;
    SDL_Thread *parse_tid;
    SDL_Thread *control_tid;
    SDL_Thread *video_tid;

    //char filename[1024];

    char *filename;
    int quit;
    bool pause;
    bool scaled;
    bool directRender;

    bool hasAudio;
    bool hasVideo;
    bool newTexture;
    char *screen;
    SDL_Texture *originalTexture;

    int scaledWidth;
    int scaledHeight;
    int videoPosX;
    int videoPosY;
    texInfo *TI;
    int read_frame_fail;

    AVIOContext *io_context;
    struct SwsContext *sws_ctx;
} VideoState;

SDL_AudioDeviceID dev;

enum {
    AV_SYNC_AUDIO_MASTER,
    AV_SYNC_VIDEO_MASTER,
    AV_SYNC_EXTERNAL_MASTER,
};

/* Since we only have one decoding thread, the Big Struct can be global in case we need it. */
VideoState *is;

extern pluginHandler *ph;
VideoState *is;
bool pluginMode;
static int global_readframe_cntout;
uint64_t global_video_pkt_pts;

void alloc_picture(void *userdata);
void video_refresh_timer(void *userdata);
int video_thread(void *arg);
double get_master_clock(VideoState *is);
void schedule_refresh(VideoState *is, int delay);
int packet_queue_get(PacketQueue *q, AVPacket *pkt, int block);

int decode_thread(void *arg);
double synchronize_video(VideoState *is, AVFrame *src_frame, double pts);
double get_video_clock(VideoState *is);
double get_audio_clock(VideoState *is);
int queue_picture(VideoState *is, AVFrame *pFrame, double pts);
void audio_callback(void *userdata, Uint8 *stream, int len);
int our_get_buffer(struct AVCodecContext *c, AVFrame *pic);
//int AudioResampling(AVCodecContext * audio_dec_ctx, AVFrame * pAudioDecodeFrame, int out_sample_fmt,
//                    int out_channels, int out_sample_rate, uint8_t* out_buf);

#ifdef DMALLOC
#include "dmalloc.h"
#endif

#endif
