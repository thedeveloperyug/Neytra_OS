// InitManager.hpp - placeholder IInitManager implementation. Dependencies are
// constructor-injected (not constructed internally), so tests/other compositions
// can supply fakes for IMountManager/IServiceManager/ILogger instead of the real ones.
#pragma once

#include "IInitManager.hpp"
#include "ILogger.hpp"
#include "IMountManager.hpp"
#include "IServiceManager.hpp"

class InitManager : public IInitManager {
public:
    InitManager(IMountManager& mountManager, IServiceManager& serviceManager, ILogger& logger);

    bool run() override;

private:
    IMountManager& mountManager_;
    IServiceManager& serviceManager_;
    ILogger& logger_;
};

