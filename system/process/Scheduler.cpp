// Scheduler.cpp - real OS priority (nice value) management via setpriority(2).
#include "Scheduler.hpp"

#include <sys/resource.h>

#include <algorithm>
#include <cerrno>
#include <cstring>

Scheduler::Scheduler(ILogger& logger) : logger_(logger) {}

bool Scheduler::setPriority(ProcessId pid, int niceValue) {
    errno = 0;
    if (setpriority(PRIO_PROCESS, pid, niceValue) != 0 && errno != 0) {
        logger_.warn("Scheduler", "setpriority(" + std::to_string(pid) + ") failed: " + std::strerror(errno));
        return false;
    }
    if (std::find(registered_.begin(), registered_.end(), pid) == registered_.end()) {
        registered_.push_back(pid);
    }
    logger_.info("Scheduler", "pid " + std::to_string(pid) + " nice=" + std::to_string(niceValue));
    return true;
}

int Scheduler::getPriority(ProcessId pid) const {
    errno = 0;
    const int value = getpriority(PRIO_PROCESS, pid);
    if (value == -1 && errno != 0) {
        return 0;  // unknown/inaccessible pid: report the default nice value
    }
    return value;
}

std::vector<ProcessId> Scheduler::orderByPriority() const {
    std::vector<ProcessId> ordered = registered_;
    std::sort(ordered.begin(), ordered.end(), [this](ProcessId a, ProcessId b) {
        return getPriority(a) < getPriority(b);  // lower nice value = higher priority
    });
    return ordered;
}

