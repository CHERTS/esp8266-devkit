#ifndef __USER_LIGHT_ADJ_H__
#define __USER_LIGHT_ADJ_H__


#define SAVE_LIGHT_PARAM  1  //save RGB params to flash when calling light_set_aim

void light_pwm_smooth_adj_proc(void);
void light_dh_pwm_adj_proc(void *Targ);
void  light_set_aim(uint32 r,uint32 g,uint32 b,uint32 w);//'white' channel is not used in default demo
void light_set_aim_r(uint32 r);
void light_set_aim_g(uint32 g);
void light_set_aim_b(uint32 b);
void light_set_aim_w(uint32 w);//'white' channel is not used in default demo

#endif

