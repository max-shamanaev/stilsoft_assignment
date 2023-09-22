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

		// ������� (� ��������) ��� ����, ����� ������
		// ��������� ���������� � ������� � ���� ����� �������
		// (�� ��������� 1 ����� �.6 �������)
		// 
		// ��� ������ �������, ��� ���������� ���������� ���
		// �������������� ������������� ������ � �������
		client->setFetchTimeout(1);

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
		Log::info("Exited");
	}

	// ��������� ������������ ������ ������
	void App::handleData()
	{
		std::unique_lock<std::mutex> dataLock{ dataMutex, std::defer_lock };
		size_t count{ 1 };
		while (!stop)
		{
			dataLock.lock();

			// �������� ������� ��������� �� ������ ������
			while (data.empty() && !stop)
				dmCondition.wait(dataLock);

			// ����� ������ � �������
			Log::info("Query #" + std::to_string(count++));
			std::copy(data.begin(), data.end(), std::ostreambuf_iterator<char>(std::cout));
			std::cout << std::flush;

			// �������� ���������� ������ �� ����������
			data.clear();
			dataLock.unlock();
			dmCondition.notify_one();
		}
	}

	// ����������� �������� http-�������� �
	// ��������� ����������� �� ������
	void App::queryData(HTTPClient& connectedClient, const std::string& request)
	{
		// �������� �������������� ���������� �������.
		// ��������������� ����������� �� ������� � 
		// ���������� send-fetch �������, �.�.
		// ������ ������ ����� ����������� ���
		// ���������/�����������

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