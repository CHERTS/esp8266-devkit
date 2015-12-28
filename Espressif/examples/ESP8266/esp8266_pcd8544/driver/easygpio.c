/*
* easygpio.c
*
* Copyright (c) 2015, eadf (https://github.com/eadf)
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
* * Redistributions of source code must retain the above copyright notice,
* this list of conditions and the following disclaimer.
* * Redistributions in binary form must reproduce the above copyright
* notice, this list of conditions and the following disclaimer in the
* documentation and/or other materials provided with the distribution.
* * Neither the name of Redis nor the names of its contributors may be used
* to endorse or promote products derived from this software without
* specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
* CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*/

#include "driver/easygpio.h"
#include "gpio.h"
#include "osapi.h"
#include "ets_sys.h"

/**
 * Returns the number of active pins in the gpioMask.
 */
uint8_t ICACHE_FLASH_ATTR
easygpio_countBits(uint32_t gpioMask) {

  uint8_t i=0;
  uint8_t numberOfPins=0;
  for (i=0; i<32; i++){
    numberOfPins += (gpioMask & 1<<i)?1:0;
  }
  return numberOfPins;
}

/**
 * Returns the gpio name and func for a specific pin.
 */
bool ICACHE_FLASH_ATTR
easygpio_getGPIONameFunc(uint8_t gpio_pin, uint32_t *gpio_name, uint8_t *gpio_func ) {

  if (gpio_pin == 6 || gpio_pin == 7 || gpio_pin == 8 || gpio_pin == 11 || gpio_pin >= 17) {
    os_printf("easygpio_getGPIONameFunc Error: There is no GPIO%d, check your code\n", gpio_pin);
    return false;
  }
  if (gpio_pin == 16) {
    os_printf("easygpio_getGPIONameFunc Error: GPIO16 is not implemented\n");
    return false;
  }
  switch ( gpio_pin ) {
    case 0:
      *gpio_func = FUNC_GPIO0;
      *gpio_name = PERIPHS_IO_MUX_GPIO0_U;
      return true;
    case 1:
      *gpio_func = FUNC_GPIO1;
      *gpio_name = PERIPHS_IO_MUX_U0TXD_U;
      return true;
    case 2:
      *gpio_func = FUNC_GPIO2;
      *gpio_name = PERIPHS_IO_MUX_GPIO2_U;
      return true;
    case 3:
      *gpio_func = FUNC_GPIO3;
      *gpio_name = PERIPHS_IO_MUX_U0RXD_U;
      return true;
    case 4:
      *gpio_func = FUNC_GPIO4;
      *gpio_name = PERIPHS_IO_MUX_GPIO4_U;
      return true;
    case 5:
      *gpio_func = FUNC_GPIO5;
      *gpio_name = PERIPHS_IO_MUX_GPIO5_U;
      return true;
    case 9:
      *gpio_func = FUNC_GPIO9;
      *gpio_name = PERIPHS_IO_MUX_SD_DATA2_U;
      return true;
    case 10:
      *gpio_func = FUNC_GPIO10;
      *gpio_name = PERIPHS_IO_MUX_SD_DATA3_U;
      return true;
    case 12:
      *gpio_func = FUNC_GPIO12;
      *gpio_name = PERIPHS_IO_MUX_MTDI_U;
      return true;
    case 13:
      *gpio_func = FUNC_GPIO13;
      *gpio_name = PERIPHS_IO_MUX_MTCK_U;
      return true;
    case 14:
      *gpio_func = FUNC_GPIO14;
      *gpio_name = PERIPHS_IO_MUX_MTMS_U;
      return true;
    case 15:
      *gpio_func = FUNC_GPIO15;
      *gpio_name = PERIPHS_IO_MUX_MTDO_U;
      return true;
    default:
      return false;
  }
  return true;
}

/**
 * Sets the pull up and pull down registers for a pin.
 * 'pullUp' takes precedence over pullDown
 */
