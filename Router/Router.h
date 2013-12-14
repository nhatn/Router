#pragma once
#include <string>
#include <algorithm>
#include <vector>
#include "Connection.h"
#include "UDPSocket.h"
using namespace std;

#define THROWS(e)  
class Router
{
private:
	vector<Connection*>connections;
	SOCKADDR_IN serverAddress;
	UDPSocket routerSocket;
	double mDropRate;
	double mDelayedRate;

	void setUp(int localPort, string serverAddress, int serverPort)THROWS(std::exception);
	void runLoop();

public:
	void Start(int localPort, string serverName, int serverPort)THROWS(std::exception);
	Router(double dropRate, double delayedRate);
	~Router(void);
};

