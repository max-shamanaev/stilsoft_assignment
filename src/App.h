#pragma once

#include "HTTPClient.h"

#include <chrono>
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

		// IP-адрес хоста / его доменное имя
		const std::string hostAddress{ "example.org" };

		// Таймауты для операций коммуникации с сервером
		// (0 для fetch ввиду п.6 задания)
		const std::chrono::milliseconds sendTimeout { 500 };
		const std::chrono::microseconds fetchTimeout{ 0 };

		// Контейнер для полученных от сервера данных
		std::vector<char> data{};

		// Синхронизация потоков 
		bool stop{ false };
		std::mutex dataMutex{};
		std::condition_variable stateChange{};
	};
}