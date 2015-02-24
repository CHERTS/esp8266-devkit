/*
    Driver for the 24xx32 series serial EEPROM from ATMEL
    Official repository: https://github.com/CHERTS/esp8266-i2c_24xx32
    Base on https://github.com/husio-org/AT24C512C and https://code.google.com/p/arduino-at24c1024/
    This driver depends on the I2C driver https://github.com/zarya/esp8266_i2c_driver/

    Device start address 0x50 to 0x57

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

#include "driver/i2c.h"
#include "driver/i2c_24xx32.h"

#include "ets_sys.h"
#include "osapi.h"

/**
 * Read a single byte from the EEPROM
 * uint8 address        : The i2c address
 * uint32_t  location   : The memory location to read
 */
uint8 ICACHE_FLASH_ATTR
eeprom_readByte(uint8 address, uint32_t location)
{
    uint8 write_address;
    uint8 data;

    i2c_start();
    i2c_writeByte(address << 1);     
    if (!i2c_check_ack())
    {
        i2c_stop();
        return 0;
    }

    i2c_writeByte((uint8_t)((location & WORD_MASK) >> 8)); // MSB
    if (!i2c_check_ack())
    {
        i2c_stop();
        return 0;
    }
    i2c_writeByte((uint8_t)(location & 0xFF)); // LSB
    if (!i2c_check_ack())
    {
        i2c_stop();
        return 0;
    }

    i2c_start();
    write_address = address << 1;
    write_address |= 1;
    i2c_writeByte(write_address);
    if (!i2c_check_ack())
    {
        i2c_stop();
        return 0;
    }
    data = i2c_readByte();
    i2c_send_ack(0); // NOACK
    i2c_stop();
    return data;
}

/**
 * Read multiple bytes from the EEPROM
 * uint8 address       : The i2c address
 * uint32_t location   : The memory location to read
 * uint32_t len        : Number of bytes to read
 */
char ICACHE_FLASH_ATTR
*eeprom_readPage(uint8 address, uint32_t location, uint32_t len)
{
    static char data[256];
    uint8 write_address;

    i2c_start();
    i2c_writeByte(address << 1);
    if (!i2c_check_ack())
    {
        i2c_stop();
        return 0;
    }

    i2c_writeByte((uint8_t)((location & WORD_MASK) >> 8)); // MSB
    if (!i2c_check_ack())
    {
        i2c_stop();
        return 0;
    }
    i2c_writeByte((uint8_t)(location & 0xFF)); // LSB
    if (!i2c_check_ack())
    {
        i2c_stop();
        return 0;
    }

    i2c_start();
    write_address = address << 1;
    write_address |= 1;
    i2c_writeByte(write_address);
    if (!i2c_check_ack())
    {
        i2c_stop();
        return 0;
    }

    uint32_t i;
    for (i = 0; i < len; i++)
    {
        data[i] = i2c_readByte();
        if(i != len-1)
        	i2c_send_ack(1); //ACK
    }
    i2c_send_ack(0); // NOACK
    i2c_stop();
    return data;
}

/**
 * Write a byte to the I2C EEPROM
 * uint8 address       : I2C Device address
 * uint32_t location   : Memory location
 * uint8 data          : Data to write to the EEPROM
 */
uint8 ICACHE_FLASH_ATTR
eeprom_writeByte(uint8 address, uint32_t location, uint8 data)
{
    i2c_start();
    //Write address
    i2c_writeByte(address << 1);     
    if (!i2c_check_ack())
    {
        i2c_stop();
        return 0;
    }

    //Write memory location
    i2c_writeByte((uint8_t)((location & WORD_MASK) >> 8)); // MSB
    if (!i2c_check_ack())
    {
        i2c_stop();
        return 0;
    }
    i2c_writeByte((uint8_t)(location & 0xFF)); // LSB
    if (!i2c_check_ack())
    {
        i2c_stop();
        return 0;
    }

    //Write data
    i2c_writeByte(data);
    if (!i2c_check_ack())
    {
        i2c_stop();
        return 0;
    }
    i2c_stop();
    return 1;
}

/**
 * Write a number of pages to the EEPROM
 * uint8 address       : Address of the module
 * uint32_t location   : Start on this memory address
 * char data           : The data to be writen to the EEPROM
 * uint32_t len        : The lenght of the data
 */
uint8 ICACHE_FLASH_ATTR
eeprom_writePage(uint8 address, uint32_t location, char data[], uint32_t len)
{
    i2c_start();
    i2c_writeByte(address << 1);     
    if (!i2c_check_ack())
    {
        i2c_stop();
        return 0;
    }

    i2c_writeByte((uint8_t)((location & WORD_MASK) >> 8)); // MSB
    if (!i2c_check_ack())
    {
        i2c_stop();
        return 0;
    }
    i2c_writeByte((uint8_t)(location & 0xFF)); // LSB
    if (!i2c_check_ack())
    {
        i2c_stop();
        return 0;
    }

    uint32_t i;
    for (i = 0; i < len; i++)
    {
        i2c_writeByte(data[i]);
        if (!i2c_check_ack())
        {
            i2c_stop();
            return 0;
        }
    }

    i2c_stop();
    return 1;
}
