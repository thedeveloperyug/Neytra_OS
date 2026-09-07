// Logger.cpp - leveled logger shared by the Neytra OS system/ subsystems.
#include "Logger.hpp"

#include <cstdio>
#include <ctime>

namespace {

const char* levelName(LogLevel level) {
    switch (level) {
        case LogLevel::Debug: return "DEBUG";
        case LogLevel::Info:  return "INFO";
        case LogLevel::Warn:  return "WARN";
        case LogLevel::Error: return "ERROR";
    }
    return "?";
}

// Matches the ANSI palette used by the boot banner (rootfs/init).
const char* levelColor(LogLevel level) {
    switch (level) {
        case LogLevel::Debug: return "\033[2m";
        case LogLevel::Info:  return "\033[1;36m";
        case LogLevel::Warn:  return "\033[1;33m";
        case LogLevel::Error: return "\033[1;31m";
    }
    return "";
}

std::string timestamp() {
    const std::time_t now = std::time(nullptr);
    std::tm tmBuf{};
    localtime_r(&now, &tmBuf);
    char buf[16];
    std::strftime(buf, sizeof(buf), "%H:%M:%S", &tmBuf);
    return buf;
}

} // namespace

Logger& Logger::instance() {
    static Logger instance;
    return instance;
}

void Logger::setMinLevel(LogLevel level) {
    minLevel_ = level;
}

bool Logger::setLogFile(const std::string& path) {
    logFile_.open(path, std::ios::app);
    return logFile_.is_open();
}

void Logger::log(LogLevel level, const std::string& tag, const std::string& message) {
    if (level < minLevel_) {
        return;
    }

    const std::string time = timestamp();
    std::FILE* stream = (level >= LogLevel::Warn) ? stderr : stdout;
    std::fprintf(stream, "%s[%s] [%s] [%s] %s\033[0m\n",
                 levelColor(level), time.c_str(), levelName(level), tag.c_str(), message.c_str());

    if (logFile_.is_open()) {
        logFile_ << '[' << time << "] [" << levelName(level) << "] [" << tag << "] " << message << '\n';
        logFile_.flush();
    }
}

void Logger::debug(const std::string& tag, const std::string& message) { log(LogLevel::Debug, tag, message); }
void Logger::info(const std::string& tag, const std::string& message)  { log(LogLevel::Info,  tag, message); }
void Logger::warn(const std::string& tag, const std::string& message)  { log(LogLevel::Warn,  tag, message); }
void Logger::error(const std::string& tag, const std::string& message) { log(LogLevel::Error, tag, message); }

