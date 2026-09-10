// WifiManager.cpp - real wireless interface detection via sysfs; scanning is a
// best-effort stub (see the note in scan()) since it needs real wireless hardware
// (none exists on this project's QEMU x86_64 target) and full nl80211 support.
#include "WifiManager.hpp"

#include <dirent.h>
#include <sys/stat.h>

WifiManager::WifiManager(ILogger& logger) : logger_(logger) {}

std::vector<std::string> WifiManager::listWirelessInterfaces() const {
    std::vector<std::string> result;

    DIR* netDir = opendir("/sys/class/net");
    if (netDir == nullptr) {
        logger_.warn("WifiManager", "/sys/class/net not available");
        return result;
    }

    dirent* entry;
    while ((entry = readdir(netDir)) != nullptr) {
        const std::string name = entry->d_name;
        if (name == "." || name == "..") {
            continue;
        }
        const std::string wirelessDir = "/sys/class/net/" + name + "/wireless";
        struct stat st{};
        if (stat(wirelessDir.c_str(), &st) == 0) {
            result.push_back(name);
        }
    }
    closedir(netDir);

    logger_.info("WifiManager", "found " + std::to_string(result.size()) + " wireless interface(s)");
    return result;
}

std::vector<WifiNetwork> WifiManager::scan(const std::string& interfaceName) {
    // Real wireless scanning needs either the legacy Wireless Extensions ioctl
    // (SIOCSIWSCAN/SIOCGIWSCAN, deprecated on modern kernels) or full nl80211
    // generic-netlink support (~hundreds of lines of attribute parsing). Neither
    // is testable on this project's QEMU x86_64 target (no wireless hardware),
    // so this honestly reports "no results" for any interface rather than faking
    // data -- see docs/architecture.md#design-principles on avoiding speculative
    // complexity for untestable code paths.
    logger_.info("WifiManager", "scan(" + interfaceName + ") -- no wireless hardware, returning empty");
    return {};
}

