#pragma once
#include <iostream>
#include <sstream>
#include <mutex>
#include <chrono>
#include <iomanip>
#include <ctime>

namespace msgnet
{

enum class LogLevel
{
    DEBUG = 0,
    INFO = 1,
    WARN = 2,
    ERROR = 3,
    NONE = 4
};

class Logger
{
public:
    static Logger &instance()
    {
        static Logger logger;
        return logger;
    }

    void setLevel(LogLevel level)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        level_ = level;
    }

    LogLevel getLevel() const
    {
        return level_;
    }

    template <typename... Args>
    void debug(Args &&...args)
    {
        log(LogLevel::DEBUG, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void info(Args &&...args)
    {
        log(LogLevel::INFO, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void warn(Args &&...args)
    {
        log(LogLevel::WARN, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void error(Args &&...args)
    {
        log(LogLevel::ERROR, std::forward<Args>(args)...);
    }

private:
    Logger() : level_(LogLevel::INFO) {}

    template <typename... Args>
    void log(LogLevel level, Args &&...args)
    {
        if (level < level_)
            return;

        std::lock_guard<std::mutex> lock(mutex_);

        // 타임스탬프
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                      now.time_since_epoch()) %
                  1000;

        std::tm tm{};
        localtime_r(&time_t, &tm);

        std::ostringstream oss;
        oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S")
            << '.' << std::setfill('0') << std::setw(3) << ms.count()
            << " [" << levelToString(level) << "] ";

        // 메시지 조합
        (oss << ... << args);
        oss << '\n';

        // 출력 (ERROR는 stderr, 나머지는 stdout)
        if (level >= LogLevel::ERROR)
        {
            std::cerr << oss.str() << std::flush;
        }
        else
        {
            std::cout << oss.str() << std::flush;
        }
    }

    const char *levelToString(LogLevel level) const
    {
        switch (level)
        {
        case LogLevel::DEBUG:
            return "DEBUG";
        case LogLevel::INFO:
            return "INFO ";
        case LogLevel::WARN:
            return "WARN ";
        case LogLevel::ERROR:
            return "ERROR";
        default:
            return "UNKNOWN";
        }
    }

    LogLevel level_;
    std::mutex mutex_;
};

// 편의 매크로
#define LOG_DEBUG(...) Logger::instance().debug(__VA_ARGS__)
#define LOG_INFO(...) Logger::instance().info(__VA_ARGS__)
#define LOG_WARN(...) Logger::instance().warn(__VA_ARGS__)
#define LOG_ERROR(...) Logger::instance().error(__VA_ARGS__)

} // namespace msgnet