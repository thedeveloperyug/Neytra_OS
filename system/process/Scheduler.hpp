// Scheduler.hpp - real OS priority (nice value) management via setpriority(2).
#pragma once

#include "IScheduler.hpp"
#include "ILogger.hpp"

#include <vector>

class Scheduler : public IScheduler {
public:
    explicit Scheduler(ILogger& logger);

    bool setPriority(ProcessId pid, int niceValue) override;
    int getPriority(ProcessId pid) const override;
    std::vector<ProcessId> orderByPriority() const override;

private:
    ILogger& logger_;
    std::vector<ProcessId> registered_;
};
