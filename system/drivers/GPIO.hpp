// GPIO.hpp - real Linux sysfs GPIO (/sys/class/gpio/...). Raspberry Pi target
// only; there are no GPIO pins on the x86_64/QEMU target this project boots
// today, so calls here fail gracefully (ENOENT) rather than crashing.
#pragma once

#include "IGPIO.hpp"
#include "ILogger.hpp"

class GPIO : public IGPIO {
public:
    explicit GPIO(ILogger& logger);

    bool exportPin(int pin) override;
    bool unexportPin(int pin) override;
    bool setDirection(int pin, GpioDirection direction) override;
    bool write(int pin, bool value) override;
    int read(int pin) override;

private:
    ILogger& logger_;
};
