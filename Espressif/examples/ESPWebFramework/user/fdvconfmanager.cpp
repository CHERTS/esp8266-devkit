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





namespace fdv
{

	//////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////
	// ConfigurationManager


    #if (FDV_INCLUDE_SERIALCONSOLE == 1)
    SerialConsole* ConfigurationManager::s_serialConsole = NULL;
    #endif

    #if (FDV_INCLUDE_SERIALBINARY == 1)
    SerialBinary*  ConfigurationManager::s_serialBinary  = NULL;
    #endif

    DateTime ConfigurationManager::s_bootTime;
	
    
    // can be re-applied
    void STC_FLASHMEM ConfigurationManager::applyUARTServices()
    {            
        // UART and serial services
        uint32_t baudRate;
        bool enableSystemOutput;
        SerialService serialService;
        getUARTParams(&baudRate, &enableSystemOutput, &serialService);
        HardwareSerial::getSerial(0)->reconfig(baudRate);
        if (!enableSystemOutput)
            DisableStdOut();
#if (FDV_INCLUDE_SERIALCONSOLE == 1)
        if (s_serialConsole)
        {
            delete s_serialConsole;
            s_serialConsole = NULL;
        }
#endif
#if (FDV_INCLUDE_SERIALBINARY == 1)
        if (s_serialBinary)
        {
            delete s_serialBinary;
            s_serialBinary = NULL;
        }
#endif
        switch (serialService)
        {
#if (FDV_INCLUDE_SERIALCONSOLE == 1)
            case SerialService_Console:
                s_serialConsole = new SerialConsole;
                break;
#endif
#if (FDV_INCLUDE_SERIALBINARY == 1)
            case SerialService_BinaryProtocol:
                s_serialBinary = new SerialBinary;
                break;
#endif
        }            
    }
    
    
    // can be re-applied
    void STC_FLASHMEM ConfigurationManager::applyWiFi()
    {
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
    }
    
    
    // can be re-applied
    // doesn't support GPIO16
    void STC_FLASHMEM ConfigurationManager::applyGPIO()
    {
        // GPIO
        for (uint32_t i = 0; i < 17; ++i)
        {
            bool configured, isOutput, pullUp, value;
            getGPIOParams(i, &configured, &isOutput, &pullUp, &value);
            if (configured)
            {
                // GPIO0..15
                GPIO gpio(i);
                if (isOutput)
                    gpio.modeOutput();
                else
                    gpio.modeInput();
                gpio.enablePullUp(pullUp);
                if (isOutput)
                    gpio.write(value);
            }
        }
    }		
    

