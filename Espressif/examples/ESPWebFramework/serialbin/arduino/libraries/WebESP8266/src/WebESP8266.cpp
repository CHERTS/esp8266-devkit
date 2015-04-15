/*
WebESP8266.cpp
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

// 
// Includes
// 
#include <Arduino.h>
#include "WebESP8266.h"
#include "WebESP8266priv.h"

#include <avr/pgmspace.h>

using namespace WebESP8266priv;


WebESP8266::WebESP8266()
	: _stream(NULL),
	  _recvID(255),
	  _sendID(0),
	  _isReady(false),
	  _platform(PLATFORM_BASELINE)
{
}


void WebESP8266::begin(Stream& stream)
{
	_stream = &stream;
	send_CMD_READY();
}


bool WebESP8266::isReady()
{
	return _isReady;
}


bool WebESP8266::checkReady()
{
	if (!isReady())
		send_CMD_READY();
	return isReady();
}


uint8_t WebESP8266::getPlatform()
{
	checkReady();
	return _platform;
}


void WebESP8266::yield()
{
	if (_stream->available() > 0)
	{
		Message msg = receive();
		if (msg.valid)
		{
			if (msg.command != CMD_ACK)
				processMessage(&msg);
		}
	}
}


// must not process CMD_ACK messages
void WebESP8266::processMessage(Message* msg)
{
	switch (msg->command)
	{
		case CMD_READY:
			handle_CMD_READY(msg);
			break;
		case CMD_IOCONF:
			handle_CMD_IOCONF(msg);
			break;
		case CMD_IOSET:
			handle_CMD_IOSET(msg);
			break;
		case CMD_IOGET:
			handle_CMD_IOGET(msg);
			break;
		case CMD_IOASET:
			handle_CMD_IOASET(msg);
			break;
		case CMD_IOAGET:
			handle_CMD_IOAGET(msg);
			break;
	}			
	msg->freeData();
}


int WebESP8266::readByte(uint32_t timeOut)
{
	SoftTimeOut timeout(timeOut);
	while (!timeout)
		if (_stream->available() > 0)
			return _stream->read();
	return -1;
}


// note: timeout resets each time a byte is received
uint32_t WebESP8266::readBuffer(uint8_t* buffer, uint32_t size, uint32_t timeOut)
{
	for (uint32_t i = 0; i != size; ++i)
	{
		int b = readByte(timeOut);
		if (b < 0)
			return i;
		*buffer++ = b;
	}
	return size;
}


Message WebESP8266::receive()
{
	Message msg;
	SoftTimeOut timeout(WAIT_MSG_TIMEOUT);
	while (!timeout)
	{
		// ID
		int16_t r = readByte(INTRA_MSG_TIMEOUT);
		if (r < 0)
			continue;
		msg.ID = r;

		// Command
		r = readByte(INTRA_MSG_TIMEOUT);
		if (r < 0)
			continue;
		msg.command = r;

		// Data Size Low
		r = readByte(INTRA_MSG_TIMEOUT);
		if (r < 0)
			continue;
		msg.dataSize = r;

		// Data Size High
		r = readByte(INTRA_MSG_TIMEOUT);
		if (r < 0)
			continue;
		msg.dataSize |= r << 8;

		// Data			
		if (msg.dataSize > 0 && msg.dataSize < MAX_DATA_SIZE)
		{
			msg.data = new uint8_t[msg.dataSize];
			if (readBuffer(msg.data, msg.dataSize, INTRA_MSG_TIMEOUT) < msg.dataSize)
			{
				msg.freeData();
				continue;
			}
		}
		
		// check ID
		if (msg.ID == _recvID)
		{
			msg.freeData();
			continue;
		}
		_recvID = msg.ID;
		
		msg.valid = true;
		return msg;
	}			
	return msg;
}


uint8_t WebESP8266::getNextID()
{
	return ++_sendID;
}


void WebESP8266::send(Message* msg)
{
	_stream->write(msg->ID);
	_stream->write(msg->command);
	_stream->write(msg->dataSize & 0xFF);
	_stream->write((msg->dataSize >> 8) & 0xFF);
	if (msg->dataSize > 0)
		_stream->write(msg->data, msg->dataSize);
}


// send ACK without parameters
void WebESP8266::sendNoParamsACK(uint8_t ackID)
{
	Message msgContainer(getNextID(), CMD_ACK, Message_ACK::SIZE);
	Message_ACK msgACK(&msgContainer, ackID);
	send(&msgContainer);
	msgContainer.freeData();			
}


Message WebESP8266::waitACK(uint8_t ackID)
{
	Message msgContainer;
	SoftTimeOut timeout(GETACK_TIMEOUT);
	while (!timeout)
	{
		msgContainer = receive();
		if (msgContainer.valid)
		{
			Message_ACK msgACK(&msgContainer);
			if (msgACK.ackID == ackID)
				return msgContainer;
			msgContainer.freeData();	// discard this ACK
		}
	}
	msgContainer.valid = false;
	return msgContainer;
}


bool WebESP8266::waitNoParamsACK(uint8_t ackID)
{
	Message msgContainer = waitACK(ackID);
	if (msgContainer.valid)
	{
		msgContainer.freeData();
		return true;
	}
	return false;
}		


void WebESP8266::handle_CMD_READY(Message* msg)
{
	// process message
	Message_CMD_READY msgCMDREADY(msg);
	_isReady  = (msgCMDREADY.protocolVersion == PROTOCOL_VERSION && strcmp_P(msgCMDREADY.magicString, STR_BINPRORDY) == 0);
	_platform = msgCMDREADY.platform;
	
	// send ACK with parameters
	Message msgContainer(getNextID(), CMD_ACK, Message_CMD_READY_ACK::SIZE);
	Message_CMD_READY_ACK msgCMDREADYACK(&msgContainer, msg->ID, PROTOCOL_VERSION, PLATFORM_THIS, STR_BINPRORDY);
	send(&msgContainer);
	msgContainer.freeData();
}


void WebESP8266::handle_CMD_IOCONF(Message* msg)
{
	// process message
	Message_CMD_IOCONF msgIOCONF(msg);
	if (msgIOCONF.flags & PIN_CONF_OUTPUT)
		pinMode(msgIOCONF.pin, OUTPUT);
	else
		pinMode(msgIOCONF.pin, (msgIOCONF.flags & PIN_CONF_PULLUP)? INPUT_PULLUP : INPUT);
				
	// send simple ACK
	sendNoParamsACK(msg->ID);
}


void WebESP8266::handle_CMD_IOSET(Message* msg)
{
	// process message
	Message_CMD_IOSET msgIOSET(msg);
	digitalWrite(msgIOSET.pin, msgIOSET.state);
	
	// send simple ACK
	sendNoParamsACK(msg->ID);
}


void WebESP8266::handle_CMD_IOGET(Message* msg)
{
	// process message
	Message_CMD_IOGET msgIOGET(msg);
	bool state = digitalRead(msgIOGET.pin);
	
	// send ACK with parameters
	Message msgContainer(getNextID(), CMD_ACK, Message_CMD_IOGET_ACK::SIZE);
	Message_CMD_IOGET_ACK msgCMDIOGETACK(&msgContainer, msg->ID, state);
	send(&msgContainer);
	msgContainer.freeData();
}


void WebESP8266::handle_CMD_IOASET(Message* msg)
{
	// process message
	Message_CMD_IOASET msgIOASET(msg);
	analogWrite(msgIOASET.pin, msgIOASET.state);
	
	// send simple ACK
	sendNoParamsACK(msg->ID);
}


void WebESP8266::handle_CMD_IOAGET(Message* msg)
{
	// process message
	Message_CMD_IOAGET msgIOAGET(msg);
	uint16_t state = analogRead(msgIOAGET.pin);
	
	// send ACK with parameters
	Message msgContainer(getNextID(), CMD_ACK, Message_CMD_IOAGET_ACK::SIZE);
	Message_CMD_IOAGET_ACK msgCMDIOGETACK(&msgContainer, msg->ID, state);
	send(&msgContainer);
	msgContainer.freeData();
}		


bool WebESP8266::send_CMD_READY()
{
	_isReady = false;
	for (uint8_t i = 0; i != MAX_RESEND_COUNT; ++i)
	{
		// send message
		uint8_t msgID = getNextID();
		Message msgContainer(msgID, CMD_READY, Message_CMD_READY::SIZE);
		Message_CMD_READY msgCMDREADY(&msgContainer, PROTOCOL_VERSION, PLATFORM_THIS, STR_BINPRORDY);
		send(&msgContainer);
		msgContainer.freeData();
		
		// wait for ACK
		msgContainer = waitACK(msgID);
		if (msgContainer.valid)
		{
			Message_CMD_READY_ACK msgCMDREADYACK(&msgContainer);
			_isReady  = (msgCMDREADYACK.protocolVersion == PROTOCOL_VERSION && strcmp_P(msgCMDREADYACK.magicString, STR_BINPRORDY) == 0);
			_platform = msgCMDREADYACK.platform;
			msgContainer.freeData();
			return true;
		}
	}
	return false;
}


bool WebESP8266::send_CMD_IOCONF(uint8_t pin, uint8_t flags)
{
	if (checkReady())
	{
		for (uint32_t i = 0; i != MAX_RESEND_COUNT; ++i)
		{
			// send message
			uint8_t msgID = getNextID();
			Message msgContainer(msgID, CMD_IOCONF, Message_CMD_IOCONF::SIZE);
			Message_CMD_IOCONF msgCMDIOCONF(&msgContainer, pin, flags);
			send(&msgContainer);
			msgContainer.freeData();
			
			// wait for ACK
			if (waitNoParamsACK(msgID))
				return true;
		}
		_isReady = false;	// no more ready
	}
	return false;
}


bool WebESP8266::send_CMD_IOSET(uint8_t pin, uint8_t state)
{
	if (checkReady())
	{
		for (uint32_t i = 0; i != MAX_RESEND_COUNT; ++i)
		{
			// send message
			uint8_t msgID = getNextID();
			Message msgContainer(msgID, CMD_IOSET, Message_CMD_IOSET::SIZE);
			Message_CMD_IOSET msgCMDIOSET(&msgContainer, pin, state);
			send(&msgContainer);
			msgContainer.freeData();
			
			// wait for ACK
			if (waitNoParamsACK(msgID))
				return true;
		}
		_isReady = false;	// no more ready
	}
	return false;
}


bool WebESP8266::send_CMD_IOGET(uint8_t pin, uint8_t* state)
{
	if (checkReady())
	{
		for (uint32_t i = 0; i != MAX_RESEND_COUNT; ++i)
		{
			// send message
			uint8_t msgID = getNextID();
			Message msgContainer(msgID, CMD_IOGET, Message_CMD_IOGET::SIZE);
			Message_CMD_IOGET msgCMDIOGET(&msgContainer, pin);
			send(&msgContainer);
			msgContainer.freeData();
			
			// wait for ACK
			msgContainer = waitACK(msgID);
			if (msgContainer.valid)
			{
				Message_CMD_IOGET_ACK msgCMDIOGETACK(&msgContainer);
				*state = msgCMDIOGETACK.state;
				msgContainer.freeData();
				return true;
			}
		}
		_isReady = false;	// no more ready
	}
	return false;
}


bool WebESP8266::send_CMD_IOASET(uint8_t pin, uint16_t state)
{
	if (checkReady())
	{
		for (uint32_t i = 0; i != MAX_RESEND_COUNT; ++i)
		{
			// send message
			uint8_t msgID = getNextID();
			Message msgContainer(msgID, CMD_IOASET, Message_CMD_IOASET::SIZE);
			Message_CMD_IOASET msgCMDIOASET(&msgContainer, pin, state);
			send(&msgContainer);
			msgContainer.freeData();
			
			// wait for ACK
			if (waitNoParamsACK(msgID))
				return true;
		}
		_isReady = false;	// no more ready
	}
	return false;
}


bool WebESP8266::send_CMD_IOAGET(uint8_t pin, uint16_t* state)
{
	if (checkReady())
	{
		for (uint32_t i = 0; i != MAX_RESEND_COUNT; ++i)
		{
			// send message
			uint8_t msgID = getNextID();
			Message msgContainer(msgID, CMD_IOAGET, Message_CMD_IOAGET::SIZE);
			Message_CMD_IOAGET msgCMDIOAGET(&msgContainer, pin);
			send(&msgContainer);
			msgContainer.freeData();
			
			// wait for ACK
			msgContainer = waitACK(msgID);
			if (msgContainer.valid)
			{
				Message_CMD_IOAGET_ACK msgCMDIOAGETACK(&msgContainer);
				*state = msgCMDIOAGETACK.state;
				msgContainer.freeData();
				return true;
			}
		}
		_isReady = false;	// no more ready
	}
	return false;
}


