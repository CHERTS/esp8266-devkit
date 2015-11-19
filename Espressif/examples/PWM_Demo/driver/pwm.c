/******************************************************************************
* Copyright 2013-2014 Espressif Systems (Wuxi)
*
* FileName: pwm.c
*
* Description: pwm driver
*
* Modification history:
*     2014/5/1, v1.0 create this file.
*******************************************************************************/
#include "ets_sys.h"
#include "os_type.h"
#include "osapi.h"

#include "user_interface.h"
#include "driver/pwm.h"

#define pwm_dbg_printf  //os_printf

struct pwm_single_param {
    uint16 gpio_set;
    uint16 gpio_clear;
    uint32 h_time;
};



struct pwm_param pwm;

LOCAL bool update_flg = 0;	//update finished flag
LOCAL bool init_flg = 0;		//first update flag

LOCAL uint8 pwm_chn_num = 0;
LOCAL uint8 pwm_out_io_num[8] = {0};	//each channel gpio number

LOCAL bool pwm_stop_flag = 0;       //start/stop frc1 timer

LOCAL struct pwm_single_param local_single[8 + 1];	 //local_single param,       on-changing
LOCAL uint8 local_channel = 0;								        //local_channel value


LOCAL struct pwm_single_param run_pwm_single[2][8 + 1];   //running para,  two sets
LOCAL uint8 run_pwm_channel[2];

LOCAL uint8 run_pwm_toggle = 0;
LOCAL struct pwm_single_param *pwm_single;
LOCAL uint8 pwm_channel;

LOCAL uint8 pwm_current_channel = 0;							//current pwm channel in pwm_tim1_intr_handler
LOCAL uint16 pwm_gpio = 0;									//all pwm gpio bits

static u32 last_gpio_hdl_frc1_tick = 0x7fffff;

//XXX: 0xffffffff/(80000000/16)=35A
//IMPORTANT:   t mus be u32 type.  u16, 450~800, err!!!!!!!!!!!
#define US_TO_RTC_TIMER_TICKS(t)          \
    ((t) ?                                   \
     (((t) > 0x35A) ?                   \
      (((t)>>2) * ((APB_CLK_FREQ>>4)/250000) + ((t)&0x3) * ((APB_CLK_FREQ>>4)/1000000))  :    \
      (((t) *(APB_CLK_FREQ>>4)) / 1000000)) :    \
     0)

#define FRC1_ENABLE_TIMER  BIT7

//TIMER PREDIVED MODE
typedef enum {
    DIVDED_BY_1 = 0,		//timer clock
    DIVDED_BY_16 = 4,	//divided by 16
    DIVDED_BY_256 = 8,	//divided by 256
} TIMER_PREDIVED_MODE;

typedef enum {			//timer interrupt mode
    TM_LEVEL_INT = 1,	// level interrupt
    TM_EDGE_INT   = 0,	//edge interrupt
} TIMER_INT_MODE;

// sort all channels' h_time,small to big
LOCAL void ICACHE_FLASH_ATTR
pwm_insert_sort(struct pwm_single_param pwm[], uint8 n)
{
    uint8 i;

    for (i = 1; i < n; i++) {
        if (pwm[i].h_time < pwm[i - 1].h_time) {
            int8 j = i - 1;
            struct pwm_single_param tmp;

            os_memcpy(&tmp, &pwm[i], sizeof(struct pwm_single_param));
            os_memcpy(&pwm[i], &pwm[i - 1], sizeof(struct pwm_single_param));

            while (tmp.h_time < pwm[j].h_time) {
                os_memcpy(&pwm[j + 1], &pwm[j], sizeof(struct pwm_single_param));
                j--;

                if (j < 0) {
                    break;
                }
            }

            os_memcpy(&pwm[j + 1], &tmp, sizeof(struct pwm_single_param));
        }
    }
}


