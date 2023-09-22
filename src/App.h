#pragma once

#include "HTTPClient.h"

#include <condition_variable>
#include <mutex>
#include <string>
#include <vector>

namespace wsApp
{
	// ����� ���������, ����������� ���������� http-������� � ��������������
	// ���������� ������������������ �������, ����������������� ����� �����
	//
	class App
	{
	public:
		void run();

	private:
		// ��������� ������ � �������
		void handleData();

		// ������ ������ � �������
		void queryData(HTTPClient& connectedClient, const std::string& request);

		// �������� ��� ���-������� (���. 3 �������) / ��� IP-�����
		const std::string hostAddress{ "www.google.com" };

		// ��������� ��� ���������� �� ������� ������
		std::vector<char> data{};

		bool stop{ false };
		std::mutex dataMutex{};
		std::condition_variable dmCondition{};
	};
}