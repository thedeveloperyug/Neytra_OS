// IDHCPClient.hpp - DHCP lease acquisition interface (depend on this, not DHCPClient).
#pragma once

#include <string>

struct DhcpLease {
    std::string ipAddress;
    std::string subnetMask;
    std::string gateway;
    std::string dnsServer;
    unsigned int leaseSeconds = 0;
};

class IDHCPClient {
public:
    virtual ~IDHCPClient() = default;

    // Runs a real DISCOVER -> OFFER -> REQUEST -> ACK exchange on `interfaceName`,
    // waiting up to `timeoutSeconds` for each reply. Returns false on timeout/failure
    // (e.g. no DHCP server reachable, which is expected in a QEMU VM with no NIC).
    virtual bool acquire(const std::string& interfaceName, DhcpLease& outLease, int timeoutSeconds) = 0;
};
