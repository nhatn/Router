#include "Router.h"
#include <iostream>
#include "Log.h"

#define ROUTER_PORT 7000
#define SERVER_PORT 5001
#define SERVER_ADDR "localhost"

int main(int args, char**argv)
{
	WSADATA wsaData;
	int result = WSAStartup(MAKEWORD(2,2), &wsaData);
	if(result == 0){
		try{
			Router router(5,0);
			router.Start(ROUTER_PORT,SERVER_ADDR,SERVER_PORT);
		}catch(std::exception&e ){
			LOG_ERROR << "Error: " << e.what() << endl;
		}
	}else{
		LOG_ERROR << "Unable to initialize Socket Library:" << result << endl;
	}
	WSACleanup();
	return result;
}