#pragma once
#include <string>
#include <algorithm>
#include <list>
#include "Connection.h"

using namespace std;
class Router
{
private:
	list<Connection>connections;
	SOCKADDR_IN server_address;
	SOCKADDR_IN router_address;

	bool setUp(int localPort, string serverAddress, int serverPort);
	void tearDown();
	bool runLoop();

public:
	bool Start(int localPort, string serverAddress, int serverPort);
	Router();
	~Router(void);
};

