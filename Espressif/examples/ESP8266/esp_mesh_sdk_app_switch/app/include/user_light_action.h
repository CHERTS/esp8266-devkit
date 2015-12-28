#ifndef _USER_LIGHT_ACTION_H
#define _USER_LIGHT_ACTION_H


#include "user_config.h"
#include "os_type.h"


//Light state. Bitfield.
typedef enum {
	LST_ACTIVE=(1<<0),
	LST_ACKED=(1<<1)
} LightStatusEnum;


typedef struct {
	uint8_t mac[6];
	uint16_t status;
} LampMacType;



#define CMD_NUM 10
#define LIGHT_DEV_NUM  10






#if LIGHT_DEVICE
void light_EspnowInit();
void light_EspnowDeinit();



#elif LIGHT_SWITCH
void  switch_EspnowInit();
void  switch_EspnowSendLightCmd(uint8 idx, uint32 channelNum, uint32* duty, uint32 period);
void switch_EspnowSendCmdByChnl(uint8 chn,uint32 channelNum, uint32* duty, uint32 period);
void switch_EspnowSendChnSync(uint8 channel);
void  switch_EspnowDeinit();
#endif










#endif
