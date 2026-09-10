// UART.hpp - real termios-configured serial I/O. Works against any character
// device (a real /dev/ttyS* on hardware, or a pty for tests -- see
// tests/drivers/drivers_test.cpp), unlike GPIO/I2C/SPI this one is fully
// testable without real target hardware.
#pragma once

#include "IUART.hpp"
#include "ILogger.hpp"

class UART : public IUART {
public:
    explicit UART(ILogger& logger);
    ~UART() override;

    bool open(const std::string& devicePath, int baudRate) override;
    long write(const std::string& data) override;
    std::string read(size_t maxLen) override;
    void close() override;

private:
    ILogger& logger_;
    int fd_ = -1;
};