#define LOW_LIMIT (6)
void ICACHE_FLASH_ATTR
pwm_start(void)
{
        uint8 i, j;
        
// step 1: init PWM_CHANNEL+1 channels param
        for (i = 0; i < pwm_chn_num; i++) 
        {
                //uint32 us = pwm.period * pwm.duty[i] / PWM_DEPTH;		//calc  single channel us time
                //local_single[i].h_time = US_TO_RTC_TIMER_TICKS(us);	//calc h_time to write FRC1_LOAD_ADDRESS
                local_single[i].h_time = (pwm.period * pwm.duty[i]*5)/PWM_DEPTH;    //calc h_time to write FRC1_LOAD_ADDRESS

                if(local_single[i].h_time<LOW_LIMIT)
                        local_single[i].h_time = LOW_LIMIT;
        
                local_single[i].gpio_set = 0;							//don't set gpio
                local_single[i].gpio_clear = 1 << pwm_out_io_num[i];	//clear single channel gpio
        }
        
        local_single[pwm_chn_num].h_time = US_TO_RTC_TIMER_TICKS(pwm.period);		//calc pwm.period channel us time
        local_single[pwm_chn_num].gpio_set = pwm_gpio;			//set all channels' gpio
        local_single[pwm_chn_num].gpio_clear = 0;					//don't clear gpio

pwm_dbg_printf("init\n");
for (i = 0; i < 4; i++) 
{
        pwm_dbg_printf("set:%x  ",local_single[i].gpio_set);
        pwm_dbg_printf("clr:%x  ",local_single[i].gpio_clear);
        pwm_dbg_printf("htm:%u  ",local_single[i].h_time);
        pwm_dbg_printf("\n");
}

// step 2: sort, small to big
        pwm_insert_sort(local_single, pwm_chn_num + 1);			//time sort small to big,
        local_channel = pwm_chn_num + 1;							//local channel number is PWM_CHANNEL+1


pwm_dbg_printf("sort\n");
for (i = 0; i < local_channel; i++) 
{
        pwm_dbg_printf("set:%x  ",local_single[i].gpio_set);
        pwm_dbg_printf("clr:%x  ",local_single[i].gpio_clear);
        pwm_dbg_printf("htm:%u  ",local_single[i].h_time);
        pwm_dbg_printf("\n");
}


// step 2.5: low limit of duty diff is 6.
        for (i =1 ; i <= pwm_chn_num; i++) 
        {
                u32 tmp = local_single[i].h_time - local_single[i - 1].h_time;
                if ( tmp < 6 ) 
                {
                        if ( tmp < 3 ) 
                                local_single[i].h_time = local_single[i - 1].h_time;
                        else
                                local_single[i].h_time = local_single[i - 1].h_time+6;
                }
        }
        
pwm_dbg_printf("remove low limit\n");
for (i = 0; i < local_channel; i++) 
{
        pwm_dbg_printf("set:%x  ",local_single[i].gpio_set);
        pwm_dbg_printf("clr:%x  ",local_single[i].gpio_clear);
        pwm_dbg_printf("htm:%u  ",local_single[i].h_time);
        pwm_dbg_printf("\n");
}
        
// step 3: combine same duty channels 
        for (i = pwm_chn_num; i > 0; i--) 
        {
                if (local_single[i].h_time == local_single[i - 1].h_time) 
                {
                        local_single[i - 1].gpio_set |= local_single[i].gpio_set;
                        local_single[i - 1].gpio_clear |= local_single[i].gpio_clear;

                        //copy channel j param to channel j-1 param
                        for (j = i + 1; j < local_channel; j++) 
                        {
                                os_memcpy(&local_single[j - 1], &local_single[j], sizeof(struct pwm_single_param));
                        }
                        local_channel--;
                }
        }
        
pwm_dbg_printf("combine\n");
for (i = 0; i < local_channel; i++) 
{
        pwm_dbg_printf("set:%x  ",local_single[i].gpio_set);
        pwm_dbg_printf("clr:%x  ",local_single[i].gpio_clear);
        pwm_dbg_printf("htm:%u  ",local_single[i].h_time);
        pwm_dbg_printf("\n");
}
        
// step 4: calc delt time
        for (i = local_channel - 1; i > 0; i--) 
        {
                local_single[i].h_time -= local_single[i - 1].h_time;
        }

// step 5: last channel needs to clean
        local_single[local_channel - 1].gpio_clear = 0;

// step 6: if first channel duty is 0, remove it
        if (local_single[0].h_time == 0) 
        {
                local_single[local_channel - 1].gpio_set &= ~local_single[0].gpio_clear;
                local_single[local_channel - 1].gpio_clear |= local_single[0].gpio_clear;

                //copy channel i param to channel i-1 param
                for (i = 1; i < local_channel; i++) 
                {
                        os_memcpy(&local_single[i - 1], &local_single[i], sizeof(struct pwm_single_param));
                }
                local_channel--;
        }


//local  OR saved ? 
pwm_dbg_printf("remove 1st 0\n");
for (i = 0; i < local_channel; i++) 
{
        pwm_dbg_printf("set:%x  ",local_single[i].gpio_set);
        pwm_dbg_printf("clr:%x  ",local_single[i].gpio_clear);
        pwm_dbg_printf("htm:%u  ",local_single[i].h_time);
        pwm_dbg_printf("\n");
}
        
        pwm_stop_flag = 0;
        // if the first update,copy local  param to pwm  param and copy local_channel to pwm_channel
        if (init_flg == 0) 
        {

//os_printf("init_set:%u\n",US_TO_RTC_TIMER_TICKS(pwm.period)-17);
                //run_pwm_toggle = 0;
                for (i = 0; i < local_channel; i++) 
                {
                        os_memcpy(&run_pwm_single[run_pwm_toggle][i], &local_single[i], sizeof(struct pwm_single_param));
                }
                run_pwm_channel[run_pwm_toggle] = local_channel;

                pwm_single = run_pwm_single[run_pwm_toggle];
                pwm_channel = run_pwm_channel[run_pwm_toggle];

                run_pwm_toggle = (run_pwm_toggle ^ 0x01);
                init_flg = 1;	//first update finished
                update_flg = 0;
                if( pwm_stop_flag!=1 )
                {
                        RTC_REG_WRITE(FRC1_LOAD_ADDRESS, US_TO_RTC_TIMER_TICKS(pwm.period)-17);	//first update finished,start
                        last_gpio_hdl_frc1_tick = RTC_REG_READ(FRC1_COUNT_ADDRESS);
                }
        }
        else
        {       // update to another run_pwm_single
                //update_flg = 0;	            //in case every update, this flag is cleared.  Can't set is flag here
//os_printf("non init_set\n");
                if(update_flg==0)
                {
                        for (i = 0; i < local_channel; i++) 
                        {
                                os_memcpy(&run_pwm_single[run_pwm_toggle][i], &local_single[i], sizeof(struct pwm_single_param));
                        }
                        run_pwm_channel[run_pwm_toggle] = local_channel;
                        update_flg = 1;	//update finished
                }
        }
        
}

