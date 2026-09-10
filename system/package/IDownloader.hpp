// IDownloader.hpp - HTTP fetch interface (depend on this, not Downloader).
#pragma once

#include <string>

class IDownloader {
public:
    virtual ~IDownloader() = default;

    // Fetches http://host[:port]/path via a raw HTTP/1.1 GET and writes the
    // response body to outputPath. No HTTPS support (would need a TLS library).
    virtual bool fetch(const std::string& url, const std::string& outputPath) = 0;
};
