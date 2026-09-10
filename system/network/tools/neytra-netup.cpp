// neytra-netup.cpp - brings up loopback plus any other real interface and
// attempts DHCP on each, applying whatever lease it gets (address + default
// route) instead of just discovering one. Meant to run in the background from
// rootfs/init at boot -- see system/README.md for the full explanation and a
// real, verified QEMU transcript.
#include "DHCPClient.hpp"
#include "Logger.hpp"
#include "NetworkManager.hpp"

#include <chrono>
#include <fstream>
#include <thread>

namespace {

// Bringing an interface administratively up (IFF_UP) doesn't mean the physical
// link has finished negotiating yet -- sending DHCP DISCOVER too early gets it
// silently dropped. Poll /sys/class/net/<name>/carrier instead of guessing a
// fixed delay; give up after maxWaitSeconds (e.g. no cable plugged in).
bool waitForCarrier(const std::string& ifaceName, int maxWaitSeconds) {
    const std::string carrierPath = "/sys/class/net/" + ifaceName + "/carrier";
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(maxWaitSeconds);
    while (std::chrono::steady_clock::now() < deadline) {
        std::ifstream carrier(carrierPath);
        std::string value;
        if (carrier && (carrier >> value) && value == "1") {
            return true;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
    return false;
}

}  // namespace

int main() {
    ILogger& log = Logger::instance();
    NetworkManager net(log);
    DHCPClient dhcp(log);

    net.setInterfaceUp("lo", true);

    bool gatewaySet = false;
    for (const auto& iface : net.listInterfaces()) {
        if (iface.name == "lo") {
            continue;
        }
        if (!net.setInterfaceUp(iface.name, true)) {
            continue;
        }
        if (!waitForCarrier(iface.name, 5)) {
            log.warn("netup", iface.name + " has no carrier (no cable/link?), trying DHCP anyway");
        }

        DhcpLease lease;
        if (!dhcp.acquire(iface.name, lease, 8)) {
            log.warn("netup", "no DHCP lease on " + iface.name);
            continue;
        }
        if (!net.setInterfaceAddress(iface.name, lease.ipAddress, lease.subnetMask)) {
            log.warn("netup", "got a lease but failed to assign " + lease.ipAddress + " to " + iface.name);
            continue;
        }
        log.info("netup", iface.name + " configured: " + lease.ipAddress + "/" + lease.subnetMask);

        if (!gatewaySet && !lease.gateway.empty()) {
            gatewaySet = net.setDefaultGateway(iface.name, lease.gateway);
        }
    }

    return 0;
}
