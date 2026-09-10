// network_test.cpp - GTest suite for SocketManager, NetworkManager, DHCPClient, WifiManager.
#include "DHCPClient.hpp"
#include "NetworkManager.hpp"
#include "SocketManager.hpp"
#include "WifiManager.hpp"
#include "Logger.hpp"

#include <gtest/gtest.h>

#include <cstdio>

class SocketManagerTest : public ::testing::Test {
protected:
    void TearDown() override {
        if (serverFd_ >= 0) sockets_.closeSocket(serverFd_);
        if (clientFd_ >= 0) sockets_.closeSocket(clientFd_);
    }

    Logger& log_ = Logger::instance();
    SocketManager sockets_{log_};
    int serverFd_ = -1;
    int clientFd_ = -1;
};

TEST_F(SocketManagerTest, UdpLoopbackRoundTrip) {
    serverFd_ = sockets_.createSocket(SocketType::UDP);
    clientFd_ = sockets_.createSocket(SocketType::UDP);
    ASSERT_GE(serverFd_, 0);
    ASSERT_GE(clientFd_, 0);
    ASSERT_TRUE(sockets_.bind(serverFd_, "127.0.0.1", 45654));
    ASSERT_TRUE(sockets_.connect(clientFd_, "127.0.0.1", 45654));
    ASSERT_EQ(sockets_.send(clientFd_, "ping"), 4);

    std::string received;
    ASSERT_EQ(sockets_.receive(serverFd_, received, 64), 4);
    EXPECT_EQ(received, "ping");
}

TEST(NetworkManagerTest, ListsLoopbackInterface) {
    Logger& log = Logger::instance();
    NetworkManager network(log);
    const auto interfaces = network.listInterfaces();

    bool foundLoopback = false;
    for (const auto& iface : interfaces) {
        if (iface.name == "lo") {
            foundLoopback = true;
        }
    }
    EXPECT_TRUE(foundLoopback);
}

// Deliberately targets a nonexistent interface name, never a real one -- this suite
// runs on real dev machines, and touching an actual interface's address/routes here
// could disrupt the host's own networking (e.g. an SSH session). Real success paths
// for setInterfaceAddress()/setDefaultGateway() are exercised by neytra-netup inside
// QEMU instead (see system/network/tools/neytra-netup.cpp, system/README.md).
TEST(NetworkManagerTest, ApplyLeaseFailsGracefullyOnUnknownInterface) {
    Logger& log = Logger::instance();
    NetworkManager network(log);
    EXPECT_FALSE(network.setInterfaceAddress("neytra-test-dummy0", "203.0.113.5", "255.255.255.0"));
    EXPECT_FALSE(network.setDefaultGateway("neytra-test-dummy0", "203.0.113.1"));
}

TEST(DHCPClientTest, FailsCleanlyWithNoServerReachable) {
    Logger& log = Logger::instance();
    DHCPClient dhcp(log);
    DhcpLease lease;
    EXPECT_FALSE(dhcp.acquire("lo", lease, 1));
}

TEST(WifiManagerTest, ScansWithoutCrashing) {
    // Host may or may not have wireless hardware; this must just not crash.
    Logger& log = Logger::instance();
    WifiManager wifi(log);
    const auto wirelessInterfaces = wifi.listWirelessInterfaces();
    for (const auto& iface : wirelessInterfaces) {
        const auto networks = wifi.scan(iface);
        (void)networks;
    }
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    Logger::instance().setMinLevel(LogLevel::Error);
    return RUN_ALL_TESTS();
}
