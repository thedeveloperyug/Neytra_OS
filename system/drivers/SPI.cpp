// SPI.cpp - real Linux spidev (/dev/spidevB.C) access.
#include "SPI.hpp"

#include <fcntl.h>
#include <linux/spi/spidev.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>

SPI::SPI(ILogger& logger) : logger_(logger) {}

SPI::~SPI() {
    close();
}

bool SPI::open(const std::string& devicePath, uint32_t speedHz) {
    fd_ = ::open(devicePath.c_str(), O_RDWR);
    if (fd_ < 0) {
        logger_.warn("SPI", "cannot open " + devicePath + " (not on this hardware): " + std::strerror(errno));
        return false;
    }
    speedHz_ = speedHz;

    const uint8_t mode = SPI_MODE_0;
    const uint8_t bitsPerWord = 8;
    ioctl(fd_, SPI_IOC_WR_MODE, &mode);
    ioctl(fd_, SPI_IOC_WR_BITS_PER_WORD, &bitsPerWord);
    ioctl(fd_, SPI_IOC_WR_MAX_SPEED_HZ, &speedHz_);
    return true;
}

std::vector<uint8_t> SPI::transfer(const std::vector<uint8_t>& tx) {
    if (fd_ < 0 || tx.empty()) {
        return {};
    }

    std::vector<uint8_t> rx(tx.size());
    spi_ioc_transfer message{};
    message.tx_buf = reinterpret_cast<uint64_t>(tx.data());
    message.rx_buf = reinterpret_cast<uint64_t>(rx.data());
    message.len = static_cast<uint32_t>(tx.size());
    message.speed_hz = speedHz_;
    message.bits_per_word = 8;

    if (ioctl(fd_, SPI_IOC_MESSAGE(1), &message) < 0) {
        logger_.error("SPI", std::string("SPI_IOC_MESSAGE failed: ") + std::strerror(errno));
        return {};
    }
    return rx;
}

void SPI::close() {
    if (fd_ >= 0) {
        ::close(fd_);
        fd_ = -1;
    }
}

