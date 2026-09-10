// GPIO.cpp - real Linux sysfs GPIO (/sys/class/gpio/...).
#include "GPIO.hpp"

#include <fstream>

GPIO::GPIO(ILogger& logger) : logger_(logger) {}

bool GPIO::exportPin(int pin) {
    std::ofstream out("/sys/class/gpio/export");
    if (!out.is_open()) {
        logger_.warn("GPIO", "sysfs gpio export unavailable (not on this hardware)");
        return false;
    }
    out << pin;
    return !out.fail();
}

bool GPIO::unexportPin(int pin) {
    std::ofstream out("/sys/class/gpio/unexport");
    if (!out.is_open()) {
        return false;
    }
    out << pin;
    return !out.fail();
}

bool GPIO::setDirection(int pin, GpioDirection direction) {
    std::ofstream out("/sys/class/gpio/gpio" + std::to_string(pin) + "/direction");
    if (!out.is_open()) {
        logger_.warn("GPIO", "pin " + std::to_string(pin) + " not exported or unavailable");
        return false;
    }
    out << (direction == GpioDirection::Output ? "out" : "in");
    return !out.fail();
}

bool GPIO::write(int pin, bool value) {
    std::ofstream out("/sys/class/gpio/gpio" + std::to_string(pin) + "/value");
    if (!out.is_open()) {
        return false;
    }
    out << (value ? "1" : "0");
    return !out.fail();
}

int GPIO::read(int pin) {
    std::ifstream in("/sys/class/gpio/gpio" + std::to_string(pin) + "/value");
    if (!in.is_open()) {
        return -1;
    }
    int value = -1;
    in >> value;
    return value;
}

