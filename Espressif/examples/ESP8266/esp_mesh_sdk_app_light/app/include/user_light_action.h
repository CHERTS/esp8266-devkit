#ifndef _USER_LIGHT_ACTION_H
#define _USER_LIGHT_ACTION_H


#include "user_config.h"
#include "os_type.h"
#include "user_simplepair.h"

#define ACT_DEBUG 1


#if LIGHT_DEVICE
int light_EspnowGetBatteryStatus(int idx, char *mac, int *status, int *voltage_mv);
void light_EspnowInit();
void light_EspnowDeinit();
#endif

#endif
