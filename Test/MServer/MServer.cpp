#include "../Util/UDPSocket.h"
#include <thread>
#include <iostream>
#include <string>
#include <random>

using namespace std;
void handleClient(SOCKADDR_IN addr,int received_packet)
{
	int send_packets = 0;
	std::default_random_engine e;
	std::uniform_int_distribution<int>d(1,5);
	UDPSocket sock;
	sock.Send(&send_packets,sizeof(int),&addr);
	cout << "Send packet " + std::to_string(send_packets++) + "\n";

	char buffer[UDP_PACKET_SIZE];
	SOCKADDR_IN client_addr;
	for (;;){
		int sz = sock.Receive(buffer,UDP_PACKET_SIZE,&client_addr);
		int number_to_sends = d(e);
		for (int i = 0; i <number_to_sends; ++i){
			sock.Send(&send_packets,sizeof(int),&client_addr);
			cout << "Send packet " + std::to_string(send_packets++) + "\n";
		}

	}
}

int main(int argc, char* argv[])
{
	WSADATA wsaData;
	int result = WSAStartup(MAKEWORD(2,2), &wsaData);
	if(0 == result){
		UDPSocket socket;
		socket.Bind(std::stoi(argv[1]));
		char buffer[UDP_PACKET_SIZE];
		for (;;){
			//The first packet is simulated accept on TCP
			SOCKADDR_IN addr;
			int sz = socket.Receive(buffer,UDP_PACKET_SIZE,&addr);
			if(sz == sizeof(int)){
				int* packetNumber = (int*)buffer;
				thread t(handleClient,addr,*packetNumber);
				t.detach();
			}
		}
	}
	WSACleanup();
	return 0;
}

