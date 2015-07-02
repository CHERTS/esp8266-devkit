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


extern "C"
{
	#include "lwip/ip_addr.h"
	#include "lwip/sockets.h"
	#include "lwip/dns.h"
	#include "lwip/netdb.h"
	#include "lwip/api.h"
	#include "lwip/netbuf.h"
    #include "lwip/inet.h"
	#include "udhcp/dhcpd.h"	

	#include <stdarg.h>
}



namespace fdv
{


	//////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////
    // IPAddress

    
    void MTD_FLASHMEM IPAddress::operator=(IPAddress const& c)
    {
        address[0] = c.address[0];
        address[1] = c.address[1];
        address[2] = c.address[2];
        address[3] = c.address[3];            
    }
    
    
    void MTD_FLASHMEM IPAddress::operator=(in_addr inaddr)
    {
        address[0] = ((uint8_t*)&inaddr.s_addr)[0];
        address[1] = ((uint8_t*)&inaddr.s_addr)[1];
        address[2] = ((uint8_t*)&inaddr.s_addr)[2];
        address[3] = ((uint8_t*)&inaddr.s_addr)[3];
    }
    
    
    void MTD_FLASHMEM IPAddress::operator=(in_addr_t inaddr)
    {
        address[0] = ((uint8_t*)&inaddr)[0];
        address[1] = ((uint8_t*)&inaddr)[1];
        address[2] = ((uint8_t*)&inaddr)[2];
        address[3] = ((uint8_t*)&inaddr)[3];
    }
    
    
    void MTD_FLASHMEM IPAddress::operator=(ip_addr_t ipaddr)
    {
        address[0] = ((uint8_t*)&ipaddr)[0];
        address[1] = ((uint8_t*)&ipaddr)[1];
        address[2] = ((uint8_t*)&ipaddr)[2];
        address[3] = ((uint8_t*)&ipaddr)[3];
    }
    
    
    void MTD_FLASHMEM IPAddress::operator=(char const* str)
    {
        if (!str || f_strlen(str) == 0)
            *this = IPAddress(0, 0, 0, 0);
        else
            *this = IPAddress(ipaddr_addr(APtr<char>(f_strdup(str)).get()));
    }
    
    
    in_addr_t MTD_FLASHMEM IPAddress::get_in_addr_t()
    {
        in_addr_t a;
        ((uint8_t*)&a)[0] = address[0];
        ((uint8_t*)&a)[1] = address[1];
        ((uint8_t*)&a)[2] = address[2];
        ((uint8_t*)&a)[3] = address[3];
        return a;
    }
    
    ip_addr_t MTD_FLASHMEM IPAddress::get_ip_addr_t()
    {
        ip_addr_t a;
        ((uint8_t*)&a.addr)[0] = address[0];
        ((uint8_t*)&a.addr)[1] = address[1];
        ((uint8_t*)&a.addr)[2] = address[2];
        ((uint8_t*)&a.addr)[3] = address[3];
        return a;
    }
    
    uint32_t MTD_FLASHMEM IPAddress::get_uint32()
    {
        uint32_t a;
        ((uint8_t*)&a)[0] = address[0];
        ((uint8_t*)&a)[1] = address[1];
        ((uint8_t*)&a)[2] = address[2];
        ((uint8_t*)&a)[3] = address[3];
        return a;
    }
    
    IPAddress::IPAddressStr MTD_FLASHMEM IPAddress::get_str()
    {
        IPAddressStr str;
        ip_addr_t a = get_ip_addr_t();
        ipaddr_ntoa_r(&a, (char*)str, 16);
        return str;
    }
    
    bool MTD_FLASHMEM IPAddress::operator==(IPAddress const& rhs)
    {
        return address[0] == rhs.address[0] && address[1] == rhs.address[1] &&
               address[2] == rhs.address[2] && address[3] == rhs.address[3];
    }
    
    bool MTD_FLASHMEM IPAddress::operator!=(IPAddress const& rhs)
    {
        return !(*this == rhs);
    }
    


