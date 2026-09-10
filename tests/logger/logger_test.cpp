// logger_test.cpp - GTest suite: exercises every log level and verifies file output.
#include "Logger.hpp"

#include <gtest/gtest.h>

#include <cstdio>
#include <fstream>
#include <sstream>
#include <string>

namespace {

// Takes ILogger&, not Logger&, to prove callers can depend on the interface alone.
void logStartupMessage(ILogger& logger) {
    logger.info("test", "invoked through ILogger interface (dependency injection demo)");
}

std::string readFile(const std::string& path) {
    std::ifstream in(path);
    std::stringstream contents;
    contents << in.rdbuf();
    return contents.str();
}

}  // namespace

class LoggerTest : public ::testing::Test {
protected:
    void SetUp() override {
        std::remove(logPath_.c_str());
        log_.setMinLevel(LogLevel::Debug);
        ASSERT_TRUE(log_.setLogFile(logPath_)) << "failed to open log file: " << logPath_;
    }

    void TearDown() override {
        std::remove(logPath_.c_str());
        log_.setMinLevel(LogLevel::Error);  // restore the shared singleton's quiet default
    }

    Logger& log_ = Logger::instance();
    const std::string logPath_ = "/tmp/neytra_logger_test.log";
};

TEST_F(LoggerTest, WritesAllLevelsAndInterfaceCallsToFile) {
    log_.debug("test", "debug message");
    log_.info("test", "info message");
    log_.warn("test", "warn message");
    log_.error("test", "error message");
    logStartupMessage(log_);

    const std::string text = readFile(logPath_);
    for (const char* word : {"DEBUG", "INFO", "WARN", "ERROR", "dependency injection demo"}) {
        EXPECT_NE(text.find(word), std::string::npos)
            << "expected \"" << word << "\" in log file, got:\n" << text;
    }
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

