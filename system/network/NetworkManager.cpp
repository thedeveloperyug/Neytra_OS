// NetworkManager.cpp - real interface enumeration (getifaddrs) and up/down control (ioctl).
#include "NetworkManager.hpp"

#include <arpa/inet.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <net/route.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <map>

NetworkManager::NetworkManager(ILogger& logger) : logger_(logger) {}

std::vector<InterfaceInfo> NetworkManager::listInterfaces() const {
    std::map<std::string, InterfaceInfo> byName;

    ifaddrs* addrs = nullptr;
    if (getifaddrs(&addrs) != 0) {
        logger_.error("NetworkManager", std::string("getifaddrs() failed: ") + std::strerror(errno));
        return {};
    }

    for (ifaddrs* it = addrs; it != nullptr; it = it->ifa_next) {
        if (it->ifa_name == nullptr) {
            continue;
        }
        InterfaceInfo& info = byName[it->ifa_name];
        info.name = it->ifa_name;
        info.isUp = (it->ifa_flags & IFF_UP) != 0;

        if (it->ifa_addr != nullptr && it->ifa_addr->sa_family == AF_INET) {
            char buf[INET_ADDRSTRLEN] = {0};
            const auto* sin = reinterpret_cast<sockaddr_in*>(it->ifa_addr);
            if (inet_ntop(AF_INET, &sin->sin_addr, buf, sizeof(buf)) != nullptr) {
                info.ipv4Address = buf;
            }
        }
    }
    freeifaddrs(addrs);

    std::vector<InterfaceInfo> result;
    result.reserve(byName.size());
    for (auto& [name, info] : byName) {
        result.push_back(info);
    }
    return result;
}

bool NetworkManager::setInterfaceUp(const std::string& name, bool up) {
    const int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) {
        logger_.error("NetworkManager", std::string("socket() failed: ") + std::strerror(errno));
        return false;
    }

    ifreq request{};
    std::strncpy(request.ifr_name, name.c_str(), IFNAMSIZ - 1);

    if (ioctl(fd, SIOCGIFFLAGS, &request) != 0) {
        logger_.error("NetworkManager", std::string("SIOCGIFFLAGS failed: ") + std::strerror(errno));
        close(fd);
        return false;
    }

    if (up) {
        request.ifr_flags |= IFF_UP;
    } else {
        request.ifr_flags &= static_cast<short>(~IFF_UP);
    }

    const bool ok = ioctl(fd, SIOCSIFFLAGS, &request) == 0;
    if (!ok) {
        logger_.warn("NetworkManager", "SIOCSIFFLAGS(" + name + ") failed (needs CAP_NET_ADMIN): " +
                     std::strerror(errno));
    } else {
        logger_.info("NetworkManager", name + (up ? " brought up" : " brought down"));
    }
    close(fd);
    return ok;
}

bool NetworkManager::setInterfaceAddress(const std::string& name, const std::string& ipv4Address,
                                          const std::string& netmask) {
    const int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) {
        logger_.error("NetworkManager", std::string("socket() failed: ") + std::strerror(errno));
        return false;
    }

    ifreq request{};
    std::strncpy(request.ifr_name, name.c_str(), IFNAMSIZ - 1);
    auto* addr = reinterpret_cast<sockaddr_in*>(&request.ifr_addr);
    addr->sin_family = AF_INET;

    bool ok = inet_pton(AF_INET, ipv4Address.c_str(), &addr->sin_addr) == 1;
    if (!ok) {
        logger_.error("NetworkManager", "invalid IPv4 address: " + ipv4Address);
    } else if (ioctl(fd, SIOCSIFADDR, &request) != 0) {
        logger_.warn("NetworkManager", "SIOCSIFADDR(" + name + ") failed (needs CAP_NET_ADMIN): " +
                     std::strerror(errno));
        ok = false;
    }

    if (ok) {
        ok = inet_pton(AF_INET, netmask.c_str(), &addr->sin_addr) == 1;
        if (!ok) {
            logger_.error("NetworkManager", "invalid netmask: " + netmask);
        } else if (ioctl(fd, SIOCSIFNETMASK, &request) != 0) {
            logger_.warn("NetworkManager", "SIOCSIFNETMASK(" + name + ") failed: " + std::strerror(errno));
            ok = false;
        }
    }

    close(fd);
    if (ok) {
        logger_.info("NetworkManager", name + " address set to " + ipv4Address + "/" + netmask);
    }
    return ok;
}

bool NetworkManager::setDefaultGateway(const std::string& name, const std::string& gateway) {
    const int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) {
        logger_.error("NetworkManager", std::string("socket() failed: ") + std::strerror(errno));
        return false;
    }

    char ifNameBuf[IFNAMSIZ] = {0};
    std::strncpy(ifNameBuf, name.c_str(), IFNAMSIZ - 1);

    rtentry route{};
    reinterpret_cast<sockaddr_in&>(route.rt_dst).sin_family = AF_INET;         // 0.0.0.0/0
    reinterpret_cast<sockaddr_in&>(route.rt_genmask).sin_family = AF_INET;     // 0.0.0.0
    auto& gw = reinterpret_cast<sockaddr_in&>(route.rt_gateway);
    gw.sin_family = AF_INET;
    if (inet_pton(AF_INET, gateway.c_str(), &gw.sin_addr) != 1) {
        logger_.error("NetworkManager", "invalid gateway address: " + gateway);
        close(fd);
        return false;
    }
    route.rt_flags = RTF_UP | RTF_GATEWAY;
    route.rt_dev = ifNameBuf;

    ioctl(fd, SIOCDELRT, &route);  // best-effort: clear any existing default route first
    const bool ok = ioctl(fd, SIOCADDRT, &route) == 0;
    if (!ok) {
        logger_.warn("NetworkManager", "SIOCADDRT default via " + gateway + " (" + name +
                     ") failed (needs CAP_NET_ADMIN): " + std::strerror(errno));
    } else {
        logger_.info("NetworkManager", "default route via " + gateway + " on " + name);
    }
    close(fd);
    return ok;
}

