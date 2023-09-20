#include "HTTPClient.h"

#include <Winerror.h>
#include <WS2tcpip.h>

#include <iostream>
#include <string>
#include <sstream>

namespace wsApp
{
	HTTPClient::HTTPClient()
	{
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
		}

		freeaddrinfo(hostInfoList);
	}

	void HTTPClient::disconnect()
	{
		closesocket(connectSocket);
		connected = false;
	}

	int HTTPClient::sendRequest(RequestMethods requestMethod, std::string_view resPath) const
	{
		if (!connected)
		{
			logError("Socket is not connected in order to send request", WSAENOTCONN);
			return 0;
		}

		std::stringstream request{};
		request << rmtosv(requestMethod) << ' ' << resPath << " HTTP/1.1\r\nHost: "
			<< hostName << "\r\n\r\n";

		int bytesSent{ send(
			connectSocket, 
			request.str().c_str(), 
			request.str().size(), 0) };

		if (bytesSent == SOCKET_ERROR)
		{
			logError("Sending request failed", WSAGetLastError());
			return 0;
		}
		else
			return bytesSent;
			
	}

	int HTTPClient::fetchResponse(std::vector<char>& outBuffer) const
	{
		if (!connected)
		{
			logError("Socket is not connected in order to fetch response", WSAENOTCONN);
			return 0;
		}

		int bytesRecieved{ recv(
			connectSocket, 
			outBuffer.data(), 
			outBuffer.size(), 0) };
		
		if (bytesRecieved == SOCKET_ERROR)
		{
			logError("Fetching response failed", WSAGetLastError());
			return 0;
		}

		return bytesRecieved;
	}
	
}