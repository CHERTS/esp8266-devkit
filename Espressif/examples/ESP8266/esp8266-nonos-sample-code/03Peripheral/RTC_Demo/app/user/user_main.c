/******************************************************************************
 * Copyright 2013-2014 Espressif Systems (Wuxi)
 *
 * FileName: user_main.c
 *
 * Description: entry file of user application
 *
 * Modification history:
 *     2014/1/1, v1.0 create this file.
*******************************************************************************/
#include "ets_sys.h"
#include "osapi.h"
#include "user_interface.h"

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


os_timer_t rtc_test_t;
#define  RTC_MAGIC  0x55aaaa55


typedef struct {
uint64 time_acc;
uint32 magic ;
uint32 time_base;
}RTC_TIMER_DEMO;


void rtc_count()
{
	os_printf("1\r\n");
    RTC_TIMER_DEMO rtc_time;
    static uint8 cnt = 0;
    system_rtc_mem_read(64, &rtc_time, sizeof(rtc_time));
    os_printf("RTC TIMER: %d \r\n",system_get_rtc_time());
    os_printf("SYS TIMER: %d \r\n",system_get_time());

    if(rtc_time.magic!=RTC_MAGIC){
		os_printf("time acc: %d \r\n",rtc_time.time_acc);
		os_printf("time base: %d \r\n",rtc_time.time_base);
		os_printf("magic : 0x%08x\r\n",rtc_time.magic);
	    os_printf("rtc time init...\r\n");
	    rtc_time.magic = RTC_MAGIC;
    	rtc_time.time_acc= 0;
    	rtc_time.time_base = system_get_rtc_time();
    	os_printf("time base : %d \r\n",rtc_time.time_base);
    }
	else{
		os_printf("magic correct\r\n");
		os_printf("time acc: %d \r\n",rtc_time.time_acc);
		os_printf("time base: %d \r\n",rtc_time.time_base);
		os_printf("magic : 0x%08x\r\n",rtc_time.magic);
    }
    
    os_printf("==================\r\n");
    os_printf("RTC time test : \r\n");
    
    uint32 rtc_t1,rtc_t2;
    uint32 st1,st2;
    uint32 cal1, cal2;
    
    rtc_t1 = system_get_rtc_time();
    st1 = system_get_time();
    
    cal1 = system_rtc_clock_cali_proc();//查询RTC时钟周期，每分钟调用一次
    os_delay_us(300);
    
    st2 = system_get_time();
    rtc_t2 = system_get_rtc_time();
    cal2 = system_rtc_clock_cali_proc();
    os_printf(" rtc_t2-t1 : %d \r\n",rtc_t2-rtc_t1);
    os_printf(" st2-t2 :  %d  \r\n",st2-st1);
    os_printf("cal 1  : %d.%d  \r\n", ((cal1*1000)>>12)/1000, ((cal1*1000)>>12)%1000 );
    os_printf("cal 2  : %d.%d \r\n",((cal2*1000)>>12)/1000,((cal2*1000)>>12)%1000 );
    os_printf("==================\r\n\r\n");
    rtc_time.time_acc += (  ((uint64)(rtc_t2 - rtc_time.time_base))  *  ( (uint64)((cal2*1000)>>12))  ) ;//???
    os_printf("rtc time acc  : %lld \r\n",rtc_time.time_acc);
    os_printf("power on time :  %lld  us\r\n", rtc_time.time_acc/1000);
    os_printf("power on time :  %lld.%02lld  S\r\n", (rtc_time.time_acc/10000000)/100, (rtc_time.time_acc/10000000)%100);
    
    rtc_time.time_base = rtc_t2;
    system_rtc_mem_write(64, &rtc_time, sizeof(rtc_time));
    os_printf("------------------------\r\n");
    
    if(5== (cnt++)){
    	os_printf("system restart\r\n");
    	system_restart();
    }else{
    	os_printf("continue ...\r\n");
    }
	os_printf("count: %d\r\n",cnt);
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
    rtc_count();
    os_printf("SDK version:%s\n", system_get_sdk_version());
    
    os_timer_disarm(&rtc_test_t);
    os_timer_setfn(&rtc_test_t,rtc_count,NULL);
    os_timer_arm(&rtc_test_t,5000,1);
}

