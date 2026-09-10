// InitManager.cpp - orchestrates the boot handoff: mount, then start services.
#include "InitManager.hpp"

InitManager::InitManager(IMountManager& mountManager, IServiceManager& serviceManager, ILogger& logger)
    : mountManager_(mountManager), serviceManager_(serviceManager), logger_(logger) {}

bool InitManager::run() {
    logger_.info("InitManager", "run() starting: mount filesystems, then start services");

    const bool mountsOk = mountManager_.mountAll();
    if (!mountsOk) {
        logger_.warn("InitManager", "one or more mounts failed; continuing anyway");
    }

    const bool servicesOk = serviceManager_.startAll();
    if (!servicesOk) {
        logger_.warn("InitManager", "one or more services failed to start");
    }

    const bool allOk = mountsOk && servicesOk;
    logger_.info("InitManager", allOk ? "run() completed successfully" : "run() completed with failures");
    return allOk;
}