/*
void ICACHE_FLASH_ATTR
pwm_stop(void)
{
    TM1_EDGE_INT_DISABLE();
    ETS_FRC1_INTR_DISABLE();
    RTC_REG_WRITE(FRC1_CTRL_ADDRESS,
                                  0);
    pwm_stop_flag = 1;
    init_flg = 0;
    GPIO_REG_WRITE(GPIO_OUT_W1TC_ADDRESS,pwm_gpio); 
    os_printf("pwm stop\n");
}
*/

/******************************************************************************
* FunctionName : pwm_set_duty
* Description  : set each channel's duty param
* Parameters   : uint8 duty    : 0 ~ PWM_DEPTH
*                uint8 channel : channel index
* Returns      : NONE
*******************************************************************************/
void ICACHE_FLASH_ATTR
pwm_set_duty(uint32 duty, uint8 channel)
{
    if (duty < 1) {
        pwm.duty[channel] = 0;
    } else {
        pwm.duty[channel] = duty;
    }
pwm_dbg_printf("channel:%u, duty:%u\n",channel, pwm.duty[channel] );
}


/******************************************************************************
* FunctionName : pwm_set_freq
* Description  : set pwm frequency
* Parameters   : uint16 freq : 100hz typically
* Returns      : NONE
*******************************************************************************/
void ICACHE_FLASH_ATTR
pwm_set_period(uint32 period)
{
    pwm.period = period;
os_printf("period:%u\n",pwm.period);
}


