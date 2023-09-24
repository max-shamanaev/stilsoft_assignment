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
		// ��������� ����������� � �������, ��� �����
		// � �� ������ ���������� � �������
		client->setFetchTimeout(fetchTimeout.count());
	
		// ����������� ������������ � �������� � ���������
		// ���������� �� ���� ������
		std::thread queryThread{ &App::queryData, this,
			std::ref(*client), request
		};
		std::thread handleThread{ &App::handleData, this };

		// ��������� ���������� ����������� �� ������� ESC
		while (!stop)
		{
			if (GetAsyncKeyState(VK_ESCAPE) & 0x01)
			{
				stop = true;
				stateChange.notify_all();
			}
		}

		// �������� ��������� ������ ������ � �������
		// � ���������� ������� �� �������
		queryThread.join();
		handleThread.join();
		client->disconnect();

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
				stateChange.wait(dataLock);

			// ����� ������ � �������
			Log::info("Handle #" + std::to_string(count++));
			std::cout.write(data.data(), data.size());
			std::cout << std::endl;

			// �������� ���������� ������ �� ����������
			data.clear();

			dataLock.unlock();
			stateChange.notify_one();
		}
	}

	// ����������� �������� http-�������� �
	// ��������� ����������� �� ������
	void App::queryData(HTTPClient& connectedClient, const std::string& request)
	{
		std::unique_lock<std::mutex> dataLock{ dataMutex, std::defer_lock };
		int bytesSent{ 0 };

		while (!stop)
		{

			bytesSent = connectedClient.sendRequest(request);
			if (bytesSent == -1)
			{
				Log::info("Stopping query thread");

				stop = true;
				stateChange.notify_all();
				break;
			}
			else if (bytesSent == 0)
			{
				std::this_thread::sleep_for(sendTimeout);
				continue;
			}

			dataLock.lock();
			while (!data.empty() && !stop)
				stateChange.wait(dataLock);

			connectedClient.fetchResponse(data);

			dataLock.unlock();
			stateChange.notify_one();
		}
	}
}