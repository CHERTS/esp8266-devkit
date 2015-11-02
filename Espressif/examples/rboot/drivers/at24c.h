////////////////////////////////////////////////////
// I2C driver for AT24C series eeproms for ESP8266.
// Copyright 2015 Richard A Burton
// richardaburton@gmail.com
// See license.txt for license terms.
////////////////////////////////////////////////////

#ifndef __AT24C_H__
#define __AT24C_H__

// should work with all atmel 24c series i2c eeproms up to 512Kb
// the 1Mb model uses a 17 bit address, where the most significant
// bit takes the place of the last bit in the device i2c address
// (so it only has A0-A1 for addressing), this driver will work
// fine on this chip but will only be able to access half of it
// (which half depends on the least significant bit of the address
// specified below)

// address can be from 0x50-0x57 depending on address pins A0-A2
#define AT24C_ADDR 0x57
#define AT24C_PAGESIZE 0x20

bool ICACHE_FLASH_ATTR at24c_readBytes(uint16 addr, uint8 *data, uint16 len);
bool ICACHE_FLASH_ATTR at24c_readNextBytes(uint8 *data, uint16 len);
void ICACHE_FLASH_ATTR at24c_writeWait();
bool ICACHE_FLASH_ATTR at24c_writeInPage(uint16 addr, uint8* data, uint8 len, bool wait);
bool ICACHE_FLASH_ATTR at24c_writeAcrossPages(uint16 addr, uint8* data, uint16 len, bool wait);
bool ICACHE_FLASH_ATTR at24c_setBytes(uint16 addr, uint8 val, uint16 len, bool wait);

// few convenient defines
#define at24c_readByte(addr, outptr) at24c_readBytes(addr, outptr, 1)
#define at24c_readNextByte(outptr) at24c_readNextBytes(outptr, 1)
#define at24c_writeByte(addr, value) at24c_writeInPage(addr, &(value), 1, true)
#define at24c_zero(addr, len) at24c_setBytes(addr, 0x00, len, true)
#define at24c_erase(addr, len) at24c_setBytes(addr, 0xff, len, true)

#endif