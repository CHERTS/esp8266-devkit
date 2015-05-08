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
	#include "udhcp/dhcpd.h"	

	#include <stdarg.h>
}



namespace fdv
{


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
	// UDPClient
    
    void MTD_FLASHMEM UDPClient::init(char const* remoteAddress, uint16_t remotePort)
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

    
    SNTPClient::SNTPClient(char const* serverIP, uint16_t port)
        : m_server(serverIP), m_port(port)
    {
        if (!m_server)
            m_server = FSTR("193.204.114.232");
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








