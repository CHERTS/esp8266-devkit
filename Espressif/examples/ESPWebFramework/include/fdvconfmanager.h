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

	
	// ConfigurationManager flash parameters
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
	static char const STR_WEBPORT[] FLASHMEM  = "WEBPORT";
	static char const STR_BAUD[] FLASHMEM     = "BAUD";
	static char const STR_SYSOUT[] FLASHMEM   = "SYSOUT";
	static char const STR_UARTSRV[] FLASHMEM  = "UARTSRV";
	

	
	class ConfigurationManager
	{		
		
		template <typename HTTPCustomServer_T>
		static void MTD_FLASHMEM applyDelayed()
		{
			if (getWiFiMode() == WiFi::AccessPoint || getWiFiMode() == WiFi::ClientAndAccessPoint)
			{
				// Access Point IP
				char const* IP;
				char const* netmask;
				char const* gateway;
				getAccessPointIPParams(&IP, &netmask, &gateway);
				IP::configureStatic(WiFi::AccessPointNetwork, IP, netmask, gateway);
			}
			
			if (getWiFiMode() == WiFi::Client || getWiFiMode() == WiFi::ClientAndAccessPoint)
			{			
				// Client IP
				bool staticIP;
				char const* IP;
				char const* netmask;
				char const* gateway;
				getClientIPParams(&staticIP, &IP, &netmask, &gateway);
				if (staticIP)
					IP::configureStatic(WiFi::ClientNetwork, IP, netmask, gateway);
				else
					IP::configureDHCP(WiFi::ClientNetwork);
			}
			
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
			
			// Web Server
			uint16_t webPort;
			getWebServerParams(&webPort);
			new HTTPCustomServer_T(webPort);
		}
	
	public:
		
		template <typename HTTPCustomServer_T>
		static void MTD_FLASHMEM apply()
		{
			// UART and serial services
			uint32_t baudRate;
			bool enableSystemOutput;
			SerialService serialService;
			getUARTParams(&baudRate, &enableSystemOutput, &serialService);
			HardwareSerial::getSerial(0)->reconfig(baudRate);
			if (!enableSystemOutput)
				DisableStdOut();
		    switch (serialService)
			{
				case SerialService_Console:
					new SerialConsole;
					break;
				case SerialService_BinaryProtocol:
					// todo
					break;
			}

			// WiFi Mode
			WiFi::setMode(getWiFiMode());
										
			if (getWiFiMode() == WiFi::AccessPoint || getWiFiMode() == WiFi::ClientAndAccessPoint)
			{
				// Access point parameters
				char const* SSID;
				char const* securityKey;
				uint8_t channel;
				WiFi::SecurityProtocol securityProtocol;
				bool hiddenSSID;
				getAccessPointParams(&SSID, &securityKey, &channel, &securityProtocol, &hiddenSSID);
				WiFi::configureAccessPoint(SSID, securityKey, channel, securityProtocol, hiddenSSID);
			}
			
			if (getWiFiMode() == WiFi::Client || getWiFiMode() == WiFi::ClientAndAccessPoint)
			{			
				// Client parameters
				char const* SSID;
				char const* securityKey;
				getClientParams(&SSID, &securityKey);
				WiFi::configureClient(SSID, securityKey);
			}

			asyncExec< applyDelayed<HTTPCustomServer_T> >(512);
		}

	
		static void MTD_FLASHMEM restore()
		{
			FlashDictionary::eraseContent();
			system_restore();
		}
		
		
		//// WiFi settings
		
		static void MTD_FLASHMEM setWiFiMode(WiFi::Mode value)
		{
			FlashDictionary::setInt(STR_WiFiMode, (int32_t)value);
		}
		
		static WiFi::Mode MTD_FLASHMEM getWiFiMode()
		{
			return (WiFi::Mode)FlashDictionary::getInt(STR_WiFiMode, (int32_t)WiFi::AccessPoint);
		}
		
		
		//// Access Point mode parameters
		
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
			static char defaultSSID[10];
			uint8_t mac[16];
			WiFi::getMACAddress(WiFi::AccessPointNetwork, mac);			
			sprintf(defaultSSID, FSTR("ESP%02X%02X%02X%02X%02X%02X"), mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
			*SSID             = FlashDictionary::getString(STR_APSSID, defaultSSID);
			*securityKey      = FlashDictionary::getString(STR_APSECKEY, FSTR(""));
			*channel          = FlashDictionary::getInt(STR_APCH, 9);
			*securityProtocol = (WiFi::SecurityProtocol)FlashDictionary::getInt(STR_APSP, (int32_t)WiFi::Open);
			*hiddenSSID       = FlashDictionary::getBool(STR_APHSSID, false);
		}
		
		
		//// Client mode parameters
		
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
		
		
		//// Client mode IP parameters
		
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
		
		
		//// Access point IP parameters
		
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
		
		
		//// DHCP server parameters
		
		static void MTD_FLASHMEM setDHCPServerParams(bool enabled, char const* startIP = NULL, char const* endIP = NULL, uint32_t maxLeases = 0)
		{
			FlashDictionary::setBool(STR_DHCPDEN, enabled);
			if (startIP && endIP && maxLeases)
			{
				FlashDictionary::setString(STR_DHCPDIP1, startIP);
				FlashDictionary::setString(STR_DHCPDIP2, endIP);
				FlashDictionary::setInt(STR_DHCPDMXL, maxLeases);
			}
		}
		
		static void MTD_FLASHMEM getDHCPServerParams(bool* enabled, char const** startIP, char const** endIP, uint32_t* maxLeases)
		{
			*enabled   = FlashDictionary::getBool(STR_DHCPDEN, true);
			*startIP   = FlashDictionary::getString(STR_DHCPDIP1, FSTR("192.168.4.100"));
			*endIP     = FlashDictionary::getString(STR_DHCPDIP2, FSTR("192.168.4.110"));
			*maxLeases = FlashDictionary::getInt(STR_DHCPDMXL, 10);
		}
		
		
		//// Web Server parameters
		
		static void MTD_FLASHMEM setWebServerParams(uint16_t port)
		{
			FlashDictionary::setInt(STR_WEBPORT, port);
		}
		
		static void MTD_FLASHMEM getWebServerParams(uint16_t* port)
		{
			*port = FlashDictionary::getInt(STR_WEBPORT, 80);
		}
		
		
		//// UART parameters
		
		static void MTD_FLASHMEM setUARTParams(uint32_t baudRate, bool enableSystemOutput, SerialService serialService)
		{
			FlashDictionary::setInt(STR_BAUD, baudRate);
			FlashDictionary::setBool(STR_SYSOUT, enableSystemOutput);
			FlashDictionary::setInt(STR_UARTSRV, (int32_t)serialService);
		}
		
		static void MTD_FLASHMEM getUARTParams(uint32_t* baudRate, bool* enableSystemOutput, SerialService* serialService)
		{
			*baudRate           = FlashDictionary::getInt(STR_BAUD, 115200);
			*enableSystemOutput = FlashDictionary::getBool(STR_SYSOUT, false);
			*serialService      = (SerialService)FlashDictionary::getInt(STR_UARTSRV, (int32_t)SerialService_Console);
		}
	};



	//////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////
	// Configuration helper web pages
	//////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////


	//////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////
	// HTTPWifiConfigurationResponse
	
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
				// set WiFi mode
				char const* clientmode = getRequest().form[FSTR("clientmode")];
				char const* apmode     = getRequest().form[FSTR("apmode")];
				if (clientmode && apmode)
					ConfigurationManager::setWiFiMode(WiFi::ClientAndAccessPoint);
				else if (clientmode)
					ConfigurationManager::setWiFiMode(WiFi::Client);
				else if (apmode)
					ConfigurationManager::setWiFiMode(WiFi::AccessPoint);
				
				// set client mode parameters
				ConfigurationManager::setClientParams(getRequest().form[FSTR("CLSSID")],
				                                      getRequest().form[FSTR("CLPSW")]);
				
				// set access point parameters
				char const* APCH    = getRequest().form[FSTR("APCH")];
				char const* APSEC   = getRequest().form[FSTR("APSEC")];
				if (APCH && APSEC)
					ConfigurationManager::setAccessPointParams(getRequest().form[FSTR("APSSID")], 
															   getRequest().form[FSTR("APPSW")], 
															   strtol(APCH, NULL, 10),
															   (WiFi::SecurityProtocol)strtol(APSEC, NULL, 10),
															   getRequest().form[FSTR("APHSSID")] != NULL);
			}
			
			// get WiFi mode
			WiFi::Mode mode = ConfigurationManager::getWiFiMode();			
			if (mode == WiFi::Client || mode == WiFi::ClientAndAccessPoint)
				addParamStr(FSTR("clientmode"), FSTR("checked"));			
			if (mode == WiFi::AccessPoint || mode == WiFi::ClientAndAccessPoint)
				addParamStr(FSTR("apmode"), FSTR("checked"));
			
			// get client mode parameters
			char const* SSID = getRequest().query[FSTR("AP")]; // get SSID from last scan?
			char const* securityKey;
			if (!SSID)
			{				
				// get from configuration among the password
				ConfigurationManager::getClientParams(&SSID, &securityKey);
				addParamStr(FSTR("CLPSW"), securityKey);
			}
			addParamStr(FSTR("CLSSID"), SSID);
			
			// get access point parameters
			uint8_t channel;
			WiFi::SecurityProtocol securityProtocol;
			bool hiddenSSID;
			ConfigurationManager::getAccessPointParams(&SSID, &securityKey, &channel, &securityProtocol, &hiddenSSID);
			addParamStr(FSTR("APSSID"), SSID);
			addParamStr(FSTR("APPSW"), securityKey);
			APtr<char> APCHStr(f_printf(FSTR("APCH%d"), channel));
			addParamStr(APCHStr.get(), FSTR("selected"));
			APtr<char> APSECStr(f_printf(FSTR("APSEC%d"), (int32_t)securityProtocol));
			addParamStr(APSECStr.get(), FSTR("selected"));
			if (hiddenSSID)
				addParamStr(FSTR("APHSSID"), FSTR("checked"));
			
			HTTPTemplateResponse::flush();
		}
	};


	//////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////
	// HTTPNetworkConfigurationResponse

	struct HTTPNetworkConfigurationResponse : public HTTPTemplateResponse
	{
		HTTPNetworkConfigurationResponse(HTTPHandler* httpHandler, char const* filename)
			: HTTPTemplateResponse(httpHandler, filename)
		{
		}
		
		virtual void MTD_FLASHMEM flush()
		{
			if (getRequest().method == HTTPHandler::Post)
			{
				// set client mode IP configuration
				ConfigurationManager::setClientIPParams(getRequest().form[FSTR("staticIP")] != NULL,
													    getRequest().form[FSTR("CLIP")],
														getRequest().form[FSTR("CLMSK")],
														getRequest().form[FSTR("CLGTW")]);
														
				// set access point IP configuration
				ConfigurationManager::setAccessPointIPParams(getRequest().form[FSTR("APIP")],
														     getRequest().form[FSTR("APMSK")],
														     getRequest().form[FSTR("APGTW")]);

				// set DHCP server configuration
				if (getRequest().form[FSTR("DHCPD")] != NULL)
					ConfigurationManager::setDHCPServerParams(true,
															  getRequest().form[FSTR("startIP")],
															  getRequest().form[FSTR("endIP")],
															  strtol(getRequest().form[FSTR("maxLeases")], NULL, 10));
				else
					ConfigurationManager::setDHCPServerParams(false);
			}
			
			WiFi::Mode mode = ConfigurationManager::getWiFiMode();
			
			// get client mode IP configuration
			bool staticIP;
			char const* IP;
			char const* netmask;
			char const* gateway;
			ConfigurationManager::getClientIPParams(&staticIP, &IP, &netmask, &gateway);
			addParamStr(FSTR("DISP_CLIPCONF"), mode == WiFi::Client || mode == WiFi::ClientAndAccessPoint? FSTR("") : FSTR("disabled"));
			if (staticIP)
				addParamStr(FSTR("staticIP"), FSTR("checked"));
			addParamStr(FSTR("CLIP"), IP);
			addParamStr(FSTR("CLMSK"), netmask);
			addParamStr(FSTR("CLGTW"), gateway);
			
			// get access point IP configuration
			ConfigurationManager::getAccessPointIPParams(&IP, &netmask, &gateway);
			addParamStr(FSTR("DISP_APIPCONF"), mode == WiFi::AccessPoint || mode == WiFi::ClientAndAccessPoint? FSTR("") : FSTR("disabled"));
			addParamStr(FSTR("APIP"), IP);
			addParamStr(FSTR("APMSK"), netmask);
			addParamStr(FSTR("APGTW"), gateway);
			
			// get DHCP server configuration
			bool DHCPDEnabled;
			char const* startIP;
			char const* endIP;
			uint32_t maxLeases;
			ConfigurationManager::getDHCPServerParams(&DHCPDEnabled, &startIP, &endIP, &maxLeases);
			if (DHCPDEnabled)
				addParamStr(FSTR("DHCPD"), FSTR("checked"));
			addParamStr(FSTR("startIP"), startIP);
			addParamStr(FSTR("endIP"), endIP);
			addParamInt(FSTR("maxLeases"), maxLeases);
			
			HTTPTemplateResponse::flush();
		}
		
	};


	//////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////
	// HTTPServicesConfigurationResponse

	struct HTTPServicesConfigurationResponse : public HTTPTemplateResponse
	{
		HTTPServicesConfigurationResponse(HTTPHandler* httpHandler, char const* filename)
			: HTTPTemplateResponse(httpHandler, filename)
		{
		}
		
		virtual void MTD_FLASHMEM flush()
		{
			if (getRequest().method == HTTPHandler::Post)
			{
				// set Web server configuration
				char const* httpport = getRequest().form[FSTR("httpport")];
				if (httpport)
					ConfigurationManager::setWebServerParams(strtol(httpport, NULL, 10));
				
				// set UART configuration
				char const* baud = getRequest().form[FSTR("baud")];
				char const* serv = getRequest().form[FSTR("serv")];
				if (baud && serv)
					ConfigurationManager::setUARTParams(strtol(baud, NULL, 10),
														getRequest().form[FSTR("debugout")] != NULL,
														(SerialService)strtol(serv, NULL, 10));
			}
			
			// get Web server configuration
			uint16_t webPort;
			ConfigurationManager::getWebServerParams(&webPort);
			addParamInt(FSTR("httpport"), webPort);
			
			// get UART configuration
			uint32_t baudRate;
			bool enableSystemOutput;
			SerialService serialService;
			ConfigurationManager::getUARTParams(&baudRate, &enableSystemOutput, &serialService);
			addParamInt(FSTR("baud"), baudRate);
			if (enableSystemOutput)
				addParamStr(FSTR("debugout"), FSTR("checked"));
			APtr<char> serialServiceStr(f_printf(FSTR("serv%d"), (int32_t)serialService));
			addParamStr(serialServiceStr.get(), FSTR("checked"));						
			
			HTTPTemplateResponse::flush();
		}
		
	};
	

	//////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////
	// HTTPWiFiScanResponse

	struct HTTPWiFiScanResponse : public HTTPTemplateResponse
	{
		HTTPWiFiScanResponse(HTTPHandler* httpHandler, char const* filename)
			: HTTPTemplateResponse(httpHandler, filename)
		{
		}
		
		virtual void MTD_FLASHMEM flush()
		{
			uint32_t count = 0;
			WiFi::APInfo* infos = WiFi::getAPList(&count, true);

			char paramKey[count][8];
			APtr<char> paramValue[count];
			for (uint32_t i = 0; i != count; ++i)
			{
				sprintf(paramKey[i], FSTR("%dAPS"), i);
				paramValue[i].reset(f_printf(FSTR("<tr> <td><a href='confwifi?AP=%s'>%s</a></td> <td>%02X:%02X:%02X:%02X:%02X:%02X</td> <td>%d</td> <td>%d</td> <td>%s</td> </tr>"), 
				                             infos[i].SSID,
											 infos[i].SSID,
											 infos[i].BSSID[0], infos[i].BSSID[1], infos[i].BSSID[2], infos[i].BSSID[3], infos[i].BSSID[4], infos[i].BSSID[5],
											 infos[i].Channel,
											 infos[i].RSSI,
											 WiFi::convSecurityProtocolToString(infos[i].AuthMode)));											 
				addParamStr(paramKey[i], paramValue[i].get());
			}

			HTTPTemplateResponse::flush();
		}
		
	};

	
}

#endif

