#ifndef WALLY_SVG
#define WALLY_SVG
//  Simple SVG plugin, based on nanosvg
#include "../lib/plugins.h"
#include "../lib/ui.h"
#include "../lib/util.h"

#include <stdio.h>
#include <string.h>
#include <float.h>
#define NANOSVG_IMPLEMENTATION
#include "nanosvg/nanosvg.h"
#define NANOSVGRAST_IMPLEMENTATION
#include "nanosvg/nanosvgrast.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "nanosvg/stb_image_write.h"

pluginHandler *ph;

#endif
