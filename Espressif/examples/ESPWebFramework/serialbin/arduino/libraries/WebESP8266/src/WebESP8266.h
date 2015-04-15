/*
WebESP8266.h
Binary Bidirectional Protocol for ESP8266

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

The latest version of this library can always be found at
https://github.com/fdivitto/ESPWebFramework
*/

#ifndef WebESP8266_h
#define WebESP8266_h

#include <inttypes.h>
#include <Stream.h>

#include "WebESP8266priv.h"


/******************************************************************************
* Definitions
******************************************************************************/

class WebESP8266
{
	
public:

	// platforms
	static uint8_t const PLATFORM_BASELINE     = 0;	// use when platform specific features aren't used
	static uint8_t const PLATFORM_ESP8266      = 1;
	static uint8_t const PLATFORM_ATMEGA328    = 2;
	static uint8_t const PLATFORM_THIS         = PLATFORM_ATMEGA328;

	// pin configuration flags
	static uint8_t const PIN_CONF_OUTPUT       = 0b00000001;	// 0 = input / 1 = output
	static uint8_t const PIN_CONF_PULLUP       = 0b00000010;	// 0 = disabled / 1 = enabled (if supported)		

	// pin identifiers - ESP8266
	static uint8_t const PIN_IDENTIFIER_ESP8266_GPIO0  = 0;
	static uint8_t const PIN_IDENTIFIER_ESP8266_GPIO2  = 2;
	static uint8_t const PIN_IDENTIFIER_ESP8266_GPIO4  = 4;
	static uint8_t const PIN_IDENTIFIER_ESP8266_GPIO5  = 5;
	static uint8_t const PIN_IDENTIFIER_ESP8266_GPIO12 = 12;
	static uint8_t const PIN_IDENTIFIER_ESP8266_GPIO13 = 13;
	static uint8_t const PIN_IDENTIFIER_ESP8266_GPIO14 = 14;
	static uint8_t const PIN_IDENTIFIER_ESP8266_GPIO15 = 15;
	static uint8_t const PIN_IDENTIFIER_ESP8266_GPIO16 = 16;

	// pin identifiers - ATmega328
	static uint8_t const PIN_IDENTIFIER_ATMEGA328_PB0  = 0;	 // Arduino 8
	static uint8_t const PIN_IDENTIFIER_ATMEGA328_PB1  = 1;	 // Arduino 9
	static uint8_t const PIN_IDENTIFIER_ATMEGA328_PB2  = 2;	 // Arduino 10
	static uint8_t const PIN_IDENTIFIER_ATMEGA328_PB3  = 3;	 // Arduino 11
	static uint8_t const PIN_IDENTIFIER_ATMEGA328_PB4  = 4;	 // Arduino 12
	static uint8_t const PIN_IDENTIFIER_ATMEGA328_PB5  = 5;	 // Arduino 13
	static uint8_t const PIN_IDENTIFIER_ATMEGA328_PC0  = 8;	 // Arduino A0
	static uint8_t const PIN_IDENTIFIER_ATMEGA328_PC1  = 9;	 // Arduino A1
	static uint8_t const PIN_IDENTIFIER_ATMEGA328_PC2  = 10; // Arduino A2
	static uint8_t const PIN_IDENTIFIER_ATMEGA328_PC3  = 11; // Arduino A3
	static uint8_t const PIN_IDENTIFIER_ATMEGA328_PC4  = 12; // Arduino A4
	static uint8_t const PIN_IDENTIFIER_ATMEGA328_PC5  = 13; // Arduino A5
	static uint8_t const PIN_IDENTIFIER_ATMEGA328_PD0  = 16; // Arduino 0
	static uint8_t const PIN_IDENTIFIER_ATMEGA328_PD1  = 17; // Arduino 1
	static uint8_t const PIN_IDENTIFIER_ATMEGA328_PD2  = 18; // Arduino 2
	static uint8_t const PIN_IDENTIFIER_ATMEGA328_PD3  = 19; // Arduino 3
	static uint8_t const PIN_IDENTIFIER_ATMEGA328_PD4  = 20; // Arduino 4
	static uint8_t const PIN_IDENTIFIER_ATMEGA328_PD5  = 21; // Arduino 5
	static uint8_t const PIN_IDENTIFIER_ATMEGA328_PD6  = 22; // Arduino 6
	static uint8_t const PIN_IDENTIFIER_ATMEGA328_PD7  = 23; // Arduino 7
	
	// Public Methods
	WebESP8266();
	void begin(Stream& stream);
	bool isReady();
	bool checkReady();
	uint8_t getPlatform();
	void yield();
	
	bool send_CMD_READY();
	bool send_CMD_IOCONF(uint8_t pin, uint8_t flags);
	bool send_CMD_IOSET(uint8_t pin, uint8_t state);
	bool send_CMD_IOGET(uint8_t pin, uint8_t* state);
	bool send_CMD_IOASET(uint8_t pin, uint16_t state);
	bool send_CMD_IOAGET(uint8_t pin, uint16_t* state);
	
private:

	// Private Methods
	WebESP8266priv::Message receive();
	int readByte(uint32_t timeOut);
	uint32_t readBuffer(uint8_t* buffer, uint32_t size, uint32_t timeOut);
	void processMessage(WebESP8266priv::Message* msg);
	void handle_CMD_READY(WebESP8266priv::Message* msg);
	uint8_t getNextID();
	void send(WebESP8266priv::Message* msg);
	WebESP8266priv::Message waitACK(uint8_t ackID);
	void sendNoParamsACK(uint8_t ackID);
	bool waitNoParamsACK(uint8_t ackID);
	void handle_CMD_IOCONF(WebESP8266priv::Message* msg);
	void handle_CMD_IOSET(WebESP8266priv::Message* msg);
	void handle_CMD_IOGET(WebESP8266priv::Message* msg);
	void handle_CMD_IOASET(WebESP8266priv::Message* msg);
	void handle_CMD_IOAGET(WebESP8266priv::Message* msg);
	
private:
	Stream* _stream;
	uint8_t _recvID;
	uint8_t _sendID;
	bool    _isReady;
	uint8_t _platform;
};





#endif
