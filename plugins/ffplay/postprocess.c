#include "postprocess.h"

unsigned postproc_version(void)
{
    return LIBPOSTPROC_VERSION_INT;
}

const char *postproc_configuration(void)
{
#ifdef WALLY_PLUGIN
    return NULL;
#else
    return FFMPEG_CONFIGURATION;
#endif
}

