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
        static uint32_t const CHUNKSIZE = 256;
        
        int32_t bytesSent = 0;
        if (isStoredInFlash(buffer))
        {
            // copy from Flash, send in chunks of CHUNKSIZE bytes
            APtr<uint8_t> rambuf(new uint8_t[length]);
            uint8_t const* src = (uint8_t const*)buffer;
            while (bytesSent < length)
            {
                uint32_t bytesToSend = min(CHUNKSIZE, length - bytesSent);
                f_memcpy(rambuf.get(), src, bytesToSend);
                uint32_t chunkBytesSent = m_remoteAddress.sin_len == 0? lwip_send(m_socket, rambuf.get(), bytesToSend, 0) :
                                                                        lwip_sendto(m_socket, rambuf.get(), bytesToSend, 0, (sockaddr*)&m_remoteAddress, sizeof(m_remoteAddress));
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
        CharChunk* chunk = m_content.addChunk((char*)data, length, false);
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
            CharChunk* chunk = m_content.getFirstChunk();
            while (chunk)
            {
                m_httpHandler->getSocket()->write((uint8_t const*)chunk->data, chunk->items);
                chunk = chunk->next;
            }
            m_content.clear();
        }
    }
    
    

	//////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////
	// ParameterReplacer

	
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
					m_result.addChunk(start, curc - start, false);
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
						m_result.addChunk(start, curc - start, false);
						m_blocks.add(curBlockKey, curBlockKeyEnd, m_result);
						m_result.clear();
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
		m_result.addChunk(start, m_strEnd - start, false);
		if (curBlockKey && curBlockKeyEnd)
		{
			m_blocks.add(curBlockKey, curBlockKeyEnd, m_result);
			m_result.clear();
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
					m_result.addChunks(&item->value); // push parameter content
				else
					break;
			}
		}
		else
		{
			// replace one parameter
			Params::Item* item = m_params->getItem(tagStart, tagEnd);
			if (item)				
				m_result.addChunks(&item->value); // push parameter content
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


    void MTD_FLASHMEM HTTPTemplateResponse::addParamStr(char const* key, char const* value)
    {
        LinkedCharChunks linkedCharChunks;
        linkedCharChunks.addChunk(value, f_strlen(value), false);
        m_params.add(key, linkedCharChunks);
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
        char buf[len + 1];			
        va_start(args, fmt);
        vsprintf(buf, fmt, args);
        va_end(args);
        
        LinkedCharChunks* linkedCharChunks = m_params.add(key);
        linkedCharChunks->addChunk(buf, len, true);	// true = need to free
    }
    
    
    void MTD_FLASHMEM HTTPTemplateResponse::addParamCharChunks(char const* key, LinkedCharChunks* value)
    {
        m_params.add(key, *value);
    }
    
    
    void MTD_FLASHMEM HTTPTemplateResponse::addParams(Params* params)
    {
        m_params.add(params);
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
            ParameterReplacer replacer((char const*)data, (char const*)data + dataLength, &m_params);
            
            // is this a specialized file (contains {%..%} blocks)?
            if (replacer.getBlocks()->getItemsCount() > 0 && replacer.getTemplateFilename() != NULL)
            {
                // this is a specialized file, add blocks as parameters
                addParams(replacer.getBlocks());
                // load template file
                if (FlashFileSystem::find(replacer.getTemplateFilename(), &mimetype, &data, &dataLength))
                {
                    // replace parameters and blocks of template file
                    ParameterReplacer templateReplacer((char const*)data, (char const*)data + dataLength, &m_params);
                    // flush resulting content
                    addContent(templateReplacer.getResult());
                    return;
                }
            }
            else
            {
                // just flush this file (contains only {{...}} blocks)
                addContent(replacer.getResult());
                return;
            }
        }
        // not found
        setStatus(STR_404_Not_Fount);
    }
    
    
    
    
	//////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////
	// UDPClient
    
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
    SNTPClient::SNTPClient(IPAddress serverIP, uint16_t port)
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








