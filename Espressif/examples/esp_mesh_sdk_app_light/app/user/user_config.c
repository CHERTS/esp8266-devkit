#include "user_config.h"
#include "osapi.h"

#if 0
#define ESP_MESH_SUPPORT    1  /*set 1: enable mesh*/
#define ESP_TOUCH_SUPPORT  1   /*set 1: enable esptouch*/
#define ESP_NOW_SUPPORT 1      /*set 1: enable espnow*/
#define ESP_WEB_SUPPORT 0      /*set 1: enable new http webserver*/
#define ESP_MDNS_SUPPORT 0     /*set 1: enable mdns*/

#define ESP_MESH_STRIP  1

#define ESP_DEBUG_MODE  1
#define ESP_RESET_DEBUG_EN 1 

#define MESH_TEST_VERSION   0x4
#endif

typedef struct{
uint8 csum;
uint8 Param_ESPMeshFunc;
uint8 Param_ESPTOUCHFunc;
uint8 Param_ESPNOWFunc;
uint8 Param_ESPWEBFunc;
uint8 Param_MDNSFunc;

}LightFuncParam;


LightFuncParam lightFuncParam;


bool ICACHE_FLASH_ATTR
config_ParamCsumCheck(void* pData, int length)
{
	uint8 csum_cal=0;
	int i ;
	char* tmp = (char*)pData;
	
	for(i=0;i<length;i++){
		csum_cal+= *(tmp+i);
	}
	
	if(csum_cal==0xff)
		return true;
	else
		return false;
}

void ICACHE_FLASH_ATTR
	config_ParamCsumSet(void* pData,uint8* csum,int length)
{
	uint8 csum_cal=0;
	int i;
	char* tmp = (char*)pData;
	
	for(i=0;i<length;i++){
		csum_cal+= *(tmp+i);
	}
	*csum = (uint8)(0xff - (csum_cal-(*csum)));
}


void ICACHE_FLASH_ATTR
config_ParamLoad(uint32 addr,uint8* pParam,int length)
{
    spi_flash_read(addr,(uint32*)pParam,length);


}

void ICACHE_FLASH_ATTR
config_ParamLoadWithProtect(uint16 start_sec,uint16 offset,uint8* pParam,int length)
{
	system_param_load(start_sec,offset,pParam,length);
}

void ICACHE_FLASH_ATTR
	config_ParamSaveWithProtect(uint16 start_sec,uint8* pParam,int length)
{
	system_param_save_with_protect(start_sec,pParam,length);
}





