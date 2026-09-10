// IMountManager.hpp - mounts the pseudo-filesystems PID 1 needs (proc/sysfs/devtmpfs).
#pragma once

#include <string>

struct MountSpec {
    std::string source;
    std::string target;
    std::string filesystemType;
};

class IMountManager {
public:
    virtual ~IMountManager() = default;

    // Mounts every configured entry in order. Returns false if any mount fails
    // (logging which one via the injected ILogger); still attempts the rest.
    virtual bool mountAll() = 0;
};
