#include "user_switch.h"
#include "user_configstore.h"
#include "os_type.h"
#include "ets_sys.h"
#include "osapi.h"
#include "c_types.h"
#include "gpio.h"

//#include "user_light_action.h"





void ICACHE_FLASH_ATTR
gpio_config(GPIO_ConfigTypeDef  *pGPIOConfig)
{
    uint16 gpio_pin_mask = pGPIOConfig->GPIO_Pin;
    uint32 io_reg;
    uint8 io_num = 0;
    uint32 pin_reg;

    if (pGPIOConfig->GPIO_Mode == GPIO_Mode_Input) {
        GPIO_AS_INPUT(gpio_pin_mask);
    } else if (pGPIOConfig->GPIO_Mode == GPIO_Mode_Output) {
        GPIO_AS_OUTPUT(gpio_pin_mask);
    }

    do {
        if ((gpio_pin_mask >> io_num) & 0x1) {
            io_reg = GPIO_PIN_REG(io_num);

            if (pGPIOConfig->GPIO_Pullup) {
                PIN_PULLUP_EN(io_reg);
            } else {
                PIN_PULLUP_DIS(io_reg);
            }

            if (pGPIOConfig->GPIO_Mode == GPIO_Mode_Out_OD) {
                pin_reg = GPIO_REG_READ(GPIO_PIN_ADDR(io_num));
                pin_reg &= (~GPIO_PIN_DRIVER_MASK);
                pin_reg |= (GPIO_PAD_DRIVER_ENABLE << GPIO_PIN_DRIVER_LSB);
                GPIO_REG_WRITE(GPIO_PIN_ADDR(io_num), pin_reg);
            } else if (pGPIOConfig->GPIO_Mode == GPIO_Mode_Sigma_Delta) {
                pin_reg = GPIO_REG_READ(GPIO_PIN_ADDR(io_num));
                pin_reg &= (~GPIO_PIN_SOURCE_MASK);
                pin_reg |= (0x1 << GPIO_PIN_SOURCE_LSB);
                GPIO_REG_WRITE(GPIO_PIN_ADDR(io_num), pin_reg);
                GPIO_REG_WRITE(GPIO_SIGMA_DELTA_ADDRESS, SIGMA_DELTA_ENABLE);
            }
            if ((0x1 << io_num) & (GPIO_Pin_0 | GPIO_Pin_2 | GPIO_Pin_4 | GPIO_Pin_5)) { //last , before intr init
                PIN_FUNC_SELECT(io_reg, 0);
            } else {
                PIN_FUNC_SELECT(io_reg, 3);
            }
            gpio_pin_intr_state_set(io_num, pGPIOConfig->GPIO_IntrType);
        }

        io_num++;
    } while (io_num < 16);
}


void ICACHE_FLASH_ATTR
	switch_GpioInputInit()
{
	GPIO_ConfigTypeDef gpio_input_conf;
	gpio_input_conf.GPIO_IntrType=GPIO_PIN_INTR_DISABLE;
	gpio_input_conf.GPIO_Mode = GPIO_Mode_Input;
	gpio_input_conf.GPIO_Pin = SWITCH_INPUT_01_IO_PIN|SWITCH_INPUT_02_IO_PIN|SWITCH_INPUT_03_IO_PIN|SWITCH_INPUT_04_IO_PIN;
	gpio_input_conf.GPIO_Pullup = GPIO_PullUp_DIS;
	gpio_config(&gpio_input_conf);
}

void ICACHE_FLASH_ATTR
	switch_GpioOutputInit()
{
	GPIO_ConfigTypeDef gpio_output_conf;
	gpio_output_conf.GPIO_IntrType=GPIO_PIN_INTR_DISABLE;
	gpio_output_conf.GPIO_Mode = GPIO_Mode_Output;
	gpio_output_conf.GPIO_Pin = SWITCH_HOLD_PIN ;
	gpio_output_conf.GPIO_Pullup = GPIO_PullUp_EN;
	gpio_config(&gpio_output_conf);
}

uint8 ICACHE_FLASH_ATTR
	switch_GetGpioVal()
{
	uint32 val_raw = (gpio_input_get())&0xffff;
	uint8 val = 0;
	if( val_raw & SWITCH_INPUT_01_IO_PIN){
		val|= 0x1;
	}
	if( val_raw & SWITCH_INPUT_02_IO_PIN){
		val|= 0x2;
	}	
	if( val_raw & SWITCH_INPUT_03_IO_PIN){
		val|= 0x4;
	}	
	if( val_raw & SWITCH_INPUT_04_IO_PIN){
		val|= 0x8;
	}

	return val;

}


uint8 switch_gpio_val = 0;

void ICACHE_FLASH_ATTR
	user_SwitchInit()
{
    switch_GpioInputInit();
    switch_GpioOutputInit();
    _SWITCH_GPIO_HOLD();
    
    //get gpio status and run callback
    //os_printf("test gpio status: 0x%02x\r\n",switch_GetGpioVal());
	switch_gpio_val = switch_GetGpioVal();
	os_printf("switch_GetGpioVal: 0x%02x\r\n",switch_gpio_val);
        
}


os_timer_t light_sync_t;

void ICACHE_FLASH_ATTR
	user_startChannelSync()
{
	_SWITCH_GPIO_HOLD();
    switch_EspnowChnSyncStart();

}



void ICACHE_FLASH_ATTR
	user_SwitchReact()
{
    os_printf("switch_GetGpioVal in action: 0x%02x\r\n",switch_gpio_val);	
        
	if(switch_gpio_val==0x0 || switch_gpio_val == 0x3){
        //_SWITCH_GPIO_RELEASE(); //first power on when exchange battery
	}
	//======================================
    switch_EspnowInit();
    os_printf("test action init ,CHANNEL %d \r\n",wifi_get_channel());
	uint32 r=0,g=0,b=0,cw=0,ww=0;
    uint32 pwm_duty_data[5];
	int ix=-1;
	switch(switch_gpio_val){
    case 0x0e:
		ix=0;
		break;
	case 0x07:
		ix=1;
		break;
	case 0x0d:
		ix=2;
		break;
	case 0x0b:
		ix=3;
		break;
	case 0x0a:
		break;
	default:
		break;
	}
	if (ix==-1) {
		//Dunno.
        _SWITCH_GPIO_RELEASE();
		return;
	}
	
	pwm_duty_data[0]=myConfig.bval[ix].r;
	pwm_duty_data[1]=myConfig.bval[ix].g;
	pwm_duty_data[2]=myConfig.bval[ix].b;
	pwm_duty_data[3]=0;
	pwm_duty_data[4]=0;

	//if there is a long press, run channel synchronization.
	os_timer_disarm(&light_sync_t);
	os_timer_setfn(&light_sync_t,user_startChannelSync,NULL);
	os_timer_arm(&light_sync_t,2000,0);

	int i;
	//for(i=0;i<CMD_NUM;i++){
	//	switch_EspnowSendLightCmd(i, 5, pwm_duty_data, 1000);
	//}
	switch_EspnowSendCmdByChnl(1, 5, pwm_duty_data, 1000);


	
	
	//======================================
    
}



