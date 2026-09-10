// ProcessManager.cpp - real fork/exec/wait-based process lifecycle management.
#include "ProcessManager.hpp"

#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>

ProcessManager::ProcessManager(ILogger& logger) : logger_(logger) {}

ProcessId ProcessManager::spawn(const std::string& path, const std::vector<std::string>& args) {
    const pid_t pid = fork();
    if (pid < 0) {
        logger_.error("ProcessManager", std::string("fork failed: ") + std::strerror(errno));
        return -1;
    }

    if (pid == 0) {
        std::vector<char*> argv;
        argv.push_back(const_cast<char*>(path.c_str()));
        for (const auto& arg : args) {
            argv.push_back(const_cast<char*>(arg.c_str()));
        }
        argv.push_back(nullptr);
        execvp(path.c_str(), argv.data());
        _exit(127);  // execvp only returns on failure
    }

    ProcessInfo info;
    info.pid = pid;
    info.command = path;
    info.running = true;
    processes_[pid] = info;
    logger_.info("ProcessManager", "spawned " + path + " (pid " + std::to_string(pid) + ")");
    return pid;
}

bool ProcessManager::wait(ProcessId pid, int& exitCode) {
    auto it = processes_.find(pid);
    if (it == processes_.end()) {
        return false;
    }

    int status = 0;
    if (waitpid(pid, &status, 0) < 0) {
        logger_.error("ProcessManager", std::string("waitpid failed: ") + std::strerror(errno));
        return false;
    }

    exitCode = WIFEXITED(status) ? WEXITSTATUS(status) : -1;
    it->second.running = false;
    it->second.exitCode = exitCode;
    logger_.info("ProcessManager", "pid " + std::to_string(pid) + " exited with " + std::to_string(exitCode));
    return true;
}

bool ProcessManager::isRunning(ProcessId pid) {
    auto it = processes_.find(pid);
    if (it == processes_.end()) {
        return false;
    }
    if (!it->second.running) {
        return false;
    }

    int status = 0;
    const pid_t result = waitpid(pid, &status, WNOHANG);
    if (result == 0) {
        return true;  // still running
    }
    if (result == pid) {
        it->second.running = false;
        it->second.exitCode = WIFEXITED(status) ? WEXITSTATUS(status) : -1;
    }
    return false;
}

bool ProcessManager::kill(ProcessId pid, int signal) {
    if (::kill(pid, signal) != 0) {
        logger_.warn("ProcessManager", "kill(" + std::to_string(pid) + ") failed: " + std::strerror(errno));
        return false;
    }
    return true;
}

std::vector<ProcessInfo> ProcessManager::list() const {
    std::vector<ProcessInfo> result;
    result.reserve(processes_.size());
    for (const auto& [pid, info] : processes_) {
        result.push_back(info);
    }
    return result;
}

