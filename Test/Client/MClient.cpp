// Client.cpp : Defines the entry point for the console application
#include "../Util/UDPSocket.h"
#include <thread>
#include <iostream>
#include <string>

using namespace std;
void sendRequests(UDPSocket*socket,SOCKADDR_IN* addr)
{
	int packet_number = 0;
	for (;;)
	{
		int to_send = 0;
		cout << "Enter number of packets to send: ";
		cin >> to_send;
		if(to_send >50000){
			to_send = 50000;
		}
		for (int i = 0; i < to_send;++i){
			int sz = socket->Send(&packet_number,sizeof(int),addr);
			if(sz > 0){
				cout <<"Send packet " + to_string(packet_number) + "\n";
			}else{
				cerr <<"Unable to send packet " + to_string(packet_number) + "\n";
			}
			++packet_number;
		}
	}
}

void receiveReplies(UDPSocket*socket)
{
	char buffer[UDP_PACKET_SIZE];
	SOCKADDR_IN addr;
	for (;;){
		int sz = socket->Receive(buffer,UDP_PACKET_SIZE,&addr,-1);
		if(sz == 4){
			int *packet_number = (int*)buffer;
			cout <<"Receive Packet " + to_string(*packet_number) + "\n";
		}
	}
}

int main(int argc, char* argv[])
{
	if(argc != 3){
		cerr << "Invalid arguments \n"<<endl;
		return -1;
	}

	WSADATA wsaData;
	int result = WSAStartup(MAKEWORD(2,2), &wsaData);
	if(0 == result){
		char buffer[UDP_PACKET_SIZE];
		SOCKADDR_IN listen_addr;
		bool ret = UDPSocket::GetAddress(&listen_addr,argv[1],std::stoi(argv[2]));
		if(ret){
			//Connect To
			UDPSocket socket;
			SOCKADDR_IN transceiver_addr;
			int packet_number = 0;
			bool success = false;
			do
			{
				int sz = socket.Send(&packet_number,sizeof(int),&listen_addr);
				cout << "Send packet " << packet_number << endl;

				sz = socket.Receive(buffer,UDP_PACKET_SIZE,&transceiver_addr,400);
				if(sz >= sizeof(int)){
					int *p = (int*)buffer;
					cout << "Receive packet " << *p << endl;
					success = true;
					break;
				}
				++packet_number;
			}while(!success);

			//Ready to rock
			thread receive_thread(receiveReplies,&socket);
			receive_thread.detach();
			sendRequests(&socket,&transceiver_addr);
		}
	}
	WSACleanup();
	return 0;
}

