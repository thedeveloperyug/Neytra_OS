// ISPI.hpp - SPI bus interface (depend on this, not SPI).
#pragma once

#include <cstdint>
#include <string>
#include <vector>

class ISPI {
public:
    virtual ~ISPI() = default;

    virtual bool open(const std::string& devicePath, uint32_t speedHz) = 0;
    // Full-duplex transfer: sends `tx`, returns the simultaneously-received bytes
    // (same length as `tx`); empty on failure.
    virtual std::vector<uint8_t> transfer(const std::vector<uint8_t>& tx) = 0;
    virtual void close() = 0;
};
