#include "LogStream.h"
#include <mutex>

LogStream::LogStream(std::shared_ptr<std::recursive_mutex>m)
	:enable(false),consoleStream(NULL),ptr_mutex(m)
{

}

LogStream::~LogStream()
{
	if(fs){
		fs->close();
	}
}

void LogStream::LogString(std::string content)
{
	if(enable){
		//std::lock_guard <std::recursive_mutex>lock(*ptr_mutex);
		if(fs && fs->is_open()){
			fs->write(content.c_str(),content.length());
		}
		if(consoleStream){
			(*consoleStream) << content;
		}
	}
}

void LogStream::SetFileHandler(std::shared_ptr<std::ofstream>handler)
{
	fs = handler;
}

void LogStream::SetConsoleHandler(std::ostream* handler)
{
	consoleStream = handler;
}

void LogStream::SetLogStatus(bool value)
{
	enable = value;
}

LogStream& operator<<(LogStream & s, StandardEndLine manip)
{
	std::stringstream ss;
	ss << manip;
	s.LogString(ss.str());
	return s;
}


