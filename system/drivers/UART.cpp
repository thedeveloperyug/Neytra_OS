// UART.cpp - real termios-configured serial I/O.
#include "UART.hpp"

#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <vector>

namespace {
speed_t toTermiosBaud(int baudRate) {
    switch (baudRate) {
        case 9600:   return B9600;
        case 19200:  return B19200;
        case 38400:  return B38400;
        case 57600:  return B57600;
        case 115200: return B115200;
        default:     return B115200;
    }
}
}  // namespace

UART::UART(ILogger& logger) : logger_(logger) {}

UART::~UART() {
    close();
}

bool UART::open(const std::string& devicePath, int baudRate) {
    fd_ = ::open(devicePath.c_str(), O_RDWR | O_NOCTTY);
    if (fd_ < 0) {
        logger_.warn("UART", "cannot open " + devicePath + ": " + std::strerror(errno));
        return false;
    }

    termios options{};
    if (tcgetattr(fd_, &options) != 0) {
        logger_.error("UART", std::string("tcgetattr failed: ") + std::strerror(errno));
        ::close(fd_);
        fd_ = -1;
        return false;
    }

    const speed_t speed = toTermiosBaud(baudRate);
    cfsetispeed(&options, speed);
    cfsetospeed(&options, speed);

    options.c_cflag |= (CLOCAL | CREAD);
    options.c_cflag &= ~PARENB;   // no parity
    options.c_cflag &= ~CSTOPB;   // 1 stop bit
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS8;       // 8 data bits
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);  // raw mode
    options.c_iflag &= ~(IXON | IXOFF | IXANY);
    options.c_oflag &= ~OPOST;

    tcsetattr(fd_, TCSANOW, &options);
    return true;
}

long UART::write(const std::string& data) {
    if (fd_ < 0) {
        return -1;
    }
    return ::write(fd_, data.data(), data.size());
}

std::string UART::read(size_t maxLen) {
    if (fd_ < 0) {
        return {};
    }
    std::vector<char> buffer(maxLen);
    const ssize_t bytesRead = ::read(fd_, buffer.data(), maxLen);
    if (bytesRead <= 0) {
        return {};
    }
    return std::string(buffer.data(), static_cast<size_t>(bytesRead));
}

void UART::close() {
    if (fd_ >= 0) {
        ::close(fd_);
        fd_ = -1;
    }
}

