#pragma once
#include "UDPSocket.h"
#include <vector>
#include <mutex>
#include <chrono>
#include <random>


using namespace std;
enum Destination
{
	Client = 0,
	Server = 1
};

struct DelayPacket
{
	char buffer[UDP_PACKET_SIZE];	//Data of packet
	int packetSize;
	Destination dest;				//Destination
	SOCKADDR_IN dest_addr;			//The address can be changed
	bool beingDelayed;				//Packet status, true means it's being delayed otherwise means empty
	long packetNumber;				//The packet number for tracking
	chrono::time_point<chrono::system_clock> timestamp; //Start delayed time
};

class Connection
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

	//Delayed packet
	DelayPacket delayedPacket;
	std::mutex delayedMutex;

	//Rate
	double mDropRate;
	double mDelayRate;

private:
	void runLoop();
	bool isDamage();
	bool isDelayed();

	void processClientPacket();
	void processServerPacket();
	bool processDelayedPacket();
	void processPacket(const char* buffer,int sz, SOCKADDR_IN* addr, Destination dest);

public:
	Connection(SOCKADDR_IN* client, SOCKADDR_IN* server, double dropRate, double delayRate);
	~Connection(void);
	void processRouterPacket(char* buffer, int sz);
	void Start();
	bool CompareSocketAddress(SOCKADDR_IN* addr);
};

