// neytra-log.cpp - tiny CLI around Logger/ILogger for use from shell scripts
// (e.g. rootfs/init), so boot-time events go through the real logging
// pipeline instead of raw `echo`. This main() is a composition root, so it's
// one of the few places allowed to call Logger::instance() directly.
//
// Usage:   neytra-log <debug|info|warn|error> <tag> <message...>
// If $NEYTRA_LOG_FILE is set, messages are also appended there (plain text,
// no ANSI) in addition to the always-on colored console output.
#include "Logger.hpp"

#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <string>

namespace {

bool parseLevel(const char* text, LogLevel& outLevel) {
    std::string level = text;
    for (char& c : level) {
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    if (level == "debug") { outLevel = LogLevel::Debug; return true; }
    if (level == "info")  { outLevel = LogLevel::Info;  return true; }
    if (level == "warn")  { outLevel = LogLevel::Warn;  return true; }
    if (level == "error") { outLevel = LogLevel::Error; return true; }
    return false;
}

}  // namespace

int main(int argc, char** argv) {
    if (argc < 4) {
        std::fprintf(stderr, "usage: %s <debug|info|warn|error> <tag> <message...>\n", argv[0]);
        return 1;
    }

    LogLevel level;
    if (!parseLevel(argv[1], level)) {
        std::fprintf(stderr, "unknown level \"%s\" (want debug|info|warn|error)\n", argv[1]);
        return 1;
    }

    const std::string tag = argv[2];
    std::string message = argv[3];
    for (int i = 4; i < argc; ++i) {
        message += ' ';
        message += argv[i];
    }

    Logger& logger = Logger::instance();
    logger.setMinLevel(LogLevel::Debug);
    if (const char* logFile = std::getenv("NEYTRA_LOG_FILE")) {
        logger.setLogFile(logFile);
    }

    logger.log(level, tag, message);
    return 0;
}
