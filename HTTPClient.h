#pragma once

#include "Utils.h"

#include <WinSock2.h>

#include <string>
#include <string_view>
#include <vector>

namespace wsApp
{
	class HTTPClient
	{
	public:
		HTTPClient();
		~HTTPClient();

		void connect(std::string_view address);
		void disconnect();
		int  sendRequest(RequestMethods requestMethod, std::string_view resPath = "/") const;
		int  fetchResponse(std::vector<char>& outBuffer) const;

		bool isConnected() const { return connected; }

	private:
		WSAHandler wsa{};

		addrinfo hints{};
		std::string hostName{};

		SOCKET connectSocket{ INVALID_SOCKET };
		bool connected{ false };
	};
}