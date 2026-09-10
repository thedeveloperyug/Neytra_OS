// shell_test.cpp - GTest suite for CommandParser, BuiltinCommands, and Shell.
#include "BuiltinCommands.hpp"
#include "CommandParser.hpp"
#include "ProcessManager.hpp"
#include "Shell.hpp"
#include "Logger.hpp"

#include <gtest/gtest.h>

#include <unistd.h>

#include <cstdio>
#include <fstream>

TEST(CommandParserTest, RejectsBlankAndCommentLines) {
    CommandParser parser;
    EXPECT_FALSE(parser.parse("   ").valid);
    EXPECT_FALSE(parser.parse("# a comment").valid);
}

TEST(CommandParserTest, SplitsQuotedArguments) {
    CommandParser parser;
    const ParsedCommand quoted = parser.parse(R"(echo "hello world" again)");
    ASSERT_TRUE(quoted.valid);
    EXPECT_EQ(quoted.command, "echo");
    ASSERT_EQ(quoted.args.size(), 2u);
    EXPECT_EQ(quoted.args[0], "hello world");
    EXPECT_EQ(quoted.args[1], "again");
}

TEST(BuiltinCommandsTest, RecognizesBuiltinsAndHandlesExit) {
    BuiltinCommands builtins(Logger::instance());
    EXPECT_TRUE(builtins.isBuiltin("cd"));
    EXPECT_FALSE(builtins.isBuiltin("ls"));

    bool shouldExit = false;
    const int exitCode = builtins.run("exit", {"3"}, shouldExit);
    EXPECT_TRUE(shouldExit);
    EXPECT_EQ(exitCode, 3);
}

class ShellTest : public ::testing::Test {
protected:
    void SetUp() override {
        savedStdinFd_ = dup(fileno(stdin));
        ASSERT_GE(savedStdinFd_, 0) << "failed to save stdin";

        std::ofstream script(scriptPath_, std::ios::trunc);
        script << "# comment line, should be skipped\n";
        script << "export GREETING=hi\n";
        script << "echo shell-test-ok\n";
        script << "exit 7\n";
    }

    void TearDown() override {
        std::fflush(stdin);
        dup2(savedStdinFd_, fileno(stdin));
        close(savedStdinFd_);
        std::remove(scriptPath_.c_str());
    }

    const std::string scriptPath_ = "/tmp/neytra_shell_test_script";
    int savedStdinFd_ = -1;
};

TEST_F(ShellTest, RunsScriptFromStdinEndToEnd) {
    ASSERT_NE(std::freopen(scriptPath_.c_str(), "r", stdin), nullptr) << "freopen stdin failed";

    Logger& log = Logger::instance();
    CommandParser parser;
    BuiltinCommands builtins(log);
    ProcessManager processes(log);
    Shell shell(parser, builtins, processes, log);
    const int shellExit = shell.run();

    EXPECT_EQ(shellExit, 7);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    Logger::instance().setMinLevel(LogLevel::Error);
    return RUN_ALL_TESTS();
}
