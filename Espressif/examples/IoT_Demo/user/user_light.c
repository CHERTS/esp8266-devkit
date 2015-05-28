/******************************************************************************
 * Copyright 2013-2014 Espressif Systems (Wuxi)
 *
 * FileName: user_light.c
 *
 * Description: light demo's function realization
 *
 * Modification history:
 *     2014/5/1, v1.0 create this file.
*******************************************************************************/
#include "ets_sys.h"
#include "osapi.h"
#include "os_type.h"
#include "mem.h"
#include "user_interface.h"

#include "user_light.h"

#if LIGHT_DEVICE

struct light_saved_param light_param;

/******************************************************************************
 * FunctionName : user_light_get_duty
 * Description  : get duty of each channel
 * Parameters   : uint8 channel : LIGHT_RED/LIGHT_GREEN/LIGHT_BLUE
 * Returns      : NONE
*******************************************************************************/
uint32 ICACHE_FLASH_ATTR
user_light_get_duty(uint8 channel)
{
    return light_param.pwm_duty[channel];
}

/******************************************************************************
 * FunctionName : user_light_set_duty
 * Description  : set each channel's duty params
 * Parameters   : uint8 duty    : 0 ~ PWM_DEPTH
 *                uint8 channel : LIGHT_RED/LIGHT_GREEN/LIGHT_BLUE
 * Returns      : NONE
*******************************************************************************/
void ICACHE_FLASH_ATTR
user_light_set_duty(uint32 duty, uint8 channel)
{
    if (duty != light_param.pwm_duty[channel]) {
        pwm_set_duty(duty, channel);

        light_param.pwm_duty[channel] = pwm_get_duty(channel);
    }
}

/******************************************************************************
 * FunctionName : user_light_get_period
 * Description  : get pwm period
 * Parameters   : NONE
 * Returns      : uint32 : pwm period
*******************************************************************************/
uint32 ICACHE_FLASH_ATTR
user_light_get_period(void)
{
    return light_param.pwm_period;
}

/******************************************************************************
 * FunctionName : user_light_set_duty
 * Description  : set pwm frequency
 * Parameters   : uint16 freq : 100hz typically
 * Returns      : NONE
*******************************************************************************/
void ICACHE_FLASH_ATTR
user_light_set_period(uint32 period)
{
    if (period != light_param.pwm_period) {
        pwm_set_period(period);

        light_param.pwm_period = pwm_get_period();
    }
}

void ICACHE_FLASH_ATTR
user_light_restart(void)
{
	spi_flash_erase_sector(PRIV_PARAM_START_SEC + PRIV_PARAM_SAVE);
	spi_flash_write((PRIV_PARAM_START_SEC + PRIV_PARAM_SAVE) * SPI_FLASH_SEC_SIZE,
	    		(uint32 *)&light_param, sizeof(struct light_saved_param));

	pwm_start();
}

/******************************************************************************
 * FunctionName : user_light_init
 * Description  : light demo init, mainy init pwm
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
#include "gpio.h"
void ICACHE_FLASH_ATTR
user_light_init(void)
{
    spi_flash_read((PRIV_PARAM_START_SEC + PRIV_PARAM_SAVE) * SPI_FLASH_SEC_SIZE,
                               (uint32 *)&light_param, sizeof(struct light_saved_param));
    light_param.pwm_period = 1000;

	uint32 io_info[][3] = {{PWM_0_OUT_IO_MUX,PWM_0_OUT_IO_FUNC,PWM_0_OUT_IO_NUM},
		                      {PWM_1_OUT_IO_MUX,PWM_1_OUT_IO_FUNC,PWM_1_OUT_IO_NUM},
		                      {PWM_2_OUT_IO_MUX,PWM_2_OUT_IO_FUNC,PWM_2_OUT_IO_NUM}};
       uint32 pwm_duty_init[3] = {0,0,0};
pwm_init(light_param.pwm_period,  pwm_duty_init ,PWM_CHANNEL,io_info);

light_set_aim(light_param.pwm_duty[LIGHT_RED],
	                light_param.pwm_duty[LIGHT_GREEN],
	                light_param.pwm_duty[LIGHT_BLUE], 
	                0);
    set_pwm_debug_en(0);
    os_printf("PWM version : %08x \r\n",get_pwm_version());
}
#endif

