#ifndef __USER_LIGHT_ADJ_H__
#define __USER_LIGHT_ADJ_H__
/*pwm.h: function and macro definition of PWM API , driver level */
/*user_light.h: user interface for light setting, user level*/
/*user_light_adj: API for color changing and lighting effects, user level*/
#include "user_config.h"

/*set target duty for PWM channels, change each channel duty gradually */
void  light_set_aim(uint32 r,uint32 g,uint32 b,uint32 cw,uint32 ww,uint32 period,uint8 ctrl_mode);//'white' channel is not used in default demo
void light_set_aim_r(uint32 r);
void light_set_aim_g(uint32 g);
void light_set_aim_b(uint32 b);
void light_set_aim_cw(uint32 cw);
void light_set_aim_ww(uint32 ww);

#endif

