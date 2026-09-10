// PackageManager.hpp - orchestrates Repository -> Downloader -> Installer, and
// tracks installed packages. All three collaborators are constructor-injected
// (see docs/architecture.md#design-principles).
#pragma once

#include "IPackageManager.hpp"
#include "IRepository.hpp"
#include "IDownloader.hpp"
#include "IInstaller.hpp"
#include "ILogger.hpp"

#include <set>

class PackageManager : public IPackageManager {
public:
    PackageManager(IRepository& repository, IDownloader& downloader, IInstaller& installer,
                   ILogger& logger, std::string installRoot);

    bool install(const std::string& packageName) override;
    bool remove(const std::string& packageName) override;
    bool isInstalled(const std::string& packageName) const override;

private:
    IRepository& repository_;
    IDownloader& downloader_;
    IInstaller& installer_;
    ILogger& logger_;
    std::string installRoot_;
    std::set<std::string> installed_;
};
