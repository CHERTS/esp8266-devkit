/*
# Created by Fabrizio Di Vittorio (fdivitto2013@gmail.com)
# Copyright (c) 2015 Fabrizio Di Vittorio.
# All rights reserved.

# GNU GPL LICENSE
#
# This module is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; latest version thereof,
# available at: <http://www.gnu.org/licenses/gpl.txt>.
#
# This module is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this module; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA
*/

#ifndef _FDVSERIALSRV_H_
#define _FDVSERIALSRV_H_

#include "fdv.h"


namespace fdv
{
	
	
	enum SerialService
	{
		SerialService_None = 0,
		SerialService_Console,
		SerialService_BinaryProtocol,		
	};
	
	
	
	//////////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////
	// SerialConsole
	
#if (FDV_INCLUDE_SERIALCONSOLE == 1)
	
	struct SerialConsole : public Task
	{

		static uint32_t const MAX_PARAMETERS = 6;	// including command name
		
	
		SerialConsole()
			: Task(false, 300)
		{
		}

		// destructor must be called outside This task
		~SerialConsole()
		{
			terminate();
		}
		
		void exec();		
		void separateParameters();				
		void routeCommand();
		
		void cmd_help();
		void cmd_reboot();		
		void cmd_restore();		
		void cmd_free();		
        void cmd_ifconfig();
		void cmd_iwlist();
        void cmd_date();        
        void cmd_ntpdate();
		void cmd_nslookup();        
        void cmd_uptime();                
		void cmd_test();		
		
	private:
		Serial*                 m_serial;
		LinkedCharChunks        m_receivedChunks;
		uint32_t                m_paramsCount;
		CharChunksIterator      m_params[MAX_PARAMETERS];
        DateTime                m_bootTime; // actually SerialConsole uptime!
	};
	
#endif	// FDV_INCLUDE_SERIALCONSOLE


#if (FDV_INCLUDE_SERIALBINARY == 1)

