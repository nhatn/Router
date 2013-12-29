#include "Log.h"

LogManager::LogManager()
	:ptr_mutex(new std::recursive_mutex()),debugLogStream(ptr_mutex),infoLogStream(ptr_mutex),errorLogStream(ptr_mutex)
{

}

LogManager::~LogManager()
{

}

LogManager& LogManager::SharedManager()
{
	static LogManager manager;
	return manager;
}

void LogManager::SetLogFileName(std::string fileName)
{
	std::shared_ptr<std::ofstream>new_fs = std::shared_ptr<std::ofstream>(new std::ofstream(fileName));
	debugLogStream.SetFileHandler(new_fs);
	infoLogStream.SetFileHandler(new_fs);
	errorLogStream.SetFileHandler(new_fs);
	if(fs){
		fs->close();
	}
	fs = new_fs;
}

void LogManager::SetLogConsole(bool enable)
{
	if(enable){
		debugLogStream.SetConsoleHandler(&std::cout);
		infoLogStream.SetConsoleHandler(&std::cout);
		errorLogStream.SetConsoleHandler(&std::cerr);
	}else{
		debugLogStream.SetConsoleHandler(NULL);
		infoLogStream.SetConsoleHandler(NULL);
		errorLogStream.SetConsoleHandler(NULL);
	}
}

void LogManager::SetLogLevel(int level)
{
	debugLogStream.SetLogStatus(0 != (level & LOG_DEBUG_CONST));
	infoLogStream.SetLogStatus(0 != (level & LOG_INFO_CONST));
	errorLogStream.SetLogStatus(0 != (level & LOG_ERROR_CONST));
}