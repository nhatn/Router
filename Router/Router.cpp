#include "Router.h"
#include "Log.h"

Router::Router(double drop, double delay)
	:mDropRate(drop),mDelayedRate(delay)
{

}

Router::~Router(void)
{

}

void Router::Start(int localPort, string serverName, int serverPort)THROWS(std::exception)
{

	SOCKADDR_IN client_addr;
	char routerBuffer[UDP_PACKET_SIZE];

	//Bind the router socket
	if(!routerSocket.Bind(localPort)){
		throw std::exception("Unable to bind router socket");
	}

	//Resolution server address
	HOSTENT* hp = gethostbyname(serverName.c_str());
	if(NULL == hp){
		throw std::exception("Server is unreachable");
	}
	bzero(&serverAddress,sizeof(serverAddress));
	memcpy(&serverAddress.sin_addr,hp->h_addr,hp->h_length);
	serverAddress.sin_port = htons(serverPort);
	serverAddress.sin_family = hp->h_addrtype;

	//Ready to rock
	for (;;){

		//Get the packet and the address
		int sz = routerSocket.Receive(routerBuffer,UDP_PACKET_SIZE,&client_addr,-1);
		if(sz > 0){
			VirtualPath* pConn = NULL;
			for(vector<VirtualPath*>::iterator iter = connections.begin(); iter != connections.end(); ++iter){
				VirtualPath* conn = *iter;
				if(conn->CompareSocketAddress(&client_addr)){
					pConn = conn;
					break;
				}
			}
			//Create new connection
			if(pConn == NULL){
				LOG_DEBUG << "Client new connection" << endl;
				pConn = new VirtualPath(&routerSocket,&client_addr,&serverAddress,mDropRate,mDelayedRate);
				connections.push_back(pConn);
				pConn->Start();
			}
			pConn->processRouterPacket(routerBuffer,sz);

		}
	}
}




