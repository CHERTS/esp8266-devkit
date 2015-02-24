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

LOCAL struct pwm_single_param pwm_single_toggle[2][PWM_CHANNEL + 1];
LOCAL struct pwm_single_param *pwm_single;

LOCAL struct pwm_param pwm;

LOCAL uint8 pwm_out_io_num[PWM_CHANNEL] = {PWM_0_OUT_IO_NUM, PWM_1_OUT_IO_NUM, PWM_2_OUT_IO_NUM};

LOCAL uint8 pwm_channel_toggle[2];
LOCAL uint8 *pwm_channel;

LOCAL uint8 pwm_toggle = 1;
LOCAL uint8 pwm_timer_down = 1;

LOCAL uint8 pwm_current_channel = 0;

LOCAL uint16 pwm_gpio = 0;

//XXX: 0xffffffff/(80000000/16)=35A
#define US_TO_RTC_TIMER_TICKS(t)          \
    ((t) ?                                   \
     (((t) > 0x35A) ?                   \
      (((t)>>2) * ((APB_CLK_FREQ>>4)/250000) + ((t)&0x3) * ((APB_CLK_FREQ>>4)/1000000))  :    \
      (((t) *(APB_CLK_FREQ>>4)) / 1000000)) :    \
     0)

//FRC1
#define FRC1_ENABLE_TIMER  BIT7

typedef enum {
    DIVDED_BY_1 = 0,
    DIVDED_BY_16 = 4,
    DIVDED_BY_256 = 8,
} TIMER_PREDIVED_MODE;

typedef enum {
    TM_LEVEL_INT = 1,
    TM_EDGE_INT   = 0,
} TIMER_INT_MODE;

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

void ICACHE_FLASH_ATTR
pwm_start(void)
{
    uint8 i, j;

    struct pwm_single_param *local_single = pwm_single_toggle[pwm_toggle ^ 0x01];
    uint8 *local_channel = &pwm_channel_toggle[pwm_toggle ^ 0x01];

    // step 1: init PWM_CHANNEL+1 channels param
    for (i = 0; i < PWM_CHANNEL; i++) {
        uint32 us = pwm.period * pwm.duty[i] / PWM_DEPTH;
        local_single[i].h_time = US_TO_RTC_TIMER_TICKS(us);
        local_single[i].gpio_set = 0;
        local_single[i].gpio_clear = 1 << pwm_out_io_num[i];
    }

    local_single[PWM_CHANNEL].h_time = US_TO_RTC_TIMER_TICKS(pwm.period);
    local_single[PWM_CHANNEL].gpio_set = pwm_gpio;
    local_single[PWM_CHANNEL].gpio_clear = 0;

    // step 2: sort, small to big
    pwm_insert_sort(local_single, PWM_CHANNEL + 1);

    *local_channel = PWM_CHANNEL + 1;

    // step 3: combine same duty channels
    for (i = PWM_CHANNEL; i > 0; i--) {
        if (local_single[i].h_time == local_single[i - 1].h_time) {
            local_single[i - 1].gpio_set |= local_single[i].gpio_set;
            local_single[i - 1].gpio_clear |= local_single[i].gpio_clear;

            for (j = i + 1; j < *local_channel; j++) {
                os_memcpy(&local_single[j - 1], &local_single[j], sizeof(struct pwm_single_param));
            }

            (*local_channel)--;
        }
    }

    // step 4: cacl delt time
    for (i = *local_channel - 1; i > 0; i--) {
        local_single[i].h_time -= local_single[i - 1].h_time;
    }

    // step 5: last channel needs to clean
    local_single[*local_channel-1].gpio_clear = 0;

    // step 6: if first channel duty is 0, remove it
    if (local_single[0].h_time == 0) {
        local_single[*local_channel - 1].gpio_set &= ~local_single[0].gpio_clear;
        local_single[*local_channel - 1].gpio_clear |= local_single[0].gpio_clear;

        for (i = 1; i < *local_channel; i++) {
            os_memcpy(&local_single[i - 1], &local_single[i], sizeof(struct pwm_single_param));
        }

        (*local_channel)--;
    }

    // if timer is down, need to set gpio and start timer
    if (pwm_timer_down == 1) {
        pwm_channel = local_channel;
        pwm_single = local_single;
        // start
        gpio_output_set(local_single[0].gpio_set, local_single[0].gpio_clear, pwm_gpio, 0);

        // yeah, if all channels' duty is 0 or 255, don't need to start timer, otherwise start...
        if (*local_channel != 1) {
            pwm_timer_down = 0;
            RTC_REG_WRITE(FRC1_LOAD_ADDRESS, local_single[0].h_time);
        }
    }

    if (pwm_toggle == 1) {
        pwm_toggle = 0;
    } else {
        pwm_toggle = 1;
    }
}

