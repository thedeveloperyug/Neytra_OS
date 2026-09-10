// SocketManager.hpp - real BSD sockets (socket/bind/connect/send/recv/close).
#pragma once

#include "ISocketManager.hpp"
#include "ILogger.hpp"

class SocketManager : public ISocketManager {
public:
    explicit SocketManager(ILogger& logger);

    int createSocket(SocketType type) override;
    bool bind(int fd, const std::string& address, int port) override;
    bool connect(int fd, const std::string& address, int port) override;
    long send(int fd, const std::string& data) override;
    long receive(int fd, std::string& outData, size_t maxLen) override;
    void closeSocket(int fd) override;

private:
    ILogger& logger_;
};
