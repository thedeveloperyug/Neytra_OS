// MountManager.cpp - real mount(2)-based implementation.
#include "MountManager.hpp"

#include <sys/mount.h>

#include <cerrno>
#include <cstring>

MountManager::MountManager(ILogger& logger)
    : MountManager(logger, std::vector<MountSpec>{
          {"proc", "/proc", "proc"},
          {"sysfs", "/sys", "sysfs"},
          {"devtmpfs", "/dev", "devtmpfs"},
      }) {}

MountManager::MountManager(ILogger& logger, std::vector<MountSpec> mounts)
    : logger_(logger), mounts_(std::move(mounts)) {}

bool MountManager::mountAll() {
    bool allOk = true;
    for (const auto& spec : mounts_) {
        if (::mount(spec.source.c_str(), spec.target.c_str(), spec.filesystemType.c_str(), 0, nullptr) != 0) {
            logger_.warn("MountManager", "mount " + spec.target + " (" + spec.filesystemType +
                         ") failed: " + std::strerror(errno));
            allOk = false;
            continue;
        }
        logger_.info("MountManager", "mounted " + spec.target + " (" + spec.filesystemType + ")");
    }
    return allOk;
}


