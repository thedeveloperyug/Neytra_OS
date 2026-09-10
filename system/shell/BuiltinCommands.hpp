// BuiltinCommands.hpp - cd, pwd, echo, export, unset, env, exit, help.
#pragma once

#include "IBuiltinCommands.hpp"
#include "ILogger.hpp"

class BuiltinCommands : public IBuiltinCommands {
public:
    explicit BuiltinCommands(ILogger& logger);

    bool isBuiltin(const std::string& command) const override;
    int run(const std::string& command, const std::vector<std::string>& args, bool& shouldExit) override;

private:
    ILogger& logger_;
};

