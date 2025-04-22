#pragma once
#include "CorePch.h"
#include "CoreTLS.h"
#include <mutex>
#include <queue>

class Logger
{
public:
    static Logger& GetInstance();

    void Init(const string& baseFilename);
    void Push(const string& message);
    bool PopAndLog();
    bool IsEmpty();

private:
    Logger() {}
    ~Logger();

    string GetCurrentTimeStamp();

    USE_LOCK;
    std::ofstream _logFile;
    std::queue<string> _logQueue;
    string _baseFilename;
};
#define LOG_PUSH(message) Logger::GetInstance().Push(message)
