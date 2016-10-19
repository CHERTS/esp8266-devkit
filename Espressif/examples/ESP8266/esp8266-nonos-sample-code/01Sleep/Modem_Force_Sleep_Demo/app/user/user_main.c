#include "ets_sys.h"
#include "osapi.h"
#include "user_interface.h"
#include "espconn.h"

#define FPM_SLEEP_MAX_TIME 0xFFFFFFF

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

void fpm_wakeup_cb_func1(void)
{
	os_printf("wakeup automatically\n");
	wifi_fpm_close();			//disable force sleep function
	struct station_config stationConf;
	wifi_set_opmode(STATION_MODE);
	os_memset(&stationConf, 0, sizeof(struct station_config));
	os_sprintf(stationConf.ssid, "TP-LINK-FD");
	os_sprintf(stationConf.password, "aaaaaaaa");
	wifi_station_set_config_current(&stationConf);
	wifi_set_event_handler_cb(wifi_handle_event_cb);
	wifi_station_connect();	
}
#ifdef SLEEP_MAX
void wakeup_func(void)
{
	wifi_fpm_do_wakeup();
	wifi_fpm_close(); // disable force sleep function
	struct station_config stationConf;
	wifi_set_opmode(STATION_MODE);
	os_memset(&stationConf, 0, sizeof(struct station_config));
	os_sprintf(stationConf.ssid, "TP-LINK-FD");
	os_sprintf(stationConf.password, "aaaaaaaa");
	wifi_station_set_config_current(&stationConf);
	wifi_set_event_handler_cb(wifi_handle_event_cb);
	wifi_station_connect();	
}
#endif
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

	wifi_station_disconnect();
	wifi_set_opmode(NULL_MODE);   				//set wifi mode to null mode
	wifi_fpm_set_sleep_type(MODEM_SLEEP_T);		//modem sleep
	wifi_fpm_open();							//enable force sleep
#ifdef SLEEP_MAX
	/* For modem sleep, FPM_SLEEP_MAX_TIME can only be wakened by calling
			wifi_fpm_do_wakeup. */
	
	wifi_fpm_do_sleep(FPM_SLEEP_MAX_TIME);
#else
	// wakeup automatically when timeout.
	os_printf("force sleep ,wait for a while...\n");
	wifi_fpm_set_wakeup_cb(fpm_wakeup_cb_func1);  // Set wakeup callback
	wifi_fpm_do_sleep(10*1000*1000);
#endif
	
}
