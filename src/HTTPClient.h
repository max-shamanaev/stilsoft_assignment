#pragma once

#include "Utils.h"

#include <WinSock2.h>

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace wsApp
{
	// Класс представляет собой http-клиента, взаимодействующего с каким-либо
	// сервером и получающего от него данные по запросу.
	// 
	// Ошибки обрабатываются без помощи исключений
	//
	// В реализации используется протокол TCP, потоковый тип сокета, и
	// неблокирующий режим сокета
	//
	class HTTPClient
	{
	public:
		HTTPClient(size_t buffLen = 4096);
		~HTTPClient();

		void connect(std::string_view address);
		void disconnect();
		int  sendRequest(std::string_view request) const;
		int  fetchResponse(std::vector<char>& dest);

		std::string formatRequest(RequestMethods requestMethod,
			std::string_view resPath = "/") const;

		void setFetchTimeout(std::uint64_t usTimeout);
		bool isConnected() const { return connected; }

	private:
		WSAHandler wsa{};

		// Поддерживаемые клиентом параметры сокета
		addrinfo hints{};

		// Каноническое имя домена
		std::string hostСName{};

		SOCKET connectSocket{ INVALID_SOCKET };
		bool nonblocking{ true };
		bool connected{ false };

		timeval fetchTimeout{ 0, 0 };
		const size_t buffLength{ 4096 };
		std::vector<char> buffer{};
	};
}