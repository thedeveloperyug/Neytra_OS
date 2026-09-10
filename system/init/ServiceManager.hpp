// ServiceManager.hpp - reads a simple service config and spawns each entry via
// the constructor-injected IProcessManager (see docs/architecture.md#design-principles).
#pragma once

#include "IServiceManager.hpp"
#include "ILogger.hpp"
#include "IProcessManager.hpp"

#include <string>
#include <vector>

struct ServiceSpec {
    std::string name;
    std::string command;
    std::vector<std::string> args;
};

class ServiceManager : public IServiceManager {
public:
    // `configPath` holds one service per line: name|command|arg1,arg2,...
    // An empty or unreadable configPath means "no services configured" (not a failure).
    ServiceManager(IProcessManager& processes, ILogger& logger, std::string configPath = "");

    bool startAll() override;

    // Exposed for testing without needing a real file on disk.
    static std::vector<ServiceSpec> parseConfig(const std::string& text);

private:
    IProcessManager& processes_;
    ILogger& logger_;
    std::string configPath_;
};

