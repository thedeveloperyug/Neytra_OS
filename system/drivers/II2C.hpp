// II2C.hpp - I2C bus interface (depend on this, not I2C).
#pragma once

#include <cstdint>
#include <string>
#include <vector>

class II2C {
public:
    virtual ~II2C() = default;

    virtual bool open(const std::string& busPath, int deviceAddress) = 0;
    virtual bool writeBytes(const std::vector<uint8_t>& data) = 0;
    virtual std::vector<uint8_t> readBytes(size_t length) = 0;
    virtual void close() = 0;
};
