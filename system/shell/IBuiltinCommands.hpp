// IBuiltinCommands.hpp - shell-builtin interface (depend on this, not BuiltinCommands).
#pragma once

#include <string>
#include <vector>

class IBuiltinCommands {
public:
    virtual ~IBuiltinCommands() = default;

    virtual bool isBuiltin(const std::string& command) const = 0;

    // Runs `command` (already known to be a builtin). Sets shouldExit=true if the
    // shell's REPL loop should terminate (i.e. the "exit" builtin was run).
    virtual int run(const std::string& command, const std::vector<std::string>& args, bool& shouldExit) = 0;
};
