// Installer.hpp - extracts archives by spawning `tar` through IProcessManager
// (not reimplementing tar's format parsing -- see docs/architecture.md#design-principles
// design principle #6 on not over-building things a standard tool already does well).
#pragma once

#include "IInstaller.hpp"
#include "ILogger.hpp"
#include "IProcessManager.hpp"

class Installer : public IInstaller {
public:
    Installer(IProcessManager& processes, ILogger& logger);

    bool install(const std::string& archivePath, const std::string& destDir) override;

private:
    IProcessManager& processes_;
    ILogger& logger_;
};