	//////////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////
	// SerialBinary
	//
	// This binary protocol is bidirectional and equivalent on both sides, there no masters and no slaves. Each part can initiate communication.
	// Data is enclosed in messages. A message contains an id, a command and optional parameters.
	// Receiver replies to commands with an ACK message, which can include optional parameters.
	// Sender must wait up to 2000ms for an ACK message. In case no ACK is received it must resend the message with the same ID for 3 times.
	// Receiver must ignore a message with the same ID of the previous one.
	// Each part should begin sending (or receive) a CMD_READY message before other messages.
	// All values are little-endian
	// Each part should handle the case when it expects an ACK but a command arrives. In this case receiver should process the command before, and then wait again for the ACK.
	// Each part must have the same protocol version.
	//
	// Example 1. How switch Arduino LED (pin13) on/off. Arduino with "FullControl" example sketch.
	// SerialBinary* sb = ConfigurationManager::getSerialBinary();	
	// while (true)
	// {
	//   if (!sb->isReady() && sb->checkReady())
	//   {
	//     // Link just established. Set pin 13 as Output.
	//     sb->send_CMD_IOCONF(13, SerialBinary::PIN_CONF_OUTPUT);
	//   }
	//   if (sb->isReady())
	//   {
	//     sb->send_CMD_IOSET(13, true);
	//     Task::delay(500);
	//     sb->send_CMD_IOSET(13, false);
	//     Task::delay(500);
	//   }
	// }
	//
	//
	// Message structure:
	//   1 uint8_t  : Incremental ID. Each part maintain an incremental ID, which identifies a message. Replies include the same ID.
	//   1 uint8_t  : Command
	//   1 uint16_t : Data size. This is 0x0000 if there isn't data
	//   n uint8_t  : Optional command parameters (See CMD Parameters below)
	//
	// Commands:
	//
	//   CMD_ACK (0x00)
	//     Description:
	//       Acknowledge of a message
	//     Parameters:
	//       1 uint8_t : ID of acknowledged message (different than this ACK message ID!)
	//       n uint8_t : Optional ACK parameters (See ACK Parameters below)
	//
	//   CMD_READY
	//     Description:
	//        Communicates this device is ready
	//     CMD Parameters:
	//       1  uint8_t : protocol version
	//       1  uint8_t : platform (see PLATFORM_XXX values)
	//       10 uint8_t : magic string "BINPRORDY\x00"
	//     ACK Parameters (added to CMD_ACK parameters):
	//       1  uint8_t : protocol version
	//       1  uint8_t : platform (see PLATFORM_XXX values)
	//       10 uint8_t : magic string "BINPRORDY\x00"
	//
	//   CMD_IOCONF
	//     Description:
	//       Configure input/output pins
	//     CMD Parameters:
	//       1  uint8_t : pin identifier (see PIN_IDENTIFIER_XX values)
	//       1  uint8_t : flags (see PIN_CONF_XXX values)
	//     ACK Parameters (added to CMD_ACK parameters):
	//       none
	//
	//   CMD_IOSET
	//     Description:
	//       Set output state of digital pin (low/high)
	//     CMD Parameters:
	//       1  uint8_t : pin identifier (see PIN_IDENTIFIER_XX values)
	//       1  uint8_t : output state (0 = low, 1 = high)
	//     ACK Parameters (added to CMD_ACK parameters):
	//        none
	//
	//   CMD_IOGET
	//     Description:
	//       Get input state of digital pin (low/high)
	//     CMD Parameters:
	//       1  uint8_t : pin identifier (see PIN_IDENTIFIER_XX values)
	//     ACK Parameters (added to CMD_ACK parameters):
	//       1  uint8_t : input state (0 = low, 1 = high)  
	//
	//   CMD_IOASET
	//     Description:
	//       Set output state of analog pin (0..1023, as PWM output)
	//     CMD Parameters:
	//       1  uint8_t  : pin identifier (see PIN_IDENTIFIER_XX values)
	//       1  uint16_t : output state (0..1023)
	//     ACK Parameters (added to CMD_ACK parameters):
	//        none
	//
	//   CMD_IOAGET
	//     Description:
	//       Get input state of analog pin (0..1023)
	//     CMD Parameters:
	//       1  uint8_t  : pin identifier (see PIN_IDENTIFIER_XX values)
	//     ACK Parameters (added to CMD_ACK parameters):
	//       1  uint16_t : input state (0..1023)  
	
	class SerialBinary
	{

		static uint8_t const  PROTOCOL_VERSION      = 1;
	
		static uint32_t const INTRA_MSG_TIMEOUT    = 200;
		static uint32_t const WAIT_MSG_TIMEOUT     = 2000;
		static uint32_t const PUTACK_TIMEOUT       = 200;
		static uint32_t const GETACK_TIMEOUT       = 2000;
		static uint32_t const ACKMSG_QUEUE_LENGTH  = 2;
		static uint32_t const MAX_RESEND_COUNT     = 3;
		
		// commands
		static uint8_t const CMD_ACK               = 0;
		static uint8_t const CMD_READY             = 1;
		static uint8_t const CMD_IOCONF            = 2;
		static uint8_t const CMD_IOSET             = 3;
		static uint8_t const CMD_IOGET             = 4;
		static uint8_t const CMD_IOASET            = 5;
		static uint8_t const CMD_IOAGET            = 6;
		
	public:
		
