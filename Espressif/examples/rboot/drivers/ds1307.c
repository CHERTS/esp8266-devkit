//////////////////////////////////////////////////
// I2C driver for DS1307 RTC for ESP8266.
// Copyright 2015 Richard A Burton
// richardaburton@gmail.com
// See license.txt for license terms.
//////////////////////////////////////////////////

#include <c_types.h>
#include <i2c_master.h>

#include "ds1307.h"

#include <osapi.h>

// convert normal decimal to binary coded decimal
static uint8 ICACHE_FLASH_ATTR decToBcd(uint8 dec) {
  return(((dec / 10) * 16) + (dec % 10));
}

// convert binary coded decimal to normal decimal
static uint8 ICACHE_FLASH_ATTR bcdToDec(uint8 bcd) {
  return(((bcd / 16) * 10) + (bcd % 16));
}

// send a number of bytes to the rtc over i2c
// returns true to indicate success
static bool ICACHE_FLASH_ATTR ds1307_send(uint8 *data, uint8 len) {

	int loop;

	// signal i2c start
	i2c_master_start();

	// write address & direction
	i2c_master_writeByte((uint8)(DS1307_ADDR << 1));
	if (!i2c_master_checkAck()) {
		//uart0_send("i2c error1\r\n");
		i2c_master_stop();
		return false;
	}

	// send the data
	for (loop = 0; loop < len; loop++) {
		i2c_master_writeByte(data[loop]);
		if (!i2c_master_checkAck()) {
			//uart0_send("i2c error2\r\n");
			i2c_master_stop();
			return false;
		}
	}

	// signal i2c stop
	i2c_master_stop();

	return true;

}

// read a number of bytes from the rtc over i2c
// returns true to indicate success
static bool ICACHE_FLASH_ATTR ds1307_recv(uint8 *data, uint8 len) {
	
	int loop;

	// signal i2c start
	i2c_master_start();

	// write address & direction
	i2c_master_writeByte((uint8)((DS1307_ADDR << 1) | 1));
	if (!i2c_master_checkAck()) {
		//uart0_send("i2c error3\r\n");
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

// set the time on the rtc
// timezone agnostic, pass whatever you like
// I suggest using GMT and applying timezone and DST when read back
// returns true to indicate success
bool ICACHE_FLASH_ATTR ds1307_setTime(struct tm *time) {
	
	uint8 data[8];

	// start register
	data[0] = DS1307_ADDR_TIME;
	// time/date data
	data[1] = decToBcd(time->tm_sec);
	data[2] = decToBcd(time->tm_min);
	data[3] = decToBcd(time->tm_hour);
	data[4] = decToBcd(time->tm_wday + 1);
	data[5] = decToBcd(time->tm_mday);
	data[6] = decToBcd(time->tm_mon + 1);
	data[7] = decToBcd(time->tm_year - 100);

	return ds1307_send(data, 8);

}

// get a byte containing just the requested bits
// pass the register address to read, a mask to apply to the register and
// an uint* for the output
// you can test this value directly as true/false for specific bit mask
// of use a mask of 0xff to just return the whole register byte
// returns true to indicate success
bool ICACHE_FLASH_ATTR ds1307_getFlag(uint8 addr, uint8 mask, uint8 *flag) {
	uint8 data[1];
	// get register
	data[0] = addr;
	if (ds1307_send(data, 1) && ds1307_recv(data, 1)) {
		// return only requested flag
		*flag = (data[0] & mask);
		return true;
	}
	return false;
}

// set/clear bits in a byte register, or replace the byte altogether
// pass the register address to modify, a byte to replace the existing
// value with or containing the bits to set/clear and one of
// DS1307_SET/DS1307_CLEAR/DS1307_REPLACE
// returns true to indicate success
bool ICACHE_FLASH_ATTR ds1307_setFlag(uint8 addr, uint8 bits, uint8 mode) {
	uint8 data[2];
	data[0] = addr;
	// get status register
	if (ds1307_send(data, 1) && ds1307_recv(data+1, 1)) {
		// clear the flag
		if (mode == DS1307_REPLACE) data[1] = bits;
		else if (mode == DS1307_SET) data[1] |= bits;
		else data[1] &= ~bits;
		if (ds1307_send(data, 2)) {
			return true;
		}
	}
	return false;
}

// enable the squarewave output
// returns true to indicate success
bool ICACHE_FLASH_ATTR ds1307_enableSquarewave() {
	return ds1307_setFlag(DS1307_ADDR_CONTROL, DS1307_CTRL_SQUAREWAVE, DS1307_SET);
}

// disable the squarewave output
// returns true to indicate success
bool ICACHE_FLASH_ATTR ds1307_disableSquarewave() {
	return ds1307_setFlag(DS1307_ADDR_CONTROL, DS1307_CTRL_SQUAREWAVE, DS1307_CLEAR);
}

// enable and set output level (disabled squarewave)
// returns true to indicate success
bool ICACHE_FLASH_ATTR ds1307_enableOutout(uint8 level) {
	uint8 flag = 0;
	if (ds1307_getFlag(DS1307_ADDR_CONTROL, 0xff, &flag)) {
		// clear squarewave flag (to enable output) and current level
		flag &= ~(DS1307_CTRL_SQUAREWAVE | DS1307_OUTPUT_LEVEL_1);
		// set the output level
		flag |= level;
		return ds1307_setFlag(DS1307_ADDR_CONTROL, flag, DS1307_REPLACE);
	}
	return false;
}

// set the frequency of the squarewave output (but does not enable it)
// pass DS1307_SQUAREWAVE_RATE_1HZ/DS1307_SQUAREWAVE_RATE_1024HZ/DS1307_SQUAREWAVE_RATE_4096HZ/DS1307_SQUAREWAVE_RATE_8192HZ
// returns true to indicate success
bool ICACHE_FLASH_ATTR ds1307_setSquarewaveFreq(uint8 freq) {
	uint8 flag = 0;
	if (ds1307_getFlag(DS1307_ADDR_CONTROL, 0xff, &flag)) {
		// clear current rate
		flag &= ~DS1307_CTRL_SQWAVE_32768HZ;
		// set new rate
		flag |= freq;
		return ds1307_setFlag(DS1307_ADDR_CONTROL, flag, DS1307_REPLACE);
	}
	return false;
}

// get the time from the rtc, populates a supplied tm struct
// returns true to indicate success
bool ICACHE_FLASH_ATTR ds1307_getTime(struct tm *time) {

	int loop;
	uint8 data[7];

	// start register address
	data[0] = DS1307_ADDR_TIME;
	if (!ds1307_send(data, 1)) {
		return false;
	}

	// read time
	if (!ds1307_recv(data, 7)) {
		return false;
	}

	// convert to unix time structure
	time->tm_sec = bcdToDec(data[0]);
	time->tm_min = bcdToDec(data[1]);
	if (data[2] & DS1307_12HOUR_FLAG) {
		// 12h
		time->tm_hour = bcdToDec(data[2] & DS1307_12HOUR_MASK);
		// pm?
		if (data[2] & DS1307_PM_FLAG) time->tm_hour += 12;
	} else {
		// 24h
		time->tm_hour = bcdToDec(data[2]);
	}
	time->tm_wday = bcdToDec(data[3]) - 1;
	time->tm_mday = bcdToDec(data[4]);
	time->tm_mon  = bcdToDec(data[5]) - 1;
	time->tm_year = bcdToDec(data[6]) + 100;
	time->tm_isdst = 0;

	// apply a time zone (if you are not using localtime on the rtc or you want to check/apply DST)
	//applyTZ(time);

	return true;
	
}
