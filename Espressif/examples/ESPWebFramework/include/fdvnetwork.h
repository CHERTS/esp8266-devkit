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


#ifndef _FDVWIFI_H_
#define _FDVWIFI_H_


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



extern "C"
{
bool wifi_station_dhcpc_start(void);
bool wifi_station_dhcpc_stop(void);
bool dhcp_set_info(dhcp_info *if_dhcp);
sint8 FUNC_FLASHMEM udhcpd_stop(void);
}	

	

namespace fdv
{
	
	//////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////
	// WiFi
	
	struct WiFi
	{
			
		enum Mode
		{
			Client               = STATION_MODE,
			AccessPoint          = SOFTAP_MODE,
			ClientAndAccessPoint = STATIONAP_MODE			
		};
		
		enum SecurityProtocol
		{
			Open          = AUTH_OPEN,
			WEP           = AUTH_WEP,
			WPA_PSK       = AUTH_WPA_PSK,
			WPA2_PSK      = AUTH_WPA2_PSK,
			WPA_WPA2_PSK  = AUTH_WPA_WPA2_PSK
		};

		static Mode MTD_FLASHMEM setMode(Mode mode)
		{
			Critical critical;
			wifi_set_opmode(mode);
		}
		
		static Mode MTD_FLASHMEM getMode()
		{
			Mode mode = (Mode)wifi_get_opmode();
			return mode;
		}

		// setMode must be called with AccessPoint or ClientAndAccessPoint
		// note: make sure there is enough stack space free otherwise mail cause reset (fatal exception)!
		// channel: 1..13
		static void MTD_FLASHMEM configureAccessPoint(char const* SSID, char const* securityKey, uint8_t channel, SecurityProtocol securityProtocol = WPA2_PSK, bool hiddenSSID = false)
		{						
			softap_config config = {0};
			wifi_softap_get_config(&config);
			f_strcpy((char *)config.ssid, SSID);
			config.ssid_len = strlen(SSID);
			f_strcpy((char *)config.password, securityKey);
			config.channel = channel;
			config.authmode = securityProtocol;
			config.ssid_hidden = (uint8)hiddenSSID;
			Critical critical;
			wifi_softap_set_config(&config);
		}
		
		// setMode must be called with Client or ClientAndAccessPoint
		static void MTD_FLASHMEM configureClient(char const* SSID, char const* securityKey)
		{
			station_config config = {0};
			f_strcpy((char *)config.ssid, SSID);
			f_strcpy((char *)config.password, securityKey);
			Critical critical;
			wifi_station_disconnect();
			wifi_station_set_config(&config);
			wifi_station_connect();
		}

	};
	

	//////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////
	// IP
	
	struct IP
	{

		enum Network
		{
			ClientNetwork      = 0,
			AccessPointNetwork = 1
		};
	
		static void MTD_FLASHMEM configureStatic(Network network, char const* IP, char const* netmask, char const* gateway)
		{
			ip_info info;
			info.ip.addr      = ipaddr_addr(Ptr<char>(f_strdup(IP)).get());
			info.netmask.addr = ipaddr_addr(Ptr<char>(f_strdup(netmask)).get());
			info.gw.addr      = ipaddr_addr(Ptr<char>(f_strdup(gateway)).get());
			Critical critical;
			if (network == ClientNetwork)
				wifi_station_dhcpc_stop();
			wifi_set_ip_info(network, &info);
		}
		
		// applies only to ClientNetwork
		static void MTD_FLASHMEM configureDHCP(Network network)
		{
			if (network == ClientNetwork)
			{
				Critical critical;
				wifi_station_dhcpc_start();
			}
		}
		
	};
	
	
	//////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////
	// DHCPServer
	
	struct DHCPServer
	{
		
		// warn: each IP in the range requires memory!
		static void MTD_FLASHMEM configure(char const* startIP, char const* endIP, uint32_t maxLeases)
		{		
			//udhcpd_stop();
			dhcp_info info = {0};
			info.start_ip      = ipaddr_addr(Ptr<char>(f_strdup(startIP)).get());
			info.end_ip        = ipaddr_addr(Ptr<char>(f_strdup(endIP)).get());
			info.max_leases    = maxLeases;
			info.auto_time     = 60;
			info.decline_time  = 60;
			info.conflict_time = 60;
			info.offer_time    = 60;
			info.min_lease_sec = 60;
			dhcp_set_info(&info);
			udhcpd_start();			
		}
    };	
	

