#include "Connection.h"


Connection::Connection(SOCKADDR_IN client,const char* buffer, SOCKADDR_IN server)
{
	bzero(&default_server,sizeof(default_server));
	bzero(&server_address,sizeof(server_address));

}


Connection::~Connection(void)
{

}

bool Connection::Process(bool dataIsReady)
{
	return 0;
}
