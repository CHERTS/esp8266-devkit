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

#ifndef _FDVCONFMANAGER_H_
#define _FDVCONFMANAGER_H_



#include "fdv.h"





namespace fdv
{


	//////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////
	// ConfigurationManager

	
	// const strings
	static char const STR_WiFiMode[] FLASHMEM = "WiFiMode";
	static char const STR_APSSID[] FLASHMEM   = "APSSID";
	static char const STR_APSECKEY[] FLASHMEM = "APSECKEY";
	static char const STR_APCH[] FLASHMEM     = "APCH";
	static char const STR_APSP[] FLASHMEM     = "APSP";
	static char const STR_APHSSID[] FLASHMEM  = "APHSSID";
	static char const STR_CLSSID[] FLASHMEM   = "CLSSID";
	static char const STR_CLSECKEY[] FLASHMEM = "CLSECKEY";
	static char const STR_CLSTATIC[] FLASHMEM = "CLSTATIC";
	static char const STR_CLIP[] FLASHMEM     = "CLIP";
	static char const STR_CLNETMSK[] FLASHMEM = "CLNETMSK";
	static char const STR_CLGTW[] FLASHMEM    = "CLGTW";
	static char const STR_APIP[] FLASHMEM     = "APIP";
	static char const STR_APNETMSK[] FLASHMEM = "APNETMSK";
	static char const STR_APGTW[] FLASHMEM    = "APGTW";
	static char const STR_DHCPDEN[] FLASHMEM  = "DHCPDEN";
	static char const STR_DHCPDIP1[] FLASHMEM = "DHCPDIP1";
	static char const STR_DHCPDIP2[] FLASHMEM = "DHCPDIP2";
	static char const STR_DHCPDMXL[] FLASHMEM = "DHCPDMXL";
	
	
	class ConfigurationManager
	{
		
	public:
		
		static void MTD_FLASHMEM apply()
		{
			// WiFi Mode
			WiFi::setMode(getWiFiMode());
			
			// Access point parameters
			char const* SSID;
			char const* securityKey;
			uint8_t channel;
			WiFi::SecurityProtocol securityProtocol;
			bool hiddenSSID;
			getAccessPointParams(&SSID, &securityKey, &channel, &securityProtocol, &hiddenSSID);
			WiFi::configureAccessPoint(SSID, securityKey, channel, securityProtocol, hiddenSSID);
			
			// Client parameters
			getClientParams(&SSID, &securityKey);
			WiFi::configureClient(SSID, securityKey);
			
			// Client IP
			bool staticIP;
			char const* IP;
			char const* netmask;
			char const* gateway;
			getClientIPParams(&staticIP, &IP, &netmask, &gateway);
			if (staticIP)
				IP::configureStatic(IP::ClientNetwork, IP, netmask, gateway);
			else
				IP::configureDHCP(IP::ClientNetwork);
			
			// Access Point IP
			getAccessPointIPParams(&IP, &netmask, &gateway);
			IP::configureStatic(IP::AccessPointNetwork, IP, netmask, gateway);
			
			// DCHP Server
			if (getWiFiMode() == WiFi::AccessPoint || getWiFiMode() == WiFi::ClientAndAccessPoint)
			{
				bool enabled;
				char const* startIP;
				char const* endIP;
				uint32_t maxLeases;
				getDHCPServerParams(&enabled, &startIP, &endIP, &maxLeases);
				if (enabled)
					DHCPServer::configure(startIP, endIP, maxLeases);
			}
		}
		
		
		static void MTD_FLASHMEM setWiFiMode(WiFi::Mode value)
		{
			FlashDictionary::setInt(STR_WiFiMode, (int32_t)value);
		}
		
		static WiFi::Mode MTD_FLASHMEM getWiFiMode()
		{
			return (WiFi::Mode)FlashDictionary::getInt(STR_WiFiMode, (int32_t)WiFi::AccessPoint);
		}
		
		
		static void MTD_FLASHMEM setAccessPointParams(char const* SSID, char const* securityKey, uint8_t channel, WiFi::SecurityProtocol securityProtocol, bool hiddenSSID)
		{
			FlashDictionary::setString(STR_APSSID, SSID);
			FlashDictionary::setString(STR_APSECKEY, securityKey);
			FlashDictionary::setInt(STR_APCH, channel);
			FlashDictionary::setInt(STR_APSP, (int32_t)securityProtocol);
			FlashDictionary::setBool(STR_APHSSID, hiddenSSID);
		}
		
