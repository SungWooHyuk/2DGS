#pragma once

#include <memory>
#include <string>
#include <chrono>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/async.h>
#include <spdlog/sinks/stdout_color_sinks.h>

class GLogger
{
public:
	// 로그 시스템 초기화
	static void Init(const std::string& serverType)
	{
		if (_logger)
			return; // 이미 초기화되어 있다면 무시

		std::string logPath = "Log/" + serverType + ".log";

		// 비동기 로깅을 위한 thread pool 초기화
		// 큐 사이즈: 8192 / 쓰레드 수: 1
		spdlog::init_thread_pool(8192, 1);

		// Rotating sink: 최대 10MB, 5개 파일까지 유지
		auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(logPath, 10485760, 5);
		file_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e][%l][thread %t] %v");

		// 비동기 로거 생성
		_logger = std::make_shared<spdlog::async_logger>(
			"GLogger",
			file_sink,
			spdlog::thread_pool(),
			spdlog::async_overflow_policy::block
			);

		spdlog::register_logger(_logger);
		_logger->set_level(spdlog::level::info);       // info 이상만 기록
		_logger->flush_on(spdlog::level::err);         // err 이상 로그는 즉시 디스크에 flush
	}

	// 일반 로그 함수 (레벨 + 포맷 문자열 + 인자들)
	template<typename... Args>
	static void Log(spdlog::level::level_enum level, const std::string& fmt, Args&&... args)
	{
		if (_logger)
			_logger->log(level, fmt, std::forward<Args>(args)...);
	}

	// Context 포함 로그 (플레이어 이름, 액션명)
	template<typename... Args>
	static void LogWithContext(spdlog::level::level_enum level, const std::string& playerName, const std::string& action, const std::string& fmt, Args&&... args)
	{
		if (_logger)
		{
			std::string tagged_fmt = fmt::format("[Player:{}][Action:{}] ", playerName, action) + fmt;
			_logger->log(level, tagged_fmt, std::forward<Args>(args)...);
		}
	}

	// 시간 측정용 로그 유틸 (LogDuration)
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
	// static inline: 헤더 중복 정의 방지 (C++17 이상)
	static inline std::shared_ptr<spdlog::logger> _logger = nullptr;
};
