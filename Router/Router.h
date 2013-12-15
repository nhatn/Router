#pragma once
#include <string>
#include <algorithm>
#include <vector>
#include "VirtualPath.h"
#include "UDPSocket.h"
using namespace std;


class Router
{
private:
	vector<VirtualPath*>connections;
	shared_ptr<SOCKADDR_IN> serverAddress;
	UDPSocket routerSocket;
	double mDropRate;
	double mDelayedRate;

public:
	void Start(int localPort, string serverName, int serverPort)THROWS(std::exception);
	Router(double dropRate, double delayedRate);
	~Router(void);
};

