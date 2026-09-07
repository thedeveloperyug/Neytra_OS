// main.cpp - composition root: constructs concrete implementations and wires them
// together via interfaces (see docs/architecture.md#design-principles). Not invoked
// by anything yet — rootfs/init (shell script) is still the real PID 1 today.
#include "InitManager.hpp"
#include "Logger.hpp"
#include "MountManager.hpp"
#include "ServiceManager.hpp"

int main_init() {
    MountManager mountManager;
    ServiceManager serviceManager;
    InitManager initManager(mountManager, serviceManager, Logger::instance());
    return initManager.run() ? 0 : 1;
}