/******************************************************************************
* FunctionName : pwm_set_freq_duty
* Description  : set pwm frequency and each channel's duty
* Parameters   : uint16 freq : 100hz typically
*                uint8 *duty : each channel's duty
* Returns      : NONE
*******************************************************************************/
LOCAL void ICACHE_FLASH_ATTR
pwm_set_period_duty(uint32 period, uint32 *duty)
{
    uint8 i;

    pwm_set_period(period);

    for (i = 0; i < pwm_chn_num; i++) {
        pwm_set_duty(duty[i], i);
    }
}


/******************************************************************************
* FunctionName : pwm_get_duty
* Description  : get duty of each channel
* Parameters   : uint8 channel : channel index
* Returns      : NONE
*******************************************************************************/
uint32 ICACHE_FLASH_ATTR
pwm_get_duty(uint8 channel)
{
    return pwm.duty[channel];
}

/******************************************************************************
* FunctionName : pwm_get_freq
* Description  : get pwm frequency
* Parameters   : NONE
* Returns      : uint16 : pwm frequency
*******************************************************************************/
uint32 ICACHE_FLASH_ATTR
pwm_get_period(void)
{
    return pwm.period;
}



/******************IMPORMTANT: NEVER TOUCH BELOW CODE************************/
LOCAL void
pwm_hdl_gpio(void) 
{
// hdl gpio , read_reg cost  0.4 us
        if(pwm_single[pwm_current_channel].h_time>6)
        {
                u16 bias = 0;   
                if(pwm_current_channel == 0)
                {
                        bias = 0;
                }
                else
                {
                        bias = 0;
                        //while(bias--){asm volatile ("nop");} 
                }
                if(pwm_single[pwm_current_channel].h_time < 28)
                {
                        u16 b=0; u16 tmp = (pwm_single[pwm_current_channel].h_time-7);
                        if( tmp&0x1 )
                            b = 5*( (tmp+1)>>1) -3;
                        else
                            b = 5*( (tmp+2)>>1 ) -5;
                        if(tmp>2)
                        {
                                if(tmp<10)
                                     b--;
                                else
                                     b++;
                        }
                        while(b--){asm volatile ("nop");} 
                }
                else
                {
                        //while( (last_gpio_hdl_frc1_tick - RTC_REG_READ(FRC1_COUNT_ADDRESS)) <pwm_single[pwm_current_channel].h_time){}
                        u32 tmp;
                        tmp = 0x7fffff + last_gpio_hdl_frc1_tick - pwm_single[pwm_current_channel].h_time + 3;
                        while(tmp < RTC_REG_READ(FRC1_COUNT_ADDRESS)) {}
                       // u16 b=1; while(b--){asm volatile ("nop");} 
                }
        }       
        
        if(pwm_single[pwm_current_channel].gpio_set != 0x00)
                GPIO_REG_WRITE(GPIO_OUT_W1TS_ADDRESS, pwm_single[pwm_current_channel].gpio_set); 
        if(pwm_single[pwm_current_channel].gpio_clear != 0x00)
                GPIO_REG_WRITE(GPIO_OUT_W1TC_ADDRESS, pwm_single[pwm_current_channel].gpio_clear); 

// update idx
        if (pwm_current_channel == (pwm_channel - 1)) 
        {
                if (update_flg == 1) 
                {
                        pwm_single = run_pwm_single[run_pwm_toggle];
                        pwm_channel = run_pwm_channel[run_pwm_toggle];
                        run_pwm_toggle = (run_pwm_toggle ^ 0x01);

                        u16 current_set = pwm_single[pwm_channel-1].gpio_set;
                        u16 tmp_clr = ~( (~pwm_gpio) | current_set );        //this time down

                        if(current_set != 0x00)
                                GPIO_REG_WRITE(GPIO_OUT_W1TS_ADDRESS, current_set); 
                        if(tmp_clr != 0x00)
                                GPIO_REG_WRITE(GPIO_OUT_W1TC_ADDRESS, tmp_clr);
                        
                        update_flg = 0;	//clear update flag
                }
                pwm_current_channel = 0;
        } 
        else 
        {
                pwm_current_channel++;
                u16 bias = 3;
                        while(bias--){asm volatile ("nop");} 
        }
}


