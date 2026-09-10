// Shell.hpp - read-eval-print loop: parses input, dispatches to builtins or spawns
// external processes. All collaborators are constructor-injected (see
// docs/architecture.md#design-principles).
#pragma once

#include "IShell.hpp"
#include "ICommandParser.hpp"
#include "IBuiltinCommands.hpp"
#include "IProcessManager.hpp"
#include "ILogger.hpp"

#include <string>

class Shell : public IShell {
public:
    Shell(ICommandParser& parser, IBuiltinCommands& builtins, IProcessManager& processes, ILogger& logger);

    int run() override;

private:
    ICommandParser& parser_;
    IBuiltinCommands& builtins_;
    IProcessManager& processes_;
    ILogger& logger_;

    std::string prompt_ = "neytra> ";
};

