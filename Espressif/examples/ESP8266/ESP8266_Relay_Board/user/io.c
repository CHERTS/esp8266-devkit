
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * Jeroen Domburg <jeroen@spritesmods.com> wrote this file. As long as you retain 
 * this notice you can do whatever you want with this stuff. If we meet some day, 
 * and you think this stuff is worth it, you can buy me a beer in return. 
 * ----------------------------------------------------------------------------
 */

#include "ets_sys.h"
#include "c_types.h"
#include "user_interface.h"
#include "espconn.h"
#include "mem.h"
#include "osapi.h"
#include "gpio.h"
#include "config.h"

#include "espmissingincludes.h"
#define BTNGPIO 0

static ETSTimer resetBtntimer;

 char currGPIO12State=0;
 char currGPIO13State=0;
 char currGPIO15State=0;

void ICACHE_FLASH_ATTR ioGPIO(int ena, int gpio) {
	//gpio_output_set is overkill. ToDo: use better macros
	if (ena) {
		gpio_output_set((1<<gpio), 0, (1<<gpio), 0);
	} else {
		gpio_output_set(0, (1<<gpio), (1<<gpio), 0);
	}
}

static void ICACHE_FLASH_ATTR resetBtnTimerCb(void *arg) {
	static int resetCnt=0;
	if (!GPIO_INPUT_GET(BTNGPIO)) {
		resetCnt++;
	} else {
		if (resetCnt>=6) { //3 sec pressed
			wifi_station_disconnect();
			wifi_set_opmode(0x3); //reset to AP+STA mode
			os_printf("Reset to AP mode. Restarting system...\n");
			system_restart();
		}
		resetCnt=0;
	}
}

void ICACHE_FLASH_ATTR ioInit() {
	
	//Set GPIO0, GPIO12-14 to output mode 
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDI_U, FUNC_GPIO12);
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTCK_U, FUNC_GPIO13);
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDO_U, FUNC_GPIO15);
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO0_U, FUNC_GPIO0);

	//set GPIO to init state
 	GPIO_OUTPUT_SET(0,(1<<0));
  	//GPIO_OUTPUT_SET(2,0);

	if( sysCfg.relay_latching_enable) {
	
	os_printf("Relay latching is %d, Relay1=%d,Relay2=%d,Relay3=%d\n\r",(int)sysCfg.relay_latching_enable,(int)sysCfg.relay_1_state,(int)sysCfg.relay_2_state,(int)sysCfg.relay_3_state);
	
		currGPIO12State=(int)sysCfg.relay_1_state;
		currGPIO13State=(int)sysCfg.relay_2_state;
		currGPIO15State=(int)sysCfg.relay_3_state;

		ioGPIO((int)sysCfg.relay_1_state,12);
		ioGPIO((int)sysCfg.relay_2_state,13);
		ioGPIO((int)sysCfg.relay_3_state,15);
	}
	
	else {
		ioGPIO(0,12);
		ioGPIO(0,13);
		ioGPIO(0,15);
	}

	//gpio_output_set(0, 0, (1<<12), (1<<BTNGPIO));
	//gpio_output_set(0, 0, (1<<13), (1<<14));
	
	os_timer_disarm(&resetBtntimer);
	os_timer_setfn(&resetBtntimer, resetBtnTimerCb, NULL);
	os_timer_arm(&resetBtntimer, 500, 1);
}
