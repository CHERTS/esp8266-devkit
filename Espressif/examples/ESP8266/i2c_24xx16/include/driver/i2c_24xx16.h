/*
    Driver for the 24xx16 series serial EEPROM from Microchip
    This driver depends on the I2C driver https://github.com/zarya/esp8266_i2c_driver/

    Copyright (C) 2014 Rudy Hardeman (zarya)

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

#ifndef __I2C_24xx16_H__
#define __I2C_24xx16_H__

#include "ets_sys.h"
#include "osapi.h"

uint8 eeprom_readByte(uint8 address, uint8 location);
char *eeprom_readPage(uint8 address, uint8 location, uint8 len);
uint8 eeprom_writeByte(uint8 address, uint8 location, uint8 data);
uint8 eeprom_writePage(uint8 address, uint8 location, char data[], uint8 len);

#endif
