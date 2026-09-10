// Shell.cpp - read-eval-print loop.
#include "Shell.hpp"

#include <iostream>

Shell::Shell(ICommandParser& parser, IBuiltinCommands& builtins, IProcessManager& processes, ILogger& logger)
    : parser_(parser), builtins_(builtins), processes_(processes), logger_(logger) {}

int Shell::run() {
    logger_.info("Shell", "starting interactive session");

    std::string line;
    int lastExitCode = 0;
    bool shouldExit = false;

    while (!shouldExit) {
        std::cout << prompt_;
        if (!std::getline(std::cin, line)) {
            break;  // EOF (e.g. piped input ran out, or Ctrl+D)
        }

        const ParsedCommand parsed = parser_.parse(line);
        if (!parsed.valid) {
            continue;  // blank line or comment
        }

        if (builtins_.isBuiltin(parsed.command)) {
            lastExitCode = builtins_.run(parsed.command, parsed.args, shouldExit);
            continue;
        }

        const ProcessId pid = processes_.spawn(parsed.command, parsed.args);
        if (pid < 0) {
            std::cerr << parsed.command << ": command not found\n";
            lastExitCode = 127;
            continue;
        }
        processes_.wait(pid, lastExitCode);
    }

    logger_.info("Shell", "session ended");
    return lastExitCode;
}

