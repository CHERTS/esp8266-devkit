//////////////////////////////////////////////////
// I2C driver for DS3231 RTC for ESP8266.
// Copyright 2015 Richard A Burton
// richardaburton@gmail.com
// See license.txt for license terms.
//////////////////////////////////////////////////

#include <c_types.h>
#include <osapi.h>
#include <i2c_master.h>

#include "ds3231.h"

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
static bool ICACHE_FLASH_ATTR ds3231_send(uint8 *data, uint8 len) {
	
	int loop;

	// signal i2c start
	i2c_master_start();

	// write address & direction
	i2c_master_writeByte((uint8)(DS3231_ADDR << 1));
	if (!i2c_master_checkAck()) {
		//uart0_send("i2c error\r\n");
		i2c_master_stop();
		return false;
	}

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

	return true;

}

// read a number of bytes from the rtc over i2c
// returns true to indicate success
static bool ICACHE_FLASH_ATTR ds3231_recv(uint8 *data, uint8 len) {
	
	int loop;

	// signal i2c start
	i2c_master_start();

	// write address & direction
	i2c_master_writeByte((uint8)((DS3231_ADDR << 1) | 1));
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

// set the time on the rtc
// timezone agnostic, pass whatever you like
// I suggest using GMT and applying timezone and DST when read back
// returns true to indicate success
bool ICACHE_FLASH_ATTR ds3231_setTime(struct tm *time) {
	
	uint8 data[8];

	// start register
	data[0] = DS3231_ADDR_TIME;
	// time/date data
	data[1] = decToBcd(time->tm_sec);
	data[2] = decToBcd(time->tm_min);
	data[3] = decToBcd(time->tm_hour);
	data[4] = decToBcd(time->tm_wday + 1);
	data[5] = decToBcd(time->tm_mday);
	data[6] = decToBcd(time->tm_mon + 1);
	data[7] = decToBcd(time->tm_year - 100);

	return ds3231_send(data, 8);

}

// set alarms
// alarm1 works with seconds, minutes, hours and day of week/month, or fires every second
// alarm2 works with minutes, hours and day of week/month, or fires every minute
// not all combinations are supported, see DS3231_ALARM1_* and DS3231_ALARM2_* defines
// for valid options
// you only need to populate the fields you are using in the tm struct, and you can
// set both alarms at the same time (pass DS3231_ALARM_1/DS3231_ALARM_2/DS3231_ALARM_BOTH)
// if only setting one alarm just pass 0 for tm struct and option field for the other alarm
// if using DS3231_ALARM1_EVERY_SECOND/DS3231_ALARM2_EVERY_MIN you can pass 0 for tm stuct
// if you want to enable interrupts for the alarms you need to do that separately
// returns true to indicate success
bool ICACHE_FLASH_ATTR ds3231_setAlarm(uint8 alarms, struct tm *time1, uint8 option1, struct tm *time2, uint8 option2) {
	
	int i = 0;
	uint8 data[8];

	// start register
	data[i++] = (alarms == DS3231_ALARM_2 ? DS3231_ADDR_ALARM2 : DS3231_ADDR_ALARM1);

	// alarm 1 data
	if (alarms != DS3231_ALARM_2) {
		data[i++] = (option1 >= DS3231_ALARM1_MATCH_SEC ? decToBcd(time1->tm_sec) : DS3231_ALARM_NOTSET);
		data[i++] = (option1 >= DS3231_ALARM1_MATCH_SECMIN ? decToBcd(time1->tm_min) : DS3231_ALARM_NOTSET);
		data[i++] = (option1 >= DS3231_ALARM1_MATCH_SECMINHOUR ? decToBcd(time1->tm_hour) : DS3231_ALARM_NOTSET);
		data[i++] = (option1 == DS3231_ALARM1_MATCH_SECMINHOURDAY ? (decToBcd(time1->tm_wday + 1) & DS3231_ALARM_WDAY) :
			(option1 == DS3231_ALARM1_MATCH_SECMINHOURDATE ? decToBcd(time1->tm_mday) : DS3231_ALARM_NOTSET));
	}

	// alarm 2 data
	if (alarms != DS3231_ALARM_1) {
		data[i++] = (option2 >= DS3231_ALARM2_MATCH_MIN ? decToBcd(time2->tm_min) : DS3231_ALARM_NOTSET);
		data[i++] = (option2 >= DS3231_ALARM2_MATCH_MINHOUR ? decToBcd(time2->tm_hour) : DS3231_ALARM_NOTSET);
		data[i++] = (option2 == DS3231_ALARM2_MATCH_MINHOURDAY ? (decToBcd(time2->tm_wday + 1) & DS3231_ALARM_WDAY) :
			(option2 == DS3231_ALARM2_MATCH_MINHOURDATE ? decToBcd(time2->tm_mday) : DS3231_ALARM_NOTSET));
	}

	return ds3231_send(data, i);

}

// get a byte containing just the requested bits
// pass the register address to read, a mask to apply to the register and
// an uint* for the output
// you can test this value directly as true/false for specific bit mask
// of use a mask of 0xff to just return the whole register byte
// returns true to indicate success
bool ICACHE_FLASH_ATTR ds3231_getFlag(uint8 addr, uint8 mask, uint8 *flag) {
	uint8 data[1];
	// get register
	data[0] = addr;
	if (ds3231_send(data, 1) && ds3231_recv(data, 1)) {
		// return only requested flag
		*flag = (data[0] & mask);
		return true;
	}
	return false;
}

// set/clear bits in a byte register, or replace the byte altogether
// pass the register address to modify, a byte to replace the existing
// value with or containing the bits to set/clear and one of
// DS3231_SET/DS3231_CLEAR/DS3231_REPLACE
// returns true to indicate success
bool ICACHE_FLASH_ATTR ds3231_setFlag(uint8 addr, uint8 bits, uint8 mode) {
	uint8 data[2];
	data[0] = addr;
	// get status register
	if (ds3231_send(data, 1) && ds3231_recv(data+1, 1)) {
		// clear the flag
		if (mode == DS3231_REPLACE) data[1] = bits;
		else if (mode == DS3231_SET) data[1] |= bits;
		else data[1] &= ~bits;
		if (ds3231_send(data, 2)) {
			return true;
		}
	}
	return false;
}

// check if oscillator has previously stopped, e.g. no power/battery or disabled
// sets flag to true if there has been a stop
// returns true to indicate success
bool ICACHE_FLASH_ATTR ds3231_getOscillatorStopFlag(bool *flag) {
	uint8 f;
	if (ds3231_getFlag(DS3231_ADDR_STATUS, DS3231_STAT_OSCILLATOR, &f)) {
		*flag = (f ? true : false);
		return true;
	}
	return false;
}

// clear the oscillator stopped flag
// returns true to indicate success
bool ICACHE_FLASH_ATTR ds3231_clearOscillatorStopFlag() {
	return ds3231_setFlag(DS3231_ADDR_STATUS, DS3231_STAT_OSCILLATOR, DS3231_CLEAR);
}

// check which alarm(s) have past
// sets alarms to DS3231_ALARM_NONE/DS3231_ALARM_1/DS3231_ALARM_2/DS3231_ALARM_BOTH
// returns true to indicate success
bool ICACHE_FLASH_ATTR ds3231_getAlarmFlags(uint8 *alarms) {
	return ds3231_getFlag(DS3231_ADDR_STATUS, DS3231_ALARM_BOTH, alarms);
}

// clear alarm past flag(s)
// pass DS3231_ALARM_1/DS3231_ALARM_2/DS3231_ALARM_BOTH
// returns true to indicate success
bool ICACHE_FLASH_ATTR ds3231_clearAlarmFlags(uint8 alarms) {
	return ds3231_setFlag(DS3231_ADDR_STATUS, alarms, DS3231_CLEAR);
}

// enable alarm interrupts (and disables squarewave)
// pass DS3231_ALARM_1/DS3231_ALARM_2/DS3231_ALARM_BOTH
// if you set only one alarm the status of the other is not changed
// you must also clear any alarm past flag(s) for alarms with
// interrupt enabled, else it will trigger immediately
// returns true to indicate success
bool ICACHE_FLASH_ATTR ds3231_enableAlarmInts(uint8 alarms) {
	return ds3231_setFlag(DS3231_ADDR_CONTROL, DS3231_CTRL_ALARM_INTS | alarms, DS3231_SET);
}

// disable alarm interrupts (does not (re-)enable squarewave)
// pass DS3231_ALARM_1/DS3231_ALARM_2/DS3231_ALARM_BOTH
// returns true to indicate success
bool ICACHE_FLASH_ATTR ds3231_disableAlarmInts(uint8 alarms) {
	// just disable specific alarm(s) requested
	// does not disable alarm interrupts generally (which would enable the squarewave)
	return ds3231_setFlag(DS3231_ADDR_CONTROL, alarms, DS3231_CLEAR);
}

// enable the output of 32khz signal
// returns true to indicate success
bool ICACHE_FLASH_ATTR ds3231_enable32khz() {
	return ds3231_setFlag(DS3231_ADDR_STATUS, DS3231_STAT_32KHZ, DS3231_SET);
}

// disable the output of 32khz signal
// returns true to indicate success
bool ICACHE_FLASH_ATTR ds3231_disable32khz() {
	return ds3231_setFlag(DS3231_ADDR_STATUS, DS3231_STAT_32KHZ, DS3231_CLEAR);
}

// enable the squarewave output (disables alarm interrupt functionality)
// returns true to indicate success
bool ICACHE_FLASH_ATTR ds3231_enableSquarewave() {
	return ds3231_setFlag(DS3231_ADDR_CONTROL, DS3231_CTRL_ALARM_INTS, DS3231_CLEAR);
}

// disable the squarewave output (which re-enables alarm interrupts, but individual
// alarm interrupts also need to be enabled, if not already, before they will trigger)
// returns true to indicate success
bool ICACHE_FLASH_ATTR ds3231_disableSquarewave() {
	return ds3231_setFlag(DS3231_ADDR_CONTROL, DS3231_CTRL_ALARM_INTS, DS3231_SET);
}

// set the frequency of the squarewave output (but does not enable it)
// pass DS3231_SQUAREWAVE_RATE_1HZ/DS3231_SQUAREWAVE_RATE_1024HZ/DS3231_SQUAREWAVE_RATE_4096HZ/DS3231_SQUAREWAVE_RATE_8192HZ
// returns true to indicate success
bool ICACHE_FLASH_ATTR ds3231_setSquarewaveFreq(uint8 freq) {
	uint8 flag = 0;
	if (ds3231_getFlag(DS3231_ADDR_CONTROL, 0xff, &flag)) {
		// clear current rate
		flag &= ~DS3231_CTRL_SQWAVE_8192HZ;
		// set new rate
		flag |= freq;
		return ds3231_setFlag(DS3231_ADDR_CONTROL, flag, DS3231_REPLACE);
	}
	return false;
}

// get the temperature as an integer (rounded down)
// returns true to indicate success
bool ICACHE_FLASH_ATTR ds3231_getTempInteger(int8 *temp) {
	uint8 data[1];
	data[0] = DS3231_ADDR_TEMP;
	// get just the integer part of the temp
	if (ds3231_send(data, 1) && ds3231_recv(data, 1)) {
		*temp = (signed)data[0];
		return true;
	}
	return false;
}

// get the temerapture as a float (in quarter degree increments)
// returns true to indicate success
bool ICACHE_FLASH_ATTR ds3231_getTempFloat(float *temp) {
	uint8 data[2];
	data[0] = DS3231_ADDR_TEMP;
	// get integer part and quarters of the temp
	if (ds3231_send(data, 1) && ds3231_recv(data, 2)) {
		*temp = ((signed)data[0]) + (((unsigned)(data[1]) >> 6) * 0.25);
		return true;
	}
	return false;
}

// get the time from the rtc, populates a supplied tm struct
// returns true to indicate success
bool ICACHE_FLASH_ATTR ds3231_getTime(struct tm *time) {
	
	int loop;
	uint8 data[7];

	// start register address
	data[0] = DS3231_ADDR_TIME;
	if (!ds3231_send(data, 1)) {
		return false;
	}

	// read time
	if (!ds3231_recv(data, 7)) {
		return false;
	}

	// convert to unix time structure
	time->tm_sec = bcdToDec(data[0]);
	time->tm_min = bcdToDec(data[1]);
	if (data[2] & DS3231_12HOUR_FLAG) {
		// 12h
		time->tm_hour = bcdToDec(data[2] & DS3231_12HOUR_MASK);
		// pm?
		if (data[2] & DS3231_PM_FLAG) time->tm_hour += 12;
	} else {
		// 24h
		time->tm_hour = bcdToDec(data[2]);
	}
	time->tm_wday = bcdToDec(data[3]) - 1;
	time->tm_mday = bcdToDec(data[4]);
	time->tm_mon  = bcdToDec(data[5] & DS3231_MONTH_MASK) - 1;
	time->tm_year = bcdToDec(data[6]) + 100;
	time->tm_isdst = 0;

	// apply a time zone (if you are not using localtime on the rtc or you want to check/apply DST)
	//applyTZ(time);

	return true;
	
}
