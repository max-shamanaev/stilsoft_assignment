/*
	Заметки:
*/

#include "HTTPClient.h"

#include <iostream>
#include <string>
#include <vector>

int main()
{
	constexpr char hostName[]{ "www.google.com" };

	wsApp::HTTPClient client{};
	client.connect(hostName);

	std::string req{ client.formatRequest(wsApp::RequestMethods::HEAD) };
	client.sendRequest(req);

	std::vector<char> response{};
	client.fetchResponse(response);

	std::cout << '\n' << response.data() << '\n';

	return 0;
}