#ifndef _SMARTCONFIG_FLOW_H
#define _SMARTCONFIG_FLOW_H
#include "user_config.h"
#include "smartconfig.h"
typedef void (*esptouch_StartAction)(void *para);
typedef void (*esptouch_FailCallback)(void *para);
typedef void (*esptouch_SuccessCallback)(void *para);

typedef struct  {
	sc_type esptouch_type;	
	esptouch_StartAction esptouch_start_cb;
	esptouch_FailCallback esptouch_fail_cb;
	esptouch_SuccessCallback esptouch_suc_cb;
} ESPTOUCH_PROC;

void esptouch_FlowStart();



#endif
