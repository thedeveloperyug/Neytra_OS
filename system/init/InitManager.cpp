// InitManager.cpp - placeholder; not implemented yet (see design/init-workflow.svg).
#include "InitManager.hpp"

InitManager::InitManager(IMountManager& mountManager, IServiceManager& serviceManager, ILogger& logger)
    : mountManager_(mountManager), serviceManager_(serviceManager), logger_(logger) {}

bool InitManager::run() {
    logger_.info("InitManager", "run() called (stub — mounting/services not implemented yet)");
    return false;
}