		// platforms		
		static uint8_t const PLATFORM_BASELINE     = 0;	// use when platform specific features aren't used
		static uint8_t const PLATFORM_ESP8266      = 1;
		static uint8_t const PLATFORM_ATMEGA328    = 2;		
		static uint8_t const PLATFORM_THIS         = PLATFORM_ESP8266;

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
		static uint8_t const PIN_IDENTIFIER_ATMEGA328_PB0  = 8;	 // Arduino 8
		static uint8_t const PIN_IDENTIFIER_ATMEGA328_PB1  = 9;	 // Arduino 9
		static uint8_t const PIN_IDENTIFIER_ATMEGA328_PB2  = 10; // Arduino 10
		static uint8_t const PIN_IDENTIFIER_ATMEGA328_PB3  = 11; // Arduino 11
		static uint8_t const PIN_IDENTIFIER_ATMEGA328_PB4  = 12; // Arduino 12
		static uint8_t const PIN_IDENTIFIER_ATMEGA328_PB5  = 13; // Arduino 13
		static uint8_t const PIN_IDENTIFIER_ATMEGA328_PC0  = 14; // Arduino A0
		static uint8_t const PIN_IDENTIFIER_ATMEGA328_PC1  = 15; // Arduino A1
		static uint8_t const PIN_IDENTIFIER_ATMEGA328_PC2  = 16; // Arduino A2
		static uint8_t const PIN_IDENTIFIER_ATMEGA328_PC3  = 17; // Arduino A3
		static uint8_t const PIN_IDENTIFIER_ATMEGA328_PC4  = 18; // Arduino A4
		static uint8_t const PIN_IDENTIFIER_ATMEGA328_PC5  = 19; // Arduino A5
		static uint8_t const PIN_IDENTIFIER_ATMEGA328_PD0  = 0;  // Arduino 0
		static uint8_t const PIN_IDENTIFIER_ATMEGA328_PD1  = 1;  // Arduino 1
		static uint8_t const PIN_IDENTIFIER_ATMEGA328_PD2  = 2;  // Arduino 2
		static uint8_t const PIN_IDENTIFIER_ATMEGA328_PD3  = 3;  // Arduino 3
		static uint8_t const PIN_IDENTIFIER_ATMEGA328_PD4  = 4;  // Arduino 4
		static uint8_t const PIN_IDENTIFIER_ATMEGA328_PD5  = 5;  // Arduino 5
		static uint8_t const PIN_IDENTIFIER_ATMEGA328_PD6  = 6;  // Arduino 6
		static uint8_t const PIN_IDENTIFIER_ATMEGA328_PD7  = 7;  // Arduino 7
		

	private:
		
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
            
