#pragma once

#include "HTTPClient.h"

#include <condition_variable>
#include <mutex>
#include <string>
#include <vector>

namespace wsApp
{
	// Класс программы, реализующей функционал http-клиента с использованием
	// нескольких синхронизированных потоков, взаимодействующих между собой
	//
	class App
	{
	public:
		void run();

	private:
		// Обработка данных с сервера
		void handleData();

		// Запрос данных у сервера
		void queryData(HTTPClient& connectedClient, const std::string& request);

		// Доменное имя веб-сервера (вкл. 3 уровень) / его IP-адрес
		const std::string hostAddress{ "www.google.com" };

		// Контейнер для полученных от сервера данных
		std::vector<char> data{};

		bool stop{ false };
		std::mutex dataMutex{};
		std::condition_variable dmCondition{};
	};
}