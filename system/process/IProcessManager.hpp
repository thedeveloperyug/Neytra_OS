// IProcessManager.hpp - process lifecycle interface (depend on this, not ProcessManager).
#pragma once

#include <string>
#include <vector>

using ProcessId = int;

struct ProcessInfo {
    ProcessId pid = -1;
    std::string command;
    bool running = false;
    int exitCode = 0;  // valid only when running == false
};

class IProcessManager {
public:
    virtual ~IProcessManager() = default;

    // fork()+execvp()s `path` with `args`. Returns the child pid, or -1 on failure.
    virtual ProcessId spawn(const std::string& path, const std::vector<std::string>& args) = 0;

    // Blocks until `pid` exits; fills in its exit code. Returns false if `pid` is unknown.
    virtual bool wait(ProcessId pid, int& exitCode) = 0;

    // Non-blocking check (WNOHANG); updates internal state if the process has exited.
    virtual bool isRunning(ProcessId pid) = 0;

    virtual bool kill(ProcessId pid, int signal) = 0;

    virtual std::vector<ProcessInfo> list() const = 0;
};