			void MTD_FLASHMEM freeData();
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
			Message_CMD_READY(Message* msg, uint8_t protocolVersion_, uint8_t platform_, char const* magicString_)
				: protocolVersion(msg->data[0]), 
				  platform(msg->data[1]), 
				  magicString((char*)msg->data + 2)
			{
				protocolVersion = protocolVersion_;
				platform        = platform_;
				f_strcpy(magicString, magicString_);
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
			Message_CMD_READY_ACK(Message* msg, uint8_t ackID_, uint8_t protocolVersion_, uint8_t platform_, char const* magicString_)
				: Message_ACK(msg, ackID_), 
				  protocolVersion(msg->data[Message_ACK::SIZE + 0]), 
				  platform(msg->data[Message_ACK::SIZE + 1]), 
				  magicString((char*)msg->data + Message_ACK::SIZE + 2)
			{
				protocolVersion = protocolVersion_;
				platform        = platform_;
				f_strcpy(magicString, magicString_);
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


		
	public:
	
		SerialBinary()
			: m_serial(HardwareSerial::getSerial(0)), 
			  m_recvID(255), 
			  m_sendID(0), 
			  m_recvACKQueue(ACKMSG_QUEUE_LENGTH), 
			  m_receiveTask(this, false, 256),
			  m_isReady(false),
			  m_platform(PLATFORM_BASELINE)
		{
		}
		
		
		~SerialBinary()
		{
			m_receiveTask.terminate();
			// todo: free pending messages
		}
		
		
		bool MTD_FLASHMEM isReady()
		{
			MutexLock lock(&m_mutex);
			return m_isReady;
		}

		
		bool MTD_FLASHMEM checkReady()
		{
			if (!isReady())
				send_CMD_READY();
			return isReady();
		}
		
		
		
		uint8_t MTD_FLASHMEM getPlatform()
		{
			checkReady();
			MutexLock lock(&m_mutex);
			return m_platform;
		}


	private:
				
		
		Message MTD_FLASHMEM receive()
		{
			Message msg;
			SoftTimeOut timeout(WAIT_MSG_TIMEOUT);
			while (!timeout)
			{
				// ID
				int16_t r = m_serial->read(INTRA_MSG_TIMEOUT);
				if (r < 0)
					continue;
				msg.ID = r;

				// Command
				r = m_serial->read(INTRA_MSG_TIMEOUT);
				if (r < 0)
					continue;
				msg.command = r;

				// Data Size Low
				r = m_serial->read(INTRA_MSG_TIMEOUT);
				if (r < 0)
					continue;
				msg.dataSize = r;

				// Data Size High
				r = m_serial->read(INTRA_MSG_TIMEOUT);
				if (r < 0)
					continue;
				msg.dataSize |= r << 8;

				// Data			
				if (msg.dataSize > 0 && msg.dataSize < (Task::getFreeHeap() >> 1))
				{
					msg.data = new uint8_t[msg.dataSize];
					if (m_serial->read(msg.data, msg.dataSize, INTRA_MSG_TIMEOUT) < msg.dataSize)
					{
						msg.freeData();
						continue;
					}
				}
				
				// check ID
				if (msg.ID == m_recvID)
				{
					msg.freeData();
					continue;
				}
				m_recvID = msg.ID;
				
				msg.valid = true;
				return msg;
			}			
			return msg;
		}
		
		
		uint8_t MTD_FLASHMEM getNextID()
		{
			MutexLock lock(&m_mutex);
			return ++m_sendID;
		}
		
		
		void MTD_FLASHMEM send(Message* msg)
		{
			MutexLock lock(&m_mutex);
			m_serial->write(msg->ID);
			m_serial->write(msg->command);
			m_serial->write(msg->dataSize & 0xFF);
			m_serial->write((msg->dataSize >> 8) & 0xFF);
			if (msg->dataSize > 0)
				m_serial->write(msg->data, msg->dataSize);
		}
		
		
		// send ACK without parameters
		void MTD_FLASHMEM sendNoParamsACK(uint8_t ackID)
		{
			Message msgContainer(getNextID(), CMD_ACK, Message_ACK::SIZE);
			Message_ACK msgACK(&msgContainer, ackID);
			send(&msgContainer);
			msgContainer.freeData();			
		}
		
		
		Message MTD_FLASHMEM waitACK(uint8_t ackID)
		{
			Message msgContainer;
			SoftTimeOut timeout(GETACK_TIMEOUT);
			while (!timeout)
			{
				if (m_recvACKQueue.receive(&msgContainer, GETACK_TIMEOUT))
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
		
		
		bool MTD_FLASHMEM waitNoParamsACK(uint8_t ackID)
		{
			Message msgContainer = waitACK(ackID);
			if (msgContainer.valid)
			{
				msgContainer.freeData();
				return true;
			}
			return false;
		}		
		

		void MTD_FLASHMEM receiveTask()
		{
			while (true)
			{
				Message msg = receive();
				if (msg.valid)
				{
					if (msg.command == CMD_ACK)
						// if message is an ACK then put it into the ACK message queue, another task will handle it
						m_recvACKQueue.send(msg, PUTACK_TIMEOUT);
					else
						processMessage(&msg);
				}
			}
		}
		
		
		// must not process CMD_ACK messages
		void MTD_FLASHMEM processMessage(Message* msg)
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
		
		
		void MTD_FLASHMEM handle_CMD_READY(Message* msg)
		{
			// process message
			Message_CMD_READY msgCMDREADY(msg);
			m_mutex.lock();
			m_isReady  = (msgCMDREADY.protocolVersion == PROTOCOL_VERSION && f_strcmp(msgCMDREADY.magicString, STR_BINPRORDY) == 0);
			m_platform = msgCMDREADY.platform;
			m_mutex.unlock();
			
			// send ACK with parameters
			Message msgContainer(getNextID(), CMD_ACK, Message_CMD_READY_ACK::SIZE);
			Message_CMD_READY_ACK msgCMDREADYACK(&msgContainer, msg->ID, PROTOCOL_VERSION, PLATFORM_THIS, STR_BINPRORDY);
			send(&msgContainer);
			msgContainer.freeData();
		}
		
		
		void MTD_FLASHMEM handle_CMD_IOCONF(Message* msg)
		{
			// process message
			Message_CMD_IOCONF msgIOCONF(msg);
			if (msgIOCONF.flags & PIN_CONF_OUTPUT)
				GPIOX(msgIOCONF.pin).modeOutput();
			else
				GPIOX(msgIOCONF.pin).modeInput();
			GPIOX(msgIOCONF.pin).enablePullUp(msgIOCONF.flags & PIN_CONF_PULLUP);
						
			// send simple ACK
			sendNoParamsACK(msg->ID);
		}
		
		
		void MTD_FLASHMEM handle_CMD_IOSET(Message* msg)
		{
			// process message
			Message_CMD_IOSET msgIOSET(msg);
			GPIOX(msgIOSET.pin).write(msgIOSET.state);
			
			// send simple ACK
			sendNoParamsACK(msg->ID);
		}
		
		
		void MTD_FLASHMEM handle_CMD_IOGET(Message* msg)
		{
			// process message
			Message_CMD_IOGET msgIOGET(msg);
			bool state = GPIOX(msgIOGET.pin).read();
			
			// send ACK with parameters
			Message msgContainer(getNextID(), CMD_ACK, Message_CMD_IOGET_ACK::SIZE);
			Message_CMD_IOGET_ACK msgCMDIOGETACK(&msgContainer, msg->ID, state);
			send(&msgContainer);
			msgContainer.freeData();
		}
		
		
		// not implemented
		void MTD_FLASHMEM handle_CMD_IOASET(Message* msg)
		{
			// process message
			// not implemented
			
			// send simple ACK
			sendNoParamsACK(msg->ID);
		}
		
		
		// not implemented
		void MTD_FLASHMEM handle_CMD_IOAGET(Message* msg)
		{
			// process message
			// not implemented
			
			// send ACK with parameters
			Message msgContainer(getNextID(), CMD_ACK, Message_CMD_IOAGET_ACK::SIZE);
			Message_CMD_IOAGET_ACK msgCMDIOGETACK(&msgContainer, msg->ID, 0);	// always returns 0!
			send(&msgContainer);
			msgContainer.freeData();
		}		
						
		
	public:
	
		bool MTD_FLASHMEM send_CMD_READY()
		{
			m_isReady = false;
			for (uint32_t i = 0; i != MAX_RESEND_COUNT; ++i)
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
					MutexLock lock(&m_mutex);
					m_isReady  = (msgCMDREADYACK.protocolVersion == PROTOCOL_VERSION && f_strcmp(msgCMDREADYACK.magicString, STR_BINPRORDY) == 0);
					m_platform = msgCMDREADYACK.platform;
					msgContainer.freeData();
					return true;
				}
			}
			return false;
		}
		
		
		bool MTD_FLASHMEM send_CMD_IOCONF(uint8_t pin, uint8_t flags)
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
				m_isReady = false;	// no more ready
			}
			return false;
		}


		bool MTD_FLASHMEM send_CMD_IOSET(uint8_t pin, uint8_t state)
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
				m_isReady = false;	// no more ready
			}
			return false;
		}


		bool MTD_FLASHMEM send_CMD_IOGET(uint8_t pin, uint8_t* state)
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
				m_isReady = false;	// no more ready
			}
			return false;
		}


		bool MTD_FLASHMEM send_CMD_IOASET(uint8_t pin, uint16_t state)
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
				m_isReady = false;	// no more ready
			}
			return false;
		}


		bool MTD_FLASHMEM send_CMD_IOAGET(uint8_t pin, uint16_t* state)
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
				m_isReady = false;	// no more ready
			}
			return false;
		}
		
		
	private:
		Serial*                                              m_serial;
		uint8_t                                              m_recvID;		
		uint8_t                                              m_sendID;
		Queue<Message>                                       m_recvACKQueue;
		MethodTask<SerialBinary, &SerialBinary::receiveTask> m_receiveTask;
		Mutex                                                m_mutex;
		bool                                                 m_isReady;
		uint8_t                                              m_platform;
	};

#endif // FDV_INCLUDE_SERIALBINARY
	
}

#endif