	//////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////
	// TCPServer
	
	template <typename ConnectionHandler_T, uint16_t MaxThreads_V, uint16_t ThreadsStackDepth_V>
	struct TCPServer
	{
		
		static uint32_t const ACCEPTWAITTIMEOUTMS = 20000;
		static uint32_t const SOCKETQUEUESIZE     = 1;	// can queue only one socket (nothing to do with working threads)
		
		TCPServer(uint16_t port)
			: m_socketQueue(SOCKETQUEUESIZE)
		{
			m_socket = lwip_socket(PF_INET, SOCK_STREAM, 0);
			sockaddr_in sLocalAddr = {0};
			sLocalAddr.sin_family = AF_INET;
			sLocalAddr.sin_len = sizeof(sockaddr_in);
			sLocalAddr.sin_addr.s_addr = htonl(INADDR_ANY);	// todo: allow other choices other than IP_ADDR_ANY
			sLocalAddr.sin_port = htons(port);
			lwip_bind(m_socket, (sockaddr*)&sLocalAddr, sizeof(sockaddr_in));			
			lwip_listen(m_socket, MaxThreads_V);

			// prepare listener task
			m_listenerTask.setStackDepth(200);
			m_listenerTask.setObject(this);
			m_listenerTask.resume();
			
			// prepare handler tasks
			for (uint16_t i = 0; i != MaxThreads_V; ++i)
			{
				m_threads[i].setStackDepth(ThreadsStackDepth_V);
				m_threads[i].setSocketQueue(&m_socketQueue);
				m_threads[i].resume();
			}
		}
		
		virtual ~TCPServer()
		{
			lwip_close(m_socket);
		}
		
		void MTD_FLASHMEM listenerTask()
		{
			while (true)
			{
				sockaddr_in clientAddr;
				socklen_t addrLen = sizeof(sockaddr_in);
				int clientSocket = lwip_accept(m_socket, (sockaddr*)&clientAddr, &addrLen);
				if (clientSocket > 0)
				{
					if (!m_socketQueue.send(clientSocket, ACCEPTWAITTIMEOUTMS))
					{
						// Timeout, no thread available, disconnecting
						lwip_close(clientSocket);
					}
				}
			}
		}
		
	private:
	
		int                                             m_socket;
		MethodTask<TCPServer, &TCPServer::listenerTask> m_listenerTask;
		ConnectionHandler_T                             m_threads[MaxThreads_V];
		Queue<int>                                      m_socketQueue;
	};
	
	
	//////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////
	// TCPConnectionHandler
	
	struct TCPConnectionHandler : public Task
	{
		
		TCPConnectionHandler()
			: m_connected(false)
		{
		}
		
		void setSocketQueue(Queue<int>* socketQueue)
		{
			m_socketQueue = socketQueue;
		}
				
		/*
		Unfortunately FIONREAD has not been compiled into lwip!
		uint32_t available()
		{
			int n = 0;
			if (lwip_ioctl(m_socket, FIONREAD, &n) < 0 || n < 0)
				n = 0;
			return n;
		}
		
		// as available() but wait for at least one byte to be available
		uint32_t availableWait()
		{
			char c;
			peek(&c, 1);
			return available();
		}
		*/
		
		// ret -1 = error, ret 0 = disconnected
		int32_t read(void* buffer, uint32_t maxLength)
		{
			int32_t bytesRecv = lwip_recv(m_socket, buffer, maxLength, 0);
			if (maxLength > 0)
				m_connected = (bytesRecv > 0);
			return bytesRecv;
		}
		
		// ret -1 = error, ret 0 = disconnected
		int32_t peek(void* buffer, uint32_t maxLength)
		{
			int32_t bytesRecv = lwip_recv(m_socket, buffer, maxLength, MSG_PEEK);
			if (maxLength > 0)
				m_connected = (bytesRecv > 0);
			return bytesRecv;
		}
		
