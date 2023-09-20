#include <WinSock2.h>
#include <WS2tcpip.h>

#include <iostream>
#include <string>
#include <vector>

namespace config
{
	constexpr char hostName[]{ "google.com" };
	constexpr char hostPort[]{ "80" };
	constexpr size_t buffSize{ 4 * 1024 };
}

int main()
{
	WSADATA wsaData{};
	int statusCode{ WSAStartup(MAKEWORD(2, 2), &wsaData) };

	if (statusCode)
	{
		std::cerr << "WSAStartup() failed with error code: " << statusCode << '\n';
		return 1;
	}

	// ѕараметры адреса хоста, которые клиент ожидает
	addrinfo hints;
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family   = AF_UNSPEC;		// IPv4/IPv6
	hints.ai_socktype = SOCK_STREAM;	// TCP
	hints.ai_protocol = IPPROTO_TCP;	

	addrinfo* hostInfoList{ nullptr };
	statusCode = getaddrinfo(config::hostName, config::hostPort, &hints, &hostInfoList);

	if (statusCode)
	{
		std::cerr << "getaddrinfo() failed with error code: " << statusCode << '\n';
		WSACleanup();
		return 1;
	}

	SOCKET connectSocket{ INVALID_SOCKET };
	for (auto* iter{ hostInfoList }; iter != nullptr; iter = iter->ai_next)
	{
		connectSocket = socket(iter->ai_family, iter->ai_socktype, iter->ai_protocol);

		if (connectSocket == INVALID_SOCKET)
		{
			std::cerr << "socket() failed wirth error code: " << WSAGetLastError() << '\n';
			WSACleanup();
			return 1;
		}

		statusCode = connect(connectSocket, iter->ai_addr, 
			static_cast<int>(iter->ai_addrlen));

		if (statusCode == SOCKET_ERROR)
		{
			closesocket(connectSocket);
			connectSocket = INVALID_SOCKET;
			continue;
		}
		
		break;
	}

	freeaddrinfo(hostInfoList);

	if (connectSocket == INVALID_SOCKET)
	{
		std::cerr << "Unable to connect to server!\n";
		WSACleanup();
		return 1;
	}
	else
		std::cout << "Successfully connected to server!\n";

	const std::string httpRequest{ "HEAD / HTTP/1.1\r\nHost: www." 
		+ std::string(config::hostName) 
		+ std::string("\r\n\r\n") };

	statusCode = send(connectSocket, httpRequest.c_str(), httpRequest.size(), 0);
	
	if (statusCode == SOCKET_ERROR)
	{
		std::cerr << "send() failed with error code: " << WSAGetLastError() << '\n';
		closesocket(connectSocket);
		WSACleanup();
		return 1;
	}

	std::vector<char> recvBuffer(config::buffSize);
	statusCode = recv(connectSocket, recvBuffer.data(), recvBuffer.size(), 0);

	if (statusCode > 0)
		std::cout << "Recieved bytes: " << statusCode << '\n';
	else if (statusCode == 0)
		std::cout << "Connection closed\n";
	else
		std::cerr << "recv() failed with error code: " << WSAGetLastError() << '\n';

	std::cout << '\n' << recvBuffer.data() << '\n';

	closesocket(connectSocket);
	WSACleanup();

	return 0;
}