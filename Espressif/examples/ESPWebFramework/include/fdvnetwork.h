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
			strcpy((char *)config.ssid, SSID);
			config.ssid_len = strlen(SSID);
			strcpy((char *)config.password, securityKey);
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
			strcpy((char *)config.ssid, SSID);
			strcpy((char *)config.password, securityKey);
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
	
		// don't need to call configureDHCP
		static void MTD_FLASHMEM configureStatic(Network network, char const* IP, char const* netmask, char const* gateway)
		{
			ip_info info;
			info.ip.addr      = ipaddr_addr(IP);
			info.netmask.addr = ipaddr_addr(netmask);
			info.gw.addr      = ipaddr_addr(gateway);
			Critical critical;
			if (network == ClientNetwork)
				wifi_station_dhcpc_stop();
			wifi_set_ip_info(network, &info);
		}
		
		// applied only to ClientNetwork
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
			info.start_ip      = ipaddr_addr(startIP);
			info.end_ip        = ipaddr_addr(endIP);
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
			void const* ramBuffer = buffer;
			if (isStoredInFlash(buffer))
				ramBuffer = f_memdup(buffer, length);
			int32_t bytesSent = lwip_send(m_socket, ramBuffer, length, 0);
			if (bytesSent > 0)
				m_connected = (bytesSent > 0);
			if (ramBuffer != buffer)
				delete[] (uint8_t*)ramBuffer;
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
	
		typedef ChunkedBuffer<char> Chunks;
		
		typedef Chunks::Iterator ChunkString;
		
		typedef IterDict<Chunks::Iterator, Chunks::Iterator> Fields;
	
	
		// implements TCPConnectionHandler
		void MTD_FLASHMEM connectionHandler()
		{			
			while (isConnected())
			{
				Chunks::Chunk* chunk = m_chunks.addChunk(CHUNK_CAPACITY);
				chunk->items = read(chunk->data, CHUNK_CAPACITY);
				if (processRequest())
					break;
			}
			m_chunks.clear();
			m_request.query.clear();
			m_request.headers.clear();
			m_request.form.clear();
		}
		
		
		bool MTD_FLASHMEM processRequest()
		{
			// look for 0x0D 0x0A 0x0D 0x0A
			ChunkString headerEnd = t_strstr(m_chunks.getIterator(), ChunkString(), CharIterator(FSTR("\x0D\x0A\x0D\x0A")));
			if (headerEnd)
			{
				// move header end after CRLFCRLF
				headerEnd += 4;
				
				ChunkString curc = m_chunks.getIterator();
				
				// extract method (GET, POST, etc..)				
				m_request.method = curc;
				while (curc != headerEnd && *curc != ' ')
					++curc;
				*curc++ = 0;	// ends method				
				
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
				ChunkString contentLengthStr = m_request.headers[FSTR("Content-Length")];
				if (contentLengthStr)
				{
					// download additional content
					int32_t contentLength = t_strtol(contentLengthStr, 10);
					int32_t missingBytes = headerEnd.getPosition() + contentLength - m_chunks.getItemsCount();
					while (isConnected() && missingBytes > 0)
					{
						Chunks::Chunk* chunk = m_chunks.addChunk(missingBytes);
						chunk->items = read(chunk->data, missingBytes);		
						missingBytes -= chunk->items;
					}
					m_chunks.append(0);	// add additional terminating "0"
					// check content type
					ChunkString contentType = m_request.headers[FSTR("Content-Type")];
					if (contentType && t_strstr(contentType, CharIterator(FSTR("application/x-www-form-urlencoded"))))
					{
						ChunkString contentStart = m_chunks.getIterator();	// cannot use directly headerEnd because added data
						contentStart += headerEnd.getPosition();
						extractURLEncodedFields(contentStart, ChunkString(), &m_request.form);
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

		
		ChunkString extractURLEncodedFields(ChunkString begin, ChunkString end, Fields* fields)
		{
			ChunkString curc = begin;
			ChunkString key = curc;
			ChunkString value;
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
					if (key && value)
					{		
						fields->add(key, value);	 // store parameter
						key = value = ChunkString(); // reset
					}
					if (*curc == ' ' || curc.isLast())
					{
						*curc++ = 0; // ends value or requested page
						break;
					}
					*curc = 0;	// ends value or requested page
					key = curc;	// bypass '&'
					++key;
				}
				++curc;
			}
			return curc;
		}
		

		ChunkString extractHeaders(ChunkString begin, ChunkString end, Fields* fields)
		{		
			ChunkString curc = begin;
			ChunkString key;
			ChunkString value;
			while (curc != end)
			{
				if (*curc == 0x0D && key && value)  // CR?
				{
					*curc = 0;	// ends key
					// store header
					fields->add(key, value);
					key = value = ChunkString(); // reset
				}					
				else if (!isspace(*curc) && !key)
				{
					// bookmark "key"
					key = curc;
				}
				else if (!value && *curc == ':')
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
		
		struct Request
		{
			ChunkString method;	        // ex: GET, POST, etc...
			ChunkString requestedPage;	// ex: "/", "/data"...						
			Fields      query;          // parsed query as key->value dictionary
			Fields      headers;		// parsed headers as key->value dictionary
			Fields      form;			// parsed form fields as key->value dictionary
		};
				
		typedef void (HTTPHandler::*PageHandler)();
				
		struct Route
		{
			char const* page;
			PageHandler pageHandler;
		};
		
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
	
		Chunks       m_chunks;
		Route const* m_routes;
		uint32_t     m_routesCount;
		Request      m_request;		// valid only inside processRequest()
	};
	
	
	//////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////
	// HTTPResponse
	
	class HTTPResponse
	{
	public:
		typedef ChunkedBuffer<char> Chunks;
	
	
		HTTPResponse(HTTPHandler* httpHandler, char const* status, char const* content = NULL)
			: m_httpHandler(httpHandler)
		{
			// status line
			m_httpHandler->writeFmt(FSTR("HTTP/1.0 %s\r\n"), status);
			// default headers
			addHeader(FSTR("Connection"), FSTR("close"));
			addHeader(FSTR("Content-Type"), FSTR("text/html; charset=UTF-8"));
			// content (if present, otherwise use addContent())
			if (content)
				addContent(content);
		}
		
		~HTTPResponse()
		{
			flush();
		}
		
		// accept RAM or Flash strings
		void MTD_FLASHMEM addHeader(char const* key, char const* value)
		{
			m_httpHandler->writeFmt(FSTR("%s: %s\r\n"), key, value);
		}
		
		// accept RAMT or Flash data
		// WARN: data is not copied! Just a pointer is stored
		void MTD_FLASHMEM addContent(void const* data, uint32_t length)
		{
			Chunks::Chunk* chunk = m_chunks.addChunk((char*)data, length, false);
		}
		
		// accept RAMT or Flash strings
		// WARN: data is not copied! Just a pointer is stored
		// can be called many times
		void MTD_FLASHMEM addContent(char const* str)
		{
			addContent(str, f_strlen(str));
		}
				
		// should be called only at the end of addContent calls
		void MTD_FLASHMEM flush()
		{
			if (m_chunks.getItemsCount() > 0)
			{
				// write content length header
				m_httpHandler->writeFmt(FSTR("Content-Length: %d\r\n\r\n"), m_chunks.getItemsCount());
				
				// write data
				Chunks::Chunk* chunk = m_chunks.getFirstChunk();
				while (chunk)
				{
					m_httpHandler->write((uint8_t const*)chunk->data, chunk->items);
					chunk = chunk->next;
				}
				m_chunks.clear();
			}
		}
		
	private:
		HTTPHandler* m_httpHandler;
		Chunks       m_chunks;
	};
	
}


#endif






