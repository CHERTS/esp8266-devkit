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
			{FSTR("/"),	         (PageHandler)&MyHTTPHandler::get_home},
			{FSTR("/confwifi"),  (PageHandler)&MyHTTPHandler::get_confwifi},
			{FSTR("/confnet"),   (PageHandler)&MyHTTPHandler::get_confnet},
			{FSTR("/confuart"),  (PageHandler)&MyHTTPHandler::get_confuart},
			{FSTR("/reboot"),    (PageHandler)&MyHTTPHandler::get_reboot},
			{FSTR("/restore"),   (PageHandler)&MyHTTPHandler::get_restore},
			{FSTR("*"),          (PageHandler)&MyHTTPHandler::get_all},
		};
		setRoutes(routes, sizeof(routes) / sizeof(Route));
	} 
	
	void MTD_FLASHMEM get_home()
	{
		//debug("get_home()\r\n");
		fdv::HTTPTemplateResponse response(this, FSTR("base.html"));
		response.addParam(FSTR("title"), FSTR("ESP8266 WebFramework"));
		response.addParam(FSTR("content"), FSTR("Please select a menu item on the left"));
		response.flush();
	}

	void MTD_FLASHMEM get_confwifi()
	{
		fdv::HTTPWifiConfigurationResponse response(this, FSTR("configwifi.html"));
		response.flush();
	}

	void MTD_FLASHMEM get_confnet()
	{
		fdv::HTTPNetworkConfigurationResponse response(this, FSTR("confignet.html"));
		response.flush();
	}

	void MTD_FLASHMEM get_confuart()
	{
		fdv::HTTPUARTConfigurationResponse response(this, FSTR("configuart.html"));
		response.flush();
	}
	
	void MTD_FLASHMEM get_reboot()
	{
		fdv::HTTPTemplateResponse response(this, FSTR("reboot.html"));
		fdv::reboot(3000);	// reboot in 3s
		response.flush();
	}

	void MTD_FLASHMEM get_restore()
	{
		if (getRequest().method == HTTPHandler::Get)
		{
			fdv::HTTPTemplateResponse response(this, FSTR("restore.html"));
			response.flush();
		}
		else
		{
			fdv::ConfigurationManager::restore();
			get_reboot();
		}
	}
	
	void MTD_FLASHMEM get_all()
	{
		//debug("get %s\r\n", fdv::APtr<char>(t_strdup(getRequest().requestedPage)).get());		 
		fdv::HTTPStaticFileResponse response(this, getRequest().requestedPage);
		response.flush();
	}			
};



struct MainTask : fdv::Task
{

	MainTask()
		: fdv::Task(false, 512)
	{		
	}

	
	void MTD_FLASHMEM exec()
	{
		//fdv::DisableStdOut(); 
		fdv::DisableWatchDog();		
		fdv::Serial* m_serial = fdv::HardwareSerial::getSerial(0);

		fdv::ConfigurationManager::apply();

		new fdv::TCPServer<MyHTTPHandler, 2, 512>(80);

		m_serial->printf(FSTR("\n\rESPWebFramework started.\n\r"));
		m_serial->printf(FSTR("Press <h> key to help.\n\r"));

		// just as serial emergency console!
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
						break;
					case 'r':
						//system_restart();
						fdv::reboot(500);
						break;
					case '0':
						fdv::FlashDictionary::eraseContent();
						m_serial->writeln("Ok");
						break;
					case '1':
						// Access point
						fdv::WiFi::setMode(fdv::WiFi::AccessPoint);
						fdv::WiFi::configureAccessPoint("MyESP", "myesp111", 9);
						fdv::IP::configureStatic(fdv::IP::AccessPointNetwork, "192.168.4.1", "255.255.255.0", "192.168.4.1");						
						m_serial->printf(FSTR("Reboot and enable DHCP server (2)\r\n"));
						m_serial->printf(FSTR("Ok\r\n"));
						break;
					case '2':
						// Enable DHCP server
						fdv::IP::configureStatic(fdv::IP::AccessPointNetwork, "192.168.4.1", "255.255.255.0", "192.168.4.1");						
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
				}
			}
		}		
	}
	
};




extern "C" void FUNC_FLASHMEM user_init(void) 
{
	new MainTask;	// never destroy!
}

