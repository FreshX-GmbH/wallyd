#include "postprocess.h"

unsigned postproc_version(void)
{
#ifdef WALLY_PLUGIN
    return 0;
#else
    return LIBPOSTPROC_VERSION_INT;
#endif
}

const char *postproc_configuration(void)
{
#ifdef WALLY_PLUGIN
    return 0L;
#else
    return FFMPEG_CONFIGURATION;
#endif
}

