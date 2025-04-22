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
	// �α� �ý��� �ʱ�ȭ
	static void Init(const std::string& serverType)
	{
		if (_logger)
			return; // �̹� �ʱ�ȭ�Ǿ� �ִٸ� ����

		std::string logPath = "Log/" + serverType + ".log";

		// �񵿱� �α��� ���� thread pool �ʱ�ȭ
		// ť ������: 8192 / ������ ��: 1
		spdlog::init_thread_pool(8192, 1);

		// Rotating sink: �ִ� 10MB, 5�� ���ϱ��� ����
		auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(logPath, 10485760, 5);
		file_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e][%l][thread %t] %v");

		// �񵿱� �ΰ� ����
		_logger = std::make_shared<spdlog::async_logger>(
			"GLogger",
			file_sink,
			spdlog::thread_pool(),
			spdlog::async_overflow_policy::block
			);

		spdlog::register_logger(_logger);
		_logger->set_level(spdlog::level::info);       // info �̻� ���
		_logger->flush_on(spdlog::level::err);         // err �̻� �α״� ��� ��ũ�� flush
	}

	// �Ϲ� �α� �Լ� (���� + ���� ���ڿ� + ���ڵ�)
	template<typename... Args>
	static void Log(spdlog::level::level_enum level, const std::string& fmt, Args&&... args)
	{
		if (_logger)
			_logger->log(level, fmt, std::forward<Args>(args)...);
	}

	// Context ���� �α� (�÷��̾� �̸�, �׼Ǹ�)
	template<typename... Args>
	static void LogWithContext(spdlog::level::level_enum level, const std::string& playerName, const std::string& action, const std::string& fmt, Args&&... args)
	{
		if (_logger)
		{
			std::string tagged_fmt = fmt::format("[Player:{}][Action:{}] ", playerName, action) + fmt;
			_logger->log(level, tagged_fmt, std::forward<Args>(args)...);
		}
	}

	// �ð� ������ �α� ��ƿ (LogDuration)
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
	// static inline: ��� �ߺ� ���� ���� (C++17 �̻�)
	static inline std::shared_ptr<spdlog::logger> _logger = nullptr;
};
