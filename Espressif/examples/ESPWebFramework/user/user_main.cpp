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

// disable macros like "read"
#ifndef LWIP_POSIX_SOCKETS_IO_NAMES
#define LWIP_POSIX_SOCKETS_IO_NAMES 0
#endif

extern "C"
{
    #include "esp_common.h"    
    #include "freertos/FreeRTOS.h"
    #include "freertos/task.h"
}

#include "fdvserial.h"
#include "fdvsync.h"
#include "fdvflash.h"
#include "fdvtask.h"
#include "fdvnetwork.h"


struct MyTCPConnectionHandler : public fdv::TCPConnectionHandler
{
	void ICACHE_FLASH_ATTR connectionHandler()
	{
		while (isConnected())
		{
			printf("C.free stack=%d bytes\n\r", fdv::Task::getFreeStack());
			printf("Waiting for data\n\r");
			char buffer[64];
			int32_t len = read(buffer, sizeof(buffer));
			if (len > 0)
			{
				printf("%d -> ", len);
				char const* data = buffer;
				while (len--)
					printf("%c", *data++);
				printf("\n\r");
				write("ok\n\r", 4);
			}
		}
		printf("disconnected\n\r");
	}
};





struct Task1 : fdv::Task
{

	Task1(fdv::Serial* serial)
		: fdv::Task(400), m_serial(serial)
	{		
	}

	fdv::Serial* m_serial;
	
	void ICACHE_FLASH_ATTR exec()
	{
		
		m_serial->printf(FSTR("\n\rESPWebFramework started.\n\r"));
		m_serial->printf(FSTR("Press h key to help.\n\r"));
		
		while (1)
		{
			if (m_serial->waitForData())
			{
				uint8_t c = m_serial->read();
				switch (c)
				{
					case 'h':
						m_serial->printf(FSTR("A.Free stack = %d bytes\n\r"), getFreeStack());
						m_serial->printf(FSTR("A.Free heap  = %d bytes\n\r"), getFreeHeap());
						m_serial->printf(FSTR("Tests:\n\r"));
						m_serial->printf(FSTR("h    = help\n\r"));
						m_serial->printf(FSTR("r    = reset\n\r"));	
						m_serial->printf(FSTR("0    = format flash filesystem\n\r"));
						m_serial->printf(FSTR("1    = start AccessPoint mode (MyESP, myesp111)\n\r"));
						m_serial->printf(FSTR("2    = start DHCP server\n\r"));
						m_serial->printf(FSTR("3    = start Client mode static IP\n\r"));
						m_serial->printf(FSTR("4    = start Client mode dynamic IP\n\r"));
						m_serial->printf(FSTR("5    = open TCP server port 80\n\r"));
						break;
					case 'r':
						system_restart();
						break;
					case '0':
						fdv::FlashFileSystem::format();
						m_serial->writeln("Ok");
						break;
					case '1':
						// Access point
						fdv::WiFi::setMode(fdv::WiFi::AccessPoint);
						fdv::WiFi::configureAccessPoint("MyESP", "myesp111", 9);
						fdv::IP::configureStatic(fdv::IP::AccessPointNetwork, "192.168.5.1", "255.255.255.0", "192.168.5.1");						
						m_serial->printf(FSTR("Reboot and enable DHCP server (2)\n\r"));
						m_serial->printf(FSTR("Ok\n\r"));
						break;
					case '2':
						// Enable DHCP server
						fdv::IP::configureStatic(fdv::IP::AccessPointNetwork, "192.168.5.1", "255.255.255.0", "192.168.5.1");						
						fdv::DHCPServer::configure("192.168.5.100", "192.168.5.110", 10);
						m_serial->printf(FSTR("Ok\n\r"));
						break;
					case '3':
						// Client mode with static IP
						fdv::WiFi::setMode(fdv::WiFi::Client);
						fdv::WiFi::configureClient("OSPITI", "31415926");
						fdv::IP::configureStatic(fdv::IP::ClientNetwork, "192.168.1.199", "255.255.255.0", "192.168.1.1");						
						m_serial->printf(FSTR("Ok\n\r"));
						break;
					case '4':
						// Client mode with dynamic IP
						fdv::WiFi::setMode(fdv::WiFi::Client);
						fdv::WiFi::configureClient("OSPITI", "31415926");
						fdv::IP::configureDHCP(fdv::IP::ClientNetwork);
						m_serial->printf(FSTR("Ok\n\r"));
						break;
					case '5':
					{
						new fdv::TCPServer<MyTCPConnectionHandler>(80);
						m_serial->printf(FSTR("Ok\n\r"));
						break;
					}
				}
			}
		}		
	}
	
};



struct MainTask : fdv::Task
{
	void ICACHE_FLASH_ATTR exec()
	{
		//fdv::DisableStdOut(); 
		fdv::DisableWatchDog();
		
		fdv::HardwareSerial serial(115200, 128);
		
		Task1 task1(&serial);
		
		suspend();
	}
};


extern "C" void /*ICACHE_FLASH_ATTR*/ user_init(void) 
{
	new MainTask;	// never destroy!
}

