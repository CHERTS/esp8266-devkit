/*
    Driver for the 24xx32 series serial EEPROM from ATMEL
    Official repository: https://github.com/CHERTS/esp8266-i2c_24xx32
    Base on https://github.com/husio-org/AT24C512C and https://code.google.com/p/arduino-at24c1024/
    This driver depends on the I2C driver https://github.com/zarya/esp8266_i2c_driver/

    Copyright (C) 2014 Mikhail Grigorev (CHERTS)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#ifndef __I2C_24XX32_H__
#define __I2C_24XX32_H__

#include "ets_sys.h"
#include "osapi.h"

#define WORD_MASK 0xFFFF

uint8 eeprom_readByte(uint8 address, uint32_t location);
char *eeprom_readPage(uint8 address, uint32_t location, uint32_t len);
uint8 eeprom_writeByte(uint8 address, uint32_t location, uint8 data);
uint8 eeprom_writePage(uint8 address, uint32_t location, char data[], uint32_t len);

#endif