		static void MTD_FLASHMEM getAccessPointParams(char const** SSID, char const** securityKey, uint8_t* channel, WiFi::SecurityProtocol* securityProtocol, bool* hiddenSSID)
		{
			*SSID             = FlashDictionary::getString(STR_APSSID, FSTR("MyESP"));
			*securityKey      = FlashDictionary::getString(STR_APSECKEY, FSTR("myesp111"));
			*channel          = FlashDictionary::getInt(STR_APCH, 9);
			*securityProtocol = (WiFi::SecurityProtocol)FlashDictionary::getInt(STR_APSP, (int32_t)WiFi::WPA2_PSK);
			*hiddenSSID       = FlashDictionary::getBool(STR_APHSSID, false);
		}
		
		
		static void MTD_FLASHMEM setClientParams(char const* SSID, char const* securityKey)
		{
			FlashDictionary::setString(STR_CLSSID, SSID);
			FlashDictionary::setString(STR_CLSECKEY, securityKey);
		}
		
		static void MTD_FLASHMEM getClientParams(char const** SSID, char const** securityKey)
		{
			*SSID        = FlashDictionary::getString(STR_CLSSID, FSTR(""));
			*securityKey = FlashDictionary::getString(STR_CLSECKEY, FSTR(""));
		}
		
		
		// IP, netmask, gateway valid only if staticIP = true
		static void MTD_FLASHMEM setClientIPParams(bool staticIP, char const* IP, char const* netmask, char const* gateway)
		{
			FlashDictionary::setBool(STR_CLSTATIC, staticIP);
			FlashDictionary::setString(STR_CLIP, IP);
			FlashDictionary::setString(STR_CLNETMSK, netmask);
			FlashDictionary::setString(STR_CLGTW, gateway);			
		}
		
		// IP, netmask, gateway valid only if staticIP = true
		static void MTD_FLASHMEM getClientIPParams(bool* staticIP, char const** IP, char const** netmask, char const** gateway)
		{
			*staticIP = FlashDictionary::getBool(STR_CLSTATIC, false);
			*IP       = FlashDictionary::getString(STR_CLIP, FSTR(""));
			*netmask  = FlashDictionary::getString(STR_CLNETMSK, FSTR(""));
			*gateway  = FlashDictionary::getString(STR_CLGTW, FSTR(""));
		}
		
		
		static void MTD_FLASHMEM setAccessPointIPParams(char const* IP, char const* netmask, char const* gateway)
		{
			FlashDictionary::setString(STR_APIP, IP);
			FlashDictionary::setString(STR_APNETMSK, netmask);
			FlashDictionary::setString(STR_APGTW, gateway);			
		}
		
		static void MTD_FLASHMEM getAccessPointIPParams(char const** IP, char const** netmask, char const** gateway)
		{
			*IP       = FlashDictionary::getString(STR_APIP, FSTR("192.168.4.1"));
			*netmask  = FlashDictionary::getString(STR_APNETMSK, FSTR("255.255.255.0"));
			*gateway  = FlashDictionary::getString(STR_APGTW, FSTR(""));
		}
		
		
		static void MTD_FLASHMEM setDHCPServerParams(bool enabled, char const* startIP, char const* endIP, uint32_t maxLeases)
		{
			FlashDictionary::setBool(STR_DHCPDEN, enabled);
			FlashDictionary::setString(STR_DHCPDIP1, startIP);
			FlashDictionary::setString(STR_DHCPDIP2, endIP);
			FlashDictionary::setInt(STR_DHCPDMXL, maxLeases);
		}
		
		static void MTD_FLASHMEM getDHCPServerParams(bool* enabled, char const** startIP, char const** endIP, uint32_t* maxLeases)
		{
			*enabled   = FlashDictionary::getBool(STR_DHCPDEN, true);
			*startIP   = FlashDictionary::getString(STR_DHCPDIP1, FSTR("192.168.4.100"));
			*endIP     = FlashDictionary::getString(STR_DHCPDIP2, FSTR("192.168.5.110"));
			*maxLeases = FlashDictionary::getInt(STR_DHCPDMXL, 10);
		}
		
	};



	//////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////
	// Configuration helper web pages
		
	struct HTTPWifiConfigurationResponse : public HTTPTemplateResponse
	{
		HTTPWifiConfigurationResponse(HTTPHandler* httpHandler, char const* filename)
			: HTTPTemplateResponse(httpHandler, filename)
		{
		}
		
		virtual void MTD_FLASHMEM flush()
		{
			if (getRequest().method == HTTPHandler::Post)
			{
				HTTPHandler::Fields::Item* clientmode = getRequest().form["clientmode"];
				HTTPHandler::Fields::Item* apmode = getRequest().form["apmode"];
				if (clientmode && apmode)
					debug("enable clientmode and acess point mode\r\n");
				else if (clientmode)
					debug("enable clientmode\r\n");
				else if (apmode)
					debug("enable access point mode\r\n");
			}
			
			WiFi::Mode mode = WiFi::getMode();
			
			if (mode == WiFi::Client || mode == WiFi::ClientAndAccessPoint)
				addParam(FSTR("clientmode"), FSTR("checked"));
			
			if (mode == WiFi::AccessPoint || mode == WiFi::ClientAndAccessPoint)
				addParam(FSTR("apmode"), FSTR("checked"));
			
			HTTPTemplateResponse::flush();
		}
	};
	
	
}

#endif

