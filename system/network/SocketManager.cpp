// SocketManager.cpp - real BSD sockets (socket/bind/connect/send/recv/close).
#include "SocketManager.hpp"

#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <vector>

SocketManager::SocketManager(ILogger& logger) : logger_(logger) {}

int SocketManager::createSocket(SocketType type) {
    const int fd = ::socket(AF_INET, type == SocketType::TCP ? SOCK_STREAM : SOCK_DGRAM, 0);
    if (fd < 0) {
        logger_.error("SocketManager", std::string("socket() failed: ") + std::strerror(errno));
    }
    return fd;
}

bool SocketManager::bind(int fd, const std::string& address, int port) {
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(static_cast<uint16_t>(port));
    addr.sin_addr.s_addr = address.empty() ? INADDR_ANY : inet_addr(address.c_str());

    if (::bind(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) != 0) {
        logger_.error("SocketManager", std::string("bind() failed: ") + std::strerror(errno));
        return false;
    }
    return true;
}

bool SocketManager::connect(int fd, const std::string& address, int port) {
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(static_cast<uint16_t>(port));
    if (inet_pton(AF_INET, address.c_str(), &addr.sin_addr) != 1) {
        logger_.error("SocketManager", "invalid address: " + address);
        return false;
    }

    if (::connect(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) != 0) {
        logger_.error("SocketManager", std::string("connect() failed: ") + std::strerror(errno));
        return false;
    }
    return true;
}

long SocketManager::send(int fd, const std::string& data) {
    const ssize_t sent = ::send(fd, data.data(), data.size(), 0);
    if (sent < 0) {
        logger_.error("SocketManager", std::string("send() failed: ") + std::strerror(errno));
    }
    return sent;
}

long SocketManager::receive(int fd, std::string& outData, size_t maxLen) {
    std::vector<char> buffer(maxLen);
    const ssize_t received = ::recv(fd, buffer.data(), maxLen, 0);
    if (received < 0) {
        logger_.error("SocketManager", std::string("recv() failed: ") + std::strerror(errno));
        return received;
    }
    outData.assign(buffer.data(), static_cast<size_t>(received));
    return received;
}

void SocketManager::closeSocket(int fd) {
    ::close(fd);
}

