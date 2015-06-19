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
	
	class ConfigurationManager
	{		
		
		template <typename HTTPCustomServer_T>
		static void MTD_FLASHMEM applyDelayed()
		{
			applyAccessPointIP();
			applyClientIP();
			applyDHCPServer();
            applyDNS();
			applyWebServer<HTTPCustomServer_T>();
			applyGPIO();
            applyDateTime();
		}
		
	
	public:
		
		template <typename HTTPCustomServer_T>
		static void MTD_FLASHMEM applyAll()
		{
			applyUARTServices();
			applyWiFi();
			asyncExec< applyDelayed<HTTPCustomServer_T> >(512);
		}
		
		
		// can be re-applied
		static void applyUARTServices();		
		
		// can be re-applied
		static void applyWiFi();

		// can be re-applied
		// doesn't support GPIO16
		static void applyGPIO();
		
		// can be re-applied
		static void applyAccessPointIP();
		
		// can be re-applied
		static void applyClientIP();
		
		// can be re-applied
		static void applyDHCPServer();

        // can be re-applied
        static void applyDNS();
        
		// cannot be re-applied
		template <typename HTTPCustomServer_T>
		static void MTD_FLASHMEM applyWebServer()
		{
			// Web Server
			uint16_t webPort;
			getWebServerParams(&webPort);
			new HTTPCustomServer_T(webPort);
		}

        // can be re-applied
        static void applyDateTime();
		
		static void MTD_FLASHMEM restore();
		
		
		//// WiFi settings
		
		static void setWiFiMode(WiFi::Mode value);
		
		static WiFi::Mode getWiFiMode();

		
		//// Access Point mode parameters
		
		static void setAccessPointParams(char const* SSID, char const* securityKey, uint8_t channel, WiFi::SecurityProtocol securityProtocol, bool hiddenSSID);
		
		static void getAccessPointParams(char const** SSID, char const** securityKey, uint8_t* channel, WiFi::SecurityProtocol* securityProtocol, bool* hiddenSSID);
		
		
		//// Client mode parameters
		
		static void setClientParams(char const* SSID, char const* securityKey);
		
		static void getClientParams(char const** SSID, char const** securityKey);
		
		
		//// Client mode IP parameters
		
		// IP, netmask, gateway valid only if staticIP = true
		static void setClientIPParams(bool staticIP, char const* IP, char const* netmask, char const* gateway);
		
		// IP, netmask, gateway valid only if staticIP = true
		static void getClientIPParams(bool* staticIP, char const** IP, char const** netmask, char const** gateway);
		
		
		//// Access point IP parameters
		
		static void setAccessPointIPParams(char const* IP, char const* netmask, char const* gateway);
		
		static void getAccessPointIPParams(char const** IP, char const** netmask, char const** gateway);
		
		
		//// DHCP server parameters
		
		static void setDHCPServerParams(bool enabled, char const* startIP = NULL, char const* endIP = NULL, uint32_t maxLeases = 0);
		
		static void getDHCPServerParams(bool* enabled, char const** startIP, char const** endIP, uint32_t* maxLeases);
		
		
        //// DNS parameters
        
        static void setDNSParams(IPAddress DNS1, IPAddress DNS2);
        
        static void getDNSParams(IPAddress* DNS1, IPAddress* DNS2);
        
        
		//// Web Server parameters
		
		static void setWebServerParams(uint16_t port);
		
		static void getWebServerParams(uint16_t* port);
		
		
		//// UART parameters
		
		static void setUARTParams(uint32_t baudRate, bool enableSystemOutput, SerialService serialService);
		
		static void getUARTParams(uint32_t* baudRate, bool* enableSystemOutput, SerialService* serialService);
		
#if (FDV_INCLUDE_SERIALCONSOLE == 1)
		static SerialConsole* getSerialConsole();
#endif
		
#if (FDV_INCLUDE_SERIALBINARY == 1)
		static SerialBinary* getSerialBinary();
#endif
		
		
		//// GPIO parameters
		
		struct GPIOInfo
		{
			uint32_t configured: 1, isOutput: 1, pullUp: 1, value: 1;
		};

		static void setGPIOParams(uint32_t gpioNum, bool configured, bool isOutput, bool pullUp, bool value);
		
		static void getGPIOParams(uint32_t gpioNum, bool* configured, bool* isOutput, bool* pullUp, bool* value);
        
        
        //// Date-time parameters
        
        static void setDateTimeParams(int8_t timezoneHours, uint8_t timezoneMinutes, char const* defaultNTPServer);
        
        static void getDateTimeParams(int8_t* timezoneHours, uint8_t* timezoneMinutes, char const** defaultNTPServer);
        
        static DateTime getBootDateTime();        
		
		
	private:
#if (FDV_INCLUDE_SERIALCONSOLE == 1)
		static SerialConsole* s_serialConsole;
#endif
#if (FDV_INCLUDE_SERIALBINARY == 1)
		static SerialBinary*  s_serialBinary;
#endif
        static DateTime       s_bootTime;
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
				char const* clientmode = getRequest().form[STR_clientmode];
				char const* apmode     = getRequest().form[STR_apmode];
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
				addParamStr(STR_clientmode, STR_checked);
			if (mode == WiFi::AccessPoint || mode == WiFi::ClientAndAccessPoint)
				addParamStr(STR_apmode, STR_checked);
			
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
			addParamStr(APCHStr.get(), STR_selected);
			APtr<char> APSECStr(f_printf(FSTR("APSEC%d"), (int32_t)securityProtocol));
			addParamStr(APSECStr.get(), STR_selected);
			if (hiddenSSID)
				addParamStr(FSTR("APHSSID"), STR_checked);
			
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
															  strtol(getRequest().form[STR_maxLeases], NULL, 10));
				else
					ConfigurationManager::setDHCPServerParams(false);
                
                // set DNS
                ConfigurationManager::setDNSParams(IPAddress(getRequest().form[STR_DNS1]), IPAddress(getRequest().form[STR_DNS2]));
			}
			
			WiFi::Mode mode = ConfigurationManager::getWiFiMode();
			
			// get client mode IP configuration
			bool staticIP;
			char const* IP;
			char const* netmask;
			char const* gateway;
			ConfigurationManager::getClientIPParams(&staticIP, &IP, &netmask, &gateway);
			addParamStr(FSTR("DISP_CLIPCONF"), mode == WiFi::Client || mode == WiFi::ClientAndAccessPoint? STR_ : STR_disabled);
			if (staticIP)
				addParamStr(FSTR("staticIP"), STR_checked);
			addParamStr(FSTR("CLIP"), IP);
			addParamStr(FSTR("CLMSK"), netmask);
			addParamStr(FSTR("CLGTW"), gateway);
			
			// get access point IP configuration
			ConfigurationManager::getAccessPointIPParams(&IP, &netmask, &gateway);
			addParamStr(FSTR("DISP_APIPCONF"), mode == WiFi::AccessPoint || mode == WiFi::ClientAndAccessPoint? STR_ : STR_disabled);
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
				addParamStr(FSTR("DHCPD"), STR_checked);
			addParamStr(FSTR("startIP"), startIP);
			addParamStr(FSTR("endIP"), endIP);
			addParamInt(STR_maxLeases, maxLeases);
			addParamStr(FSTR("DISP_DHCPD"), mode == WiFi::AccessPoint || mode == WiFi::ClientAndAccessPoint? STR_ : STR_disabled);
			
            // get DNS server configuration
            IPAddress DNS1, DNS2;
            ConfigurationManager::getDNSParams(&DNS1, &DNS2);
            IPAddress::IPAddressStr DNS1str = DNS1.get_str();
            IPAddress::IPAddressStr DNS2str = DNS2.get_str();
            addParamStr(STR_DNS1, DNS1str);
            addParamStr(STR_DNS2, DNS2str);
            
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
				char const* httpport = getRequest().form[STR_httpport];
				if (httpport)
					ConfigurationManager::setWebServerParams(strtol(httpport, NULL, 10));
				
				// set UART configuration
				char const* baud = getRequest().form[STR_baud];
				char const* serv = getRequest().form[FSTR("serv")];
				if (baud && serv)
				{
					ConfigurationManager::setUARTParams(strtol(baud, NULL, 10),
														getRequest().form[STR_debugout] != NULL,
														(SerialService)strtol(serv, NULL, 10));
					ConfigurationManager::applyUARTServices();
				}
			}
			
			// get Web server configuration
			uint16_t webPort;
			ConfigurationManager::getWebServerParams(&webPort);
			addParamInt(STR_httpport, webPort);
			
			// get UART configuration
			uint32_t baudRate;
			bool enableSystemOutput;
			SerialService serialService;
			ConfigurationManager::getUARTParams(&baudRate, &enableSystemOutput, &serialService);
			addParamInt(STR_baud, baudRate);
			if (enableSystemOutput)
				addParamStr(STR_debugout, STR_checked);
			APtr<char> serialServiceStr(f_printf(FSTR("serv%d"), (int32_t)serialService));
			addParamStr(serialServiceStr.get(), STR_checked);						
			
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

			LinkedCharChunks linkedChunks;
			for (uint32_t i = 0; i != count; ++i)
			{
				linkedChunks.addChunk(f_printf(FSTR("<tr> <td><a href='confwifi?AP=%s'>%s</a></td> <td>%02X:%02X:%02X:%02X:%02X:%02X</td> <td>%d</td> <td>%d</td> <td>%s</td> </tr>"), 
				                               infos[i].SSID,
											   infos[i].SSID,
											   infos[i].BSSID[0], infos[i].BSSID[1], infos[i].BSSID[2], infos[i].BSSID[3], infos[i].BSSID[4], infos[i].BSSID[5],
											   infos[i].Channel,
											   infos[i].RSSI,
											   WiFi::convSecurityProtocolToString(infos[i].AuthMode)),
									  true);
			}
			addParamCharChunks(FSTR("APS"), &linkedChunks);
			

			HTTPTemplateResponse::flush();
		}
		
	};

	
	//////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////
	// HTTPGPIOConfigurationResponse

	struct HTTPGPIOConfigurationResponse : public HTTPTemplateResponse
	{
		HTTPGPIOConfigurationResponse(HTTPHandler* httpHandler, char const* filename)
			: HTTPTemplateResponse(httpHandler, filename)
		{
		}
		
		virtual void MTD_FLASHMEM flush()
		{
			if (getRequest().method == HTTPHandler::Post)
			{
				char const* gpio = getRequest().form[FSTR("GPIO")];
				if (getRequest().form[FSTR("configured")])
				{					
					char const* mode     = getRequest().form[FSTR("mode")];
					char const* pullUp   = getRequest().form[FSTR("pullup")];
					ConfigurationManager::setGPIOParams(strtol(gpio, NULL, 10), true, f_strcmp(mode, FSTR("out")) == 0, pullUp != NULL, false);
					ConfigurationManager::applyGPIO();
				}
				else if (gpio)
				{
					ConfigurationManager::setGPIOParams(strtol(gpio, NULL, 10), false, false, false, false);
				}
			}
			
			bool configured, isOutput, pullUp, value;
			
			char const* gpio = getRequest().query[FSTR("gpio")];
			char const* val  = getRequest().query[FSTR("val")];
			if (gpio && val)
			{
				uint8_t gpion = strtol(gpio, NULL, 10);
				ConfigurationManager::getGPIOParams(gpion, &configured, &isOutput, &pullUp, &value);
				value = *val - '0';
				ConfigurationManager::setGPIOParams(gpion, configured, isOutput, pullUp, value);
				GPIO(gpion).write(value);
			}
				
			LinkedCharChunks linkedChunks;
			for (uint32_t i = 0; i != 16; ++i)
			{
				if (i != 1 && i != 3 && (i < 6 || i > 11))
				{
					bool configured, isOutput, pullUp, value;
					ConfigurationManager::getGPIOParams(i, &configured, &isOutput, &pullUp, &value);
					
					linkedChunks.addChunk(f_printf(FSTR("<tr> <td>%d</td> <td><form method='POST'>"), i), true);
					linkedChunks.addChunk(f_printf(FSTR("Enabled <input type='checkbox' name='configured' value='1' onclick=\"document.getElementById('GPIO%d').disabled=!this.checked\" %s>"), i, configured? STR_checked:STR_), true);
					linkedChunks.addChunk(f_printf(FSTR("<fieldset class='inline' id='GPIO%d' %s>"), i, configured? STR_:STR_disabled), true);
					linkedChunks.addChunk(f_printf(FSTR("<select name='mode'><option value='in' %s>IN</option><option value='out' %s>OUT</option></select>"), 
					                               isOutput? STR_:STR_selected, 
												   isOutput? STR_selected:STR_), 
										  true);
					linkedChunks.addChunk(f_printf(FSTR("     PullUp <input type='checkbox' name='pullup' value='1' %s> </fieldset>"), pullUp? STR_checked:STR_), true);
					linkedChunks.addChunk(f_printf(FSTR("<input type='hidden' name='GPIO' value='%d'>"), i), true);
					linkedChunks.addChunk(FSTR("<input type='submit' value='Save'></form></td>"));
					if (configured)
					{
						if (isOutput)
						{
							linkedChunks.addChunk(f_printf(FSTR("<td><a href='confgpio?gpio=%d&val=%d' class='link_button2'>%s</a></td> </tr>"), i, !value, value? STR_HI:STR_LO), true);
						}
						else
						{
							linkedChunks.addChunk(f_printf(FSTR("<td>%s</td> </tr>"), GPIO(i).read()? STR_HI:STR_LO), true);
						}
					}
					else
					{
						linkedChunks.addChunk(FSTR("<td></td></tr>"));
					}
				}
			}
			addParamCharChunks(FSTR("GPIOS"), &linkedChunks);
				
			HTTPTemplateResponse::flush();
		}
		
	};



	//////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////
	// HTTPTimeConfigurationResponse

	struct HTTPTimeConfigurationResponse : public HTTPTemplateResponse
	{
		HTTPTimeConfigurationResponse(HTTPHandler* httpHandler, char const* filename)
			: HTTPTemplateResponse(httpHandler, filename)
		{
		}
		
		virtual void MTD_FLASHMEM flush()
		{
			if (getRequest().method == HTTPHandler::Post)
			{
                // set current date and time
                char const* dateStr = getRequest().form[STR_date];
                char const* timeStr = getRequest().form[STR_time];
                if (dateStr && timeStr)
                {
                    DateTime dt;
                    dt.decode(dateStr, FSTR("%d/%m/%Y"));
                    dt.decode(timeStr, FSTR("%H:%M:%S"));
                    DateTime::setCurrentDateTime(dt);
                }
                
                // set timezone and NTP server
                char const* tzh = getRequest().form[STR_tzh];
                char const* tzm = getRequest().form[STR_tzm];
                char const* ntpsrv = getRequest().form[STR_ntpsrv];
                if (tzh && tzm)
                {
                    ConfigurationManager::setDateTimeParams(strtol(tzh, NULL, 10),
                                                            strtol(tzm, NULL, 10),
                                                            ntpsrv? ntpsrv : STR_);                                                            
                    ConfigurationManager::applyDateTime();
                }
			}
            
            // get current date
            char dateStr[11];
            DateTime::now().format(dateStr, FSTR("%d/%m/%Y"));
            addParamStr(STR_date, dateStr);
            
            // get current time
            char timeStr[9];
            DateTime::now().format(timeStr, FSTR("%H:%M:%S"));
            addParamStr(STR_time, timeStr);
            
            // get timezone and NTP server
            int8_t timezoneHours;
            uint8_t timezoneMinutes;
            char const* defaultNTPServer;
            ConfigurationManager::getDateTimeParams(&timezoneHours, &timezoneMinutes, &defaultNTPServer);
            addParamInt(STR_tzh, timezoneHours);
            addParamInt(STR_tzm, timezoneMinutes);
            addParamStr(STR_ntpsrv, defaultNTPServer);
			
			HTTPTemplateResponse::flush();
		}
		
	};
	
	
}

#endif

