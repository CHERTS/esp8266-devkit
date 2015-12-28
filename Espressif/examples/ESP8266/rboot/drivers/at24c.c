////////////////////////////////////////////////////
// I2C driver for AT24C series eeproms for ESP8266.
// Copyright 2015 Richard A Burton
// richardaburton@gmail.com
// See license.txt for license terms.
////////////////////////////////////////////////////

#include <c_types.h>
#include <osapi.h>
#include <i2c_master.h>

#include "at24c.h"

// set the current data address, this is the start of the write command
// next either send the data to be written, or start a read instead
// returns true to indicate success
static bool ICACHE_FLASH_ATTR at24c_setAddr(uint16 addr) {

	uint8 loop;
	uint8 data[2];

	// signal i2c start
	i2c_master_start();

	// write i2c address & direction
	i2c_master_writeByte((uint8)(AT24C_ADDR << 1));
	if (!i2c_master_checkAck()) {
		//uart0_send("i2c error\r\n");
		i2c_master_stop();
		return false;
	}

	// write data address
	data[0] = (uint8)(((unsigned)addr) >> 8);
	data[1] = (uint8)(((unsigned)addr) & 0xff);
	for (loop = 0; loop < 2; loop++) {
		i2c_master_writeByte(data[loop]);
		if (!i2c_master_checkAck()) {
			//uart0_send("i2c error\r\n");
			i2c_master_stop();
			return false;
		}
	}

	return true;
}

// read from the current position
// returns true to indicate success
bool ICACHE_FLASH_ATTR at24c_readNextBytes(uint8 *data, uint16 len) {

	int loop;

	// signal i2c start
	i2c_master_start();

	// write i2c address & direction
	i2c_master_writeByte((uint8)((AT24C_ADDR << 1) | 1));
	if (!i2c_master_checkAck()) {
		//uart0_send("i2c error\r\n");
		i2c_master_stop();
		return false;
	}

	// read bytes
	for (loop = 0; loop < len; loop++) {
		data[loop] = i2c_master_readByte();
		// send ack (except after last byte, then we send nack)
		if (loop < (len - 1)) i2c_master_send_ack(); else i2c_master_send_nack();
	}

	// signal i2c stop
	i2c_master_stop();

	return true;
}

// read from anywhere
// sets the address then does a normal read
// returns true to indicate success
bool ICACHE_FLASH_ATTR at24c_readBytes(uint16 addr, uint8 *data, uint16 len) {
	// set data address
	if (!at24c_setAddr(addr)) return false;
	// perform the read
	return at24c_readNextBytes(data, len);
}

// wait for a write operation to complete
// by 'acknowledge polling'
void ICACHE_FLASH_ATTR at24c_writeWait() {
	do {
		i2c_master_start();
		i2c_master_writeByte((uint8)((AT24C_ADDR << 1) | 1));
	} while (!i2c_master_checkAck());
	i2c_master_stop();
}

// write within a page
// note if you try to write past a page boundary the write will
// wrap back to the start of the same page, so you need to know
// how much you're writing and where you're writing it to
// you don't need to start writing at the start of a page, but if you
// start in the middle you'll be able to write less before wrapping
// optionally wait for the eeprom to complete the write
// returns true to indicate success
bool ICACHE_FLASH_ATTR at24c_writeInPage(uint16 addr, uint8* data, uint8 len, bool wait) {

	int loop;

	// set data address (includes i2c setup,
	// so no need to call i2c_master_start here)
	if (!at24c_setAddr(addr)) return false;

	// send the data
	for (loop = 0; loop < len; loop++) {
		i2c_master_writeByte(data[loop]);
		if (!i2c_master_checkAck()) {
			//uart0_send("i2c error\r\n");
			i2c_master_stop();
			return false;
		}
	}

	// signal i2c stop
	i2c_master_stop();

	// optionally, wait until the eeprom signals the write is finished
	if (wait) at24c_writeWait();

	return true;
}

// writes across pages
// you do not need to worry about how long your data is or where you
// are writing it as it will be written in multiple parts across
// successive pages, optionally waiting for the last write to complete
// (we always have to wait for any earlier writes to complete)
// note: does not check if you are trying to write past the end of the
// eeprom!
// returns true to indicate success
bool ICACHE_FLASH_ATTR at24c_writeAcrossPages(uint16 addr, uint8* data, uint16 len, bool wait) {

	uint8 wlen;

	// work out number of bytes available in starting page
	wlen = AT24C_PAGESIZE - (addr % AT24C_PAGESIZE);
	// is that more than we actually need?
	if (wlen > len) wlen = len;

	while(wlen > 0) {
		// reduce remaining length
		len -= wlen;
		// write the page
		if (!at24c_writeInPage(addr, data, wlen, (len > 0 ? true : wait))) {
			return false;
		}
		// advance the eeprom address and our data pointer
		addr += wlen;
		data += wlen;
		// work out how much to write next time
		wlen = (len < AT24C_PAGESIZE ? len : AT24C_PAGESIZE);
	}

	return true;
}

// set an area of eeprom to specified value (like memset)
// optionally wait for completion of last write
bool ICACHE_FLASH_ATTR at24c_setBytes(uint16 addr, uint8 val, uint16 len, bool wait) {

	uint8 wlen;
	uint8 data[AT24C_PAGESIZE];

	// set the temp write buffer to user's choice of value
	os_memset(data, val, AT24C_PAGESIZE);

	// work out number of bytes available in starting page
	wlen = AT24C_PAGESIZE - (addr % AT24C_PAGESIZE);
	// is that more than we actually need?
	if (wlen > len) wlen = len;

	while(wlen > 0) {
		// reduce remaining length
		len -= wlen;
		// write the page
		if (!at24c_writeInPage(addr, data, wlen, (len > 0 ? true : wait))) {
			return false;
		}
		// advance the eeprom address
		addr += wlen;
		// work out how much to write next time
		wlen = (len < AT24C_PAGESIZE ? len : AT24C_PAGESIZE);
	}

	return true;
}
