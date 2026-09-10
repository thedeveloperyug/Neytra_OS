// NetworkManager.hpp - real interface enumeration (getifaddrs) and up/down control (ioctl).
#pragma once

#include "INetworkManager.hpp"
#include "ILogger.hpp"

class NetworkManager : public INetworkManager {
public:
    explicit NetworkManager(ILogger& logger);

    std::vector<InterfaceInfo> listInterfaces() const override;
    bool setInterfaceUp(const std::string& name, bool up) override;
    bool setInterfaceAddress(const std::string& name, const std::string& ipv4Address,
                              const std::string& netmask) override;
    bool setDefaultGateway(const std::string& name, const std::string& gateway) override;

private:
    ILogger& logger_;
};
