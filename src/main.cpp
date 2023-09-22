#include "App.h"

int main()
{
	wsApp::App* app{ new wsApp::App() };

	app->run();

	delete app;

	return 0;
}