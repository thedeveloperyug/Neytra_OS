// DHCPClient.hpp - real DHCP (RFC 2131) DISCOVER/OFFER/REQUEST/ACK client over raw UDP.
#pragma once

#include "IDHCPClient.hpp"
#include "ILogger.hpp"

#include <cstdint>
#include <vector>

class DHCPClient : public IDHCPClient {
public:
    explicit DHCPClient(ILogger& logger);

    bool acquire(const std::string& interfaceName, DhcpLease& outLease, int timeoutSeconds) override;

private:
    ILogger& logger_;

    bool getInterfaceMac(const std::string& interfaceName, uint8_t mac[6]) const;
    std::vector<uint8_t> buildPacket(uint8_t messageType, uint32_t transactionId,
                                      const uint8_t mac[6], uint32_t requestedIp,
                                      uint32_t serverId) const;
    bool parseReply(const std::vector<uint8_t>& packet, uint32_t transactionId,
                     uint8_t& outMessageType, uint32_t& outYourIp, uint32_t& outServerId,
                     DhcpLease& outLease) const;
};
