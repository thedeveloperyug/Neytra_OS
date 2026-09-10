// IIPC.hpp - inter-process messaging interface (depend on this, not IPC).
#pragma once

#include <string>

class IIPC {
public:
    virtual ~IIPC() = default;

    // Creates a named FIFO at `name` (a filesystem path) if it doesn't already exist.
    virtual bool createChannel(const std::string& name) = 0;

    // Blocking send/receive of one message over the named channel.
    virtual bool send(const std::string& name, const std::string& message) = 0;
    virtual bool receive(const std::string& name, std::string& outMessage) = 0;

    virtual void closeChannel(const std::string& name) = 0;
};
