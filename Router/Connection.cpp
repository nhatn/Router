#include "Connection.h"
#include <thread>
#include "Log.h"

#define CYCLE_TIME 30
Connection::Connection(SOCKADDR_IN* client, SOCKADDR_IN* server,double dropRate, double delayRate)
	:mDropRate(dropRate),mDelayRate(delayRate)
{
	memcpy(&client_address,client,sizeof(client_address));
	memcpy(&default_server,server,sizeof(default_server));
	memcpy(&server_address,server,sizeof(server_address));
	isRunning = 0;
}


void Connection::Start()
{
	isRunning = 1;
	std::thread receivingThread(&Connection::runLoop,this);
	receivingThread.detach();
}

Connection::~Connection(void)
{
	LOG_DEBUG << "Release Connection" << endl;
}

bool Connection::processDelayedPacket()
{
	std::lock_guard<mutex>lock(delayedMutex);
	if(delayedPacket.beingDelayed){
		chrono::time_point<chrono::system_clock> now = chrono::system_clock::now();
		long long elapsed = chrono::duration_cast<chrono::microseconds>(now - delayedPacket.timestamp).count();
		if(elapsed > CYCLE_TIME * 3000){
			UDPSocket* pSock;
			if(delayedPacket.dest == Destination::Client){
				pSock = &server_socket;
			}else{
				pSock = &client_socket;
			}
			delayedPacket.beingDelayed = false;
			LOG_INFO << "Send delayed packet" << endl;
			pSock->Send(delayedPacket.buffer,delayedPacket.packetSize,&delayedPacket.dest_addr);
		}
	}
	return delayedPacket.beingDelayed;
}

void Connection::processClientPacket()
{
	SOCKADDR_IN addr;
	char buffer[UDP_PACKET_SIZE];
	int sz = client_socket.Receive(buffer,UDP_PACKET_SIZE,&addr,0);
	if(sz > 0){
		memcpy(&client_address,&addr,sizeof(addr));
		processPacket(buffer,sz,&server_address,Destination::Server);
	}
}

void Connection::processServerPacket()
{
	SOCKADDR_IN addr;
	char buffer[UDP_PACKET_SIZE];
	int sz = server_socket.Receive(buffer,UDP_PACKET_SIZE,&addr,0);
	if(sz > 0){
		memcpy(&server_address,&addr,sizeof(server_address));
		processPacket(buffer,sz,&client_address,Destination::Client);
	}
}

void Connection::processRouterPacket(char* buffer, int sz)
{
	processPacket(buffer,sz,&default_server,Destination::Server);
}

void Connection::processPacket(const char* buffer,int sz,SOCKADDR_IN* addr, Destination dest)
{
	UDPSocket*pSock;
	long packet_number;
	if(dest == Client){
		pSock = &client_socket;
		packet_number = ++client_packets;
	}else{
		packet_number = ++server_packets;
		pSock = &server_socket;
	}

	bool hasDelayedPacket = processDelayedPacket();
	if(isDamage()){
		LOG_INFO << "Dropped packet " << endl;

	}else if(!hasDelayedPacket && isDelayed()){
		LOG_INFO << "Delayed packet " << endl;
		std::lock_guard<mutex>lock(delayedMutex);
		delayedPacket.beingDelayed = true;
		delayedPacket.dest = dest;
		memcpy(&delayedPacket.dest_addr,addr,sizeof(delayedPacket.dest_addr));
		memcpy(&delayedPacket.buffer,buffer,sz);
		delayedPacket.packetSize = sz;
		delayedPacket.packetNumber = packet_number;
		delayedPacket.timestamp = chrono::system_clock::now();

	}else{
		LOG_INFO << "Forward packet " << endl;
		pSock->Send(buffer,sz,addr);
	}

}


void Connection::runLoop()
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
			processDelayedPacket();
		}

	}

}

bool Connection::CompareSocketAddress(SOCKADDR_IN* addr)
{
	return(client_address.sin_port == addr->sin_port && client_address.sin_addr.S_un.S_addr == addr->sin_addr.S_un.S_addr);
}

bool Connection::isDamage()
{
	static std::default_random_engine e;
	static std::uniform_int_distribution<int>d(1,10000);
	return d(e) <= mDropRate;
}

bool Connection::isDelayed()
{
	static std::default_random_engine e;
	static std::uniform_int_distribution<int>d(1,10000);
	return d(e) <= mDelayRate;
}