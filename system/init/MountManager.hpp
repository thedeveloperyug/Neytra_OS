// MountManager.hpp - real mount(2)-based implementation, with a configurable
// mount list so tests don't have to touch the real /proc, /sys, /dev.
#pragma once

#include "IMountManager.hpp"
#include "ILogger.hpp"

#include <vector>

class MountManager : public IMountManager {
public:
    // Defaults to the standard proc/sysfs/devtmpfs set rootfs/init also mounts.
    explicit MountManager(ILogger& logger);
    MountManager(ILogger& logger, std::vector<MountSpec> mounts);

    bool mountAll() override;

private:
    ILogger& logger_;
    std::vector<MountSpec> mounts_;
};

