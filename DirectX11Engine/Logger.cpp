#include "Logger.h"
#include <chrono>
#include <iostream>

void Logger::Log(std::string Message, LogSeverity Severity)
{
	std::chrono::zoned_time ZonedTime{ "Europe/Amsterdam", (std::chrono::system_clock::now()) };
	std::cout << ZonedTime << " : " << GetSeverityString(Severity) << " - " << Message << std::endl;
}

std::string Logger::GetSeverityString(LogSeverity Severity)
{
	switch (Severity)
	{
	case LogSeverity::Log:
		return "Log";
		break;
	case LogSeverity::Warning:
		return "Warning";
		break;
	case LogSeverity::Error:
		return "Error";
		break;
	}
}
