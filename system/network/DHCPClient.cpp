// DHCPClient.cpp - real DHCP (RFC 2131) DISCOVER/OFFER/REQUEST/ACK client over raw UDP.
#include "DHCPClient.hpp"

#include <arpa/inet.h>
#include <net/if.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <ctime>

namespace {
constexpr uint8_t kDhcpDiscover = 1;
constexpr uint8_t kDhcpOffer = 2;
constexpr uint8_t kDhcpRequest = 3;
constexpr uint8_t kDhcpAck = 5;
constexpr uint8_t kMagicCookie[4] = {99, 130, 83, 99};
constexpr int kClientPort = 68;
constexpr int kServerPort = 67;

#pragma pack(push, 1)
struct DhcpHeader {
    uint8_t op;
    uint8_t htype;
    uint8_t hlen;
    uint8_t hops;
    uint32_t xid;
    uint16_t secs;
    uint16_t flags;
    uint32_t ciaddr;
    uint32_t yiaddr;
    uint32_t siaddr;
    uint32_t giaddr;
    uint8_t chaddr[16];
    uint8_t sname[64];
    uint8_t file[128];
    uint8_t magicCookie[4];
};
#pragma pack(pop)
}  // namespace

DHCPClient::DHCPClient(ILogger& logger) : logger_(logger) {}

bool DHCPClient::getInterfaceMac(const std::string& interfaceName, uint8_t mac[6]) const {
    const int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) {
        return false;
    }
    ifreq request{};
    std::strncpy(request.ifr_name, interfaceName.c_str(), IFNAMSIZ - 1);
    const bool ok = ioctl(fd, SIOCGIFHWADDR, &request) == 0;
    if (ok) {
        std::memcpy(mac, request.ifr_hwaddr.sa_data, 6);
    }
    close(fd);
    return ok;
}

std::vector<uint8_t> DHCPClient::buildPacket(uint8_t messageType, uint32_t transactionId,
                                              const uint8_t mac[6], uint32_t requestedIp,
                                              uint32_t serverId) const {
    DhcpHeader header{};
    header.op = 1;  // BOOTREQUEST
    header.htype = 1;  // Ethernet
    header.hlen = 6;
    header.xid = htonl(transactionId);
    header.flags = htons(0x8000);  // ask for a broadcast reply (we have no IP yet)
    std::memcpy(header.chaddr, mac, 6);
    std::memcpy(header.magicCookie, kMagicCookie, 4);

    std::vector<uint8_t> packet(reinterpret_cast<uint8_t*>(&header),
                                 reinterpret_cast<uint8_t*>(&header) + sizeof(header));

    packet.push_back(53);
    packet.push_back(1);
    packet.push_back(messageType);

    if (messageType == kDhcpRequest) {
        packet.push_back(50);
        packet.push_back(4);
        const uint32_t netRequestedIp = requestedIp;  // already in network order
        const auto* bytes = reinterpret_cast<const uint8_t*>(&netRequestedIp);
        packet.insert(packet.end(), bytes, bytes + 4);

        packet.push_back(54);
        packet.push_back(4);
        const auto* serverBytes = reinterpret_cast<const uint8_t*>(&serverId);
        packet.insert(packet.end(), serverBytes, serverBytes + 4);
    }

    packet.push_back(55);  // parameter request list
    packet.push_back(3);
    packet.push_back(1);   // subnet mask
    packet.push_back(3);   // router
    packet.push_back(6);   // domain name server

    packet.push_back(255);  // end
    return packet;
}

bool DHCPClient::parseReply(const std::vector<uint8_t>& packet, uint32_t transactionId,
                             uint8_t& outMessageType, uint32_t& outYourIp, uint32_t& outServerId,
                             DhcpLease& outLease) const {
    if (packet.size() < sizeof(DhcpHeader)) {
        return false;
    }
    DhcpHeader header{};
    std::memcpy(&header, packet.data(), sizeof(header));
    if (ntohl(header.xid) != transactionId) {
        return false;  // reply to someone else's transaction
    }
    if (std::memcmp(header.magicCookie, kMagicCookie, 4) != 0) {
        return false;
    }

    outYourIp = header.yiaddr;
    outMessageType = 0;
    outServerId = 0;

    size_t pos = sizeof(DhcpHeader);
    while (pos < packet.size()) {
        const uint8_t option = packet[pos++];
        if (option == 255) {
            break;
        }
        if (option == 0) {
            continue;  // padding
        }
        if (pos >= packet.size()) {
            break;
        }
        const uint8_t len = packet[pos++];
        if (pos + len > packet.size()) {
            break;
        }

        if (option == 53 && len >= 1) {
            outMessageType = packet[pos];
        } else if (option == 54 && len == 4) {
            std::memcpy(&outServerId, &packet[pos], 4);
        } else if (option == 1 && len == 4) {
            char buf[INET_ADDRSTRLEN] = {0};
            in_addr addr{};
            std::memcpy(&addr, &packet[pos], 4);
            inet_ntop(AF_INET, &addr, buf, sizeof(buf));
            outLease.subnetMask = buf;
        } else if (option == 3 && len >= 4) {
            char buf[INET_ADDRSTRLEN] = {0};
            in_addr addr{};
            std::memcpy(&addr, &packet[pos], 4);
            inet_ntop(AF_INET, &addr, buf, sizeof(buf));
            outLease.gateway = buf;
        } else if (option == 6 && len >= 4) {
            char buf[INET_ADDRSTRLEN] = {0};
            in_addr addr{};
            std::memcpy(&addr, &packet[pos], 4);
            inet_ntop(AF_INET, &addr, buf, sizeof(buf));
            outLease.dnsServer = buf;
        } else if (option == 51 && len == 4) {
            uint32_t leaseSeconds = 0;
            std::memcpy(&leaseSeconds, &packet[pos], 4);
            outLease.leaseSeconds = ntohl(leaseSeconds);
        }
        pos += len;
    }

    return true;
}

