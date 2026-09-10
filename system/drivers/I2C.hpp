// I2C.hpp - real Linux i2c-dev (/dev/i2c-N) access. Raspberry Pi target only.
#pragma once

#include "II2C.hpp"
#include "ILogger.hpp"

class I2C : public II2C {
public:
    explicit I2C(ILogger& logger);
    ~I2C() override;

    bool open(const std::string& busPath, int deviceAddress) override;
    bool writeBytes(const std::vector<uint8_t>& data) override;
    std::vector<uint8_t> readBytes(size_t length) override;
    void close() override;

private:
    ILogger& logger_;
    int fd_ = -1;
};
