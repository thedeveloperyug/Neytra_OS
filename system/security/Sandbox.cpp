// Sandbox.cpp - fork/exec-based process confinement (rlimits, optional chroot+setuid).
#include "Sandbox.hpp"

#include <sys/resource.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <vector>

Sandbox::Sandbox(ILogger& logger) : logger_(logger) {}

int Sandbox::run(const std::string& path, const std::vector<std::string>& args,
                  unsigned int uid, const std::string& chrootDir,
                  const SandboxLimits& limits) {
    const pid_t pid = fork();
    if (pid < 0) {
        logger_.error("Sandbox", std::string("fork failed: ") + std::strerror(errno));
        return -1;
    }

    if (pid == 0) {
        // Child: apply confinement, then exec. Any failure here exits non-zero
        // rather than returning, so the parent never mistakes this for the real program.
        if (!chrootDir.empty()) {
            if (chroot(chrootDir.c_str()) != 0 || chdir("/") != 0) {
                _exit(126);
            }
        }
        if (limits.maxMemoryBytes > 0) {
            struct rlimit rl{static_cast<rlim_t>(limits.maxMemoryBytes), static_cast<rlim_t>(limits.maxMemoryBytes)};
            setrlimit(RLIMIT_AS, &rl);
        }
        if (limits.maxCpuSeconds > 0) {
            struct rlimit rl{static_cast<rlim_t>(limits.maxCpuSeconds), static_cast<rlim_t>(limits.maxCpuSeconds)};
            setrlimit(RLIMIT_CPU, &rl);
        }
        if (limits.maxProcesses > 0) {
            struct rlimit rl{static_cast<rlim_t>(limits.maxProcesses), static_cast<rlim_t>(limits.maxProcesses)};
            setrlimit(RLIMIT_NPROC, &rl);
        }
        if (uid != 0) {
            if (setgid(uid) != 0 || setuid(uid) != 0) {
                _exit(126);
            }
        }

        std::vector<char*> argv;
        argv.push_back(const_cast<char*>(path.c_str()));
        for (const auto& arg : args) {
            argv.push_back(const_cast<char*>(arg.c_str()));
        }
        argv.push_back(nullptr);

        execvp(path.c_str(), argv.data());
        _exit(127);  // execvp only returns on failure
    }

    int status = 0;
    if (waitpid(pid, &status, 0) < 0) {
        logger_.error("Sandbox", std::string("waitpid failed: ") + std::strerror(errno));
        return -1;
    }

    const int exitCode = WIFEXITED(status) ? WEXITSTATUS(status) : -1;
    logger_.info("Sandbox", "ran " + path + " (pid " + std::to_string(pid) +
                 ") -> exit " + std::to_string(exitCode));
    return exitCode;
}