	//////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////
	// WiFi
	
    
    WiFi::Mode STC_FLASHMEM WiFi::setMode(Mode mode)
    {
        Critical critical;
        wifi_set_opmode(mode);
    }
    
    
    WiFi::Mode STC_FLASHMEM WiFi::getMode()
    {
        Mode mode = (Mode)wifi_get_opmode();
        return mode;
    }
    
    
    char const* STC_FLASHMEM WiFi::convSecurityProtocolToString(SecurityProtocol securityProtocol)
    {
        char const* authMode = STR_;
        switch (securityProtocol)
        {
            case WiFi::Open:
                authMode = FSTR("Open");
                break;
            case WiFi::WEP:
                authMode = FSTR("WEP");
                break;
            case WiFi::WPA_PSK:
                authMode = FSTR("WPA-PSK");
                break;
            case WiFi::WPA2_PSK:
                authMode = FSTR("WPA2-PSK");
                break;
            case WiFi::WPA_WPA2_PSK:
                authMode = FSTR("WPA-WPA2-PSK");
                break;
        }
        return authMode;
    }			

    
    // setMode must be called with AccessPoint or ClientAndAccessPoint
    // note: make sure there is enough stack space free otherwise mail cause reset (fatal exception)!
    // channel: 1..13
    void STC_FLASHMEM WiFi::configureAccessPoint(char const* SSID, char const* securityKey, uint8_t channel, SecurityProtocol securityProtocol, bool hiddenSSID)
    {						
        softap_config config = {0};
        wifi_softap_get_config(&config);
        f_strcpy((char *)config.ssid, SSID);
        config.ssid_len = f_strlen(SSID);
        f_strcpy((char *)config.password, securityKey);
        config.channel = channel;
        config.authmode = securityProtocol;
        config.ssid_hidden = (uint8)hiddenSSID;
        Critical critical;
        wifi_softap_set_config(&config);
    }
    
    
    // setMode must be called with Client or ClientAndAccessPoint
    void STC_FLASHMEM WiFi::configureClient(char const* SSID, char const* securityKey)
    {
        station_config config = {0};
        f_strcpy((char *)config.ssid, SSID);
        f_strcpy((char *)config.password, securityKey);
        Critical critical;
        wifi_station_disconnect();
        wifi_station_set_config(&config);
        wifi_station_connect();
    }
    
    
    // fills MAC with MAC address of the specified network
    // MAC must be a pointer to 6 bytes buffer
    void STC_FLASHMEM WiFi::getMACAddress(WiFi::Network network, uint8_t* MAC)
    {
        wifi_get_macaddr((uint8_t)network, MAC);
    }
    
    
    WiFi::ClientConnectionStatus STC_FLASHMEM WiFi::getClientConnectionStatus()
    {
        return (ClientConnectionStatus)wifi_station_get_connect_status();
    }
    
    
    // returns access point list
    WiFi::APInfo* STC_FLASHMEM WiFi::getAPList(uint32_t* count, bool rescan)
    {
        if (rescan)
        {
            Mode prevMode = getMode();
            if (prevMode == AccessPoint)
                setMode(ClientAndAccessPoint);
            wifi_station_scan(NULL, scanDoneCB);
            getAPInfo()->receive();	// wait for completion
            if (prevMode != getMode())
                setMode(prevMode);
        }
        APInfo* infos;
        getAPInfo(&infos, count);
        return infos;
    }

    
    void STC_FLASHMEM WiFi::scanDoneCB(void* arg, STATUS status)
    {
        if (status == OK)
        {
            // count items
            uint32_t count = 0;
            for (bss_info* bss_link = ((bss_info*)arg)->next.stqe_next; bss_link; bss_link = bss_link->next.stqe_next)
                ++count;
            // fill items
            APInfo* infos;
            getAPInfo(&infos, &count, count);
            for (bss_info* bss_link = ((bss_info*)arg)->next.stqe_next; bss_link; bss_link = bss_link->next.stqe_next, ++infos)
            {
                memcpy(infos->BSSID, bss_link->bssid, 6);
                memset(infos->SSID, 0, 33);
                memcpy(infos->SSID, bss_link->ssid, 32);
                infos->Channel  = bss_link->channel;
                infos->RSSI     = bss_link->rssi;
                infos->AuthMode = (SecurityProtocol)bss_link->authmode;
                infos->isHidden = (bool)bss_link->is_hidden;
            }
            getAPInfo()->send();
        }
    }
    
    
    // allocateCount >= 0 -> allocate (or reallocate or free) AP info
    // allocateCount < 0  -> get infos
    Queue<bool>* STC_FLASHMEM WiFi::getAPInfo(APInfo** infos, uint32_t* count, int32_t allocateCount)
    {
        static APInfo*  s_infos = NULL;
        static uint32_t s_count = 0;
        static Queue<bool>* s_queue = new Queue<bool>(1);	// never deleted
        if (allocateCount >= 0)
        {
            if (s_infos != NULL)
                delete[] s_infos;
            s_infos = NULL;
            s_count = 0;
            if (allocateCount > 0)
            {
                s_infos = new APInfo[allocateCount];
                s_count = allocateCount;
            }
        }
        if (infos && count)
        {
            *infos = s_infos;
            *count = s_count;
        }
        return s_queue;
    }
		


