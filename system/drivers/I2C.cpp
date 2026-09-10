// I2C.cpp - real Linux i2c-dev (/dev/i2c-N) access.
#include "I2C.hpp"

#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>

I2C::I2C(ILogger& logger) : logger_(logger) {}

I2C::~I2C() {
    close();
}

bool I2C::open(const std::string& busPath, int deviceAddress) {
    fd_ = ::open(busPath.c_str(), O_RDWR);
    if (fd_ < 0) {
        logger_.warn("I2C", "cannot open " + busPath + " (not on this hardware): " + std::strerror(errno));
        return false;
    }
    if (ioctl(fd_, I2C_SLAVE, deviceAddress) < 0) {
        logger_.error("I2C", std::string("I2C_SLAVE ioctl failed: ") + std::strerror(errno));
        ::close(fd_);
        fd_ = -1;
        return false;
    }
    return true;
}

bool I2C::writeBytes(const std::vector<uint8_t>& data) {
    if (fd_ < 0) {
        return false;
    }
    return ::write(fd_, data.data(), data.size()) == static_cast<ssize_t>(data.size());
}

std::vector<uint8_t> I2C::readBytes(size_t length) {
    std::vector<uint8_t> buffer(length);
    if (fd_ < 0) {
        return {};
    }
    const ssize_t bytesRead = ::read(fd_, buffer.data(), length);
    if (bytesRead < 0) {
        return {};
    }
    buffer.resize(static_cast<size_t>(bytesRead));
    return buffer;
}

void I2C::close() {
    if (fd_ >= 0) {
        ::close(fd_);
        fd_ = -1;
    }
}

