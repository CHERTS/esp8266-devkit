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

#include "fdv.h"


struct MyHTTPHandler : public fdv::HTTPHandler
{
	MyHTTPHandler()
	{
		static const Route routes[] =
		{
			{FSTR("/"),	     (PageHandler)&MyHTTPHandler::get_home},
			{FSTR("/test1"), (PageHandler)&MyHTTPHandler::get_test1},
			{FSTR("*"),      (PageHandler)&MyHTTPHandler::get_all},
		};
		setRoutes(routes, sizeof(routes) / sizeof(Route));
	} 
	
	void MTD_FLASHMEM get_home()
	{
		debug(FSTR("\r\nget_home()\r\n"));
		fdv::HTTPResponse(this, FSTR("200 OK"), FSTR("<html><head></head><body><h1>This is Home Page</h1></body></html>")).flush();
	}

	void MTD_FLASHMEM get_test1()
	{
		debug("get_test1()\r\n");
		fdv::HTTPResponse(this, FSTR("200 OK"), FSTR("<html><head></head><body><h1>This is test1</h1></body></html>"));
	}

	void MTD_FLASHMEM get_all()
	{
		debug("get_all()\r\n");
		fdv::HTTPResponse(this, FSTR("404 Not Found"), FSTR("Page not found!"));
	}			
};



struct Task1 : fdv::Task
{

	Task1(fdv::Serial* serial)
		: fdv::Task(false, 400), m_serial(serial)
	{		
	}

	fdv::Serial* m_serial;
	

	
	void MTD_FLASHMEM exec()
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
						m_serial->printf(FSTR("A.Free stack = %d bytes\r\n"), getFreeStack());
						m_serial->printf(FSTR("A.Free heap  = %d bytes\r\n"), getFreeHeap());
						m_serial->printf(FSTR("Tests:\r\n"));
						m_serial->printf(FSTR("h    = help\r\n"));
						m_serial->printf(FSTR("r    = reset\r\n"));	
						m_serial->printf(FSTR("0    = format flash filesystem\r\n"));
						m_serial->printf(FSTR("1    = start AccessPoint mode\r\n"));
						m_serial->printf(FSTR("2    = start DHCP server\r\n"));
						m_serial->printf(FSTR("3    = start Client mode static IP\r\n"));
						m_serial->printf(FSTR("4    = start Client mode dynamic IP\r\n"));
						m_serial->printf(FSTR("5    = open TCP server port 80\r\n"));
						break;
					case 'r':
						system_restart();
						break;
					case '0':
						fdv::FlashDictionary::eraseContent();
						m_serial->writeln("Ok");
						break;
					case '1':
						// Access point
						fdv::WiFi::setMode(fdv::WiFi::AccessPoint);
						fdv::WiFi::configureAccessPoint("MyESP", "myesp111", 9);
						fdv::IP::configureStatic(fdv::IP::AccessPointNetwork, "192.168.5.1", "255.255.255.0", "192.168.5.1");						
						m_serial->printf(FSTR("Reboot and enable DHCP server (2)\r\n"));
						m_serial->printf(FSTR("Ok\r\n"));
						break;
					case '2':
						// Enable DHCP server
						fdv::IP::configureStatic(fdv::IP::AccessPointNetwork, "192.168.5.1", "255.255.255.0", "192.168.5.1");						
						fdv::DHCPServer::configure("192.168.5.100", "192.168.5.110", 10);
						m_serial->printf(FSTR("Ok\r\n"));
						break;
					case '3':
						// Client mode with static IP
						fdv::WiFi::setMode(fdv::WiFi::Client);
						fdv::WiFi::configureClient("OSPITI", "P31415926");
						fdv::IP::configureStatic(fdv::IP::ClientNetwork, "192.168.1.199", "255.255.255.0", "192.168.1.1");						
						m_serial->printf(FSTR("Ok\r\n"));
						break;
					case '4':
						// Client mode with dynamic IP
						fdv::WiFi::setMode(fdv::WiFi::Client);
						fdv::WiFi::configureClient("OSPITI", "P31415926");
						fdv::IP::configureDHCP(fdv::IP::ClientNetwork);
						m_serial->printf(FSTR("Ok\r\n"));
						break;
					case '5':
					{
						new fdv::TCPServer<MyHTTPHandler, 2, 512>(80);
						m_serial->printf(FSTR("Ok\r\n"));
						break;
					}
					
				}
			}
		}		
	}
	
};



struct MainTask : fdv::Task
{
	MainTask()
		: fdv::Task(false)	
	{
	}
	
	void MTD_FLASHMEM exec()
	{
		//fdv::DisableStdOut(); 
		fdv::DisableWatchDog();
		
		fdv::HardwareSerial serial(115200, 128);
		
		Task1 task1(&serial);
		
		suspend();
	}
};


extern "C" void FUNC_FLASHMEM user_init(void) 
{
	new MainTask;	// never destroy!
}

