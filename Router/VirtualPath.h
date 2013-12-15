#pragma once
#include "UDPSocket.h"
#include <vector>
#include <mutex>
#include <chrono>
#include <random>
#include <string>
#include <memory>

using namespace std;

enum PacketRoute
{
	ClientToParentServer = 0,
	ClientToChildServer  = 1,
	ParentServerToClient = 2,
	ChildServerToClient  = 3
};

struct DelayPacket
{
	char buffer[UDP_PACKET_SIZE];	//Data of packet
	int packetSize;
	PacketRoute route;				//Packet Route
	SOCKADDR_IN destAddr;			//The address can be changed
	bool beingDelayed;				//Packet status, true means it's being delayed otherwise means empty
	long packetNumber;				//The packet number for tracking
};


class VirtualPath
{
private:
	volatile int isRunning;
	SOCKADDR_IN client_address; 
	SOCKADDR_IN server_address;

	SOCKADDR_IN default_server; //Default forwarding address

	UDPSocket client_socket;	//Is used to communicate with client side
	long client_packets;

	UDPSocket server_socket;	//Is used to communicate with server side
	long server_packets;

	//Router Socket.
	UDPSocket* routerSocket;

	//Delayed packet
	DelayPacket delayedPacket;
	std::mutex delayedMutex;
	int mDelayedTurns;

	//Rate
	double mDropRate;
	double mDelayRate;

private:
	void runLoop();
	bool isDamage();
	bool isDelayed();

	void processClientPacket();
	void processServerPacket();
	bool forwardDelayedPacket();
	void processPacket(const char* buffer,int sz, SOCKADDR_IN* addr, PacketRoute route);

	UDPSocket* socketOfRoute(PacketRoute route);
	string sourceOfRoute(PacketRoute route);
	string destinationOfRoute(PacketRoute route);

public:
	VirtualPath(UDPSocket*router,SOCKADDR_IN* client, SOCKADDR_IN* server, double dropRate, double delayRate);
	~VirtualPath(void);
	void processRouterPacket(char* buffer, int sz);
	void Start();
	bool CompareSocketAddress(SOCKADDR_IN* addr);
};

