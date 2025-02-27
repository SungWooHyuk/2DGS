#include "pch.h"
#include "Logger.h"

Logger& Logger::GetInstance()
{
	static Logger instance;
	return instance;
}

void Logger::Init(const string& baseFilename)
{
	_baseFilename = baseFilename;
	string timestamp = GetCurrentTimeStamp();
	string filename = _baseFilename + "_" + timestamp + ".txt";
	_logFile.open(filename, std::ios::app);

	if (!_logFile.is_open())
	{
		throw runtime_error("Failed to open log file");
	}
}

void Logger::Push(const string& message)
{
	stringstream ss;
	ss << GetCurrentTimeStamp() << " [Thread " << LThreadId << "] " << message;

	{
		WRITE_LOCK;
		_logQueue.push(ss.str());
	}
}

bool Logger::PopAndLog()
{
	WRITE_LOCK;
	if (_logQueue.empty())
		return false;

	_logFile << _logQueue.front() << std::endl;
	_logQueue.pop();
	_logFile.flush();
	return true;
}

bool Logger::IsEmpty()
{
	READ_LOCK;
	return _logQueue.empty();
}

Logger::~Logger()
{
	if (_logFile.is_open())
	{
		_logFile.close();
	}
}

string Logger::GetCurrentTimeStamp()
{
	auto now = std::chrono::system_clock::now();
	auto in_time_t = std::chrono::system_clock::to_time_t(now);

	std::stringstream ss;
	ss << std::put_time(std::localtime(&in_time_t), "%Y%m%d_%H%M%S");
	return ss.str();
}