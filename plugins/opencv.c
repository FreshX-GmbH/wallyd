#include "../lib/plugins.h"
#include "../lib/util.h"
#include <stdbool.h>

#define PLUGIN_SCOPE "opencv"

#ifdef WALLY_OPENCV
#include <opencv/cv.h>
#include <opencv/highgui.h>

int saveImage(char *filename)
{
    int c;
    IplImage* color_img;
    slog(LVL_NOISY,DEBUG,"Saving file to %s",filename);
    CvCapture* cv_cap = cvCaptureFromCAM(0);

    if(!cv_cap){
        slog(LVL_QUIET,ERROR, "Could not open cam");
        return -1;
    }
    color_img = cvQueryFrame(cv_cap);
    if(color_img != 0){
        slog(LVL_NOISY,DEBUG,"Saving file to %s",filename);
        cvSaveImage(filename, color_img, 0);
    }
    cvReleaseCapture( &cv_cap );
    return 0;
}
#else
int saveImage(char *filename){
    slog(LVL_INFO,WARN,"Plugin "PLUGIN_SCOPE" not enabled at compule time.");
    return false;
}
#endif

char *cleanupPlugin(void *p){
     slog(LVL_NOISY,DEBUG,"Plugin "PLUGIN_SCOPE" uninitialized");
     return NULL;
}

char *initPlugin(pluginHandler *_ph){
    slog(LVL_NOISY,FULLDEBUG,"Plugin "PLUGIN_SCOPE" initializing");
    exportSync(PLUGIN_SCOPE"::saveImage",(*saveImage));
    slog(LVL_NOISY,FULLDEBUG,"Plugin initialized. PH is at 0x%x",_ph);
    return PLUGIN_SCOPE;
}
