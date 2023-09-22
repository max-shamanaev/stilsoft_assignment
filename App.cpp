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

		std::string request{ client->formatRequest(wsApp::RequestMethods::HEAD) };

		std::thread queryThread{ &App::queryData, this,
			std::ref(*client), std::ref(request)
		};
		
		std::thread handleThread{ &App::handleData, this };

		// ��������� ���������� ����������� �� ������� ESC
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
	}

	void App::handleData()
	{
		std::unique_lock<std::mutex> lock{ dataMutex, std::defer_lock };
		size_t count{ 1 };
		while (!stop)
		{
			lock.lock();

			// �������� ������� ��������� �� ������ ������
			dmCondition.wait(lock, [this] { return !data.empty(); });

			// ����� ������ � �������
			logInfo("Query #" + std::to_string(count++));
			std::copy(data.begin(), data.end(), std::ostream_iterator<char>(std::cout));

			// �������� ���������� ������
			data.clear(); 

			lock.unlock();
			dmCondition.notify_one();
		}
	}

	void App::queryData(HTTPClient& connectedClient, const std::string& request)
	{
		// �������� �������������� ���������� �������.
		// ��������������� ����������� �� ������� � 
		// ���������� send-fetch �������, �.�.
		// ������ ������ ����� ����������� ���
		// ���������/�����������

		// ����������� �������� http-�������� �
		// ��������� ����������� �� ������
		std::unique_lock<std::mutex> lock{ dataMutex, std::defer_lock };
		while (!stop)
		{
			connectedClient.sendRequest(request);

			lock.lock();
			dmCondition.wait(lock, [this] { return data.empty(); });

			connectedClient.fetchResponse(data);

			lock.unlock();
			dmCondition.notify_one();
		}
	}
}