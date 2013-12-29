#pragma once
#include "LogStream.h"
class LogManager 
{
private:
	std::shared_ptr<std::ofstream> fs;
	std::shared_ptr<std::recursive_mutex>ptr_mutex;

	//Log Manager method
	LogManager();
	~LogManager();

public:
	static LogManager& SharedManager();

	//Allow to access directly
	LogStream debugLogStream;
	LogStream infoLogStream;
	LogStream errorLogStream;

	//Forward to
	void SetLogFileName(std::string fileName);
	void SetLogConsole(bool enable);
	void SetLogLevel(int level);

};

#define LOG_ERROR_CONST	0x01
#define LOG_INFO_CONST	0x02
#define LOG_DEBUG_CONST	0x04

#define LOG_LEVEL_NONE		0
#define LOG_LEVEL_ERROR		0 | LOG_ERROR_CONST
#define LOG_LEVEL_INFO		0 | LOG_LEVEL_ERROR | LOG_INFO_CONST
#define LOG_LEVEL_DEBUG		0 | LOG_LEVEL_INFO  | LOG_DEBUG_CONST

#ifndef LOG_DEBUG
#define LOG_DEBUG LogManager::SharedManager().debugLogStream << "DBG: " 
#endif

#ifndef LOG_INFO
#define LOG_INFO LogManager::SharedManager().infoLogStream << "INF: " 
#endif

#ifndef LOG_ERROR
#define LOG_ERROR LogManager::SharedManager().errorLogStream << "ERR: " 
#endif

extern int log_level_from_desc(std::string desc);


