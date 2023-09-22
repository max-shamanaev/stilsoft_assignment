#include "HTTPClient.h"
#include "Utils.h"

#include <WinSock2.h>
#include <Winerror.h>
#include <WS2tcpip.h>

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <iterator>
#include <string>
#include <sstream>

namespace wsApp
{
	HTTPClient::HTTPClient(size_t buffLen)
		: buffLength{ buffLen }, buffer(buffLen)
	{
		assert(buffLen != 0 && "Buffer length was set to 0");

		ZeroMemory(&hints, sizeof(hints));
		hints.ai_flags    = AI_CANONNAME;
		hints.ai_family   = AF_UNSPEC;		
		hints.ai_socktype = SOCK_STREAM;	
		hints.ai_protocol = IPPROTO_TCP;
	}

	HTTPClient::~HTTPClient()
	{
		closesocket(connectSocket);
	}

	void HTTPClient::connect(std::string_view address)
	{
		if (connected)
		{
			Log::error("The socket is already connected", WSAEISCONN);
			return;
		}

		addrinfo* hostInfoList{ nullptr };
		auto errCode{ getaddrinfo(address.data(), "http", &hints, &hostInfoList) };

		if (errCode)
		{
			Log::error("Failed to getaddrinfo()", errCode);
			return;
		}

		addrinfo* hostInfo{ hostInfoList };

		// CNAME возвращается только для первого узла в списке
		hostСName = hostInfo->ai_canonname; 

		while (hostInfo != nullptr)
		{
			connectSocket = socket(
				hostInfo->ai_family, 
				hostInfo->ai_socktype,
				hostInfo->ai_protocol);

			if (connectSocket == INVALID_SOCKET)
			{
				Log::error("Failed to create socket", WSAGetLastError());
				return;
			}

			errCode = ::connect(
				connectSocket, 
				hostInfo->ai_addr,
				static_cast<int>(hostInfo->ai_addrlen));

			if (errCode == SOCKET_ERROR)
			{
				closesocket(connectSocket);
				connectSocket = INVALID_SOCKET;
				hostInfo = hostInfo->ai_next;
			}
			else
				break;
		}

		if (connectSocket == INVALID_SOCKET)
			Log::error("Unable to connect to the server!", WSAGetLastError());
		else
		{
			Log::info("Successfully connected to " + hostСName);

			connected = true;

			// включение неблокирующего режима для сокета
			errCode = ioctlsocket(connectSocket, FIONBIO,
				reinterpret_cast<u_long*>(&nonblocking));

			if (errCode == SOCKET_ERROR)
				Log::error("Error enabling non-blocking mode", WSAGetLastError());
		}

		// Информация, возвращаемая getaddrinfo() динамически размещена
		// и требует очистки
		freeaddrinfo(hostInfoList);
	}

	void HTTPClient::disconnect()
	{
		closesocket(connectSocket);
		connected = false;
	}

	int HTTPClient::sendRequest(std::string_view request) const
	{
		assert(connected && "sendRequest() on unconnected socket");

		int bytesSent{ send(
		   connectSocket,
		   request.data(),
		   request.size(), 0) };

		if (bytesSent == SOCKET_ERROR)
		{
			if (WSAGetLastError() != WSAEWOULDBLOCK)
				Log::error("Sending request failed", WSAGetLastError());

			return 0;
		}
		else
			return bytesSent;
			
	}

	int HTTPClient::fetchResponse(std::vector<char>& dest)
	{
		assert(connected && "fetchResponse() on unconnected socket");

		std::string response{};
		int bytesRecieved{ 0 };
		int descRdy{ 0 }, recvRes{ 0 };
		fd_set readfds{};

		do
		{
			// Определение готовности сокета к чтению.
			// non-blocking мод и select() позволяют определить, когда
			// передача от хоста завершена
			FD_ZERO(&readfds);
			FD_SET(connectSocket, &readfds);
			descRdy = select(0, &readfds, nullptr, nullptr, &fetchTimeout);

			if (descRdy == SOCKET_ERROR)
			{
				Log::error("Failed to determine socket's status", WSAGetLastError());
				return 0;
			}

			if (FD_ISSET(connectSocket, &readfds))
			{
				recvRes = recv(connectSocket, buffer.data(), buffer.size(), 0);

				if (recvRes == SOCKET_ERROR)
				{
					Log::error("Fetching response failed", WSAGetLastError());
					return 0;
				}
				else if (recvRes == 0)
					break;

				response += buffer.data();
				bytesRecieved += recvRes;
			}

		} while (descRdy > 0);

		// Перенос ответа в предоставленный аргументом контейнер
		dest.resize(response.size());
		std::move(response.begin(), response.end(), dest.begin());

		return bytesRecieved;
	}

	std::string HTTPClient::formatRequest(RequestMethods requestMethod,
		std::string_view resPath) const
	{
		std::stringstream request{};
		request << rmtosv(requestMethod) << ' ' << resPath << " HTTP/1.1\r\nHost: "
			<< hostСName << "\r\n\r\n";

		return request.str();
	}

	void HTTPClient::setFetchTimeout(std::uint64_t usTimeout)
	{
		fetchTimeout.tv_sec  = static_cast<long>(usTimeout / 1'000'000);
		fetchTimeout.tv_usec = static_cast<long>(usTimeout % 1'000'000);
	}
}