#include "pch.h"
#include "ConsoleLog.h"
#include <cstdarg>
#include <cstdio>
#include <ctime>
#include <filesystem>

ConsoleLog::ConsoleLog()
{
    _stdOut = ::GetStdHandle(STD_OUTPUT_HANDLE);
    _stdErr = ::GetStdHandle(STD_ERROR_HANDLE);
    OpenNewLogFile();
}

ConsoleLog::~ConsoleLog()
{
    if (_logFile.is_open())
    {
        _logFile.close();
    }
}

void ConsoleLog::WriteStdOut(Color color, const WCHAR* format, ...)
{
    if (format == nullptr)
        return;

    SetColor(true, color);

    va_list ap;
    va_start(ap, format);
    ::vwprintf(format, ap);
    va_end(ap);

    fflush(stdout);

    SetColor(true, Color::WHITE);

    // Also write to log file
    va_start(ap, format);
    WriteLogFile(format, ap);
    va_end(ap);
}

void ConsoleLog::WriteStdErr(Color color, const WCHAR* format, ...)
{
    WCHAR buffer[BUFFER_SIZE];

    if (format == nullptr)
        return;

    SetColor(false, color);

    va_list ap;
    va_start(ap, format);
    ::vswprintf_s(buffer, BUFFER_SIZE, format, ap);
    va_end(ap);

    ::fwprintf_s(stderr, buffer);
    fflush(stderr);

    SetColor(false, Color::WHITE);

    // Also write to log file
    va_start(ap, format);
    WriteLogFile(buffer);
    va_end(ap);
}

void ConsoleLog::WriteLogFile(const WCHAR* format, ...)
{
    if (!_logFile.is_open() || format == nullptr)
        OpenNewLogFile();

    std::time_t now = std::time(nullptr);
    std::tm localTime;
    localtime_s(&localTime, &now);

    WCHAR timeBuffer[100];
    wcsftime(timeBuffer, sizeof(timeBuffer) / sizeof(WCHAR), L"%Y-%m-%d %H:%M:%S", &localTime);

    _logFile << L"[" << timeBuffer << L"] ";

    va_list ap;
    va_start(ap, format);

    WCHAR logBuffer[BUFFER_SIZE];
    ::vswprintf_s(logBuffer, BUFFER_SIZE, format, ap);
    _logFile << logBuffer << std::endl;

    va_end(ap);
}

void ConsoleLog::SetColor(bool stdOut, Color color)
{
    static WORD SColors[] =
    {
        0,
        FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE,          // WHITE
        FOREGROUND_RED | FOREGROUND_INTENSITY,                        // RED
        FOREGROUND_GREEN | FOREGROUND_INTENSITY,                      // GREEN
        FOREGROUND_BLUE | FOREGROUND_INTENSITY,                       // BLUE
        FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY      // YELLOW
    };

    ::SetConsoleTextAttribute(stdOut ? _stdOut : _stdErr, SColors[static_cast<int32_t>(color)]);
}

void ConsoleLog::OpenNewLogFile()
{
    std::wstring logDirectory = L"logs";
    std::filesystem::create_directory(logDirectory);

    std::time_t now = std::time(nullptr);
    std::tm localTime;
    localtime_s(&localTime, &now);

    WCHAR timeBuffer[100];
    wcsftime(timeBuffer, sizeof(timeBuffer) / sizeof(WCHAR), L"%Y%m%d_%H%M%S", &localTime);

    std::wstring logFileName = logDirectory + L"/log_" + timeBuffer + L".txt";
    _logFile.open(logFileName, std::ios::out);

    if (!_logFile.is_open())
    {
        ::fwprintf_s(stderr, L"Failed to open log file: %s\n", logFileName.c_str());
    }
}