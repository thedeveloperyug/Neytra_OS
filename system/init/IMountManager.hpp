// IMountManager.hpp - mounts the pseudo-filesystems PID 1 needs (proc/sysfs/devtmpfs).
#pragma once

class IMountManager {
public:
    virtual ~IMountManager() = default;

    // Mounts proc, sysfs, and devtmpfs. Returns false if any mount fails.
    virtual bool mountAll() = 0;
};
