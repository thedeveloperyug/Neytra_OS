// PackageManager.cpp - orchestrates Repository -> Downloader -> Installer.
#include "PackageManager.hpp"

PackageManager::PackageManager(IRepository& repository, IDownloader& downloader, IInstaller& installer,
                               ILogger& logger, std::string installRoot)
    : repository_(repository), downloader_(downloader), installer_(installer), logger_(logger),
      installRoot_(std::move(installRoot)) {}

bool PackageManager::install(const std::string& packageName) {
    const auto entry = repository_.find(packageName);
    if (!entry.has_value()) {
        logger_.error("PackageManager", "package not found in repository: " + packageName);
        return false;
    }

    const std::string archivePath = "/tmp/" + packageName + ".tar.download";
    if (!downloader_.fetch(entry->url, archivePath)) {
        logger_.error("PackageManager", "download failed for " + packageName);
        return false;
    }

    if (!installer_.install(archivePath, installRoot_)) {
        logger_.error("PackageManager", "install failed for " + packageName);
        return false;
    }

    installed_.insert(packageName);
    logger_.info("PackageManager", "installed " + packageName + " (" + entry->version + ")");
    return true;
}

bool PackageManager::remove(const std::string& packageName) {
    if (installed_.erase(packageName) == 0) {
        logger_.warn("PackageManager", "cannot remove, not installed: " + packageName);
        return false;
    }
    logger_.info("PackageManager", "removed " + packageName);
    return true;
}

bool PackageManager::isInstalled(const std::string& packageName) const {
    return installed_.count(packageName) != 0;
}

