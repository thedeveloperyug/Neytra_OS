// INetworkManager.hpp - interface enumeration/control (depend on this, not NetworkManager).
#pragma once

#include <string>
#include <vector>

struct InterfaceInfo {
    std::string name;
    std::string ipv4Address;
    bool isUp = false;
};

class INetworkManager {
public:
    virtual ~INetworkManager() = default;

    virtual std::vector<InterfaceInfo> listInterfaces() const = 0;

    // Brings an interface up/down. Requires CAP_NET_ADMIN (root); returns false
    // (and logs why) otherwise.
    virtual bool setInterfaceUp(const std::string& name, bool up) = 0;

    // Assigns an IPv4 address + netmask to `name` -- e.g. to apply a DHCP lease
    // (see IDHCPClient) once one has been acquired. Requires CAP_NET_ADMIN.
    virtual bool setInterfaceAddress(const std::string& name, const std::string& ipv4Address,
                                      const std::string& netmask) = 0;

    // Replaces the default route (0.0.0.0/0) with one via `gateway` through `name`.
    // Requires CAP_NET_ADMIN.
    virtual bool setDefaultGateway(const std::string& name, const std::string& gateway) = 0;
};
