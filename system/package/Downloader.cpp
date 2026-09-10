// Downloader.cpp - real HTTP/1.1 GET client over a raw TCP socket (no TLS).
#include "Downloader.hpp"

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>

#include <cstdlib>
#include <fstream>

Downloader::Downloader(ISocketManager& sockets, ILogger& logger) : sockets_(sockets), logger_(logger) {}

Downloader::ParsedUrl Downloader::parseUrl(const std::string& url) const {
    ParsedUrl result;
    const std::string prefix = "http://";
    if (url.rfind(prefix, 0) != 0) {
        return result;  // only plain HTTP is supported
    }

    const std::string rest = url.substr(prefix.size());
    const size_t slash = rest.find('/');
    std::string hostPort = slash == std::string::npos ? rest : rest.substr(0, slash);
    result.path = slash == std::string::npos ? "/" : rest.substr(slash);

    const size_t colon = hostPort.find(':');
    if (colon == std::string::npos) {
        result.host = hostPort;
        result.port = 80;
    } else {
        result.host = hostPort.substr(0, colon);
        result.port = std::atoi(hostPort.substr(colon + 1).c_str());
    }

    result.valid = !result.host.empty();
    return result;
}

bool Downloader::fetch(const std::string& url, const std::string& outputPath) {
    const ParsedUrl parsed = parseUrl(url);
    if (!parsed.valid) {
        logger_.error("Downloader", "unsupported or invalid URL: " + url);
        return false;
    }

    // Resolve the hostname to an IPv4 address ourselves; ISocketManager::connect
    // takes a numeric address.
    addrinfo hints{};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    addrinfo* resolved = nullptr;
    if (getaddrinfo(parsed.host.c_str(), nullptr, &hints, &resolved) != 0 || resolved == nullptr) {
        logger_.error("Downloader", "DNS resolution failed for " + parsed.host);
        return false;
    }
    char ipBuf[INET6_ADDRSTRLEN] = {0};
    inet_ntop(AF_INET, &reinterpret_cast<sockaddr_in*>(resolved->ai_addr)->sin_addr, ipBuf, sizeof(ipBuf));
    const std::string ip = ipBuf;
    freeaddrinfo(resolved);

    const int fd = sockets_.createSocket(SocketType::TCP);
    if (fd < 0) {
        return false;
    }
    if (!sockets_.connect(fd, ip, parsed.port)) {
        logger_.error("Downloader", "connect to " + parsed.host + ":" + std::to_string(parsed.port) + " failed");
        sockets_.closeSocket(fd);
        return false;
    }

    const std::string request = "GET " + parsed.path + " HTTP/1.1\r\n"
                                 "Host: " + parsed.host + "\r\n"
                                 "Connection: close\r\n\r\n";
    if (sockets_.send(fd, request) < 0) {
        sockets_.closeSocket(fd);
        return false;
    }

    std::string response;
    std::string chunk;
    while (sockets_.receive(fd, chunk, 4096) > 0) {
        response += chunk;
    }
    sockets_.closeSocket(fd);

    const size_t headerEnd = response.find("\r\n\r\n");
    if (headerEnd == std::string::npos) {
        logger_.error("Downloader", "malformed HTTP response from " + parsed.host);
        return false;
    }
    const std::string body = response.substr(headerEnd + 4);

    std::ofstream out(outputPath, std::ios::binary | std::ios::trunc);
    if (!out.is_open()) {
        logger_.error("Downloader", "cannot write to " + outputPath);
        return false;
    }
    out.write(body.data(), static_cast<std::streamsize>(body.size()));

    logger_.info("Downloader", "fetched " + url + " -> " + outputPath + " (" +
                 std::to_string(body.size()) + " bytes)");
    return true;
}

