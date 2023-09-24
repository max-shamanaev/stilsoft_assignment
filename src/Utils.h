#pragma once

#include <WinSock2.h>

#include <cassert>
#include <string_view>
#include <iostream>
#include <mutex>

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
			return "";
		}
	}

	// Thread-safe базовый лог
	// 
	// Примечание: используются ескейп-коды
	// для раскрашивания лог сообщений, которые
	// могут не поддерживаться некоторыми терминалами
	class Log
	{
	public:
		static void error(std::string_view errMessage, int errCode)
		{
			std::lock_guard<std::mutex> lock{ logMutex };
			std::cout << "\033[31m[ERROR " << errCode << "]: "
				<< errMessage << "\033[0m\n";
		}

		static void info(std::string_view message)
		{
			std::lock_guard<std::mutex> lock{ logMutex };
			std::cout << "\033[32m[INFO] " << message << "\033[0m\n";
		}

	private:
		static inline std::mutex logMutex{};
	};

	// RAII-style обертка для автоматической
	// инициализации и очистки winsock dll
	// для каждого HTTPClient-а
	class WSAHandler
	{
	public:
		WSAHandler() 
		{
			auto errCode{ WSAStartup(MAKEWORD(2, 2), &wsaData) };
			assert(!errCode && "Couldn't find usable WinSock DLL");
		}

		~WSAHandler()
		{
			WSACleanup(); 
		}

	private:
		WSADATA wsaData{};
	};
}