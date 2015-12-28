#ifndef __PWM_H__
#define __PWM_H__

// see eagle_soc.h for these definitions
/*
#define PWM_0_OUT_IO_MUX  PERIPHS_IO_MUX_MTMS_U
#define PWM_0_OUT_IO_NUM  14
#define PWM_0_OUT_IO_FUNC FUNC_GPIO14
*/

#define PWM_0_OUT_IO_MUX  PERIPHS_IO_MUX_MTDO_U
#define PWM_0_OUT_IO_NUM  15
#define PWM_0_OUT_IO_FUNC FUNC_GPIO15

#define PWM_CHANNEL 1


struct pwm_single_param {
	uint16 gpio_set;
	uint16 gpio_clear;
    uint16 h_time;
};

struct pwm_param {
    uint16 period;
    uint16 freq;
    uint8  duty[PWM_CHANNEL];
};

#define PWM_DEPTH 255
#define PWM_1S 1000000

uint8_t duty;

void pwm_init(uint16 freq, uint8 *duty);
void pwm_start(void);

void pwm_set_duty(uint8 duty, uint8 channel);
uint8 pwm_get_duty(uint8 channel);
void pwm_set_freq(uint16 freq);
uint16 pwm_get_freq(void);
#endif

