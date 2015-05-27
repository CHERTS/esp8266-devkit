#ifndef __USER_LIGHT_H__
#define __USER_LIGHT_H__

#include "pwm.h"

#define PWM_CHANNEL	3

#define PWM_0_OUT_IO_MUX PERIPHS_IO_MUX_MTDI_U
#define PWM_0_OUT_IO_NUM 12
#define PWM_0_OUT_IO_FUNC  FUNC_GPIO12

#define PWM_1_OUT_IO_MUX PERIPHS_IO_MUX_MTDO_U
#define PWM_1_OUT_IO_NUM 15
#define PWM_1_OUT_IO_FUNC  FUNC_GPIO15

#define PWM_2_OUT_IO_MUX PERIPHS_IO_MUX_MTCK_U
#define PWM_2_OUT_IO_NUM 13
#define PWM_2_OUT_IO_FUNC  FUNC_GPIO13

/* NOTICE---this is for 512KB spi flash.
 * you can change to other sector if you use other size spi flash. */
#define PRIV_PARAM_START_SEC		0x3C

#define PRIV_PARAM_SAVE     0

#define LIGHT_RED       0
#define LIGHT_GREEN     1
#define LIGHT_BLUE      2
//#define LIGHT_LEVEL     3  //not used

struct light_saved_param {
    uint32  pwm_period;
    uint32  pwm_duty[PWM_CHANNEL];
    //uint8  pad[6-PWM_CHANNEL];
};

void user_light_init(void);
uint32 user_light_get_duty(uint8 channel);
void user_light_set_duty(uint32 duty, uint8 channel);
uint32 user_light_get_period(void);
void user_light_set_period(uint32 period);

#endif

