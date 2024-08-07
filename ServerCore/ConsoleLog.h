#pragma once

/*---------------
	ConsoleLog
----------------*/

enum class Color
{
    BLACK,
    WHITE,
    RED,
    GREEN,
    BLUE,
    YELLOW,
};

class ConsoleLog
{
    enum { BUFFER_SIZE = 4096 };

public:
    ConsoleLog();
    ~ConsoleLog();

    void WriteStdOut(Color color, const WCHAR* str, ...);
    void WriteStdErr(Color color, const WCHAR* str, ...);
    void WriteLogFile(const WCHAR* format, ...);

protected:
    void SetColor(bool stdOut, Color color);

private:
    HANDLE _stdOut;
    HANDLE _stdErr;
    std::wofstream _logFile;

    void OpenNewLogFile();
};