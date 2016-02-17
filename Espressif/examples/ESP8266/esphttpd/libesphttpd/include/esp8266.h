// Combined include file for esp8266


#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef FREERTOS
#include <stdint.h>
#include <espressif/esp_common.h>

#else
#include <c_types.h>
#include <ip_addr.h>
#include <espconn.h>
#include <ets_sys.h>
#include <gpio.h>
#include <mem.h>
#include <osapi.h>
#include <user_interface.h>
#include <upgrade.h>
#endif

#include "platform.h"
#include "espmissingincludes.h"