static void ICACHE_FLASH_ATTR
easygpio_setupPullsByName(uint32_t gpio_name, EasyGPIO_PullStatus pullStatus) {

  if (EASYGPIO_PULLUP == pullStatus){
    //PIN_PULLDWN_DIS(gpio_name);
    PIN_PULLUP_EN(gpio_name);
  } else if (EASYGPIO_PULLDOWN == pullStatus){
    PIN_PULLUP_DIS(gpio_name);
    //PIN_PULLDWN_EN(gpio_name);
  } else {
    //PIN_PULLDWN_DIS(gpio_name);
    PIN_PULLUP_DIS(gpio_name);
  }
}

/**
 * Sets the pull up and pull down registers for a pin.
 * 'pullUp' takes precedence over pullDown
 */
bool ICACHE_FLASH_ATTR
easygpio_pullMode(uint8_t gpio_pin, EasyGPIO_PullStatus pullStatus) {
  uint32_t gpio_name;
  uint8_t gpio_func;

  if (!easygpio_getGPIONameFunc(gpio_pin, &gpio_name, &gpio_func) ) {
    return false;
  }

  easygpio_setupPullsByName(gpio_name, pullStatus);
  return true;
}

/**
 * Sets the 'gpio_pin' pin as an input GPIO and sets the pull up and
 * pull down registers for that pin.
 */
bool ICACHE_FLASH_ATTR
easygpio_pinMode(uint8_t gpio_pin, EasyGPIO_PullStatus pullStatus, EasyGPIO_PinMode pinMode) {
  uint32_t gpio_name;
  uint8_t gpio_func;

  if (!easygpio_getGPIONameFunc(gpio_pin, &gpio_name, &gpio_func) ) {
    return false;
  }

  PIN_FUNC_SELECT(gpio_name, gpio_func);
  easygpio_setupPullsByName(gpio_name, pullStatus);

  if (EASYGPIO_OUTPUT != pinMode) {
    GPIO_DIS_OUTPUT(gpio_pin);
  }
  return true;
}

/**
 * Sets the 'gpio_pin' pin as a GPIO and sets the interrupt to trigger on that pin
 */
bool ICACHE_FLASH_ATTR
easygpio_attachInterrupt(uint8_t gpio_pin, EasyGPIO_PullStatus pullStatus, void (*interruptHandler)(int8_t key)) {
  uint32_t gpio_name;
  uint8_t gpio_func;

  if (gpio_pin == 16) {
    os_printf("easygpio_setupInterrupt Error: GPIO16 does not have interrupts\n");
    return false;
  }
  if (!easygpio_getGPIONameFunc(gpio_pin, &gpio_name, &gpio_func) ) {
    return false;
  }

  ETS_GPIO_INTR_ATTACH(interruptHandler, NULL);
  ETS_GPIO_INTR_DISABLE();

  PIN_FUNC_SELECT(gpio_name, gpio_func);

  easygpio_setupPullsByName(gpio_name, pullStatus);

  // disable output
  GPIO_DIS_OUTPUT(gpio_pin);

  gpio_register_set(GPIO_PIN_ADDR(gpio_pin), GPIO_PIN_INT_TYPE_SET(GPIO_PIN_INTR_DISABLE)
                    | GPIO_PIN_PAD_DRIVER_SET(GPIO_PAD_DRIVER_DISABLE)
                    | GPIO_PIN_SOURCE_SET(GPIO_AS_PIN_SOURCE));

  //clear gpio14 status
  GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, BIT(gpio_pin));
  ETS_GPIO_INTR_ENABLE();

  return true;
}

/**
 * Detach the interrupt handler from the 'gpio_pin' pin.
 */
bool ICACHE_FLASH_ATTR
easygpio_detachInterrupt(uint8_t gpio_pin) {

  if (gpio_pin == 16) {
    os_printf("easygpio_setupInterrupt Error: GPIO16 does not have interrupts\n");
    return false;
  }

  // Don't know how to detach interrupt, yet.
  // Quick and dirty fix - just disable the interrupt
  gpio_pin_intr_state_set(GPIO_ID_PIN(gpio_pin), GPIO_PIN_INTR_DISABLE);
  return true;
}

