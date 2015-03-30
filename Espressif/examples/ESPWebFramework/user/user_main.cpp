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

using namespace fdv;


struct MyHTTPHandler : public HTTPHandler
{
	MyHTTPHandler()
	{
		static const Route routes[] =
		{
			{FSTR("/"),	         (PageHandler)&MyHTTPHandler::get_home},
			{FSTR("/confwifi"),  (PageHandler)&MyHTTPHandler::get_confwifi},
			{FSTR("/wifiscan"),  (PageHandler)&MyHTTPHandler::get_wifiscan},
			{FSTR("/confnet"),   (PageHandler)&MyHTTPHandler::get_confnet},
			{FSTR("/confserv"),  (PageHandler)&MyHTTPHandler::get_confserv},
			{FSTR("/confgpio"),  (PageHandler)&MyHTTPHandler::get_confgpio},
			{FSTR("/reboot"),    (PageHandler)&MyHTTPHandler::get_reboot},
			{FSTR("/restore"),   (PageHandler)&MyHTTPHandler::get_restore},
			{FSTR("*"),          (PageHandler)&MyHTTPHandler::get_all},
		};
		setRoutes(routes, sizeof(routes) / sizeof(Route));
	} 
	
	void MTD_FLASHMEM get_home()
	{
		HTTPTemplateResponse response(this, FSTR("home.html"));
		response.flush();
	}

	void MTD_FLASHMEM get_confwifi()
	{
		HTTPWifiConfigurationResponse response(this, FSTR("configwifi.html"));
		response.flush();
	}

	void MTD_FLASHMEM get_wifiscan()
	{
		HTTPWiFiScanResponse response(this, FSTR("wifiscan.html"));
		response.flush();
	}

	void MTD_FLASHMEM get_confnet()
	{
		HTTPNetworkConfigurationResponse response(this, FSTR("confignet.html"));
		response.flush();
	}

	void MTD_FLASHMEM get_confserv()
	{
		HTTPServicesConfigurationResponse response(this, FSTR("confserv.html"));
		response.flush();
	}

	void MTD_FLASHMEM get_confgpio()
	{
		HTTPGPIOConfigurationResponse response(this, FSTR("confgpio.html"));
		response.flush();
	}
	
	void MTD_FLASHMEM get_reboot()
	{
		HTTPTemplateResponse response(this, FSTR("reboot.html"));
		reboot(3000);	// reboot in 3s
		response.flush();
	}

	void MTD_FLASHMEM get_restore()
	{
		if (getRequest().method == HTTPHandler::Get)
		{
			HTTPTemplateResponse response(this, FSTR("restore.html"));
			response.flush();
		}
		else
		{
			ConfigurationManager::restore();
			get_reboot();
		}
	}
	
	void MTD_FLASHMEM get_all()
	{
		HTTPStaticFileResponse response(this, getRequest().requestedPage);
		response.flush();
	}			
};


extern "C" void FUNC_FLASHMEM user_init(void) 
{
	DisableWatchDog();				
	ConfigurationManager::applyAll< TCPServer<MyHTTPHandler, 2, 512> >();
}

