// ILogger.hpp - logging interface. Depend on this, not the concrete Logger, so
// callers stay decoupled from the singleton and can be given a fake/mock in tests.
#pragma once

#include <string>

enum class LogLevel { Debug, Info, Warn, Error };

class ILogger {
public:
    virtual ~ILogger() = default;

    virtual void log(LogLevel level, const std::string& tag, const std::string& message) = 0;

    virtual void debug(const std::string& tag, const std::string& message) = 0;
    virtual void info(const std::string& tag, const std::string& message) = 0;
    virtual void warn(const std::string& tag, const std::string& message) = 0;
    virtual void error(const std::string& tag, const std::string& message) = 0;
};
