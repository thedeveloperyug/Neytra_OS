// IServiceManager.hpp - starts/stops/supervises services once InitManager hands off to it.
#pragma once

class IServiceManager {
public:
    virtual ~IServiceManager() = default;

    // Starts every configured service by spawning it through IProcessManager.
    // Returns false if any service failed to spawn (still attempts the rest).
    virtual bool startAll() = 0;
};
