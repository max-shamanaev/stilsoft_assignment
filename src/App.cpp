#include "App.h"
#include "HTTPClient.h"
#include "Utils.h"

#include <WinUser.h>

#include <functional>
#include <iostream>
#include <iterator>
#include <memory>
#include <string>
#include <thread>
#include <vector>

namespace wsApp
{
	void App::run()
	{
		std::unique_ptr<wsApp::HTTPClient> client{ 
			std::make_unique<wsApp::HTTPClient>()
		};

		client->connect(hostAddress);

		if (!client->isConnected())
			return;

		std::string request{ client->formatRequest(wsApp::RequestMethods::HEAD) };

		// Таймаут (в микросек) для того, чтобы данные
		// медленнее заносились в консоль и были более читаемы
		// (по умолчанию 0 ввиду п.6 задания)
		client->setFetchTimeout(0);
	
		std::thread queryThread{ &App::queryData, this,
			std::ref(*client), request
		};
		
		std::thread handleThread{ &App::handleData, this };

		// Программа бесконечно выполняется до нажатия ESC
		while (true)
		{
			if (GetAsyncKeyState(VK_ESCAPE) & 0x01)
			{
				stop = true;
				break;
			}
		}

		queryThread.join();
		handleThread.join();
		Log::info("Exited");
	}

	// Обработка поступаемого потока данных
	void App::handleData()
	{
		std::unique_lock<std::mutex> dataLock{ dataMutex, std::defer_lock };
		size_t count{ 1 };
		while (!stop)
		{
			dataLock.lock();

			// Контроль наличия считанных из сокета данных
			while (data.empty() && !stop)
				dmCondition.wait(dataLock);

			// Вывод данных в консоль
			Log::info("Query #" + std::to_string(count++));

			Log::getMutex().lock();
			std::cout.write(data.data(), data.size());
			std::cout << std::endl;
			Log::getMutex().unlock();

			// Удаление выведенных данных из контейнера
			data.clear();

			dataLock.unlock();
			dmCondition.notify_one();
		}
	}

	// Бесконечная отправка http-запросов и
	// получение результатов из сокета
	void App::queryData(HTTPClient& connectedClient, const std::string& request)
	{
		// Проверка установленного соединения опущена.
		// Ответственность возлагается на коллера и 
		// реализацию send-fetch методов, т.к.
		// данную ошибку можно расценивать как
		// фатальную/нефатальную

		std::unique_lock<std::mutex> dataLock{ dataMutex, std::defer_lock };
		while (!stop)
		{
			connectedClient.sendRequest(request);

			dataLock.lock();
			while (!data.empty() && !stop)
				dmCondition.wait(dataLock);

			connectedClient.fetchResponse(data);

			dataLock.unlock();
			dmCondition.notify_one();
		}
	}
}