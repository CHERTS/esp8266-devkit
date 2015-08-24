#ifndef __USER_CONFIGSTORE_H__
#define __USER_CONFIGSTORE_H__

#include "user_light_action.h"
#include "c_types.h"

#define CONFIG_MAC_CNT 16

typedef struct {
	uint32_t r;
	uint32_t g;
	uint32_t b;
	uint32_t cw;
	uint32_t ww;
} LedCol;


typedef struct {
	uint8_t chsum;
	uint8_t seq;
	uint8_t pad;
//	uint8_t wifiChan;
	LedCol bval[5];
//	LampMacType macData[CONFIG_MAC_CNT];
	int wlanChannel[CONFIG_MAC_CNT];
} MyConfig;

extern MyConfig myConfig;
void configLoad();
void configSave();

#endif