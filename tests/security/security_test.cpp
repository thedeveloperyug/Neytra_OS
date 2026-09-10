// security_test.cpp - GTest suite for UserManager, PermissionManager, Sandbox.
#include "PermissionManager.hpp"
#include "Sandbox.hpp"
#include "UserManager.hpp"
#include "Logger.hpp"

#include <gtest/gtest.h>

#include <cstdio>
#include <cstdlib>

class UserManagerTest : public ::testing::Test {
protected:
    void TearDown() override { std::remove(passwdPath_.c_str()); }

    Logger& log_ = Logger::instance();
    UserManager users_{log_};
    const std::string passwdPath_ = "/tmp/neytra_passwd_test";
};

TEST_F(UserManagerTest, SeedsRootAndRoundTripsThroughFile) {
    ASSERT_TRUE(users_.findByUid(0).has_value()) << "expected root (uid 0) to be seeded";
    EXPECT_EQ(users_.findByUid(0)->username, "root");

    ASSERT_TRUE(users_.addUser(UserRecord{"neytra", 1000, 1000, "/home/neytra", "/bin/sh"}));
    EXPECT_TRUE(users_.findByName("neytra").has_value());
    EXPECT_EQ(users_.listUsers().size(), 2u);

    ASSERT_TRUE(users_.saveToFile(passwdPath_));

    UserManager reloaded(log_);
    ASSERT_TRUE(reloaded.loadFromFile(passwdPath_));
    EXPECT_TRUE(reloaded.findByName("neytra").has_value());
}

TEST(PermissionManagerTest, GrantCheckRevokeAndRootBypass) {
    // Pure in-memory rule table -- nothing to tear down.
    PermissionManager perms(Logger::instance());

    EXPECT_FALSE(perms.check(1000, "/etc/config", Permission::Write))
        << "expected unprivileged uid to be denied before grant";
    perms.grant(1000, "/etc/config", Permission::Write);
    EXPECT_TRUE(perms.check(1000, "/etc/config", Permission::Write))
        << "expected write to be allowed after grant";
    perms.revoke(1000, "/etc/config", Permission::Write);
    EXPECT_FALSE(perms.check(1000, "/etc/config", Permission::Write))
        << "expected write to be denied after revoke";
    EXPECT_TRUE(perms.check(0, "/etc/config", Permission::Write)) << "expected root to always be allowed";
}

TEST(SandboxTest, RunsProcessWithRlimitsOnly) {
    // No chroot/uid drop here -- those need root; rlimits only. Sandbox::run() is
    // synchronous (forks, waits, reaps internally), so there's no leftover process.
    Sandbox sandbox(Logger::instance());
    SandboxLimits limits;
    limits.maxCpuSeconds = 5;
    const int exitCode = sandbox.run("/bin/true", {}, 0, "", limits);
    EXPECT_EQ(exitCode, 0);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    Logger::instance().setMinLevel(LogLevel::Error);
    return RUN_ALL_TESTS();
}
