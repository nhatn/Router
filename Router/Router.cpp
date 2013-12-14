#include "Router.h"
#include "Log.h"

Router::Router()
{

}
Router::~Router(void)
{
	tearDown();
}

bool Router::setUp(int localPort, string serverAddress, int serverPort)
{
	return true;
}

void Router::tearDown()
{

}

bool Router::Start(int localPort, string serverAddress, int serverPort)
{
	//First setup local connection & server address
	//If setup success, enter runLoop that processes incoming packets
	if(setUp(localPort,serverAddress,serverPort)){
		runLoop();
		return true;
	}else{
		LOG_ERROR <<"Unable to start the Router" << endl;
		return false;
	}
}

bool Router::runLoop()
{
	return true;
}


