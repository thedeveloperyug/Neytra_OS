// Downloader.hpp - real HTTP/1.1 GET client over a raw TCP socket (no TLS).
#pragma once

#include "IDownloader.hpp"
#include "ILogger.hpp"
#include "ISocketManager.hpp"

class Downloader : public IDownloader {
public:
    Downloader(ISocketManager& sockets, ILogger& logger);

    bool fetch(const std::string& url, const std::string& outputPath) override;

private:
    ISocketManager& sockets_;
    ILogger& logger_;

    struct ParsedUrl {
        std::string host;
        int port = 80;
        std::string path = "/";
        bool valid = false;
    };
    ParsedUrl parseUrl(const std::string& url) const;
};
