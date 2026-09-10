// init_test.cpp - GTest suite: verifies InitManager's constructor-injected wiring,
// with real (but sandboxed) mounting and service-spawning logic behind it.
#include "InitManager.hpp"
#include "Logger.hpp"
#include "MountManager.hpp"
#include "ProcessManager.hpp"
#include "ServiceManager.hpp"

#include <gtest/gtest.h>

#include <sys/mount.h>
#include <unistd.h>

#include <cstdio>
#include <cstdlib>
#include <fstream>

class InitManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Mount a tmpfs onto a fresh temp dir instead of touching the real /proc,/sys,/dev
        // -- safe regardless of whether this test happens to run as root or not.
        char tmpDirTemplate[] = "/tmp/neytra_mount_test_XXXXXX";
        const char* tmpDir = mkdtemp(tmpDirTemplate);
        ASSERT_NE(tmpDir, nullptr) << "mkdtemp failed";
        tmpDir_ = tmpDir;

        std::ofstream conf(configPath_, std::ios::trunc);
        conf << "test-service|/bin/true|\n";
    }

    void TearDown() override {
        std::remove(configPath_.c_str());
        umount(tmpDir_.c_str());  // best-effort; harmless if it was never mounted
        rmdir(tmpDir_.c_str());
    }

    std::string tmpDir_;
    const std::string configPath_ = "/tmp/neytra_services_test.conf";
};

TEST_F(InitManagerTest, RunsMountAndServiceWiring) {
    Logger& log = Logger::instance();
    MountManager mountManager(log, {MountSpec{"tmpfs", tmpDir_, "tmpfs"}});
    ProcessManager processes(log);
    ServiceManager serviceManager(processes, log, configPath_);
    InitManager initManager(mountManager, serviceManager, log);

    const bool ok = initManager.run();

    if (geteuid() != 0) {
        // Unprivileged: mount(2) is expected to fail with EPERM, so run() reporting
        // failure is the CORRECT result here -- the wiring already ran without crashing,
        // which is all this environment can prove.
        GTEST_SKIP() << "mount privilege unavailable (uid=" << geteuid()
                     << "), skipping strict result check";
    }

    EXPECT_TRUE(ok) << "expected run() to succeed when running as root";
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    Logger::instance().setMinLevel(LogLevel::Error);
    return RUN_ALL_TESTS();
}

