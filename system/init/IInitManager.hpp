// IInitManager.hpp - top-level PID 1 orchestrator, meant to eventually replace rootfs/init.
#pragma once

class IInitManager {
public:
    virtual ~IInitManager() = default;

    // Runs the full boot handoff: mount filesystems, then start services.
    virtual bool run() = 0;
};
