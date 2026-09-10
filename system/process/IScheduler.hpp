// IScheduler.hpp - process priority interface (depend on this, not Scheduler).
#pragma once

#include "IProcessManager.hpp"

#include <vector>

class IScheduler {
public:
    virtual ~IScheduler() = default;

    // Sets the OS nice value for `pid` (-20 highest priority .. 19 lowest). Requires
    // appropriate privilege to lower niceValue below 0 on most systems.
    virtual bool setPriority(ProcessId pid, int niceValue) = 0;
    virtual int getPriority(ProcessId pid) const = 0;

    // Registered pids ordered by priority, highest priority (lowest nice value) first.
    virtual std::vector<ProcessId> orderByPriority() const = 0;
};
