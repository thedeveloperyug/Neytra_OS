// IPC.cpp - real named-pipe (FIFO) based inter-process communication.
#include "IPC.hpp"

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <vector>

IPC::IPC(ILogger& logger) : logger_(logger) {}

bool IPC::createChannel(const std::string& name) {
    if (mkfifo(name.c_str(), 0666) != 0 && errno != EEXIST) {
        logger_.error("IPC", "mkfifo(" + name + ") failed: " + std::strerror(errno));
        return false;
    }
    channels_[name] = true;
    logger_.info("IPC", "channel ready: " + name);
    return true;
}

bool IPC::send(const std::string& name, const std::string& message) {
    // O_WRONLY on a FIFO blocks until a reader opens it -- that's the intended
    // rendezvous semantics for this simple channel.
    const int fd = open(name.c_str(), O_WRONLY);
    if (fd < 0) {
        logger_.error("IPC", "open(" + name + ", O_WRONLY) failed: " + std::strerror(errno));
        return false;
    }
    const ssize_t written = write(fd, message.data(), message.size());
    close(fd);
    if (written < 0 || static_cast<size_t>(written) != message.size()) {
        logger_.error("IPC", "write to " + name + " failed: " + std::strerror(errno));
        return false;
    }
    return true;
}

bool IPC::receive(const std::string& name, std::string& outMessage) {
    const int fd = open(name.c_str(), O_RDONLY);
    if (fd < 0) {
        logger_.error("IPC", "open(" + name + ", O_RDONLY) failed: " + std::strerror(errno));
        return false;
    }

    outMessage.clear();
    std::vector<char> buffer(4096);
    ssize_t bytesRead;
    while ((bytesRead = read(fd, buffer.data(), buffer.size())) > 0) {
        outMessage.append(buffer.data(), static_cast<size_t>(bytesRead));
    }
    close(fd);
    return true;
}

void IPC::closeChannel(const std::string& name) {
    unlink(name.c_str());
    channels_.erase(name);
}

