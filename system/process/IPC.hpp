// IPC.hpp - real named-pipe (FIFO) based inter-process communication.
#pragma once

#include "IIPC.hpp"
#include "ILogger.hpp"

#include <map>

class IPC : public IIPC {
public:
    explicit IPC(ILogger& logger);

    bool createChannel(const std::string& name) override;
    bool send(const std::string& name, const std::string& message) override;
    bool receive(const std::string& name, std::string& outMessage) override;
    void closeChannel(const std::string& name) override;

private:
    ILogger& logger_;
    std::map<std::string, bool> channels_;  // name -> created
};
