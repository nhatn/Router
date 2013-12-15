#include "VirtualPath.h"
#include <thread>
#include "Log.h"

#define CYCLE_TIME 30
VirtualPath::VirtualPath(UDPSocket* router,SOCKADDR_IN* client, SOCKADDR_IN* server,double dropRate, double delayRate)
	:mDropRate(dropRate),mDelayRate(delayRate),routerSocket(router)
{
	memcpy(&client_address,client,sizeof(client_address));
	memcpy(&default_server,server,sizeof(default_server));
	memcpy(&server_address,server,sizeof(server_address));
	isRunning = 0;
	mDelayedTurns = 0;
}


void VirtualPath::Start()
{
	isRunning = 1;
	std::thread receivingThread(&VirtualPath::runLoop,this);
	receivingThread.detach();
}

VirtualPath::~VirtualPath(void)
{
	LOG_DEBUG << "Release Connection" << endl;
}

UDPSocket* VirtualPath::socketOfRoute(PacketRoute route)
{
	UDPSocket* pSock;
	switch (route)
	{
	case ClientToParentServer:
		pSock = &server_socket;
		break;

	case ClientToChildServer:
		pSock = &server_socket;
		break;

	case ParentServerToClient:
		pSock = routerSocket;
		break;

	case ChildServerToClient:
		pSock = &client_socket;
		break;

	default:
		pSock = routerSocket;
		break;
	}
	return pSock;
}

string VirtualPath::sourceOfRoute(PacketRoute route)
{
	string source = "";
	switch (route)
	{
	case ClientToParentServer:
		source = "Client";
		break;

	case ClientToChildServer:
		source = "Client";
		break;

	case ParentServerToClient:
		source = "Server";
		break;

	case ChildServerToClient:
		source = "Server";
		break;

	default:
		source = "Unknown";
		break;
	}
	return source;
}

string VirtualPath::destinationOfRoute(PacketRoute route)
{
	string dest = "";
	switch (route)
	{
	case ClientToParentServer:
		dest = "Server";
		break;

	case ClientToChildServer:
		dest = "Server";
		break;

	case ParentServerToClient:
		dest = "Client";
		break;

	case ChildServerToClient:
		dest = "Client";
		break;

	default:
		dest = "Unknown";
		break;
	}
	return dest;
}

bool VirtualPath::forwardDelayedPacket()
{
	std::lock_guard<mutex>lock(delayedMutex);
	if(delayedPacket.beingDelayed){
		++mDelayedTurns;
		if(mDelayedTurns > 3){
			PacketRoute route = delayedPacket.route;
			UDPSocket *pSock = socketOfRoute(route);
			delayedPacket.beingDelayed = false;
			LOG_INFO << "Send delayed packet" << endl;
			pSock->Send(delayedPacket.buffer,delayedPacket.packetSize,&delayedPacket.destAddr);
			mDelayedTurns = 0;
		}
	}
	return delayedPacket.beingDelayed;
}

void VirtualPath::processClientPacket()
{
	SOCKADDR_IN addr;
	char buffer[UDP_PACKET_SIZE];
	int sz = client_socket.Receive(buffer,UDP_PACKET_SIZE,&addr,0);
	if(sz > 0){
		memcpy(&client_address,&addr,sizeof(addr));
		processPacket(buffer,sz,&server_address,PacketRoute::ClientToChildServer);
	}
}

void VirtualPath::processServerPacket()
{
	SOCKADDR_IN addr;
	char buffer[UDP_PACKET_SIZE];
	int sz = server_socket.Receive(buffer,UDP_PACKET_SIZE,&addr,0);
	if(sz > 0){
		memcpy(&server_address,&addr,sizeof(server_address));
		if(server_address.sin_port == default_server.sin_port){
			processPacket(buffer,sz,&client_address,PacketRoute::ParentServerToClient);
		}else{
			processPacket(buffer,sz,&client_address,PacketRoute::ChildServerToClient);
		}
	}
}

void VirtualPath::processRouterPacket(char* buffer, int sz)
{
	processPacket(buffer,sz,&default_server,PacketRoute::ClientToParentServer);
}

void VirtualPath::processPacket(const char* buffer,int sz,SOCKADDR_IN* addr, PacketRoute route)
{
	//Tracking information
	long packet_number;
	string source = sourceOfRoute(route);
	string dest = destinationOfRoute(route);
	if(route == ClientToChildServer || route == ClientToParentServer){
		++client_packets;
		packet_number = client_packets;
	}else{
		++server_packets;
		packet_number = server_packets;
	}

	//Forward the delayed packet if needed
	bool hasDelayedPacket = forwardDelayedPacket();

	//Dropping packet
	if(isDamage()){
		LOG_INFO<< "Packet " + to_string(packet_number) + " received from " + source + " has been dropped by router!\n";

	//Delay packet
	}else if(!hasDelayedPacket && isDelayed()){
		LOG_INFO<< "Packet " + to_string(packet_number) + " received from " + source + " has been delayed by router!\n";
		std::lock_guard<mutex>lock(delayedMutex);
		delayedPacket.beingDelayed = true;
		delayedPacket.route = route;
		memcpy(&delayedPacket.destAddr,addr,sizeof(delayedPacket.destAddr));
		memcpy(&delayedPacket.buffer,buffer,sz);
		delayedPacket.packetSize = sz;
		delayedPacket.packetNumber = packet_number;

	//Forward packet
	}else{
		UDPSocket*pSock = socketOfRoute(route);
		LOG_DEBUG << "Forward packet " + to_string(packet_number) + " from " + source + " to " + dest + "\n";
		pSock->Send(buffer,sz,addr);
	}

}


void VirtualPath::runLoop()
{
	//Waiting for both packet
	for (;isRunning;){
		FD_SET readfds;
		FD_ZERO(&readfds);
		FD_SET(client_socket.GetSocket(),&readfds);
		FD_SET(server_socket.GetSocket(),&readfds);
		TIMEVAL timeval;
		timeval.tv_sec = 0 ;
		timeval.tv_usec = CYCLE_TIME * 1000; 
		int availablefds = select(2,&readfds,NULL,NULL,&timeval);

		//New packet arrived
		if(0 < availablefds){
			//Packet from server
			if(FD_ISSET(client_socket.GetSocket(),&readfds)){
				processClientPacket();
			}

			//Packet from client
			if(FD_ISSET(server_socket.GetSocket(),&readfds)){
				processServerPacket();
			}
		}else{
			forwardDelayedPacket();
		}

	}

}

bool VirtualPath::CompareSocketAddress(SOCKADDR_IN* addr)
{
	return(client_address.sin_port == addr->sin_port && client_address.sin_addr.S_un.S_addr == addr->sin_addr.S_un.S_addr);
}

bool VirtualPath::isDamage()
{
	static std::default_random_engine e;
	static std::uniform_int_distribution<int>d(1,10000);
	return d(e) <= mDropRate;
}

bool VirtualPath::isDelayed()
{
	static std::default_random_engine e;
	static std::uniform_int_distribution<int>d(1,10000);
	return d(e) <= mDelayRate;
}