		// buffer can stay in RAM of Flash
		// ret -1 = error, ret 0 = disconnected
		int32_t write(void const* buffer, uint32_t length)
		{
			static uint32_t const CHUNKSIZE = 256;
			
			int32_t bytesSent = 0;
			if (isStoredInFlash(buffer))
			{
				// copy from Flash, send in chunks of CHUNKSIZE bytes
				Ptr<uint8_t> rambuf(new uint8_t[length]);
				uint8_t const* src = (uint8_t const*)buffer;
				while (bytesSent < length)
				{
					uint32_t bytesToSend = min(CHUNKSIZE, length - bytesSent);
					f_memcpy(rambuf.get(), src, bytesToSend);			
					uint32_t chunkBytesSent = lwip_send(m_socket, rambuf.get(), bytesToSend, 0);
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
				bytesSent = lwip_send(m_socket, buffer, length, 0);
			}
			if (length > 0)
				m_connected = (bytesSent > 0);
			return bytesSent;
		}
		
		// str can stay in RAM of Flash
		// ret -1 = error, ret 0 = disconnected
		int32_t write(char const* str)
		{
			return write((uint8_t const*)str, f_strlen(str));
		}
		
		// like printf
		// buf can stay in RAM or Flash
		// "strings" of args can stay in RAM or Flash
		uint16_t MTD_FLASHMEM writeFmt(char const *fmt, ...)
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
		
		void MTD_FLASHMEM close()
		{
			if (m_socket > 0)
			{
				lwip_close(m_socket);
				m_socket = 0;
			}
			m_connected = false;
		}
		
		bool MTD_FLASHMEM isConnected()
		{
			return m_connected;
		}
		
		void MTD_FLASHMEM exec()
		{
			while (true)
			{
				if (m_socketQueue->receive(&m_socket))
				{
					m_connected = true;
					connectionHandler();
					close();
				}
			}
		}
		
		void MTD_FLASHMEM setNoDelay(bool value)
		{
			int32_t one = (int32_t)value;
			lwip_setsockopt(m_socket, IPPROTO_TCP, TCP_NODELAY, &one, sizeof(one));
		}
		
		// applications override this
		virtual void connectionHandler() = 0;
		
	private:
		int         m_socket;
		Queue<int>* m_socketQueue;
		bool        m_connected;
	};
    
	
	
	//////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////
	// HTTPHandler
	
	class HTTPHandler : public TCPConnectionHandler
	{
	
		static uint32_t const CHUNK_CAPACITY = 32;
	
	public:
	
		typedef IterDict<CharChunksIterator, CharChunksIterator> Fields;
		
		enum Method
		{
			Unsupported,
			Get,
			Post,
			Head,
		};
		
		struct Request
		{
			Method             method;	        // ex: GET, POST, etc...
			CharChunksIterator requestedPage;	// ex: "/", "/data"...						
			Fields             query;           // parsed query as key->value dictionary
			Fields             headers;		    // parsed headers as key->value dictionary
			Fields             form;			// parsed form fields as key->value dictionary
		};
				
		typedef void (HTTPHandler::*PageHandler)();
				
		struct Route
		{
			char const* page;
			PageHandler pageHandler;
		};

		
	private:
	
