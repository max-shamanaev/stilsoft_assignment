#include "HTTPClient.h"

#include <WinSock2.h>
#include <WS2tcpip.h>

#include <iostream>
#include <vector>

namespace config
{
	constexpr char hostName[]{ "www.google.com" };
	constexpr size_t buffSize{ 4 * 1024 };
}

int main()
{
	wsApp::HTTPClient client{};
	client.connect(config::hostName);
	client.sendRequest(wsApp::RequestMethods::HEAD);

	std::vector<char> buffer(config::buffSize);
	client.fetchResponse(buffer);

	std::cout << '\n' << buffer.data() << '\n';

	return 0;
}