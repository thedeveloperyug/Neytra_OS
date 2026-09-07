// IServiceManager.hpp - starts/stops/supervises services once InitManager hands off to it.
#pragma once

class IServiceManager {
public:
    virtual ~IServiceManager() = default;

    // Starts every configured service (e.g. read from etc/init.d/*). Returns false on failure.
    virtual bool startAll() = 0;
};
