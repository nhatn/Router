#include "UDPSocket.h"
#pragma comment (lib,"Ws2_32.lib")

UDPSocket::UDPSocket()
{
	sock = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
}


UDPSocket::~UDPSocket(void)
{
	Close();
}

bool UDPSocket::Bind(int localPort)
{
	struct sockaddr_in address;
	bzero(&address,sizeof(address));
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = htonl(INADDR_ANY);
	address.sin_port = htons(localPort);
	int iResult = ::bind(sock, (struct sockaddr*)&address,sizeof(address));
	if (iResult == SOCKET_ERROR) {
		return false;
	}else{
		return true;
	}
}

int UDPSocket::Receive(char* buffer,unsigned int sz, SOCKADDR_IN* sender, int timeout /* = -1 */)
{
	if(timeout >= 0){
		FD_SET readfds;
		FD_ZERO(&readfds);
		FD_SET(sock,&readfds);
		TIMEVAL timeval;
		timeval.tv_sec = 0 ;
		timeval.tv_usec = timeout * 1000; 
		int availablefds = select(1,&readfds,NULL,NULL,&timeval);
		if(0 >= availablefds || !FD_ISSET(sock,&readfds)){
			return 0;
		}
	}
	int addr_sz = (sender == NULL)? 0 : sizeof(SOCKADDR_IN);
	SOCKADDR *addr_ptr = (sender == NULL) ? NULL: (SOCKADDR*)sender;

	int result = ::recvfrom(sock,buffer,sz,0,addr_ptr,&addr_sz);
	return result;
}

int UDPSocket::Send(const void*buffer,unsigned int sz, SOCKADDR_IN* destination)
{
	int result = ::sendto(sock,(const char*)buffer,sz,0,(SOCKADDR*)destination,sizeof(SOCKADDR_IN));
	if(result < 0){
		int error = GetLastError();
		std::cerr << "Error code " << error << std::endl;
	}
	return result;
}

void UDPSocket::Close()
{
	closesocket(sock);
	std::cout << "Close Socket" << std::endl;
}

SOCKET UDPSocket::GetSocket()
{
	return sock;
}

bool UDPSocket::GetAddress(SOCKADDR_IN*addr,std::string hostname,int port)
{
	//Resolution server address
	HOSTENT* hp = gethostbyname(hostname.c_str());
	if(NULL == hp){
		return false;
	}
	bzero(addr,sizeof(hostname));
	memcpy(&addr->sin_addr,hp->h_addr,hp->h_length);
	addr->sin_port = htons(port);
	addr->sin_family = hp->h_addrtype;
	return true;
}