	//////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////
	// IP
	
    void STC_FLASHMEM IP::configureStatic(WiFi::Network network, char const* IP, char const* netmask, char const* gateway)
    {
        ip_info info;
        info.ip.addr      = ipaddr_addr(APtr<char>(f_strdup(IP)).get());
        info.netmask.addr = ipaddr_addr(APtr<char>(f_strdup(netmask)).get());
        info.gw.addr      = ipaddr_addr(APtr<char>(f_strdup(gateway)).get());
        Critical critical;
        if (network == WiFi::ClientNetwork)
            wifi_station_dhcpc_stop();
        wifi_set_ip_info(network, &info);
    }
    
    
    // applies only to ClientNetwork
    void STC_FLASHMEM IP::configureDHCP(WiFi::Network network)
    {
        if (network == WiFi::ClientNetwork)
        {
            Critical critical;
            wifi_station_dhcpc_start();
        }
    }
    
    
    // fills IP with IP address of the specified network
    // IP, netmask, gateway must be a pointer to 4 bytes buffer
    void STC_FLASHMEM IP::getIPInfo(WiFi::Network network, uint8_t* IP, uint8_t* netmask, uint8_t* gateway)
    {
        ip_info info;
        wifi_get_ip_info(network, &info);
        for (uint32_t i = 0; i != 4; ++i)
        {
            IP[i]      = ((uint8_t*)&info.ip.addr)[i];
            netmask[i] = ((uint8_t*)&info.netmask.addr)[i];
            gateway[i] = ((uint8_t*)&info.gw.addr)[i];
        }
    }
		
        
        
	//////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////
	// DHCPServer
	
    // warn: each IP in the range requires memory!
    void STC_FLASHMEM DHCPServer::configure(char const* startIP, char const* endIP, uint32_t maxLeases)
    {		
        //udhcpd_stop();
        dhcp_info info = {0};
        info.start_ip      = ipaddr_addr(APtr<char>(f_strdup(startIP)).get());
        info.end_ip        = ipaddr_addr(APtr<char>(f_strdup(endIP)).get());
        info.max_leases    = maxLeases;
        info.auto_time     = 60;
        info.decline_time  = 60;
        info.conflict_time = 60;
        info.offer_time    = 60;
        info.min_lease_sec = 60;
        dhcp_set_info(&info);
        udhcpd_start();			
    }
        
        

   	//////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////
    // NSLookup (DNS client)
    
    
    MTD_FLASHMEM NSLookup::NSLookup(char const* hostname)
    {    
        m_ipaddr = lookup(hostname);
    }
    
    
    // returns IPAddress(0, 0, 0, 0) on fail
    IPAddress MTD_FLASHMEM NSLookup::lookup(char const* hostname)
    {
        APtr<char> memHostName( f_strdup(hostname) );
        addrinfo* addrinfo;
        if (lwip_getaddrinfo(memHostName.get(), NULL, NULL, &addrinfo) == 0 && addrinfo)
        {
            // according to lwip documentation uses only first item of "addrinfo"
            sockaddr_in* sa = (sockaddr_in*)(addrinfo->ai_addr);
            in_addr_t addr = sa->sin_addr.s_addr;
            lwip_freeaddrinfo(addrinfo);
            return IPAddress(addr);
        }
        return IPAddress(0, 0, 0, 0);   // fail!
    }
    
    
    IPAddress MTD_FLASHMEM NSLookup::get()
    {
        return m_ipaddr;
    }
    
    
    void MTD_FLASHMEM NSLookup::setDNSServer(uint32_t num, IPAddress server)
    {
        ip_addr_t a = server.get_ip_addr_t();
        dns_setserver(num, &a);
    }
    
    
    IPAddress MTD_FLASHMEM NSLookup::getDNSServer(uint32_t num)
    {
        return IPAddress(dns_getserver(num));
    }
    

        

