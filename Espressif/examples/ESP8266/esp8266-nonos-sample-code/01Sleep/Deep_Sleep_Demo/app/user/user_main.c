#include "ets_sys.h"
#include "osapi.h"
#include "user_interface.h"
#include "espconn.h"

/******************************************************************************
 * FunctionName : user_rf_cal_sector_set
 * Description  : SDK just reversed 4 sectors, used for rf init data and paramters.
 *                We add this function to force users to set rf cal sector, since
 *                we don't know which sector is free in user's application.
 *                sector map for last several sectors : ABCCC
 *                A : rf cal
 *                B : rf init data
 *                C : sdk parameters
 * Parameters   : none
 * Returns      : rf cal sector
*******************************************************************************/
uint32 ICACHE_FLASH_ATTR
user_rf_cal_sector_set(void)
{
    enum flash_size_map size_map = system_get_flash_size_map();
    uint32 rf_cal_sec = 0;

    switch (size_map) {
        case FLASH_SIZE_4M_MAP_256_256:
            rf_cal_sec = 128 - 5;
            break;

        case FLASH_SIZE_8M_MAP_512_512:
            rf_cal_sec = 256 - 5;
            break;

        case FLASH_SIZE_16M_MAP_512_512:
        case FLASH_SIZE_16M_MAP_1024_1024:
            rf_cal_sec = 512 - 5;
            break;

        case FLASH_SIZE_32M_MAP_512_512:
        case FLASH_SIZE_32M_MAP_1024_1024:
            rf_cal_sec = 1024 - 5;
            break;

        default:
            rf_cal_sec = 0;
            break;
    }

    return rf_cal_sec;
}

void UdpRecvCb(void *arg, char *pdata, unsigned short len)
{
	os_printf("udp message received:\n");
	os_printf("%s\r\n", pdata);
}


struct espconn udp_client;
esp_udp udp;


void wifi_handle_event_cb(System_Event_t *evt)
{
    os_printf("event %x\n", evt->event);
	int res;
    switch (evt->event) {
         case EVENT_STAMODE_GOT_IP:
			 udp_client.type=ESPCONN_UDP;
			 udp_client.proto.udp=&udp;
			 udp.local_port=8080;
			 espconn_regist_recvcb(&udp_client,UdpRecvCb);
			 res = espconn_create(&udp_client);
			 os_printf("%d\n",res);
             break;
         default:
             break;
 }
} 


os_timer_t set_sleep_time;
void set_sleep()
{
	os_printf("start sleeping...\n");
	system_deep_sleep(10000000);
}
/******************************************************************************
 * FunctionName : user_init
 * Description  : entry of user application, init user function here
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
#include "driver/uart.h"
void user_init(void)
{
    os_printf("SDK version:%s\n", system_get_sdk_version());
	
    struct station_config stationConf;
	wifi_set_opmode_current(STATION_MODE);
	os_memset(&stationConf, 0, sizeof(struct station_config));
	os_sprintf(stationConf.ssid, "TP-LINK-FD");
	os_sprintf(stationConf.password, "aaaaaaaa");
	wifi_station_set_config_current(&stationConf);
	wifi_set_event_handler_cb(wifi_handle_event_cb);
	wifi_station_connect();
	os_printf("system restart\n");
	os_timer_disarm(&set_sleep_time);
	os_timer_setfn(&set_sleep_time, set_sleep, NULL);
	
	os_timer_arm(&set_sleep_time, 10000, 1);
}
