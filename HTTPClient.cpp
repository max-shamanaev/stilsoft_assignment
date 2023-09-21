#include "HTTPClient.h"

#include <Winerror.h>
#include <WS2tcpip.h>

#include <algorithm>
#include <cassert>
#include <iostream>
#include <iterator>
#include <string>
#include <sstream>

namespace wsApp
{
	HTTPClient::HTTPClient(size_t buffLen)
		: buffLength{ buffLen }, buffer(buffLen)
	{
		assert(buffLen != 0 && "Buffer length was set to 0");

		// ѕараметры адреса хоста, которые клиент ожидает
		ZeroMemory(&hints, sizeof(hints));
		hints.ai_flags    = AI_CANONNAME;
		hints.ai_family   = AF_UNSPEC;		// IPv4/IPv6
		hints.ai_socktype = SOCK_STREAM;	// TCP
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
			logError("The socket is already connected", WSAEISCONN);
			return;
		}

		addrinfo* hostInfoList{ nullptr };
		auto errCode{ getaddrinfo(address.data(), "http", &hints, &hostInfoList) };

		if (errCode)
		{
			logError("Failed to getaddrinfo()", errCode);
			return;
		}

		addrinfo* hostInfo{ hostInfoList };
		hostName = hostInfo->ai_canonname;

		while (hostInfo != nullptr)
		{
			connectSocket = socket(
				hostInfo->ai_family, 
				hostInfo->ai_socktype,
				hostInfo->ai_protocol);

			if (connectSocket == INVALID_SOCKET)
			{
				logError("Failed to create socket", WSAGetLastError());
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
			logError("Unable to connect to the server!", WSAGetLastError());
		else
		{
			std::cout << "Successfully connected to " << hostName << '\n';

			connected = true;
			errCode = ioctlsocket(connectSocket, FIONBIO,
				reinterpret_cast<u_long*>(&nonblocking));

			if (errCode == SOCKET_ERROR)
				logError("Error enabling non-blocking mode", WSAGetLastError());
		}

		freeaddrinfo(hostInfoList);
	}

	void HTTPClient::disconnect()
	{
		closesocket(connectSocket);
		connected = false;
	}

	int HTTPClient::sendRequest(std::string_view request) const
	{
		if (!connected)
		{
			logError("Socket is not connected in order to send request", WSAENOTCONN);
			return 0;
		}

		int bytesSent{ send(
			connectSocket, 
			request.data(), 
			request.size(), 0) };

		if (bytesSent == SOCKET_ERROR)
		{
			logError("Sending request failed", WSAGetLastError());
			return 0;
		}
		else
			return bytesSent;
			
	}

	int HTTPClient::fetchResponse(std::vector<char>& dest)
	{
		if (!connected)
		{
			logError("Socket is not connected in order to fetch response", WSAENOTCONN);
			return 0;
		}

		std::string response{};
		int bytesRecieved{ 0 };
		int descRdy{ 0 }, recvRes{ 0 };
		fd_set readfds{};

		// ѕри слишком низком таймауте сокет не
		// определ€етс€ как готовый к чтению
		timeval timeout{ 0, 100'000 }; // 100ms

		do
		{
			FD_ZERO(&readfds);
			FD_SET(connectSocket, &readfds);
			descRdy = select(0, &readfds, nullptr, nullptr, &timeout);

			if (descRdy == SOCKET_ERROR)
			{
				logError("Failed to determine socket's status", WSAGetLastError());
				return 0;
			}

			if (FD_ISSET(connectSocket, &readfds))
			{
				recvRes = recv(connectSocket, buffer.data(), buffer.size(), 0);

				if (recvRes == SOCKET_ERROR)
				{
					logError("Fetching response failed", WSAGetLastError());
					return 0;
				}
				else if (recvRes == 0)
					break;

				response += buffer.data();
				bytesRecieved += recvRes;
			}

		} while (descRdy > 0);

		std::move(response.begin(), response.end(), std::back_inserter(dest));

		return bytesRecieved;
	}
	

	std::string HTTPClient::formatRequest(RequestMethods requestMethod,
		std::string_view resPath) const
	{
		std::stringstream request{};
		request << rmtosv(requestMethod) << ' ' << resPath << " HTTP/1.1\r\nHost: "
			<< hostName << "\r\n\r\n";

		return request.str();
	}
}