		// implements TCPConnectionHandler
		void MTD_FLASHMEM connectionHandler()
		{			
			while (isConnected())
			{
				CharChunk* chunk = m_receivedData.addChunk(CHUNK_CAPACITY);
				chunk->items = read(chunk->data, CHUNK_CAPACITY);
				if (processRequest())
					break;
			}
			m_receivedData.clear();
			m_request.query.clear();
			m_request.headers.clear();
			m_request.form.clear();
		}
		
		
		bool MTD_FLASHMEM processRequest()
		{
			// look for 0x0D 0x0A 0x0D 0x0A
			CharChunksIterator headerEnd = t_strstr(m_receivedData.getIterator(), CharChunksIterator(), CharIterator(FSTR("\x0D\x0A\x0D\x0A")));
			if (headerEnd.isValid())
			{
				// move header end after CRLFCRLF
				headerEnd += 4;
				
				CharChunksIterator curc = m_receivedData.getIterator();
				
				// extract method (GET, POST, etc..)				
				CharChunksIterator method = curc;
				while (curc != headerEnd && *curc != ' ')
					++curc;
				*curc++ = 0;	// ends method
				if (t_strcmp(method, CharIterator(FSTR("GET"))) == 0)
					m_request.method = Get;
				else if (t_strcmp(method, CharIterator(FSTR("POST"))) == 0)
					m_request.method = Post;
				else if (t_strcmp(method, CharIterator(FSTR("HEAD"))) == 0)
					m_request.method = Head;
				else
					m_request.method = Unsupported;
				
				// extract requested page and query parameters
				m_request.requestedPage = curc;
				while (curc != headerEnd)
				{
					if (*curc == '?')
					{
						*curc++ = 0;	// ends requestedPage
						curc = extractURLEncodedFields(curc, headerEnd, &m_request.query);
						break;
					}
					else if (*curc == ' ')
					{
						*curc++ = 0;	// ends requestedPage
						break;
					}
					++curc;
				}					
				
				// bypass HTTP version
				while (curc != headerEnd && *curc != 0x0D)
					++curc;
								
				// extract headers
				curc = extractHeaders(curc, headerEnd, &m_request.headers);
				
				// look for data (maybe POST data)
				Fields::Item* contentLengthStr = m_request.headers[FSTR("Content-Length")];
				if (contentLengthStr)
				{
					// download additional content
					int32_t contentLength = t_strtol(contentLengthStr->value, 10);
					int32_t missingBytes = headerEnd.getPosition() + contentLength - m_receivedData.getItemsCount();
					while (isConnected() && missingBytes > 0)
					{
						CharChunk* chunk = m_receivedData.addChunk(missingBytes);
						chunk->items = read(chunk->data, missingBytes);		
						missingBytes -= chunk->items;
					}
					m_receivedData.append(0);	// add additional terminating "0"
					// check content type
					Fields::Item* contentType = m_request.headers[FSTR("Content-Type")];
					if (contentType && t_strstr(contentType->value, CharIterator(FSTR("application/x-www-form-urlencoded"))).isValid())
					{
						CharChunksIterator contentStart = m_receivedData.getIterator();	// cannot use directly headerEnd because added data
						contentStart += headerEnd.getPosition();
						extractURLEncodedFields(contentStart, CharChunksIterator(), &m_request.form);
					}
				}
				
				dispatch();
				
				return true;
			}
			else
			{
				// header is not complete
				return false;
			}
		}

		
		CharChunksIterator extractURLEncodedFields(CharChunksIterator begin, CharChunksIterator end, Fields* fields)
		{
			CharChunksIterator curc = begin;
			CharChunksIterator key = curc;
			CharChunksIterator value;
			while (curc != end)
			{
				if (*curc == '=')
				{
					*curc = 0;	// ends key
					value = curc;
					++value;	// bypass '='
				}
				else if (*curc == '&' || *curc == ' ' || curc.isLast())
				{
					bool endLoop = (*curc == ' ' || curc.isLast());
					*curc++ = 0; // zero-ends value
					if (key.isValid() && value.isValid())
					{		
						fields->add(key, value);	 // store parameter
						key = value = CharChunksIterator(); // reset
					}
					if (endLoop)
						break;
					key = curc;
				}
				else
					++curc;
			}
			return curc;
		}
		

		CharChunksIterator extractHeaders(CharChunksIterator begin, CharChunksIterator end, Fields* fields)
		{		
			CharChunksIterator curc = begin;
			CharChunksIterator key;
			CharChunksIterator value;
			while (curc != end)
			{
				if (*curc == 0x0D && key.isValid() && value.isValid())  // CR?
				{
					*curc = 0;	// ends key
					// store header
					fields->add(key, value);
					key = value = CharChunksIterator(); // reset
				}					
				else if (!isspace(*curc) && !key.isValid())
				{
					// bookmark "key"
					key = curc;
				}
				else if (!value.isValid() && *curc == ':')
				{
					*curc++ = 0;	// ends value
					// bypass spaces
					while (curc != end && isspace(*curc))
						++curc;
					// bookmark value
					value = curc;
				}
				++curc;					
			}
			return curc;
		}
		
		
		virtual void MTD_FLASHMEM dispatch()
		{
			for (uint32_t i = 0; i != m_routesCount; ++i)
			{
				if (f_strcmp(FSTR("*"), m_routes[i].page) == 0 || t_strcmp(m_request.requestedPage, CharIterator(m_routes[i].page)) == 0)
				{
					(this->*m_routes[i].pageHandler)();
					return;
				}
			}
			// not found (routes should always have route "*" to handle 404 not found)
		}
		

	public:
		
		void MTD_FLASHMEM setRoutes(Route const* routes, uint32_t routesCount)
		{
			m_routes      = routes;
			m_routesCount = routesCount;
		}
		
		// valid only inside processRequest()
		Request& MTD_FLASHMEM getRequest()
		{
			return m_request;
		}
		
		
	private:
	
		LinkedCharChunks m_receivedData;
		Route const*     m_routes;
		uint32_t         m_routesCount;
		Request          m_request;		// valid only inside processRequest()
	};
	
	
	//////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////
	// HTTPResponse
	