	//////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////
    // Socket

    
    // ret -1 = error, ret 0 = disconnected
    int32_t MTD_FLASHMEM Socket::read(void* buffer, uint32_t maxLength)
    {
        int32_t bytesRecv = lwip_recv(m_socket, buffer, maxLength, 0);
        if (maxLength > 0)
            m_connected = (bytesRecv > 0);
        return bytesRecv;
    }
    
    // ret -1 = error, ret 0 = disconnected
    int32_t MTD_FLASHMEM Socket::peek(void* buffer, uint32_t maxLength)
    {
        int32_t bytesRecv = lwip_recv(m_socket, buffer, maxLength, MSG_PEEK);
        if (maxLength > 0)
            m_connected = (bytesRecv > 0);
        return bytesRecv;
    }
    
    
    // buffer can stay in RAM of Flash
    // ret -1 = error, ret 0 = disconnected
    int32_t MTD_FLASHMEM Socket::write(void const* buffer, uint32_t length)
    {
        static uint32_t const MAXCHUNKSIZE = 128;
        
        int32_t bytesSent = 0;
        if (isStoredInFlash(buffer))
        {
            // copy from Flash, send in chunks of up to MAXCHUNKSIZE bytes
            uint8_t rambuf[min(length, MAXCHUNKSIZE)];
            uint8_t const* src = (uint8_t const*)buffer;
            while (bytesSent < length)
            {
                uint32_t bytesToSend = min(MAXCHUNKSIZE, length - bytesSent);
                f_memcpy(rambuf, src, bytesToSend);
                uint32_t chunkBytesSent = m_remoteAddress.sin_len == 0? lwip_send(m_socket, rambuf, bytesToSend, 0) :
                                                                        lwip_sendto(m_socket, rambuf, bytesToSend, 0, (sockaddr*)&m_remoteAddress, sizeof(m_remoteAddress));
                if (chunkBytesSent == 0)
                {
                    // error
                    bytesSent = 0;
                    break;
                }
                bytesSent += chunkBytesSent;
                src += chunkBytesSent;
            }
        }
        else
        {
            // just send as is
            bytesSent = m_remoteAddress.sin_len == 0? lwip_send(m_socket, buffer, length, 0) :
                                                      lwip_sendto(m_socket, buffer, length, 0, (sockaddr*)&m_remoteAddress, sizeof(m_remoteAddress));
        }
        if (length > 0)
            m_connected = (bytesSent > 0);
        return bytesSent;
    }
    
    
    // str can stay in RAM of Flash
    // ret -1 = error, ret 0 = disconnected
    int32_t MTD_FLASHMEM Socket::write(char const* str)
    {
        return write((uint8_t const*)str, f_strlen(str));
    }
    
    
    // like printf
    // buf can stay in RAM or Flash
    // "strings" of args can stay in RAM or Flash
    uint16_t MTD_FLASHMEM Socket::writeFmt(char const *fmt, ...)
    {
        va_list args;
        
        va_start(args, fmt);
        uint16_t len = vsprintf(NULL, fmt, args);
        va_end(args);

        char buf[len + 1];
        
        va_start(args, fmt);
        vsprintf(buf, fmt, args);
        va_end(args);
        
        write(buf, len);

        return len;
    }
    
    
    void MTD_FLASHMEM Socket::close()
    {
        if (m_socket > 0)
        {
            lwip_close(m_socket);
            m_socket = 0;
        }
        m_connected = false;
    }
    

    void MTD_FLASHMEM Socket::setNoDelay(bool value)
    {
        int32_t one = (int32_t)value;
        lwip_setsockopt(m_socket, IPPROTO_TCP, TCP_NODELAY, &one, sizeof(one));
    }

    
    
    // from now Socket will use "sendto" instead of "send"
    void MTD_FLASHMEM Socket::setRemoteAddress(IPAddress remoteAddress, uint16_t remotePort)
    {
        memset(&m_remoteAddress, 0, sizeof(sockaddr_in));
        m_remoteAddress.sin_family      = AF_INET;
        m_remoteAddress.sin_len         = sizeof(sockaddr_in);
        m_remoteAddress.sin_addr.s_addr = remoteAddress.get_in_addr_t();
        m_remoteAddress.sin_port        = htons(remotePort);
    }
    
    
    // timeOut in ms (0 = no timeout)
    void MTD_FLASHMEM Socket::setTimeOut(uint32_t timeOut)
    {
        lwip_setsockopt(m_socket, SOL_SOCKET, SO_RCVTIMEO, (void *)&timeOut, sizeof(timeOut));
    }
    
    
    