/******************************************************************************
* FunctionName : pwm_period_timer
* Description  : pwm period timer function, output high level,
*                start each channel's high level timer
* Parameters   : NONE
* Returns      : NONE
*******************************************************************************/
void pwm_tim1_intr_handler(void)
{
        //uint8 i = 0;
        RTC_CLR_REG_MASK(FRC1_INT_ADDRESS, FRC1_INT_CLR_MASK);

        pwm_hdl_gpio();
        
        if (pwm_channel != 1) 
        {
                // less than 5us, then continus.
                while(  pwm_single[pwm_current_channel].h_time < 28  )
                {
                        last_gpio_hdl_frc1_tick = RTC_REG_READ(FRC1_COUNT_ADDRESS);
                        pwm_hdl_gpio();
                }
               // if(pwm_stop_flag!=1)
                {
                        RTC_REG_WRITE(FRC1_LOAD_ADDRESS, pwm_single[pwm_current_channel].h_time-17);
                        last_gpio_hdl_frc1_tick = RTC_REG_READ(FRC1_COUNT_ADDRESS);
                }
        } 
        else 
        {
                //if(pwm_stop_flag!=1)
                {    
                        RTC_REG_WRITE(FRC1_LOAD_ADDRESS, US_TO_RTC_TIMER_TICKS(pwm.period)-17);
                        last_gpio_hdl_frc1_tick = RTC_REG_READ(FRC1_COUNT_ADDRESS);
                }
        }

}
/******************IMPORMTANT: NEVER TOUCH ABOVE CODE************************/




/******************************************************************************
* FunctionName : pwm_init
* Description  : pwm gpio, param and timer initialization
* Parameters   : uint16 freq : pwm freq param
*                uint8 *duty : each channel's duty
* Returns      : NONE
*******************************************************************************/
#include "gpio.h"
void ICACHE_FLASH_ATTR
pwm_init(uint32 period, uint32 *duty,uint32 pwm_channel_num,uint32 (*pin_info_list)[3])
{
    uint8 i;
    pwm_chn_num = pwm_channel_num;
    
    #define NMI_SOURCE_SEL_REG 0x3ff00000
    WRITE_PERI_REG(NMI_SOURCE_SEL_REG, (READ_PERI_REG(NMI_SOURCE_SEL_REG)&~0x1F)|(0x1+0x7*2) );
    NmiTimSetFunc( pwm_tim1_intr_handler );
    RTC_REG_WRITE(FRC1_CTRL_ADDRESS,
                                  DIVDED_BY_16
                                  | FRC1_ENABLE_TIMER
                                  | TM_EDGE_INT);
    //RTC_REG_WRITE(FRC1_LOAD_ADDRESS, 0);

    //=================


    for (i = 0; i < pwm_channel_num; i++) {
	PIN_FUNC_SELECT(pin_info_list[i][0], pin_info_list[i][1]);
	pwm_out_io_num[i]=pin_info_list[i][2];
        pwm_gpio |= (1 << pwm_out_io_num[i]);
    }
    
    GPIO_REG_WRITE(GPIO_ENABLE_W1TS_ADDRESS, pwm_gpio); 

    pwm_set_period_duty(period, duty);

    pwm_start();

    //ETS_FRC_TIMER1_INTR_ATTACH(pwm_tim1_intr_handler, NULL);
    TM1_EDGE_INT_ENABLE();
    ETS_FRC1_INTR_ENABLE();

    os_printf("test pwm channel num: %d \r\n",pwm_chn_num);
}

