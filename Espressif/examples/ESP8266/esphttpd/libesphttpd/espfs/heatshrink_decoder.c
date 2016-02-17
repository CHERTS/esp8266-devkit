#include "espfs.h"
#ifdef ESPFS_HEATSHRINK
//Stupid wrapper so we don't have to move c-files around
//Also loads httpd-specific config.

#ifdef __ets__
//esp build

#include <esp8266.h>

#endif

#include "heatshrink_config_custom.h"
#include "../lib/heatshrink/heatshrink_decoder.c"


#endif
