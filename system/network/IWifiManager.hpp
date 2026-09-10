// IWifiManager.hpp - wireless interface interface (depend on this, not WifiManager).
#pragma once

#include <string>
#include <vector>

struct WifiNetwork {
    std::string ssid;
    int signalDbm = 0;
};

class IWifiManager {
public:
    virtual ~IWifiManager() = default;

    // Interfaces with a /sys/class/net/<name>/wireless directory. Empty on
    // hardware with no wireless adapter (e.g. this project's QEMU x86_64 target).
    virtual std::vector<std::string> listWirelessInterfaces() const = 0;

    virtual std::vector<WifiNetwork> scan(const std::string& interfaceName) = 0;
};
