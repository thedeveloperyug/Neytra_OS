// ServiceManager.hpp - placeholder IServiceManager implementation.
#pragma once

#include "IServiceManager.hpp"

class ServiceManager : public IServiceManager {
public:
    bool startAll() override;
};

