#pragma once

#include <WinSock2.h>

#include <cassert>
#include <string_view>
#include <iostream>

namespace wsApp
{
	enum class RequestMethods
	{
		GET,
		HEAD,
		OPTIONS,
	};

	inline std::string_view rmtosv(RequestMethods method)
	{
		switch (method)
		{
		case RequestMethods::GET:
			return "GET";
		case RequestMethods::HEAD:
			return "HEAD";
		case RequestMethods::OPTIONS:
			return "OPTIONS";
		default:
			assert("Unimplemented conversion");
			return "UNDEFINED";
		}
	}

	inline void logError(std::string_view errMessage, int errCode) 
	{
		std::cerr << "[ERROR " << errCode << "]: " << errMessage << '\n';
	}

	inline void logInfo(std::string_view message)
	{
		std::cout << "[INFO] " << message << '\n';
	}

	class WSAHandler
	{
	public:
		WSAHandler() 
		{
			auto errCode{ WSAStartup(MAKEWORD(2, 2), &wsaData) };
			if (errCode)
				logError("Couldn't find usable WinSock DLL", errCode);
		}

		~WSAHandler()
		{
			WSACleanup(); 
		}

		const WSADATA& get() const { return wsaData; }

	private:
		WSADATA wsaData{};
	};
}