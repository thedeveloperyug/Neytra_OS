// drivers_test.cpp - GTest suite for GPIO, I2C, SPI, UART.
//
// GPIO/I2C/SPI need real Raspberry Pi hardware (sysfs GPIO, /dev/i2c-*,
// /dev/spidev*) that simply doesn't exist on this project's QEMU x86_64 target
// -- so these tests only assert they FAIL GRACEFULLY (no crash, sensible
// false/-1/empty return) rather than asserting success.
//
// UART is different: it's just termios over any character device, so its test
// exercises it for real using a pseudo-terminal (pty) pair.
#include "GPIO.hpp"
#include "I2C.hpp"
#include "SPI.hpp"
#include "UART.hpp"
#include "Logger.hpp"

#include <gtest/gtest.h>

#include <fcntl.h>
#include <pty.h>
#include <unistd.h>

#include <cstdio>

TEST(GPIOTest, FailsGracefullyWithoutRealHardware) {
    // Never obtains a real fd/pin on this host, so there's nothing to tear down.
    GPIO gpio(Logger::instance());
    EXPECT_FALSE(gpio.exportPin(999999));
}

TEST(I2CTest, FailsGracefullyWithoutRealHardware) {
    I2C i2c(Logger::instance());
    EXPECT_FALSE(i2c.open("/dev/i2c-0", 0x50));
}

TEST(SPITest, FailsGracefullyWithoutRealHardware) {
    SPI spi(Logger::instance());
    EXPECT_FALSE(spi.open("/dev/spidev0.0", 500000));
}

class UARTTest : public ::testing::Test {
protected:
    void TearDown() override {
        if (masterFd_ >= 0) close(masterFd_);
    }

    int masterFd_ = -1;
};

TEST_F(UARTTest, RoundTripsOverPty) {
    int slaveFd = -1;
    char slaveName[256];
    ASSERT_EQ(openpty(&masterFd_, &slaveFd, slaveName, nullptr, nullptr), 0);
    close(slaveFd);  // UART::open() will open the slave side itself by path

    UART uart(Logger::instance());
    ASSERT_TRUE(uart.open(slaveName, 115200));
    ASSERT_GE(uart.write("hello-uart"), 0);

    char buffer[32] = {0};
    const ssize_t bytesFromMaster = ::read(masterFd_, buffer, sizeof(buffer));
    uart.close();

    ASSERT_GT(bytesFromMaster, 0);
    EXPECT_EQ(std::string(buffer, static_cast<size_t>(bytesFromMaster)), "hello-uart");
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    Logger::instance().setMinLevel(LogLevel::Error);
    return RUN_ALL_TESTS();
}
