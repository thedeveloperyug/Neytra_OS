// IUART.hpp - serial port interface (depend on this, not UART).
#pragma once

#include <string>

class IUART {
public:
    virtual ~IUART() = default;

    virtual bool open(const std::string& devicePath, int baudRate) = 0;
    virtual long write(const std::string& data) = 0;
    virtual std::string read(size_t maxLen) = 0;
    virtual void close() = 0;
};
