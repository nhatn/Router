#pragma once
#include <iostream>
using namespace std;

class Log
{
public:
	Log(void);
	~Log(void);
};

#define LOG_VERBOSE cout
#define LOG_DEBUG	cout
#define LOG_INFO	cout
#define LOG_ERROR	cout


