// Logger.hpp - default ILogger implementation shared by the Neytra OS system/ subsystems.
#pragma once

#include "ILogger.hpp"

#include <fstream>
#include <string>

class Logger : public ILogger {
public:
    static Logger& instance();

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    // Messages below this level are dropped. Default: LogLevel::Info.
    void setMinLevel(LogLevel level);

    // Also append plain (uncolored) output to this file; returns false on failure.
    bool setLogFile(const std::string& path);

    void log(LogLevel level, const std::string& tag, const std::string& message) override;

    void debug(const std::string& tag, const std::string& message) override;
    void info(const std::string& tag, const std::string& message) override;
    void warn(const std::string& tag, const std::string& message) override;
    void error(const std::string& tag, const std::string& message) override;

private:
    Logger() = default;

    LogLevel minLevel_ = LogLevel::Info;
    std::ofstream logFile_;
};


