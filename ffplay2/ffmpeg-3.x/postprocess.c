#include "config.h"
#include "postprocess.h"

unsigned postproc_version(void)
{
    return LIBPOSTPROC_VERSION_INT;
}

const char *postproc_configuration(void)
{
    return FFMPEG_CONFIGURATION;
}