bool DHCPClient::acquire(const std::string& interfaceName, DhcpLease& outLease, int timeoutSeconds) {
    uint8_t mac[6] = {0};
    if (!getInterfaceMac(interfaceName, mac)) {
        logger_.error("DHCPClient", "cannot read MAC address for " + interfaceName);
        return false;
    }

    const int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) {
        logger_.error("DHCPClient", std::string("socket() failed: ") + std::strerror(errno));
        return false;
    }

    const int allowBroadcast = 1;
    setsockopt(fd, SOL_SOCKET, SO_BROADCAST, &allowBroadcast, sizeof(allowBroadcast));
#ifdef SO_BINDTODEVICE
    setsockopt(fd, SOL_SOCKET, SO_BINDTODEVICE, interfaceName.c_str(),
               static_cast<socklen_t>(interfaceName.size()));
#endif

    timeval timeout{};
    timeout.tv_sec = timeoutSeconds;
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

    sockaddr_in local{};
    local.sin_family = AF_INET;
    local.sin_port = htons(kClientPort);
    local.sin_addr.s_addr = INADDR_ANY;
    if (bind(fd, reinterpret_cast<sockaddr*>(&local), sizeof(local)) != 0) {
        logger_.warn("DHCPClient", std::string("bind(:68) failed (needs root): ") + std::strerror(errno));
        close(fd);
        return false;
    }

    sockaddr_in broadcast{};
    broadcast.sin_family = AF_INET;
    broadcast.sin_port = htons(kServerPort);
    broadcast.sin_addr.s_addr = INADDR_BROADCAST;

    const uint32_t xid = static_cast<uint32_t>(std::time(nullptr));

    // --- DISCOVER ---
    const std::vector<uint8_t> discover = buildPacket(kDhcpDiscover, xid, mac, 0, 0);
    if (sendto(fd, discover.data(), discover.size(), 0,
               reinterpret_cast<sockaddr*>(&broadcast), sizeof(broadcast)) < 0) {
        logger_.warn("DHCPClient", std::string("sendto(DISCOVER) failed: ") + std::strerror(errno));
        close(fd);
        return false;
    }
    logger_.info("DHCPClient", "DISCOVER sent on " + interfaceName);

    // --- OFFER ---
    std::vector<uint8_t> buffer(2048);
    const ssize_t offerLen = recv(fd, buffer.data(), buffer.size(), 0);
    if (offerLen <= 0) {
        logger_.warn("DHCPClient", "no DHCPOFFER received within " + std::to_string(timeoutSeconds) + "s");
        close(fd);
        return false;
    }
    buffer.resize(static_cast<size_t>(offerLen));

    uint8_t messageType = 0;
    uint32_t offeredIp = 0, serverId = 0;
    DhcpLease lease;
    if (!parseReply(buffer, xid, messageType, offeredIp, serverId, lease) || messageType != kDhcpOffer) {
        logger_.warn("DHCPClient", "unexpected reply while waiting for DHCPOFFER");
        close(fd);
        return false;
    }
    logger_.info("DHCPClient", "OFFER received");

    // --- REQUEST ---
    const std::vector<uint8_t> request = buildPacket(kDhcpRequest, xid, mac, offeredIp, serverId);
    sendto(fd, request.data(), request.size(), 0, reinterpret_cast<sockaddr*>(&broadcast), sizeof(broadcast));

    // --- ACK ---
    const ssize_t ackLen = recv(fd, buffer.data(), buffer.size(), 0);
    close(fd);
    if (ackLen <= 0) {
        logger_.warn("DHCPClient", "no DHCPACK received");
        return false;
    }
    buffer.resize(static_cast<size_t>(ackLen));

    uint32_t ackedIp = 0;
    if (!parseReply(buffer, xid, messageType, ackedIp, serverId, lease) || messageType != kDhcpAck) {
        logger_.warn("DHCPClient", "unexpected reply while waiting for DHCPACK");
        return false;
    }

    in_addr ipAddr{};
    ipAddr.s_addr = ackedIp;
    char ipBuf[INET_ADDRSTRLEN] = {0};
    inet_ntop(AF_INET, &ipAddr, ipBuf, sizeof(ipBuf));
    lease.ipAddress = ipBuf;

    outLease = lease;
    logger_.info("DHCPClient", "ACK received, leased " + lease.ipAddress);
    return true;
}

