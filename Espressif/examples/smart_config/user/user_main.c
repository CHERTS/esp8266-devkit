#include "ets_sys.h"
#include "osapi.h"
#include "driver/uart.h"

#include "user_interface.h"
#include "smartconfig.h"

void ICACHE_FLASH_ATTR
smartconfig_done(void *data)
{
	struct station_config *sta_conf = data;

	wifi_station_set_config(sta_conf);
	wifi_station_disconnect();
	wifi_station_connect();

	os_printf("SmartConfig done\r\n");
}

void user_init(void)
{
	UARTInit(BIT_RATE_115200);
	os_delay_us(1000);

	os_printf("\r\nSDK version:%s\r\n", system_get_sdk_version());

	smartconfig_start(SC_TYPE_AIRKISS, smartconfig_done);
}