	//////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////
	// HTTPResponse
	
    MTD_FLASHMEM HTTPResponse::HTTPResponse(HTTPHandler* httpHandler, char const* status, char const* content)
        : m_httpHandler(httpHandler), m_status(status)
    {
        // content (if present, otherwise use addContent())
        if (content)
            addContent(content);
    }
    
    
    HTTPHandler::Request& MTD_FLASHMEM HTTPResponse::getRequest()
    {
        return m_httpHandler->getRequest();
    }
    
    
    // accept RAM or Flash strings
    void MTD_FLASHMEM HTTPResponse::addHeader(char const* key, char const* value)
    {
        m_headers.add(key, value);
    }
    
    
    // accept RAM or Flash data
    // WARN: data is not copied! Just a pointer is stored
    void MTD_FLASHMEM HTTPResponse::addContent(void const* data, uint32_t length)
    {
        m_content.addChunk((char*)data, length, false);
    }
    
    
    // accept RAM or Flash strings
    // WARN: data is not copied! Just a pointer is stored
    // can be called many times
    void MTD_FLASHMEM HTTPResponse::addContent(char const* str)
    {
        addContent(str, f_strlen(str));
    }
      
      
    // can be called many times
    // WARN: src content is not copied! Just data pointers are stored
    void MTD_FLASHMEM HTTPResponse::addContent(LinkedCharChunks* src)
    {
        m_content.addChunks(src);
    }
            
    // should be called only after setStatus, addHeader and addContent
    void MTD_FLASHMEM HTTPResponse::flush()
    {
        // status line
        m_httpHandler->getSocket()->writeFmt(FSTR("HTTP/1.1 %s\r\n"), m_status);

        // HTTPResponse headers
        addHeader(FSTR("Connection"), FSTR("close"));			

        // user headers
        for (uint32_t i = 0; i != m_headers.getItemsCount(); ++i)
        {
            Fields::Item* item = m_headers[i];
            m_httpHandler->getSocket()->writeFmt(FSTR("%s: %s\r\n"), APtr<char>(t_strdup(item->key)).get(), APtr<char>(t_strdup(item->value)).get());
        }

        // content length header
        m_httpHandler->getSocket()->writeFmt(FSTR("%s: %d\r\n\r\n"), STR_Content_Length, m_content.getItemsCount());

        // actual content
        if (m_content.getItemsCount() > 0)
        {			
            CharChunksIterator iter = m_content.getIterator();
            CharChunkBase* chunk = iter.getCurrentChunk();
            while (chunk)
            {
                m_httpHandler->getSocket()->write((uint8_t const*)chunk->data, chunk->getItems());
                chunk = iter.moveToNextChunk();
            }
            m_content.clear();
        }
    }
    
    

	//////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////
	// ParameterReplacer

    MTD_FLASHMEM ParameterReplacer::ParameterReplacer()
        : m_params(NULL), m_blockParams(NULL), m_strStart(NULL), m_strEnd(NULL)
    {
        m_results.add(new LinkedCharChunks);
    }
    
    
    MTD_FLASHMEM ParameterReplacer::~ParameterReplacer()
    {
        for (uint32_t i = 0; i != m_results.size(); ++i)
            delete m_results[i];
    }
    
	
    void MTD_FLASHMEM ParameterReplacer::start(char const* strStart, char const* strEnd, Params* params, BlockParams* blockParams)
    {
        m_params      = params;
        m_blockParams = blockParams;
        m_strStart    = strStart;
        m_strEnd      = strEnd;
        processInput();
    }
    
    
	void MTD_FLASHMEM ParameterReplacer::processInput()
	{
		char const* curc  = m_strStart;
		char const* start = curc;
		char const* curBlockKey      = NULL;
		char const* curBlockKeyEnd   = NULL;
		while (curc != m_strEnd)
		{
			char c0 = getChar(curc);
			if (c0 == '{')
			{
				char c1 = getChar(curc + 1);
				if (c1 == '{')
				{
					// found "{{"
					// push previous content
					m_results.last()->addChunk(start, curc - start, false);
					// process parameter tag
					start = curc = replaceTag(curc);
					continue;
				}
				else if (c1 == '%')
				{
					// found "{%"
					// push previous content
					if (curBlockKey && curBlockKeyEnd)
					{
						m_results.last()->addChunk(start, curc - start, false);
						m_blocks.add(curBlockKey, curBlockKeyEnd, m_results.last());
                        m_results.add(new LinkedCharChunks);
					}
					// process block tag
					curBlockKey = extractTagStr(curc, &curBlockKeyEnd);
					start = curc = curBlockKeyEnd + 2;	// bypass "%}"
					// if this is the first block tag then this is the template file name
					if (m_template.get() == NULL)
					{
						m_template.reset(f_strdup(curBlockKey, curBlockKeyEnd));
						curBlockKey = NULL;
						curBlockKeyEnd = NULL;
					}
					continue;
				}
			}
			++curc;
		}
		m_results.last()->addChunk(start, m_strEnd - start, false);
		if (curBlockKey && curBlockKeyEnd)
		{
			m_blocks.add(curBlockKey, curBlockKeyEnd, m_results.last());
		}			
	}
	

