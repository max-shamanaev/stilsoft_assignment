#pragma once

#include "HTTPClient.h"

#include <chrono>
#include <condition_variable>
#include <mutex>
#include <string>
#include <vector>

namespace wsApp
{
	class App
	{
	public:
		void run();

	private:
		// Обработка данных с сервера
		void handleData();

		// Запрос данных у сервера
		void queryData(HTTPClient& connectedClient, const std::string& request);

		// Доменное имя веб-сервера / его IP-адрес
		const std::string hostAddress{ "www.google.com" };

		// Контейнер для полученных от сервера данных
		std::vector<char> data{};

		bool stop{ false };
		std::mutex dataMutex{};
		std::condition_variable dmCondition{};
	};
}