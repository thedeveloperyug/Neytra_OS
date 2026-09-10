// IGPIO.hpp - GPIO pin interface (depend on this, not GPIO).
#pragma once

enum class GpioDirection { Input, Output };

class IGPIO {
public:
    virtual ~IGPIO() = default;

    virtual bool exportPin(int pin) = 0;
    virtual bool unexportPin(int pin) = 0;
    virtual bool setDirection(int pin, GpioDirection direction) = 0;
    virtual bool write(int pin, bool value) = 0;
    virtual int read(int pin) = 0;  // 0/1, or -1 on error
};
