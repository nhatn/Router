#pragma once
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <memory>
#include <mutex>

class LogStream
{
private:
	std::shared_ptr<std::ofstream>fs;
	std::ostream* consoleStream;
	bool enable;
	std::shared_ptr<std::recursive_mutex>ptr_mutex;
public:
	LogStream(std::shared_ptr<std::recursive_mutex>recursive_mutex);
	~LogStream();

	//Handler
	void SetFileHandler(std::shared_ptr<std::ofstream>str);
	void SetConsoleHandler(std::ostream* console);

	//Log level
	void LogString(std::string content);
	void SetLogStatus(bool enable);
};

template <typename T>
LogStream & operator<<(LogStream & s, const std::basic_string<T>& bs )
{
	std::stringstream ss;
	ss << bs;
	s.LogString(ss.str());
	return s;
}

template <typename T>
LogStream & operator<<(LogStream & s, T const &t) 
{
	std::stringstream ss;
	ss << t;
	s.LogString(ss.str());
	return s;
}

template <typename T>
LogStream & operator<<(LogStream & s, std::ostream &(std::ostream&))
{
	return s;
}


//Support std::endl
typedef std::basic_ostream<char, std::char_traits<char> > CoutType;
typedef CoutType& (*StandardEndLine)(CoutType&);
LogStream& operator<<(LogStream & s, StandardEndLine manip);
