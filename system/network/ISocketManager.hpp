// ISocketManager.hpp - BSD socket wrapper interface (depend on this, not SocketManager).
#pragma once

#include <cstddef>
#include <string>

enum class SocketType { TCP, UDP };

class ISocketManager {
public:
    virtual ~ISocketManager() = default;

    virtual int createSocket(SocketType type) = 0;  // returns fd, or -1 on failure
    virtual bool bind(int fd, const std::string& address, int port) = 0;
    virtual bool connect(int fd, const std::string& address, int port) = 0;
    virtual long send(int fd, const std::string& data) = 0;
    virtual long receive(int fd, std::string& outData, size_t maxLen) = 0;
    virtual void closeSocket(int fd) = 0;
};
