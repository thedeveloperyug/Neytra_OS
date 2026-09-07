// init_test.cpp - smoke test: verifies InitManager's constructor-injected wiring
// compiles and runs through the IMountManager/IServiceManager/ILogger interfaces.
#include "InitManager.hpp"
#include "Logger.hpp"
#include "MountManager.hpp"
#include "ServiceManager.hpp"

#include <cstdio>

int main() {
    MountManager mountManager;
    ServiceManager serviceManager;
    InitManager initManager(mountManager, serviceManager, Logger::instance());

    // Both sub-steps are still unimplemented, so run() is expected to report failure —
    // this test exists to prove the interface-based wiring compiles and executes,
    // not to prove real mounting/service-starting logic (there isn't any yet).
    if (initManager.run()) {
        std::fprintf(stderr, "expected run() to report failure (stub), but it succeeded\n");
        return 1;
    }

    std::printf("init_test: OK (interface-based wiring verified)\n");
    return 0;
}
