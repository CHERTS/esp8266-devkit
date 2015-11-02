//////////////////////////////////////////////////
// I2C driver for DS1307 RTC for ESP8266.
// Copyright 2015 Richard A Burton
// richardaburton@gmail.com
// See license.txt for license terms.
//////////////////////////////////////////////////

#ifndef __DS1307_H__
#define __DS1307_H__

#include <time.h>

#define DS1307_ADDR 0x68

#define DS1307_CTRL_OUTPUT         0x80
#define DS1307_CTRL_SQUAREWAVE     0x10
#define DS1307_CTRL_SQWAVE_32768HZ 0x03
#define DS1307_CTRL_SQWAVE_8192HZ  0x02
#define DS1307_CTRL_SQWAVE_4096HZ  0x01
#define DS1307_CTRL_SQWAVE_1HZ     0x00

#define DS1307_OUTPUT_LEVEL_1 0x80
#define DS1307_OUTPUT_LEVEL_0 0x00

#define DS1307_ADDR_TIME    0x00
#define DS1307_ADDR_CONTROL 0x07

#define DS1307_SET     0
#define DS1307_CLEAR   1
#define DS1307_REPLACE 2

#define DS1307_12HOUR_FLAG 0x40
#define DS1307_12HOUR_MASK 0x1f
#define DS1307_PM_FLAG     0x20

bool ICACHE_FLASH_ATTR ds1307_setTime(struct tm *time);
bool ICACHE_FLASH_ATTR ds1307_getOscillatorStopFlag();
bool ICACHE_FLASH_ATTR ds1307_clearOscillatorStopFlag();
bool ICACHE_FLASH_ATTR ds1307_enable32khz();
bool ICACHE_FLASH_ATTR ds1307_disable32khz();
bool ICACHE_FLASH_ATTR ds1307_enableSquarewave();
bool ICACHE_FLASH_ATTR ds1307_disableSquarewave();
bool ICACHE_FLASH_ATTR ds1307_getTime(struct tm *time);

#endif
