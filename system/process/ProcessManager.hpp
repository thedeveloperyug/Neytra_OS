// ProcessManager.hpp - real fork/exec/wait-based process lifecycle management.
#pragma once

#include "IProcessManager.hpp"
#include "ILogger.hpp"

#include <map>

class ProcessManager : public IProcessManager {
public:
    explicit ProcessManager(ILogger& logger);

    ProcessId spawn(const std::string& path, const std::vector<std::string>& args) override;
    bool wait(ProcessId pid, int& exitCode) override;
    bool isRunning(ProcessId pid) override;
    bool kill(ProcessId pid, int signal) override;
    std::vector<ProcessInfo> list() const override;

private:
    ILogger& logger_;
    std::map<ProcessId, ProcessInfo> processes_;
};
