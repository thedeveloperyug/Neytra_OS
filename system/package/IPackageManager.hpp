// IPackageManager.hpp - package operations interface (depend on this, not PackageManager).
#pragma once

#include <string>

class IPackageManager {
public:
    virtual ~IPackageManager() = default;

    virtual bool install(const std::string& packageName) = 0;
    virtual bool remove(const std::string& packageName) = 0;
    virtual bool isInstalled(const std::string& packageName) const = 0;
};
