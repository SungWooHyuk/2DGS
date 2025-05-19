#pragma once

#include <memory>
#include <string>
#include <chrono>
#include <unordered_map>
#include <mutex>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/async.h>
#include <spdlog/sinks/stdout_color_sinks.h>

class GLogger
{
public:
	static void Initialize(const std::string& serverType)
	{

		// 이미 존재하는 로거면 무시
		if (_loggers.find(serverType) != _loggers.end())
			return;

		spdlog::init_thread_pool(8192, 1);

		std::string logPath = "Log/" + serverType + ".log";
		auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(logPath, 10485760, 5);
		file_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e][%l][thread %t] %v");

		auto logger = std::make_shared<spdlog::async_logger>(
			serverType, // 이름 다르게
			file_sink,
			spdlog::thread_pool(),
			spdlog::async_overflow_policy::block
			);

		spdlog::register_logger(logger);
		logger->set_level(spdlog::level::info);
		logger->flush_on(spdlog::level::err);

		_loggers[serverType] = logger;
		_currentLogger = logger; // 마지막 초기화된 서버를 기본 로거로
	}

	template<typename... Args>
	static void Log(spdlog::level::level_enum level, const std::string& fmt, Args&&... args)
	{
		if (_currentLogger)
			_currentLogger->log(level, fmt, std::forward<Args>(args)...);
	}

	template<typename... Args>
	static void LogWithContext(spdlog::level::level_enum level, const std::string& playerName, const std::string& action, const std::string& fmt, Args&&... args)
	{
		if (_currentLogger)
		{
			std::string tagged_fmt = fmt::format("[Player:{}][Action:{}] ", playerName, action) + fmt;
			_currentLogger->log(level, tagged_fmt, std::forward<Args>(args)...);
		}
	}

	class LogDuration
	{
	public:
		LogDuration(const std::string& label) : _label(label), _start(std::chrono::steady_clock::now()) {}
		~LogDuration()
		{
			auto end = std::chrono::steady_clock::now();
			auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - _start).count();
			GLogger::Log(spdlog::level::info, "[Duration][{}] {}ms", _label, ms);
		}

	private:
		std::string _label;
		std::chrono::steady_clock::time_point _start;
	};

private:
	static inline std::unordered_map<std::string, std::shared_ptr<spdlog::logger>> _loggers;
	static inline std::shared_ptr<spdlog::logger> _currentLogger = nullptr;
};
