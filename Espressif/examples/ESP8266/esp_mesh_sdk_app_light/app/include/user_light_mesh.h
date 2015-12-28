#ifndef USER_LIGHT_MESH_H
#define USER_LIGHT_MESH_H
#include "user_config.h"
#if ESP_MESH_SUPPORT

#include "os_type.h"
#include "eagle_soc.h"
#include "c_types.h"
#include "osapi.h"
#include "ets_sys.h"
#include "mem.h"
#include "user_config.h"
#include "user_light_adj.h"
#include "user_light.h"
#include "user_interface.h"

typedef void (*mesh_FailCallback)(void *para);
typedef void (*mesh_SuccessCallback)(void *para);
typedef void (*mesh_InitTimeoutCallback)(void *para);


typedef struct  {
	mesh_FailCallback mesh_fail_cb;
	mesh_SuccessCallback mesh_suc_cb;
	mesh_InitTimeoutCallback mesh_init_tout_cb;
	uint32 start_time;
	uint32 init_retry;
} LIGHT_MESH_PROC;


void user_MeshInit();
void user_MeshSetInfo();
void mesh_StopReconnCheck();
char* mesh_GetMdevMac();


#endif
#endif
