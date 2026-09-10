// BuiltinCommands.cpp - cd, pwd, echo, export, unset, env, exit, help.
#include "BuiltinCommands.hpp"

#include <unistd.h>

#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <iostream>

extern char** environ;

namespace {
const char* kBuiltinNames[] = {"cd", "pwd", "echo", "export", "unset", "env", "exit", "help"};
}  // namespace

BuiltinCommands::BuiltinCommands(ILogger& logger) : logger_(logger) {}

bool BuiltinCommands::isBuiltin(const std::string& command) const {
    for (const char* name : kBuiltinNames) {
        if (command == name) {
            return true;
        }
    }
    return false;
}

int BuiltinCommands::run(const std::string& command, const std::vector<std::string>& args, bool& shouldExit) {
    shouldExit = false;

    if (command == "cd") {
        const std::string target = args.empty() ? (std::getenv("HOME") ? std::getenv("HOME") : "/") : args[0];
        if (chdir(target.c_str()) != 0) {
            std::cerr << "cd: " << target << ": " << std::strerror(errno) << "\n";
            return 1;
        }
        return 0;
    }

    if (command == "pwd") {
        char buffer[4096];
        if (getcwd(buffer, sizeof(buffer)) != nullptr) {
            std::cout << buffer << "\n";
            return 0;
        }
        std::cerr << "pwd: " << std::strerror(errno) << "\n";
        return 1;
    }

    if (command == "echo") {
        for (size_t i = 0; i < args.size(); ++i) {
            if (i > 0) {
                std::cout << ' ';
            }
            std::cout << args[i];
        }
        std::cout << "\n";
        return 0;
    }

    if (command == "export") {
        for (const auto& assignment : args) {
            const size_t eq = assignment.find('=');
            if (eq == std::string::npos) {
                continue;
            }
            setenv(assignment.substr(0, eq).c_str(), assignment.substr(eq + 1).c_str(), 1);
        }
        return 0;
    }

    if (command == "unset") {
        for (const auto& name : args) {
            unsetenv(name.c_str());
        }
        return 0;
    }

    if (command == "env") {
        for (char** entry = environ; *entry != nullptr; ++entry) {
            std::cout << *entry << "\n";
        }
        return 0;
    }

    if (command == "exit") {
        shouldExit = true;
        return args.empty() ? 0 : std::atoi(args[0].c_str());
    }

    if (command == "help") {
        std::cout << "Neytra shell builtins: cd pwd echo export unset env exit help\n";
        return 0;
    }

    logger_.warn("BuiltinCommands", "unhandled builtin: " + command);
    return 1;
}

