#pragma once
#include <WinSock2.h>
#include <WS2tcpip.h>

//Clear buffer macro
#define UDP_PACKET_SIZE 4096
#define bzero(ptr,sz) memset(ptr,0,sz)
class UDPSocket
{
private:
	SOCKET sock;

public:
	UDPSocket();
	~UDPSocket(void);

	//Bind to specific port. Return true if successful otherwise return false and log error
	bool Bind(int localPort);

	//UDP send
	int Send(const char*buffer,unsigned int sz, SOCKADDR_IN* destination);

	//Receive data. Data will be overwrite to buffer
	//If you do not want to receive sender address, pass NULL
	//If time out is -1, it will block current execution until data is available
	//Time out is millisecond
	//Return 0 means timeout but no packet arrived
	//Return negative means error
	int Receive(char* buffer,unsigned int sz, SOCKADDR_IN* sender, int timeout = -1);

	//Close socket. Release resource
	void Close();

	//Get FDS
	SOCKET GetSocket();
};