	class HTTPResponse
	{
	public:
		typedef IterDict<CharIterator, CharIterator> Fields;
	
	
		HTTPResponse(HTTPHandler* httpHandler, char const* status, char const* content = NULL)
			: m_httpHandler(httpHandler), m_status(status)
		{
			// content (if present, otherwise use addContent())
			if (content)
				addContent(content);
		}
		
		HTTPHandler* MTD_FLASHMEM getHttpHandler()
		{
			return m_httpHandler;
		}
		
		HTTPHandler::Request& MTD_FLASHMEM getRequest()
		{
			return m_httpHandler->getRequest();
		}
		
		void MTD_FLASHMEM setStatus(char const* status)
		{
			m_status = status;
		}
		
		// accept RAM or Flash strings
		void MTD_FLASHMEM addHeader(char const* key, char const* value)
		{
			m_headers.add(key, value);
		}
		
		// accept RAM or Flash data
		// WARN: data is not copied! Just a pointer is stored
		void MTD_FLASHMEM addContent(void const* data, uint32_t length)
		{
			CharChunk* chunk = m_content.addChunk((char*)data, length, false);
		}
		
		// accept RAM or Flash strings
		// WARN: data is not copied! Just a pointer is stored
		// can be called many times
		void MTD_FLASHMEM addContent(char const* str)
		{
			addContent(str, f_strlen(str));
		}
				
		// can be called many times
		// WARN: src content is not copied! Just data pointers are stored
		void MTD_FLASHMEM addContent(LinkedCharChunks* src)
		{
			m_content.addChunks(src);
		}
				
		// should be called only after setStatus, addHeader and addContent
		virtual void MTD_FLASHMEM flush()
		{
			// status line
			m_httpHandler->writeFmt(FSTR("HTTP/1.1 %s\r\n"), m_status);
			// HTTPResponse headers
			addHeader(FSTR("Connection"), FSTR("close"));			
			// user headers
			for (uint32_t i = 0; i != m_headers.getItemsCount(); ++i)
			{
				Fields::Item* item = m_headers[i];
				m_httpHandler->writeFmt(FSTR("%s: %s\r\n"), Ptr<char>(t_strdup(item->key)).get(), Ptr<char>(t_strdup(item->value)).get());
			}
			// content length header
			m_httpHandler->writeFmt(FSTR("Content-Length: %d\r\n\r\n"), m_content.getItemsCount());
			// actual content
			if (m_content.getItemsCount() > 0)
			{				
				CharChunk* chunk = m_content.getFirstChunk();
				while (chunk)
				{
					m_httpHandler->write((uint8_t const*)chunk->data, chunk->items);
					chunk = chunk->next;
				}
				m_content.clear();
			}
		}
		
	private:
		HTTPHandler*     m_httpHandler;
		char const*      m_status;		
		Fields           m_headers;
		LinkedCharChunks m_content;
	};


	//////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////
	// HTTPStaticFileResponse
	struct HTTPStaticFileResponse : public HTTPResponse
	{
		template <typename Iterator>
		HTTPStaticFileResponse(HTTPHandler* httpHandler, Iterator filename)
			: HTTPResponse(httpHandler, NULL)
		{			
			if (*filename == '/')
				++filename;
			m_filename.reset(t_strdup(filename));
		}
		
		virtual void MTD_FLASHMEM flush()
		{
			char const* mimetype;
			void const* data;
			uint16_t dataLength;
			if (FlashFileSystem::find(m_filename.get(), &mimetype, &data, &dataLength))
			{
				// found				
				setStatus(FSTR("200 OK"));
				addHeader(FSTR("Content-Type"), mimetype);
				addContent(data, dataLength);
			}
			else
			{
				// not found
				setStatus(FSTR("404 Not Found"));
			}
			HTTPResponse::flush();
		}
		
	private:
	
		Ptr<char> m_filename;
	};


	//////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////
	// ParameterReplacer
	// If input contains {%..%} blocks only getBlocks() should be used and getResult() contains empty data.
	// If input contains only {{}} tags only getREsult() should be used.
	
	struct ParameterReplacer
	{
		typedef ObjectDict<LinkedCharChunks> Params;
		
		ParameterReplacer(char const* strStart, char const* strEnd, Params* params)
			: m_params(params), m_strStart(strStart), m_strEnd(strEnd)
		{
			processInput();
		}
		
