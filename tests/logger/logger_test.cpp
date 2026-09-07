// logger_test.cpp - smoke test: exercises every log level and verifies file output.
#include "Logger.hpp"

#include <cstdio>
#include <fstream>
#include <sstream>
#include <string>

namespace {
// Takes ILogger&, not Logger&, to prove callers can depend on the interface alone.
void logStartupMessage(ILogger& logger) {
    logger.info("test", "invoked through ILogger interface (dependency injection demo)");
}
}  // namespace

int main() {
    const std::string logPath = "/tmp/neytra_logger_test.log";
    std::remove(logPath.c_str());

    Logger& log = Logger::instance();
    log.setMinLevel(LogLevel::Debug);
    if (!log.setLogFile(logPath)) {
        std::fprintf(stderr, "failed to open log file: %s\n", logPath.c_str());
        return 1;
    }

    log.debug("test", "debug message");
    log.info("test", "info message");
    log.warn("test", "warn message");
    log.error("test", "error message");
    logStartupMessage(log);

    std::ifstream in(logPath);
    std::stringstream contents;
    contents << in.rdbuf();
    const std::string text = contents.str();

    for (const char* word : {"DEBUG", "INFO", "WARN", "ERROR", "dependency injection demo"}) {
        if (text.find(word) == std::string::npos) {
            std::fprintf(stderr, "expected \"%s\" in log file, got:\n%s\n", word, text.c_str());
            return 1;
        }
    }

    std::printf("logger_test: OK\n");
    return 0;
}

