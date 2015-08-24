#ifndef __USER_LIGHT_H__
#define __USER_LIGHT_H__
/*pwm.h: function and macro definition of PWM API , driver level */
/*user_light.h: user interface for light API, user level*/
/*user_light_adj: API for color changing and lighting effects, user level*/
#include "user_config.h"

#include "pwm.h"

struct light_saved_param {
    uint32  pwm_period;
    uint32  pwm_duty[PWM_CHANNEL];
};

#define LIGHT_RED       0
#define LIGHT_GREEN     1
#define LIGHT_BLUE      2
#define LIGHT_COLD_WHITE      3
#define LIGHT_WARM_WHITE      4


void user_light_init(void);
uint32 user_light_get_duty(uint8 channel);
void user_light_set_duty(uint32 duty, uint8 channel);
uint32 user_light_get_period(void);
void user_light_set_period(uint32 period);

#endif

