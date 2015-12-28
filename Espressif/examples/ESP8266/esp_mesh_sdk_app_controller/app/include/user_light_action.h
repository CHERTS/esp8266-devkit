#ifndef _USER_LIGHT_ACTION_H
#define _USER_LIGHT_ACTION_H

#include "user_config.h"
#include "os_type.h"

enum{
COLOR_SET = 0,
COLOR_CHG ,
COLOR_TOGGLE,
COLOR_LEVEL,
LIGHT_RESET
};


#define CMD_NUM 10
#define LIGHT_DEV_NUM  10

/*right now , all the MAC address is hard coded and can not be changed */
/*One controller is bind to the certain groups of MAC address.*/
/*next , we will add a encryption for ESP-NOW.*/
/* and send broadcast instead on unicast.*/
#define ESPNOW_ENCRYPT  1
#define ESPNOW_KEY_HASH 0

#define ESPNOW_KEY_LEN 16


#define DEV_MAC_LEN 6

#define ESPNOW_PARAM_MAGIC 0x5c5caacc
#define ESPNOW_PARAM_SEC 0X7D
#define ESPNOW_DATA_MAGIC 0x5cc5




#if LIGHT_DEVICE
void light_EspnowInit();
void light_EspnowDeinit();
#elif LIGHT_SWITCH
void  switch_EspnowInit();
void  switch_EspnowSendLightCmd(uint16 idx, uint16 channelNum, uint32* duty, uint32 period,uint32 code);
void switch_EspnowSendCmdByChnl(uint16 chn,uint16 channelNum, uint32* duty, uint32 period,uint32 code);
void switch_EspnowSendChnSync(uint8 channel);
void  switch_EspnowDeinit();
#endif










#endif