		LinkedCharChunks* MTD_FLASHMEM getResult()
		{
			return &m_result;
		}
		
		Params* MTD_FLASHMEM getBlocks()
		{
			return &m_blocks;
		}
		
		char const* MTD_FLASHMEM getTemplateFilename()
		{
			return m_template.get();
		}
		
	private:
		
		void MTD_FLASHMEM processInput()
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
		
		char const* MTD_FLASHMEM replaceTag(char const* curc)
		{
			char const* tagEnd;
			char const* tagStart = extractTagStr(curc, &tagEnd);
			Params::Item* item = m_params->getItem(tagStart, tagEnd);
			if (item)
			{
				// push parameter content
				m_result.addChunks(&item->value);				
			}
			return tagEnd + 2;	// bypass "}}"
		}
		
		char const* extractTagStr(char const* curc, char const** tagEnd)
		{
			char const* tagStart = curc + 2; // by pass "{{" or "{%"
			*tagEnd = tagStart;
			while (*tagEnd < m_strEnd && getChar(*tagEnd) != '}' && getChar(*tagEnd) != '%')
				++*tagEnd;
			return tagStart;
		}
		
	private:
		Params*           m_params;
		char const*       m_strStart;
		char const*       m_strEnd;
		LinkedCharChunks  m_result;
		Params            m_blocks;
		Ptr<char>         m_template;	// template file name (filled with the first {%...%} block)
	};
	

	//////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////
	// HTTPTemplateResponse
	//
	// Parameters are tagged with: 
	//   {{parameter_name}}           
	// example: 
	//   {{something}}
	// Parameters cannot stay in template file.
	//
	// A template can only contain {{}} tags, which specify the blocks or parameters to replace. Example:
	// Content of file "base.html":
	//   <html>
	//   <head> {{the_head}} </head>
	//   <body> {{the_body}} </body>
	//   </html>
	//
	// Specialized file can replace template blocks using {%block_name%}. Example:
	// Content of file "home.html":
	//   {%base.html%}                 <- first block specifies the tamplate filename
	//   {%the_head%}                  <- start of "the_head" block
	//   <title>Home Page</title>
	//   {%the_body%}                  <- start of "the_body" block
	//   <h1>Hello {{name}}</h1>       <- here the parameter "name" will be replaced with its actual value
	//
	// Specialized file must contain at least one {%%} tag, and may or may not contain {{}} tags (parameters).
	// Parameter tags and block tags must have different names.
	// The template file can contain parameters that will be replaced as they was in the specialized file.
	//
	// No further spaces are allowed inside {{}} and {%%} tags
	// No syntax error checkings are done, so be careful!
	// Only one level of inheritance is allowed (specialized file -> template file)
	
	struct HTTPTemplateResponse : public HTTPResponse
	{
		typedef ObjectDict<LinkedCharChunks> Params;

		HTTPTemplateResponse(HTTPHandler* httpHandler, char const* filename)
			: HTTPResponse(httpHandler, NULL), m_filename(filename)
		{			
		}
		
		void setFilename(char const* filename)
		{
			m_filename = filename;
		}
		
		void MTD_FLASHMEM addParam(char const* key, char const* value)
		{
			LinkedCharChunks linkedCharChunks;
			linkedCharChunks.addChunk(value, f_strlen(value), false);
			m_params.add(key, linkedCharChunks);
		}
		
		void MTD_FLASHMEM addParam(char const* key, LinkedCharChunks* value)
		{
			m_params.add(key, *value);
		}
		
		void MTD_FLASHMEM addParams(Params* params)
		{
			m_params.add(params);
		}
		
		Params* MTD_FLASHMEM getParams()
		{
			return &m_params;
		}
		
		virtual void MTD_FLASHMEM flush()
		{
			processFileRequest();
			HTTPResponse::flush();
		}
		
	private:
	
		void MTD_FLASHMEM processFileRequest()
		{
			char const* mimetype;
			void const* data;
			uint16_t dataLength;
			if (m_filename && FlashFileSystem::find(m_filename, &mimetype, &data, &dataLength))
			{
				// found
				setStatus(FSTR("200 OK"));
				addHeader(FSTR("Content-Type"), FSTR("text/html"));
				
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
			setStatus(FSTR("404 Not Found"));
		}
		
	private:
		char const*      m_filename;
		Params           m_params;		
	};





	
}	// fdv namespace


#endif






