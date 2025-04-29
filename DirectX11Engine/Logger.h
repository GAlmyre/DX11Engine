#pragma once

#include <string>

enum class LogSeverity
{
	Log, 
	Warning, 
	Error
};

// Class to handle all the logging behaviour
class Logger
{
public:
	static void Log(std::string Message, LogSeverity Severity = LogSeverity::Log);

private:
	static std::string GetSeverityString(LogSeverity Severity);
};

