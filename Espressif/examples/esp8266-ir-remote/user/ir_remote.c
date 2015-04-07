/*
 * ir_remote.c
 *
 * Version 1.0, 06.04.2015
 * Written by Valeriy Kucherenko
 * For details, see https://github.com/valkuc/esp8266-ir-remote
 */

#include "esp_common.h"

#include "freertos/FreeRTOS.h"

#include "gpio.h"
#include "ir_remote.h"
#include "ir_remote_def.h"

static uint32_t _frc1_ticks;
static uint16_t _gpio_pin_num;

static bool _logic_high, _logic_low;

static volatile bool _pwm_enable;
static volatile bool _pwm_lvl;

static inline void delay(uint16_t delay_us)
{
	uint32 t1 = system_get_time();
	while (system_get_time() - t1 < delay_us) asm volatile ("nop");
}

void pwm_tim1_intr_handler()
{
	CLEAR_PERI_REG_MASK(FRC1_INT_ADDRESS, FRC1_INT_CLR_MASK);

	if (_pwm_enable)
	{
		GPIO_OUTPUT_SET(_gpio_pin_num, _pwm_lvl);
		_pwm_lvl ^= 1;
	}
	else if (_pwm_lvl == _logic_high)
	{
		_pwm_lvl = _logic_low;
		GPIO_OUTPUT_SET(_gpio_pin_num, _pwm_lvl);
	}

	WRITE_PERI_REG(FRC1_LOAD_ADDRESS, _frc1_ticks);
}

static void ICACHE_FLASH_ATTR mark(uint16_t time)
{
	_pwm_enable = true;
	//if (time > 0) os_delay_us(time); // Not sure, but probably it's incorrect to call os_delay_us from RTOS task
	if (time > 0) delay(time);
}

static void ICACHE_FLASH_ATTR space(uint16_t time)
{
	_pwm_enable = false;
	//if (time > 0) os_delay_us(time); // Not sure, but probably it's incorrect to call os_delay_us from RTOS task
	if (time > 0) delay(time);
}

static void ICACHE_FLASH_ATTR set_carrier_frequence(uint16_t freq)
{
	uint32_t ticks = FREQ_TO_TICKS(freq);
	if (_frc1_ticks != ticks)
	{
		WRITE_PERI_REG(FRC1_LOAD_ADDRESS, ticks);
		_frc1_ticks = ticks;
	}
}

void ICACHE_FLASH_ATTR ir_remote_init(uint16 gpio_pin_num, bool invert_logic_level)
{
	_gpio_pin_num = gpio_pin_num;

	_logic_low = invert_logic_level;
	_logic_high = !_logic_low;
	_pwm_lvl = _logic_low;

	GPIO_ConfigTypeDef gpioCfg;
	gpioCfg.GPIO_Pin = BIT(_gpio_pin_num);
	gpioCfg.GPIO_Mode = GPIO_Mode_Output;
	gpioCfg.GPIO_Pullup = GPIO_PullUp_DIS;
	gpio_config(&gpioCfg);

	GPIO_OUTPUT_SET(_gpio_pin_num, _logic_low);

	portENTER_CRITICAL();
	_xt_isr_attach(ETS_FRC_TIMER1_INUM, pwm_tim1_intr_handler);
	TM1_EDGE_INT_ENABLE();
	_xt_isr_unmask((1 << ETS_FRC_TIMER1_INUM));

	CLEAR_PERI_REG_MASK(FRC1_INT_ADDRESS, FRC1_INT_CLR_MASK);
	WRITE_PERI_REG(FRC1_CTRL_ADDRESS, CLOCK_DIV_256 | FRC1_ENABLE_TIMER | TM_EDGE_INT);
	WRITE_PERI_REG(FRC1_LOAD_ADDRESS, 0);
	portEXIT_CRITICAL();
}

void ICACHE_FLASH_ATTR ir_remote_send_nec(uint32_t data, uint8_t nbits)
{
	set_carrier_frequence(NEC_FREQUENCY);

	mark(NEC_HDR_MARK);
	space(NEC_HDR_SPACE);

	uint8_t i;
	for (i = 0; i < nbits; i++)
	{
		if (data & TOPBIT)
		{
			mark(NEC_BIT_MARK);
			space(NEC_ONE_SPACE);
		}
		else
		{
			mark(NEC_BIT_MARK);
			space(NEC_ZERO_SPACE);
		}
		data <<= 1;
	}
	mark(NEC_BIT_MARK);
	space(0);
}

void ICACHE_FLASH_ATTR ir_remote_send_panasonic(uint16_t address, uint32_t data)
{
	set_carrier_frequence(PANASONIC_FREQUENCY);

	mark(PANASONIC_HDR_MARK);
	space(PANASONIC_HDR_SPACE);

	uint8_t i;
	for (i = 0; i < 16; i++)
	{
		mark(PANASONIC_BIT_MARK);
		if (address & 0x8000)
		{
			space(PANASONIC_ONE_SPACE);
		}
		else
		{
			space(PANASONIC_ZERO_SPACE);
		}
		address <<= 1;
	}
	for (i = 0; i < 32; i++)
	{
		mark(PANASONIC_BIT_MARK);
		if (data & TOPBIT)
		{
			space(PANASONIC_ONE_SPACE);
		}
		else
		{
			space(PANASONIC_ZERO_SPACE);
		}
		data <<= 1;
	}
	mark(PANASONIC_BIT_MARK);
	space(0);
}

void ICACHE_FLASH_ATTR ir_remote_send_sony(uint32_t data, uint8_t nbits)
{
	set_carrier_frequence(SONY_FREQUENCY);

	mark(SONY_HDR_MARK);
	space(SONY_HDR_SPACE);

	data = data << (32 - nbits);

	uint8_t i;
	for (i = 0; i < nbits; i++)
	{
		if (data & TOPBIT)
		{
			mark(SONY_ONE_MARK);
			space(SONY_HDR_SPACE);
		}
		else
		{
			mark(SONY_ZERO_MARK);
			space(SONY_HDR_SPACE);
		}
		data <<= 1;
	}
}

void ICACHE_FLASH_ATTR ir_remote_send_samsung(uint32_t data, uint8_t nbits)
{
	set_carrier_frequence(SAMSUNG_FREQUENCY);

	mark(SAMSUNG_HDR_MARK);
	space(SAMSUNG_HDR_SPACE);

	uint8_t i;
	for (i = 0; i < nbits; i++)
	{
		if (data & TOPBIT)
		{
			mark(SAMSUNG_BIT_MARK);
			space(SAMSUNG_ONE_SPACE);
		}
		else
		{
			mark(SAMSUNG_BIT_MARK);
			space(SAMSUNG_ZERO_SPACE);
		}
		data <<= 1;
	}
	mark(SAMSUNG_BIT_MARK);
	space(0);
}

void ICACHE_FLASH_ATTR ir_remote_send_raw(uint16_t buf[], uint16_t len, uint16_t freq_hz)
{
	set_carrier_frequence(SONY_FREQUENCY);

	uint16_t i;
	for (i = 0; i < len; i++)
	{
		if (i & 1)
		{
			space(buf[i]);
		}
		else
		{
			mark(buf[i]);
		}
	}
	space(0);
}
