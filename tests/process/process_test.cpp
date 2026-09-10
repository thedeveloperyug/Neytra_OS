// process_test.cpp - GTest suite for ProcessManager, Scheduler, IPC.
#include "IPC.hpp"
#include "ProcessManager.hpp"
#include "Scheduler.hpp"
#include "Logger.hpp"

#include <gtest/gtest.h>

#include <sys/wait.h>
#include <unistd.h>

#include <cstdio>

TEST(ProcessManagerTest, SpawnsAndReapsExitCodes) {
    // ProcessManager::wait() reaps synchronously -- nothing left to tear down.
    ProcessManager processes(Logger::instance());

    const ProcessId okPid = processes.spawn("/bin/true", {});
    int exitCode = -1;
    ASSERT_GE(okPid, 0);
    ASSERT_TRUE(processes.wait(okPid, exitCode));
    EXPECT_EQ(exitCode, 0);

    const ProcessId shPid = processes.spawn("/bin/sh", {"-c", "exit 5"});
    ASSERT_GE(shPid, 0);
    ASSERT_TRUE(processes.wait(shPid, exitCode));
    EXPECT_EQ(exitCode, 5);
}

class SchedulerTest : public ::testing::Test {
protected:
    void SetUp() override { originalPriority_ = Scheduler(Logger::instance()).getPriority(getpid()); }
    void TearDown() override { Scheduler(Logger::instance()).setPriority(getpid(), originalPriority_); }

    int originalPriority_ = 0;
};

TEST_F(SchedulerTest, SetsAndOrdersByPriority) {
    // Raising your own nice value never needs privilege.
    Scheduler scheduler(Logger::instance());
    const ProcessId self = getpid();

    ASSERT_TRUE(scheduler.setPriority(self, 5));
    EXPECT_EQ(scheduler.getPriority(self), 5);

    const auto ordered = scheduler.orderByPriority();
    ASSERT_FALSE(ordered.empty());
    EXPECT_EQ(ordered.front(), self);
}

class IPCTest : public ::testing::Test {
protected:
    void SetUp() override { unlink(fifoPath_.c_str()); }
    void TearDown() override {
        ipc_.closeChannel(fifoPath_);
        unlink(fifoPath_.c_str());
    }

    Logger& log_ = Logger::instance();
    IPC ipc_{log_};
    const std::string fifoPath_ = "/tmp/neytra_ipc_test_fifo";
};

TEST_F(IPCTest, SendReceiveRoundTripsOverFifo) {
    // fork() so send()'s blocking FIFO open has a reader on the other end.
    ASSERT_TRUE(ipc_.createChannel(fifoPath_));

    const pid_t child = fork();
    if (child == 0) {
        ipc_.send(fifoPath_, "hello-ipc");
        _exit(0);
    }

    std::string received;
    const bool ok = ipc_.receive(fifoPath_, received);
    int childStatus = 0;
    waitpid(child, &childStatus, 0);

    ASSERT_TRUE(ok);
    EXPECT_EQ(received, "hello-ipc");
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    Logger::instance().setMinLevel(LogLevel::Error);
    return RUN_ALL_TESTS();
}
