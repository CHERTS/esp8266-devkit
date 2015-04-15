/*
WebESP8266priv.h
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

#ifndef WebESP8266priv_h
#define WebESP8266priv_h

#include <Arduino.h>
#include <inttypes.h>
#include <Stream.h>

#include <avr/pgmspace.h>


namespace WebESP8266priv
{

	// Protocol version
	static uint8_t const  PROTOCOL_VERSION      = 1;

	// Timings, etc...
	static uint32_t const INTRA_MSG_TIMEOUT    = 200;
	static uint32_t const WAIT_MSG_TIMEOUT     = 2000;
	static uint32_t const PUTACK_TIMEOUT       = 200;
	static uint32_t const GETACK_TIMEOUT       = 2000;
	static uint32_t const ACKMSG_QUEUE_LENGTH  = 2;
	static uint32_t const MAX_RESEND_COUNT     = 3;	
	static uint32_t const MAX_DATA_SIZE        = 256;
	
	// Commands
	static uint8_t const CMD_ACK               = 0;
	static uint8_t const CMD_READY             = 1;
	static uint8_t const CMD_IOCONF            = 2;
	static uint8_t const CMD_IOSET             = 3;
	static uint8_t const CMD_IOGET             = 4;
	static uint8_t const CMD_IOASET            = 5;
	static uint8_t const CMD_IOAGET            = 6;

	// Strings
	static char const STR_BINPRORDY[] PROGMEM  = "BINPRORDY";
	
	// calculates time difference in milliseconds, taking into consideration the time overflow
	// note: time1 must be less than time2 (time1 < time2)
	inline uint32_t millisDiff(uint32_t time1, uint32_t time2)
	{
		if (time1 > time2)
			// overflow
			return 0xFFFFFFFF - time1 + time2;
		else
			return time2 - time1;
	}
	

	///////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////
	// SoftTimeOut
	
	class SoftTimeOut
	{
		public:
			SoftTimeOut(uint32_t time)
				: m_timeOut(time), m_startTime(millis())
			{
			}

			operator bool()
			{
				return millisDiff(m_startTime, millis()) > m_timeOut;
			}
			
			void reset(uint32_t time)
			{
				m_timeOut   = time;
				m_startTime = millis();				
			}

		private:
			uint32_t m_timeOut;
			uint32_t m_startTime;
	};	

	
	///////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////
	// Messages

	// warn: memory must be excplitely delete using freeData(). Don't create a destructor to free data!
	struct Message
	{
		bool     valid;
		uint8_t  ID;
		uint8_t  command;
		uint16_t dataSize;
		uint8_t* data;
		
		Message()
			: valid(false), ID(0), command(0), dataSize(0), data(NULL)
		{
		}
		Message(uint8_t ID_, uint8_t command_, uint16_t dataSize_)
			: valid(true), ID(ID_), command(command_), dataSize(dataSize_), data(dataSize_? new uint8_t[dataSize_] : NULL)
		{
		}
		void freeData()
		{
			if (data != NULL)
			{
				delete[] data;
				data = NULL;
			}
		}
	};


	// base for ACK messages 
	struct Message_ACK
	{
		static uint16_t const SIZE = 1;
		
		uint8_t& ackID;
		
		// used to decode message
		Message_ACK(Message* msg)
			: ackID(msg->data[0])
		{
		}
		// used to encode message
		Message_ACK(Message* msg, uint8_t ackID_)
			: ackID(msg->data[0])
		{
			ackID = ackID_;
		}
	};


	struct Message_CMD_READY
	{
		static uint16_t const SIZE = 12;
		
		uint8_t& protocolVersion;
		uint8_t& platform;
		char*    magicString;
		
		// used to decode message
		Message_CMD_READY(Message* msg)
			: protocolVersion(msg->data[0]), 
			  platform(msg->data[1]), 
			  magicString((char*)msg->data + 2)
		{
		}
		// used to encode message
		Message_CMD_READY(Message* msg, uint8_t protocolVersion_, uint8_t platform_, PGM_P magicString_)
			: protocolVersion(msg->data[0]), 
			  platform(msg->data[1]), 
			  magicString((char*)msg->data + 2)
		{
			protocolVersion = protocolVersion_;
			platform        = platform_;
			strcpy_P(magicString, magicString_);
		}
		
	};


	struct Message_CMD_READY_ACK : Message_ACK
	{
		static uint16_t const SIZE = Message_ACK::SIZE + 12;
		
		uint8_t& protocolVersion;
		uint8_t& platform;
		char*    magicString;
		
		// used to decode message
		Message_CMD_READY_ACK(Message* msg)
				: Message_ACK(msg), 
				  protocolVersion(msg->data[Message_ACK::SIZE + 0]), 
				  platform(msg->data[Message_ACK::SIZE + 1]), 
				  magicString((char*)msg->data + Message_ACK::SIZE + 2)
		{
		}
		// used to encode message
		Message_CMD_READY_ACK(Message* msg, uint8_t ackID_, uint8_t protocolVersion_, uint8_t platform_, PGM_P magicString_)
				: Message_ACK(msg, ackID_), 
				  protocolVersion(msg->data[Message_ACK::SIZE + 0]), 
				  platform(msg->data[Message_ACK::SIZE + 1]), 
				  magicString((char*)msg->data + Message_ACK::SIZE + 2)
		{
			protocolVersion = protocolVersion_;
			platform        = platform_;
			strcpy_P(magicString, magicString_);
		}			
	};
	
	
	struct Message_CMD_IOCONF
	{
		static uint16_t const SIZE = 2;
		
		uint8_t& pin;
		uint8_t& flags;
		
		// used to decode message
		Message_CMD_IOCONF(Message* msg)
			: pin(msg->data[0]), 
			  flags(msg->data[1])
		{
		}
		// used to encode message
		Message_CMD_IOCONF(Message* msg, uint8_t pin_, uint8_t flags_)
			: pin(msg->data[0]), 
			  flags(msg->data[1])
		{
			pin   = pin_;
			flags = flags_;
		}			
	};


	struct Message_CMD_IOSET
	{
		static uint16_t const SIZE = 2;
		
		uint8_t& pin;
		uint8_t& state;
		
		// used to decode message
		Message_CMD_IOSET(Message* msg)
			: pin(msg->data[0]), 
			  state(msg->data[1])
		{
		}
		// used to encode message
		Message_CMD_IOSET(Message* msg, uint8_t pin_, uint8_t state_)
			: pin(msg->data[0]), 
			  state(msg->data[1])
		{
			pin   = pin_;
			state = state_;
		}			
	};


	struct Message_CMD_IOGET
	{
		static uint16_t const SIZE = 1;
		
		uint8_t& pin;
		
		// used to decode message
		Message_CMD_IOGET(Message* msg)
			: pin(msg->data[0])
		{
		}
		// used to encode message
		Message_CMD_IOGET(Message* msg, uint8_t pin_)
			: pin(msg->data[0])
		{
			pin   = pin_;
		}			
	};


	struct Message_CMD_IOGET_ACK : Message_ACK
	{
		static uint16_t const SIZE = Message_ACK::SIZE + 1;
		
		uint8_t& state;
		
		// used to decode message
		Message_CMD_IOGET_ACK(Message* msg)
			: Message_ACK(msg), 
			  state(msg->data[Message_ACK::SIZE + 0])
		{
		}
		// used to encode message
		Message_CMD_IOGET_ACK(Message* msg, uint8_t ackID_, uint8_t state_)
			: Message_ACK(msg, ackID_), 
			  state(msg->data[Message_ACK::SIZE + 0])
		{
			state = state_;
		}			
	};


	struct Message_CMD_IOASET
	{
		static uint16_t const SIZE = 3;
		
		uint8_t&  pin;
		uint16_t& state;
		
		// used to decode message
		Message_CMD_IOASET(Message* msg)
			: pin(msg->data[0]), 
			  state(*(uint16_t*)(msg->data + 1))
		{
		}
		// used to encode message
		Message_CMD_IOASET(Message* msg, uint8_t pin_, uint16_t state_)
			: pin(msg->data[0]), 
			  state(*(uint16_t*)(msg->data + 1))
		{
			pin   = pin_;
			state = state_;
		}			
	};


	struct Message_CMD_IOAGET
	{
		static uint16_t const SIZE = 1;
		
		uint8_t& pin;
		
		// used to decode message
		Message_CMD_IOAGET(Message* msg)
			: pin(msg->data[0])
		{
		}
		// used to encode message
		Message_CMD_IOAGET(Message* msg, uint8_t pin_)
			: pin(msg->data[0])
		{
			pin   = pin_;
		}			
	};
	

	struct Message_CMD_IOAGET_ACK : Message_ACK
	{
		static uint16_t const SIZE = Message_ACK::SIZE + 2;
		
		uint16_t& state;
		
		// used to decode message
		Message_CMD_IOAGET_ACK(Message* msg)
			: Message_ACK(msg), 
			  state(*(uint16_t*)(msg->data + Message_ACK::SIZE + 0))
		{
		}
		// used to encode message
		Message_CMD_IOAGET_ACK(Message* msg, uint8_t ackID_, uint16_t state_)
			: Message_ACK(msg, ackID_), 
			  state(*(uint16_t*)(msg->data + Message_ACK::SIZE + 0))
		{
			state = state_;
		}			
	};
	

}	// end of namespace

#endif
