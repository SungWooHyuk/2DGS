#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/async.h>
#include <spdlog/sinks/stdout_color_sinks.h>

class GLogger
{
public:
    static void Initialize(const std::string& serverType)
    {
        if (_loggers.find(serverType) != _loggers.end())
            return;

        spdlog::init_thread_pool(8192, 1);
        std::string logPath = "Log/" + serverType + ".log";

        auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(logPath, 10485760, 5);
        file_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e][%l][thread %t] %v");

        auto logger = std::make_shared<spdlog::async_logger>(
            serverType,
            file_sink,
            spdlog::thread_pool(),
            spdlog::async_overflow_policy::block
            );

        spdlog::register_logger(logger);
        logger->set_level(spdlog::level::info);
        logger->flush_on(spdlog::level::err);

        _loggers[serverType] = logger;

        // 최초 초기화 시 default logger 설정
        if (!_currentLogger)
            _currentLogger = logger;
    }

    static void SetCurrentLogger(const std::string& serverType)
    {
        auto it = _loggers.find(serverType);
        if (it != _loggers.end())
            _currentLogger = it->second;
    }

    template<typename... Args>
    static void Log(spdlog::level::level_enum level, const std::string& fmt, Args&&... args)
    {
        if (_currentLogger)
            _currentLogger->log(level, fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    static void LogWithContext(spdlog::level::level_enum level, const std::string& name, const std::string& action, const std::string& fmt, Args&&... args)
    {
        if (_currentLogger)
        {
            std::string tagged_fmt = fmt::format("[{}][{}] ", name, action) + fmt;
            _currentLogger->log(level, tagged_fmt, std::forward<Args>(args)...);
        }
    }

private:
    static inline std::unordered_map<std::string, std::shared_ptr<spdlog::logger>> _loggers;
    static inline std::shared_ptr<spdlog::logger> _currentLogger = nullptr;
};