	char const* MTD_FLASHMEM ParameterReplacer::replaceTag(char const* curc)
	{
		char const* tagEnd;
		char const* tagStart = extractTagStr(curc, &tagEnd);
		if (getChar(tagStart) == '#')
		{
			// replace multiple parameters ('0param', '1param', ...)
			++tagStart;			
			uint32_t tagLen = tagEnd - tagStart;
			char tag[tagLen];
			f_memcpy(tag, tagStart, tagLen);
			tag[tagLen] = 0;
			for (uint32_t index = 0; ; ++index)
			{
				char const* fulltagname = f_printf(FSTR("%d%s"), index, tag);
				Params::Item* item = m_params->getItem(fulltagname);
				if (item)
					m_results.last()->addChunks(&item->value); // push parameter content
				else
					break;
			}
		}
		else
		{
			// replace one parameter
			Params::Item* item = m_params->getItem(tagStart, tagEnd);
			if (item)				
				m_results.last()->addChunks(&item->value); // push parameter content
            else if(m_blockParams)
            {
                BlockParams::Item* item = m_blockParams->getItem(tagStart, tagEnd);
                if (item)
                    m_results.last()->addChunks(item->value);   // push block parameter content
            }
		}
		return tagEnd + 2;	// bypass "}}"
	}
	
	
	char const* MTD_FLASHMEM ParameterReplacer::extractTagStr(char const* curc, char const** tagEnd)
	{
		char const* tagStart = curc + 2; // by pass "{{" or "{%"
		*tagEnd = tagStart;
		while (*tagEnd < m_strEnd && getChar(*tagEnd) != '}' && getChar(*tagEnd) != '%')
			++*tagEnd;
		return tagStart;
	}
	

    
	//////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////
	// HTTPTemplateResponse

    
    MTD_FLASHMEM HTTPTemplateResponse::HTTPTemplateResponse(HTTPHandler* httpHandler, char const* filename)
        : HTTPResponse(httpHandler, NULL), m_filename(filename)
    {			
    }
    

    void MTD_FLASHMEM HTTPTemplateResponse::addParamStr(char const* key, char const* value)
    {
        LinkedCharChunks* linkedCharChunks = m_params.add(key);
        linkedCharChunks->addChunk(value);
    }
    
    
    void MTD_FLASHMEM HTTPTemplateResponse::addParamInt(char const* key, int32_t value)
    {
        char* valueStr = f_printf(FSTR("%d"), value);
        LinkedCharChunks* linkedCharChunks = m_params.add(key);
        linkedCharChunks->addChunk(valueStr, f_strlen(valueStr), true);	// true = need to free
    }
    
    
    void MTD_FLASHMEM HTTPTemplateResponse::addParamFmt(char const* key, char const *fmt, ...)
    {
        va_list args;			
        va_start(args, fmt);
        uint16_t len = vsprintf(NULL, fmt, args);
        va_end(args);
        char* buf = new char[len + 1];
        va_start(args, fmt);
        vsprintf(buf, fmt, args);
        va_end(args);
        
        LinkedCharChunks* linkedCharChunks = m_params.add(key);
        linkedCharChunks->addChunk(buf, len, true);	// true = need to free
    }
    
    
    LinkedCharChunks* MTD_FLASHMEM HTTPTemplateResponse::addParamCharChunks(char const* key)
    {
        return m_params.add(key);
    }
    
    
    HTTPTemplateResponse::Params* MTD_FLASHMEM HTTPTemplateResponse::getParams()
    {
        return &m_params;
    }
    
    
    void MTD_FLASHMEM HTTPTemplateResponse::flush()
    {
        // {{now}} predefined parameter : display date/time
        char datetimeStr[34];
        DateTime::now().format(datetimeStr, FSTR("%c"));
        addParamStr(STR_now, datetimeStr);

        processFileRequest();
        HTTPResponse::flush();
    }
    

