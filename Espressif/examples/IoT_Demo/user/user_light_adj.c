
#include "ets_sys.h"
#include "osapi.h"
#include "os_type.h"
#include "mem.h"
#include "user_interface.h"

#include "user_light.h"
#include "user_light_adj.h"
#include "pwm.h"

#define ABS_MINUS(x,y) (x<y?(y-x):(x-y))
uint8 light_sleep_flg = 0;

uint16  min_ms = 15;
uint16  min_us = 15000;
uint32  current_duty[3] = {0};

static uint32 w_aim=0; //only use RGB in this demo
bool change_finish=true;
os_timer_t timer_pwm_adj;

u32 duty_step[3] = {200,200,200};//step number to change the color

void light_dh_pwm_adj_proc(void *Targ)
{
    uint8 i;
    for(i=0;i<3;i++){	
        if(duty_step[i] == 0){
            //pwm.duty[i] = current_duty[i];
            pwm_set_duty(current_duty[i], i);
        }
        else{
            if(ABS_MINUS(current_duty[i],pwm_get_duty(i))<duty_step[i]){
                //pwm.duty[i] = current_duty[i];
                pwm_set_duty( current_duty[i] , i );
            }
            else{
                if(current_duty[i] > pwm_get_duty(i)){
                    //pwm.duty[i] += duty_step[i];
                    pwm_set_duty( pwm_get_duty(i)+duty_step[i] , i );
                }
                else if(current_duty[i] < pwm_get_duty(i) ){
                    //pwm.duty[i] -= duty_step[i];
                    pwm_set_duty( pwm_get_duty(i)-duty_step[i], i );
                }
            }
        }
    }
    
    //os_printf("duty:%u,%u,%u\r\n", pwm.duty[0],pwm.duty[1],pwm.duty[2] );
    pwm_start();	
    if(pwm_get_duty(0) != current_duty[0]
        ||pwm_get_duty(1) != current_duty[1]
        ||pwm_get_duty(2) != current_duty[2]){
        change_finish = 0;		
        os_timer_disarm(&timer_pwm_adj);
        os_timer_setfn(&timer_pwm_adj, (os_timer_func_t *)light_dh_pwm_adj_proc, NULL);
        os_timer_arm(&timer_pwm_adj, min_ms, 0);	
    }
    else{
        change_finish = 1;	
        os_timer_disarm(&timer_pwm_adj);
        if( ( ( /*pwm.duty[0]*/pwm_get_duty(0)) | (/*pwm.duty[1]*/pwm_get_duty(1)) | (/*pwm.duty[2]*/pwm_get_duty(2)) ) ==0 ){
            if(light_sleep_flg==0){
                os_printf("light sleep en\r\n");
                wifi_set_sleep_type(LIGHT_SLEEP_T);
                light_sleep_flg = 1;
            }
        }
    }

}

void select_current_duty(void)
{
    uint8 i = 0;
    for(i=0;i<3;i++){
        if(current_duty[i]<=0)
        current_duty[i] = 0;
    }
}

void light_pwm_smooth_adj_proc(void)
{
    select_current_duty();
    light_dh_pwm_adj_proc(NULL);
}

void ICACHE_FLASH_ATTR
	light_save_target_duty()
{
#if SAVE_LIGHT_PARAM
	extern struct light_saved_param light_param;
	light_param.pwm_duty[0] = current_duty[0];
	light_param.pwm_duty[1] = current_duty[1];
	light_param.pwm_duty[2] = current_duty[2];
	
	spi_flash_erase_sector(PRIV_PARAM_START_SEC + PRIV_PARAM_SAVE);
	spi_flash_write((PRIV_PARAM_START_SEC + PRIV_PARAM_SAVE) * SPI_FLASH_SEC_SIZE,
	    		(uint32 *)&light_param, sizeof(struct light_saved_param));
#endif

}


void ICACHE_FLASH_ATTR
light_set_aim(uint32 r,uint32 g,uint32 b,uint32 w)
{
    current_duty[0]=r;
    current_duty[1]=g;
    current_duty[2]=b;
    w_aim=w;
    light_save_target_duty();

    if(change_finish){
        duty_step[0] = ABS_MINUS( current_duty[0] , pwm_get_duty(0) )/(150/min_ms);
        duty_step[1] = ABS_MINUS( current_duty[1] , pwm_get_duty(1) )/(150/min_ms);
        duty_step[2] = ABS_MINUS( current_duty[2] , pwm_get_duty(2) )/(150/min_ms);
        light_pwm_smooth_adj_proc();
    }
}


void ICACHE_FLASH_ATTR
light_set_aim_r(uint32 r)
{
    current_duty[0]=r;
    light_save_target_duty();
	
    if(change_finish){
        duty_step[0] = ABS_MINUS( current_duty[0] , pwm_get_duty(0) )/(150/min_ms);
        light_pwm_smooth_adj_proc();
    }
}


void ICACHE_FLASH_ATTR
light_set_aim_g(uint32 g)
{
    current_duty[1]=g;
    light_save_target_duty();
    if(change_finish){
        duty_step[1] = ABS_MINUS( current_duty[1] , pwm_get_duty(1) )/(150/min_ms);
        light_pwm_smooth_adj_proc();
    }
}


void ICACHE_FLASH_ATTR
light_set_aim_b(uint32 b)
{
    current_duty[2]=b;
    light_save_target_duty();
    if(change_finish){
        duty_step[2] = ABS_MINUS( current_duty[2] , pwm_get_duty(2) )/(150/min_ms);
        light_pwm_smooth_adj_proc();
    }
}


void ICACHE_FLASH_ATTR
light_set_aim_w(uint32 w)  //not used fo this RGB demo
{
    w_aim=w;
    if(change_finish){
        light_pwm_smooth_adj_proc();
    }
}




