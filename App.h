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
		// ��������� ������ � �������
		void handleData();

		// ������ ������ � �������
		void queryData(HTTPClient& connectedClient, const std::string& request);

		// �������� ��� ���-������� / ��� IP-�����
		const std::string hostAddress{ "www.google.com" };

		// ��������� ��� ���������� �� ������� ������
		std::vector<char> data{};

		bool stop{ false };
		std::mutex dataMutex{};
		std::condition_variable dmCondition{};
	};
}