    void MTD_FLASHMEM HTTPTemplateResponse::processFileRequest()
    {
        char const* mimetype;
        void const* data;
        uint16_t dataLength;
        if (m_filename && FlashFileSystem::find(m_filename, &mimetype, &data, &dataLength))
        {
            // found
            setStatus(STR_200_OK);
            addHeader(STR_Content_Type, FSTR("text/html"));
            
            // replace parameters
            m_replacer.start((char const*)data, (char const*)data + dataLength, &m_params, NULL);
            
            // is this a specialized file (contains {%..%} blocks)?
            if (m_replacer.getBlocks()->getItemsCount() > 0 && m_replacer.getTemplateFilename() != NULL)
            {
                // this is a specialized file
                // load template file
                if (FlashFileSystem::find(m_replacer.getTemplateFilename(), &mimetype, &data, &dataLength))
                {
                    // replace parameters and blocks of template file
                    m_templateReplacer.start((char const*)data, (char const*)data + dataLength, &m_params, m_replacer.getBlocks());
                    // flush resulting content
                    addContent(m_templateReplacer.getResult());
                    return;
                }
            }
            else
            {
                // just flush this file (contains only {{...}} blocks)
                addContent(m_replacer.getResult());
                return;
            }
        }
        // not found
        setStatus(STR_404_Not_Fount);
    }
    
    
    
    
	//////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////
	// UDPClient
    
    MTD_FLASHMEM UDPClient::UDPClient(IPAddress remoteAddress, uint16_t remotePort)
    {
        init(remoteAddress, remotePort);
    }
    
    
    MTD_FLASHMEM UDPClient::~UDPClient()
    {
        m_socket.close();
    }
    
    
    void MTD_FLASHMEM UDPClient::init(IPAddress remoteAddress, uint16_t remotePort)
    {
        m_socket = lwip_socket(PF_INET, SOCK_DGRAM, 0);
        
        sockaddr_in localAddress     = {0};
        localAddress.sin_family      = AF_INET;
        localAddress.sin_len         = sizeof(sockaddr_in);
        localAddress.sin_addr.s_addr = htonl(INADDR_ANY);
        localAddress.sin_port        = htons(getUDPPort());
        lwip_bind(m_socket.getSocket(), (sockaddr*)&localAddress, sizeof(sockaddr_in));			
        
        m_socket.setRemoteAddress(remoteAddress, remotePort);
    }

    
    uint16_t MTD_FLASHMEM UDPClient::getUDPPort()
    {
        static uint16_t s_port = 59999;
        s_port = (++s_port == 0? 60000 : s_port);
        return s_port;
    }
    
    
    ////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////
    // SNTPClient

    // serverIP is a uint8_t[4] IP address or NULL
    MTD_FLASHMEM SNTPClient::SNTPClient(IPAddress serverIP, uint16_t port)
        : m_server(serverIP), m_port(port)
    {
    }
    
    
    bool MTD_FLASHMEM SNTPClient::query(uint64_t* outValue) const
    {
      // send req (mode 3), unicast, version 4
      uint8_t const MODE_CLIENT   = 3;
      uint8_t const VERSION       = 4;
      uint8_t const BUFLEN        = 48;
      uint32_t const REPLYTIMEOUT = 3000;
      uint8_t buf[BUFLEN];
      memset(&buf[0], 0, BUFLEN);
      buf[0] = MODE_CLIENT | (VERSION << 3);
      
      UDPClient UDP(m_server, m_port);
      Socket* socket = UDP.getSocket();
      socket->setTimeOut(REPLYTIMEOUT);
      
      if (socket->write(&buf[0], BUFLEN))
      {
        // get reply
        if (socket->read(&buf[0], BUFLEN) == BUFLEN)
        {
          memcpy(outValue, &buf[40], sizeof(uint64_t));
          return true;  // ok
        }
      }

      return false;  // error
    }

    
    
    
	
}	// fdv namespace








