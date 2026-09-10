// IInstaller.hpp - archive extraction interface (depend on this, not Installer).
#pragma once

#include <string>

class IInstaller {
public:
    virtual ~IInstaller() = default;

    // Extracts a .tar/.tar.gz archive into destDir (created if missing).
    virtual bool install(const std::string& archivePath, const std::string& destDir) = 0;
};
