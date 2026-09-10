// SPI.hpp - real Linux spidev (/dev/spidevB.C) access. Raspberry Pi target only.
#pragma once

#include "ISPI.hpp"
#include "ILogger.hpp"

class SPI : public ISPI {
public:
    explicit SPI(ILogger& logger);
    ~SPI() override;

    bool open(const std::string& devicePath, uint32_t speedHz) override;
    std::vector<uint8_t> transfer(const std::vector<uint8_t>& tx) override;
    void close() override;

private:
    ILogger& logger_;
    int fd_ = -1;
    uint32_t speedHz_ = 500000;
};
