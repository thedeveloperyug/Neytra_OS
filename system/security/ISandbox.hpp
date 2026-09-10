// ISandbox.hpp - process-confinement interface (depend on this, not Sandbox).
#pragma once

#include <string>
#include <vector>

struct SandboxLimits {
    long maxMemoryBytes = 0;  // RLIMIT_AS; 0 = unlimited
    long maxCpuSeconds = 0;   // RLIMIT_CPU; 0 = unlimited
    long maxProcesses = 0;    // RLIMIT_NPROC; 0 = unlimited
};

class ISandbox {
public:
    virtual ~ISandbox() = default;

    // Runs `path` with `args` confined by `limits`. If `chrootDir` is non-empty,
    // chroot()s into it first (requires root). If `uid` is non-zero, drops to that
    // uid/gid after chrooting (requires starting as root). Returns the child's exit
    // code, or -1 if the process could not be launched at all.
    virtual int run(const std::string& path, const std::vector<std::string>& args,
                     unsigned int uid, const std::string& chrootDir,
                     const SandboxLimits& limits) = 0;
};