/******************************************************************************
 * FunctionName : pwm_set_duty
 * Description  : set each channel's duty params
 * Parameters   : uint8 duty    : 0 ~ PWM_DEPTH
 *                uint8 channel : channel index
 * Returns      : NONE
*******************************************************************************/
void ICACHE_FLASH_ATTR
pwm_set_duty(uint8 duty, uint8 channel)
{
    if (duty < 1) {
        pwm.duty[channel] = 0;
    } else if (duty >= PWM_DEPTH) {
    	pwm.duty[channel] = PWM_DEPTH;
    } else {
    	pwm.duty[channel] = duty;
    }
}

/******************************************************************************
 * FunctionName : pwm_set_freq
 * Description  : set pwm frequency
 * Parameters   : uint16 freq : 100hz typically
 * Returns      : NONE
*******************************************************************************/
void ICACHE_FLASH_ATTR
pwm_set_freq(uint16 freq)
{
    if (freq > 500) {
        pwm.freq = 500;
    } else if (freq < 1) {
        pwm.freq = 1;
    } else {
        pwm.freq = freq;
    }

    pwm.period = PWM_1S / pwm.freq;
}

/******************************************************************************
 * FunctionName : pwm_set_freq_duty
 * Description  : set pwm frequency and each channel's duty
 * Parameters   : uint16 freq : 100hz typically
 *                uint8 *duty : each channel's duty
 * Returns      : NONE
*******************************************************************************/
LOCAL void ICACHE_FLASH_ATTR
pwm_set_freq_duty(uint16 freq, uint8 *duty)
{
    uint8 i;

    pwm_set_freq(freq);

    for (i = 0; i < PWM_CHANNEL; i++) {
        pwm_set_duty(duty[i], i);
    }
}

/******************************************************************************
 * FunctionName : pwm_get_duty
 * Description  : get duty of each channel
 * Parameters   : uint8 channel : channel index
 * Returns      : NONE
*******************************************************************************/
uint8 ICACHE_FLASH_ATTR
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
uint16 ICACHE_FLASH_ATTR
pwm_get_freq(void)
{
    return pwm.freq;
}

/******************************************************************************
 * FunctionName : pwm_period_timer
 * Description  : pwm period timer function, output high level,
 *                start each channel's high level timer
 * Parameters   : NONE
 * Returns      : NONE
*******************************************************************************/
LOCAL void
pwm_tim1_intr_handler(void)
{
    RTC_CLR_REG_MASK(FRC1_INT_ADDRESS, FRC1_INT_CLR_MASK);

    if (pwm_current_channel == (*pwm_channel - 1)) {
        pwm_single = pwm_single_toggle[pwm_toggle];
        pwm_channel = &pwm_channel_toggle[pwm_toggle];

        gpio_output_set(pwm_single[*pwm_channel - 1].gpio_set,
                        pwm_single[*pwm_channel - 1].gpio_clear,
                        pwm_gpio,
                        0);

        pwm_current_channel = 0;

        if (*pwm_channel != 1) {
            RTC_REG_WRITE(FRC1_LOAD_ADDRESS, pwm_single[pwm_current_channel].h_time);
        } else {
            pwm_timer_down = 1;
        }
    } else {
        gpio_output_set(pwm_single[pwm_current_channel].gpio_set,
                        pwm_single[pwm_current_channel].gpio_clear,
                        pwm_gpio, 0);

        pwm_current_channel++;
        RTC_REG_WRITE(FRC1_LOAD_ADDRESS, pwm_single[pwm_current_channel].h_time);
    }
}

/******************************************************************************
 * FunctionName : pwm_init
 * Description  : pwm gpio, params and timer initialization
 * Parameters   : uint16 freq : pwm freq param
 *                uint8 *duty : each channel's duty
 * Returns      : NONE
*******************************************************************************/
void ICACHE_FLASH_ATTR
pwm_init(uint16 freq, uint8 *duty)
{
    uint8 i;

    RTC_REG_WRITE(FRC1_CTRL_ADDRESS,  //FRC2_AUTO_RELOAD|
                  DIVDED_BY_16
                  | FRC1_ENABLE_TIMER
                  | TM_EDGE_INT);
    RTC_REG_WRITE(FRC1_LOAD_ADDRESS, 0);

    PIN_FUNC_SELECT(PWM_0_OUT_IO_MUX, PWM_0_OUT_IO_FUNC);
    PIN_FUNC_SELECT(PWM_1_OUT_IO_MUX, PWM_1_OUT_IO_FUNC);
    PIN_FUNC_SELECT(PWM_2_OUT_IO_MUX, PWM_2_OUT_IO_FUNC);

    for (i = 0; i < PWM_CHANNEL; i++) {
        pwm_gpio |= (1 << pwm_out_io_num[i]);
    }

    pwm_set_freq_duty(freq, duty);

    pwm_start();
    
    ETS_FRC_TIMER1_INTR_ATTACH(pwm_tim1_intr_handler, NULL);
    TM1_EDGE_INT_ENABLE();
    ETS_FRC1_INTR_ENABLE();
}