    // can be re-applied
    void STC_FLASHMEM ConfigurationManager::applyAccessPointIP()
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
    }			
    
    
    // can be re-applied
    void STC_FLASHMEM ConfigurationManager::applyClientIP()
    {
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
    }			

        
    // can be re-applied
    void STC_FLASHMEM ConfigurationManager::applyDHCPServer()
    {
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
    
    
    void STC_FLASHMEM ConfigurationManager::applyDNS()
    {
        IPAddress DNS1, DNS2;
        getDNSParams(&DNS1, &DNS2);
        NSLookup::setDNSServer(0, DNS1);
        NSLookup::setDNSServer(1, DNS2);
    }
        
    
    // can be re-applied
    void STC_FLASHMEM ConfigurationManager::applyDateTime()
    {
        int8_t timezoneHours;
        uint8_t timezoneMinutes;
        char const* defaultNTPServer;
        getDateTimeParams(&timezoneHours, &timezoneMinutes, &defaultNTPServer);
        DateTime::setDefaults(timezoneHours, timezoneMinutes, defaultNTPServer);
        s_bootTime = DateTime::now();
    }

    
    void STC_FLASHMEM ConfigurationManager::restore()
    {
        FlashDictionary::eraseContent();
        system_restore();
    }
        
    
    void STC_FLASHMEM ConfigurationManager::setWiFiMode(WiFi::Mode value)
    {
        FlashDictionary::setInt(STR_WiFiMode, (int32_t)value);
    }
    
    
    WiFi::Mode STC_FLASHMEM ConfigurationManager::getWiFiMode()
    {
        return (WiFi::Mode)FlashDictionary::getInt(STR_WiFiMode, (int32_t)WiFi::AccessPoint);
    }
    
    
    void STC_FLASHMEM ConfigurationManager::setAccessPointParams(char const* SSID, char const* securityKey, uint8_t channel, WiFi::SecurityProtocol securityProtocol, bool hiddenSSID)
    {
        FlashDictionary::setString(STR_APSSID, SSID);
        FlashDictionary::setString(STR_APSECKEY, securityKey);
        FlashDictionary::setInt(STR_APCH, channel);
        FlashDictionary::setInt(STR_APSP, (int32_t)securityProtocol);
        FlashDictionary::setBool(STR_APHSSID, hiddenSSID);
    }
    
    
    void STC_FLASHMEM ConfigurationManager::getAccessPointParams(char const** SSID, char const** securityKey, uint8_t* channel, WiFi::SecurityProtocol* securityProtocol, bool* hiddenSSID)
    {
        static char defaultSSID[10];
        uint8_t mac[16];
        WiFi::getMACAddress(WiFi::AccessPointNetwork, mac);			
        sprintf(defaultSSID, FSTR("ESP%02X%02X%02X%02X%02X%02X"), mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
        *SSID             = FlashDictionary::getString(STR_APSSID, defaultSSID);
        *securityKey      = FlashDictionary::getString(STR_APSECKEY, STR_);
        *channel          = FlashDictionary::getInt(STR_APCH, 9);
        *securityProtocol = (WiFi::SecurityProtocol)FlashDictionary::getInt(STR_APSP, (int32_t)WiFi::Open);
        *hiddenSSID       = FlashDictionary::getBool(STR_APHSSID, false);
    }
    
    
    void STC_FLASHMEM ConfigurationManager::setClientParams(char const* SSID, char const* securityKey)
    {
        FlashDictionary::setString(STR_CLSSID, SSID);
        FlashDictionary::setString(STR_CLSECKEY, securityKey);
    }
    
    
    void STC_FLASHMEM ConfigurationManager::getClientParams(char const** SSID, char const** securityKey)
    {
        *SSID        = FlashDictionary::getString(STR_CLSSID, STR_);
        *securityKey = FlashDictionary::getString(STR_CLSECKEY, STR_);
    }
    
        
    // IP, netmask, gateway valid only if staticIP = true
    void STC_FLASHMEM ConfigurationManager::setClientIPParams(bool staticIP, char const* IP, char const* netmask, char const* gateway)
    {
        FlashDictionary::setBool(STR_CLSTATIC, staticIP);
        FlashDictionary::setString(STR_CLIP, IP);
        FlashDictionary::setString(STR_CLNETMSK, netmask);
        FlashDictionary::setString(STR_CLGTW, gateway);			
    }
    
    
    // IP, netmask, gateway valid only if staticIP = true
    void STC_FLASHMEM ConfigurationManager::getClientIPParams(bool* staticIP, char const** IP, char const** netmask, char const** gateway)
    {
        *staticIP = FlashDictionary::getBool(STR_CLSTATIC, false);
        *IP       = FlashDictionary::getString(STR_CLIP, STR_);
        *netmask  = FlashDictionary::getString(STR_CLNETMSK, STR_);
        *gateway  = FlashDictionary::getString(STR_CLGTW, STR_);
    }
    
    
    void STC_FLASHMEM ConfigurationManager::setAccessPointIPParams(char const* IP, char const* netmask, char const* gateway)
    {
        FlashDictionary::setString(STR_APIP, IP);
        FlashDictionary::setString(STR_APNETMSK, netmask);
        FlashDictionary::setString(STR_APGTW, gateway);			
    }
    
    
    void STC_FLASHMEM ConfigurationManager::getAccessPointIPParams(char const** IP, char const** netmask, char const** gateway)
    {
        *IP       = FlashDictionary::getString(STR_APIP, FSTR("192.168.4.1"));
        *netmask  = FlashDictionary::getString(STR_APNETMSK, FSTR("255.255.255.0"));
        *gateway  = FlashDictionary::getString(STR_APGTW, STR_);
    }
    
    
    void STC_FLASHMEM ConfigurationManager::setDHCPServerParams(bool enabled, char const* startIP, char const* endIP, uint32_t maxLeases)
    {
        FlashDictionary::setBool(STR_DHCPDEN, enabled);
        if (startIP && endIP && maxLeases)
        {
            FlashDictionary::setString(STR_DHCPDIP1, startIP);
            FlashDictionary::setString(STR_DHCPDIP2, endIP);
            FlashDictionary::setInt(STR_DHCPDMXL, maxLeases);
        }
    }
    
    
    void STC_FLASHMEM ConfigurationManager::getDHCPServerParams(bool* enabled, char const** startIP, char const** endIP, uint32_t* maxLeases)
    {
        *enabled   = FlashDictionary::getBool(STR_DHCPDEN, true);
        *startIP   = FlashDictionary::getString(STR_DHCPDIP1, FSTR("192.168.4.100"));
        *endIP     = FlashDictionary::getString(STR_DHCPDIP2, FSTR("192.168.4.110"));
        *maxLeases = FlashDictionary::getInt(STR_DHCPDMXL, 10);
    }
    
    
    void STC_FLASHMEM ConfigurationManager::setDNSParams(IPAddress DNS1, IPAddress DNS2)
    {
        FlashDictionary::setInt(STR_DNS1, DNS1.get_uint32());
        FlashDictionary::setInt(STR_DNS2, DNS2.get_uint32());
    }
    
    
    void STC_FLASHMEM ConfigurationManager::getDNSParams(IPAddress* DNS1, IPAddress* DNS2)
    {
        *DNS1 = FlashDictionary::getInt(STR_DNS1, IPAddress(8, 8, 8, 8).get_uint32());
        *DNS2 = FlashDictionary::getInt(STR_DNS2, IPAddress(8, 8, 4, 4).get_uint32());
    }
    
    
    void STC_FLASHMEM ConfigurationManager::setWebServerParams(uint16_t port)
    {
        FlashDictionary::setInt(STR_WEBPORT, port);
    }
    
    
    void STC_FLASHMEM ConfigurationManager::getWebServerParams(uint16_t* port)
    {
        *port = FlashDictionary::getInt(STR_WEBPORT, 80);
    }
    
    
    void STC_FLASHMEM ConfigurationManager::setUARTParams(uint32_t baudRate, bool enableSystemOutput, SerialService serialService)
    {
        FlashDictionary::setInt(STR_BAUD, baudRate);
        FlashDictionary::setBool(STR_SYSOUT, enableSystemOutput);
        FlashDictionary::setInt(STR_UARTSRV, (int32_t)serialService);
    }
    
    
    void STC_FLASHMEM ConfigurationManager::getUARTParams(uint32_t* baudRate, bool* enableSystemOutput, SerialService* serialService)
    {
        *baudRate           = FlashDictionary::getInt(STR_BAUD, 115200);
        *enableSystemOutput = FlashDictionary::getBool(STR_SYSOUT, false);
        *serialService      = (SerialService)FlashDictionary::getInt(STR_UARTSRV, (int32_t)SerialService_Console);
    }
    
    
#if (FDV_INCLUDE_SERIALCONSOLE == 1)
    SerialConsole* STC_FLASHMEM ConfigurationManager::getSerialConsole()
    {
        return s_serialConsole;
    }
#endif
    
    
#if (FDV_INCLUDE_SERIALBINARY == 1)
    SerialBinary* STC_FLASHMEM ConfigurationManager::getSerialBinary()
    {
        return s_serialBinary;
    }
#endif


    void STC_FLASHMEM ConfigurationManager::setGPIOParams(uint32_t gpioNum, bool configured, bool isOutput, bool pullUp, bool value)
    {
        APtr<char> key(f_printf(FSTR("GPIO%d"), gpioNum));
        GPIOInfo info = {configured, isOutput, pullUp, value};
        FlashDictionary::setValue(key.get(), &info, sizeof(GPIOInfo));
    }
    
    
    void STC_FLASHMEM ConfigurationManager::getGPIOParams(uint32_t gpioNum, bool* configured, bool* isOutput, bool* pullUp, bool* value)
    {
        APtr<char> key(f_printf(FSTR("GPIO%d"), gpioNum));
        uint8_t const* infoPtr = FlashDictionary::getValue(key.get());			
        if (infoPtr)
        {
            uint32_t infoInt = getDWord(infoPtr);
            GPIOInfo* info = (GPIOInfo*)&infoInt;
            *configured = info->configured;
            *isOutput   = info->isOutput;
            *pullUp     = info->pullUp;
            *value      = info->value;
        }
        else
        {
            // defaults
            *configured = false;
            *isOutput   = false;
            *pullUp     = false;
            *value      = false;
        }
    }
    
    
    void STC_FLASHMEM ConfigurationManager::setDateTimeParams(int8_t timezoneHours, uint8_t timezoneMinutes, char const* defaultNTPServer)
    {
        FlashDictionary::setInt(STR_TZHH, timezoneHours);
        FlashDictionary::setInt(STR_TZMM, timezoneMinutes);
        FlashDictionary::setString(STR_DEFNTPSRV, defaultNTPServer);
    }
    
    
    void STC_FLASHMEM ConfigurationManager::getDateTimeParams(int8_t* timezoneHours, uint8_t* timezoneMinutes, char const** defaultNTPServer)
    {
        *timezoneHours    = FlashDictionary::getInt(STR_TZHH, 0);
        *timezoneMinutes  = FlashDictionary::getInt(STR_TZMM, 0);
        *defaultNTPServer = FlashDictionary::getString(STR_DEFNTPSRV, FSTR("ntp1.inrim.it"));
    }
    
    
    DateTime STC_FLASHMEM ConfigurationManager::getBootDateTime()
    {
        return s_bootTime;
    }
    
    
}



