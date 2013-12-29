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
	client_packets = 0;
	server_packets = 0;
	delayedPacket.beingDelayed = false;
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
		source = "client";
		break;

	case ClientToChildServer:
		source = "client";
		break;

	case ParentServerToClient:
		source = "server";
		break;

	case ChildServerToClient:
		source = "server";
		break;

	default:
		source = "unknown";
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
		dest = "server";
		break;

	case ClientToChildServer:
		dest = "server";
		break;

	case ParentServerToClient:
		dest = "client";
		break;

	case ChildServerToClient:
		dest = "client";
		break;

	default:
		dest = "unknown";
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
			string source = sourceOfRoute(route);
			string dest = destinationOfRoute(route);
			delayedPacket.beingDelayed = false;
			pSock->Send(delayedPacket.buffer,delayedPacket.packetSize,&delayedPacket.destAddr);
			LOG_INFO << "Forward delayed packet " + to_string(delayedPacket.packetNumber) + " from " + source + " to " + dest + "!\n";
			mDelayedTurns = 0;
		}
	}
	return delayedPacket.beingDelayed;
}

void VirtualPath::processClientPacket()
{
	SOCKADDR_IN addr;
	char buffer[UDP_PACKET_SIZE];
	int sz = client_socket.Receive(buffer,UDP_PACKET_SIZE,&addr);
	if(sz > 0){
		memcpy(&client_address,&addr,sizeof(addr));
		processPacket(buffer,sz,&server_address,PacketRoute::ClientToChildServer);
	}else{
		LOG_DEBUG << "Error while receiving packets from client\n";
	}
}

void VirtualPath::processServerPacket()
{
	SOCKADDR_IN addr;
	char buffer[UDP_PACKET_SIZE];
	int sz = server_socket.Receive(buffer,UDP_PACKET_SIZE,&addr);
	if(sz > 0){
		memcpy(&server_address,&addr,sizeof(server_address));
		if(server_address.sin_port == default_server.sin_port){
			processPacket(buffer,sz,&client_address,PacketRoute::ParentServerToClient);
		}else{
			processPacket(buffer,sz,&client_address,PacketRoute::ChildServerToClient);
		}
	}else{
		LOG_DEBUG << "Error while receiving packets from client\n";
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
	LOG_DEBUG << "Router received packet " + to_string(packet_number) + " from " + source + "\n";

	//Forward the delayed packet if needed
	bool hasDelayedPacket = forwardDelayedPacket();

	//Dropping packet
	if(isDamage()){
		LOG_INFO<< "Packet " + to_string(packet_number) + " from " + source + " has been dropped by router!\n";

	//Delay packet
	}else if(!hasDelayedPacket && isDelayed()){
		LOG_INFO<< "Packet " + to_string(packet_number) + " from " + source + " has been delayed by router!\n";
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

	bool client_priority = false;
	SOCKET client_fd = client_socket.GetSocket();
	SOCKET server_fd = server_socket.GetSocket();

	for (;isRunning;){

		//Check if there are any packets arrived or not?
		FD_SET readfds;
		FD_ZERO(&readfds);
		FD_SET(client_fd,&readfds);
		FD_SET(server_fd,&readfds);
		TIMEVAL timeval;
		timeval.tv_sec = 0 ;
		timeval.tv_usec = CYCLE_TIME * 1000; 
		int availablefds = select(2,&readfds,NULL,NULL,&timeval);

		//New packet arrived
		if(0 < availablefds){

			//Neither client or server should be given a higher priority but equally
			//If packet from server then next packet will likely come from opposite site
			//To reduce the waiting time, the priority will be toggled for each new packet

			//Client is given higher priority
			if(client_priority){
				if(FD_ISSET(client_fd,&readfds)){
					processClientPacket();
					client_priority = false;
				}
				if(FD_ISSET(server_fd,&readfds)){
					processServerPacket();
				}

			//Server is given higher priority
			}else{
				if(FD_ISSET(server_fd,&readfds)){
					processServerPacket();
					client_priority = true;
				}
				if(FD_ISSET(client_fd,&readfds)){
					processClientPacket();
				}
			}

		//No packet received, check to forward delayed packet
		}else{
			forwardDelayedPacket();
		}

	}

}

bool VirtualPath::CompareSocketAddress(SOCKADDR_IN* addr)
{
	return(client_address.sin_port == addr->sin_port 
		&& client_address.sin_addr.S_un.S_addr == addr->sin_addr.S_un.S_addr);
}

bool VirtualPath::isDamage()
{
	//Randomize new algorithm
	static std::default_random_engine e;
	static std::uniform_int_distribution<int>d(1,10000);
	return d(e) <= (mDropRate * 100);
}

bool VirtualPath::isDelayed()
{
	static std::default_random_engine e;
	static std::uniform_int_distribution<int>d(1,10000);
	return d(e) <= (mDelayRate * 100);
}