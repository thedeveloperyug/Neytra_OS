// WifiManager.hpp - real wireless interface detection via sysfs; best-effort scan
// via the legacy Wireless Extensions ioctl (SIOCSIWSCAN/SIOCGIWSCAN).
#pragma once

#include "IWifiManager.hpp"
#include "ILogger.hpp"

class WifiManager : public IWifiManager {
public:
    explicit WifiManager(ILogger& logger);

    std::vector<std::string> listWirelessInterfaces() const override;
    std::vector<WifiNetwork> scan(const std::string& interfaceName) override;

private:
    ILogger& logger_;
};
