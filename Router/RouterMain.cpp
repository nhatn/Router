#include "Router.h"
#include <iostream>
#include "Log.h"
#include <memory>
using namespace std;
#define ROUTER_PORT 7000
#define SERVER_PORT 6000
#define SERVER_ADDR "Xyz"

struct Parameter
{
	string serverName;	//Required
	int serverPort;		//Default
	int routerPort;		//Default
	double droppedRate; //Required
	double delayedRate; //Required
	int loglevel;		//Default
};

shared_ptr<Parameter>readParameters(int args, char **argv)
{
	//Default values
	shared_ptr<Parameter>para(new Parameter());
	para->routerPort = ROUTER_PORT;
	para->serverPort = SERVER_PORT;
	para->loglevel = 2;
	
	//Empty values
	bool serverProvided = false;
	bool dropProvided = false;
	bool delayProvided = false;

	//Parse arguments
	try
	{
		for (int idx = 1; idx < args - 1; idx += 2)
		{
			string opt(argv[idx]);
			string value(argv[idx + 1]);

			if(0 == opt.compare("--routerport")){
				para->routerPort = stoi(value);

			}else if(0 == opt.compare("--serverport")){
				para->serverPort = stoi(value);

			}else if(0 == opt.compare("--servername")){
				para->serverName = value;
				serverProvided = true;

			}else if(0 == opt.compare("--drop")){
				para->droppedRate = stof(value);
				dropProvided = true;

			}else if(0 == opt.compare("--delay")){
				para->delayedRate = stof(value);
				delayProvided = true;
			}else if(0 == opt.compare("--loglevel")){
				para->loglevel = log_level_from_desc(value);
			}
		}
	}
	catch (exception e)
	{

	}
	

	//Ask users to enter required values
	if(!serverProvided){
		cout << "Enter Server Name:";
		cin >> para->serverName;
	}
	if(!dropProvided){
		cout << "Enter Drop Rate:";
		cin >> para->droppedRate;
	}
	if(!delayProvided){
		cout << "Enter Delay Rate:";
		cin >> para->delayedRate;
	}

	return para;
}


//Router.exe --routerport 5000 --servername Nhat-PC --serverport 5001 --delay 5 --drop 5 --loglevel debug
int main(int args, char**argv)
{
	WSADATA wsaData;

	shared_ptr<Parameter>parameter = readParameters(args,argv);

	//Setup logger
	LogManager::SharedManager().SetLogFileName("Router.log");
	LogManager::SharedManager().SetLogConsole(true); 
	LogManager::SharedManager().SetLogLevel(parameter->loglevel);

	//Initialize socket library
	WSAStartup(MAKEWORD(2,2), &wsaData);
	try{
		char hostname[256] = {'\0'};
		gethostname(hostname,256);
		LOG_INFO << "Router is running at " << hostname << ":" << parameter->routerPort << endl;
		LOG_INFO << "Server is at " << parameter->serverName << ":" << parameter->serverPort << endl;
		LOG_INFO << "Drop rate: " << (int)parameter->droppedRate << ", delay rate: " << parameter->delayedRate << endl;

		//Start Router
		Router router(parameter->droppedRate,parameter->delayedRate);
		router.Start(parameter->routerPort,parameter->serverName,parameter->serverPort);

	}catch(std::exception&e ){
		LOG_ERROR << "Error: " << e.what() << endl;
	}

	//Clean socket library
	WSACleanup();
	return 0;
}