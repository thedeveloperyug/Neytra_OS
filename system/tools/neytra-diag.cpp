// neytra-diag.cpp - interactive diagnostic CLI that exercises the real system/
// modules from inside a booted VM (not just via host-side ctest). Each
// subcommand drives the same classes tests/ use, but prints human-readable
// results instead of assertions. See system/README.md for the full guide.
//
// Usage: neytra-diag [security|network|wifi|package|drivers|init|all]
//        (no argument runs "all")
#include "Logger.hpp"

#include "UserManager.hpp"
#include "PermissionManager.hpp"
#include "Sandbox.hpp"

#include "NetworkManager.hpp"
#include "SocketManager.hpp"
#include "WifiManager.hpp"
#include "DHCPClient.hpp"

#include "Repository.hpp"

#include "GPIO.hpp"
#include "I2C.hpp"
#include "SPI.hpp"
#include "UART.hpp"

#include "MountManager.hpp"
#include "ProcessManager.hpp"
#include "ServiceManager.hpp"

#include <pty.h>
#include <sys/mount.h>
#include <unistd.h>

#include <algorithm>
#include <cstdio>
#include <fstream>
#include <iostream>

namespace {

void diagSecurity(ILogger& log) {
    std::cout << "== security ==\n";

    UserManager users(log);
    std::cout << "- root seeded: " << (users.findByUid(0).has_value() ? "yes" : "no") << "\n";
    users.addUser(UserRecord{"demo", 1000, 1000, "/home/demo", "/bin/sh"});
    std::cout << "- users after adding \"demo\": ";
    for (const auto& u : users.listUsers()) {
        std::cout << u.username << "(uid=" << u.uid << ") ";
    }
    std::cout << "\n";

    PermissionManager perms(log);
    const bool beforeGrant = perms.check(1000, "/etc/demo", Permission::Write);
    perms.grant(1000, "/etc/demo", Permission::Write);
    const bool afterGrant = perms.check(1000, "/etc/demo", Permission::Write);
    std::cout << "- uid 1000 write /etc/demo: before grant=" << beforeGrant
              << " after grant=" << afterGrant << "\n";

    Sandbox sandbox(log);
    SandboxLimits limits;
    limits.maxCpuSeconds = 2;
    const int exitCode = sandbox.run("/bin/echo", {"sandboxed-hello"}, 0, "", limits);
    std::cout << "- sandboxed `echo sandboxed-hello` exit code: " << exitCode << "\n";
}

void diagNetwork(ILogger& log) {
    std::cout << "== network ==\n";

    NetworkManager net(log);
    const auto interfaces = net.listInterfaces();
    bool hasEth0 = false;
    for (const auto& iface : interfaces) {
        std::cout << "- interface " << iface.name << " ip=" << iface.ipv4Address
                  << " up=" << iface.isUp << "\n";
        if (iface.name == "eth0") hasEth0 = true;
    }

    SocketManager sockets(log);
    const int serverFd = sockets.createSocket(SocketType::UDP);
    const int clientFd = sockets.createSocket(SocketType::UDP);
    bool loopbackOk = false;
    if (serverFd >= 0 && clientFd >= 0 && sockets.bind(serverFd, "127.0.0.1", 45700) &&
        sockets.connect(clientFd, "127.0.0.1", 45700) && sockets.send(clientFd, "ping") == 4) {
        std::string received;
        loopbackOk = sockets.receive(serverFd, received, 64) == 4 && received == "ping";
    }
    if (serverFd >= 0) sockets.closeSocket(serverFd);
    if (clientFd >= 0) sockets.closeSocket(clientFd);
    std::cout << "- UDP loopback self-test: " << (loopbackOk ? "ok" : "failed") << "\n";

    WifiManager wifi(log);
    const auto wireless = wifi.listWirelessInterfaces();
    std::cout << "- wireless interfaces: " << (wireless.empty() ? "none (expected, no wifi hardware)"
                                                                 : std::to_string(wireless.size()))
              << " (see `neytra-diag wifi` to scan)\n";

    const std::string dhcpTarget = hasEth0 ? "eth0" : "lo";
    if (hasEth0) net.setInterfaceUp("eth0", true);
    std::cout << "- attempting DHCP on " << dhcpTarget << " (up to 3s)...\n";
    DHCPClient dhcp(log);
    DhcpLease lease;
    if (dhcp.acquire(dhcpTarget, lease, 3)) {
        std::cout << "  leased " << lease.ipAddress << " from gateway " << lease.gateway << "\n";
    } else {
        std::cout << "  no lease (expected on \"lo\", or if QEMU networking is disabled)\n";
    }
}

void diagWifi(ILogger& log) {
    std::cout << "== wifi ==\n";

    WifiManager wifi(log);
    const auto interfaces = wifi.listWirelessInterfaces();
    if (interfaces.empty()) {
        std::cout << "- no wireless interfaces found (checked /sys/class/net/*/wireless)\n";
        std::cout << "- nothing to scan on this hardware\n";
        return;
    }

    for (const auto& iface : interfaces) {
        std::cout << "- scanning " << iface << "...\n";
        const auto networks = wifi.scan(iface);
        if (networks.empty()) {
            std::cout << "  no networks reported (scan() is a real-hardware-only stub -- see WifiManager.cpp)\n";
            continue;
        }
        for (const auto& found : networks) {
            std::cout << "  " << found.ssid << " (" << found.signalDbm << " dBm)\n";
        }
    }
}

void diagPackage(ILogger& log) {
    std::cout << "== package ==\n";

    const std::string indexPath = "/tmp/neytra_diag_pkg_index";
    {
        std::ofstream index(indexPath, std::ios::trunc);
        index << "demo|1.0|http://127.0.0.1:0/demo.tar\n";
    }
    Repository repo(log);
    repo.loadFromFile(indexPath);
    const auto entry = repo.find("demo");
    std::cout << "- Repository loaded \"demo\" entry: "
              << (entry.has_value() ? ("yes, version " + entry->version) : "no") << "\n";
    std::remove(indexPath.c_str());

    std::cout << "- Downloader/Installer need a real reachable package URL to demo further;\n"
                 "  see tests/package/package_test.cpp for the full pipeline against a fake server.\n";
}

void diagDrivers(ILogger& log) {
    std::cout << "== drivers ==\n";

    GPIO gpio(log);
    std::cout << "- GPIO exportPin(999999): "
              << (gpio.exportPin(999999) ? "ok" : "failed (expected -- no real GPIO hardware here)") << "\n";

    I2C i2c(log);
    std::cout << "- I2C open(/dev/i2c-0): "
              << (i2c.open("/dev/i2c-0", 0x50) ? "ok" : "failed (expected -- no real I2C hardware here)") << "\n";

    SPI spi(log);
    std::cout << "- SPI open(/dev/spidev0.0): "
              << (spi.open("/dev/spidev0.0", 500000) ? "ok" : "failed (expected -- no real SPI hardware here)") << "\n";

    int masterFd = -1;
    int slaveFd = -1;
    char slaveName[256];
    if (openpty(&masterFd, &slaveFd, slaveName, nullptr, nullptr) == 0) {
        close(slaveFd);
        UART uart(log);
        bool ok = uart.open(slaveName, 115200) && uart.write("hello-uart") >= 0;
        char buffer[32] = {0};
        const ssize_t bytesRead = ok ? ::read(masterFd, buffer, sizeof(buffer)) : -1;
        ok = ok && bytesRead > 0 && std::string(buffer, static_cast<size_t>(bytesRead)) == "hello-uart";
        std::cout << "- UART loopback over pty: " << (ok ? "ok" : "failed") << "\n";
        uart.close();
        close(masterFd);
    } else {
        std::cout << "- UART loopback over pty: skipped (openpty failed)\n";
    }
}

void diagInit(ILogger& log) {
    std::cout << "== init ==\n";

    char tmpDirTemplate[] = "/tmp/neytra_diag_mount_XXXXXX";
    const char* tmpDir = mkdtemp(tmpDirTemplate);
    if (tmpDir == nullptr) {
        std::cout << "- mkdtemp failed, skipping\n";
        return;
    }

    MountManager mounts(log, {MountSpec{"tmpfs", tmpDir, "tmpfs"}});
    const bool mountOk = mounts.mountAll();
    std::cout << "- mount tmpfs onto a temp dir: " << (mountOk ? "ok" : "failed") << "\n";

    const std::string configPath = "/tmp/neytra_diag_services.conf";
    {
        std::ofstream conf(configPath, std::ios::trunc);
        conf << "diag-demo|/bin/echo|diag-service-ran\n";
    }
    ProcessManager processes(log);
    ServiceManager services(processes, log, configPath);
    const bool servicesOk = services.startAll();
    std::cout << "- spawn demo service from config: " << (servicesOk ? "ok" : "failed") << "\n";
    std::remove(configPath.c_str());

    if (mountOk) umount(tmpDir);
    rmdir(tmpDir);
}

void printUsage(const char* argv0) {
    std::fprintf(stderr, "usage: %s [security|network|wifi|package|drivers|init|all]\n", argv0);
}

}  // namespace

int main(int argc, char** argv) {
    std::cout << std::unitbuf;  // keep interleaving with Logger's unbuffered stderr sane
    Logger& log = Logger::instance();
    const std::string which = argc > 1 ? argv[1] : "all";

    if (which == "security") { diagSecurity(log); return 0; }
    if (which == "network")  { diagNetwork(log);  return 0; }
    if (which == "wifi")     { diagWifi(log);     return 0; }
    if (which == "package")  { diagPackage(log);  return 0; }
    if (which == "drivers")  { diagDrivers(log);  return 0; }
    if (which == "init")     { diagInit(log);     return 0; }
    if (which == "all") {
        diagSecurity(log);
        diagNetwork(log);
        diagWifi(log);
        diagPackage(log);
        diagDrivers(log);
        diagInit(log);
        return 0;
    }

    printUsage(argv[0]);
    return 1;
}
