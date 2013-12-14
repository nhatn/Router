#pragma once
#include "UDPSocket.h"

//Clear buffer macro
#define bzero(ptr,sz) memset(ptr,0,sz)

class Connection
{
private:
	//Client address
	SOCKADDR_IN client_address;

	//Entry per connection (can be used)
	SOCKADDR_IN server_address;

	//The main entry of server applications
	SOCKADDR_IN default_server;

public:
	bool Process(bool dataIsReady);
	Connection(SOCKADDR_IN client,const char* buffer, SOCKADDR_IN server);
	~Connection(void);
};

