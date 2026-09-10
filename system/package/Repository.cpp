// Repository.cpp - in-memory package index loaded from a flat text file.
#include "Repository.hpp"

#include <fstream>
#include <sstream>

Repository::Repository(ILogger& logger) : logger_(logger) {}

bool Repository::loadFromFile(const std::string& path) {
    std::ifstream in(path);
    if (!in.is_open()) {
        logger_.warn("Repository", "cannot open index: " + path);
        return false;
    }

    std::string line;
    size_t loaded = 0;
    while (std::getline(in, line)) {
        if (line.empty() || line[0] == '#') {
            continue;
        }
        std::istringstream fields(line);
        std::string name, version, url;
        if (!std::getline(fields, name, '|') || !std::getline(fields, version, '|') ||
            !std::getline(fields, url)) {
            continue;
        }
        byName_[name] = PackageEntry{name, version, url};
        ++loaded;
    }
    logger_.info("Repository", "loaded " + std::to_string(loaded) + " package(s) from " + path);
    return true;
}

std::optional<PackageEntry> Repository::find(const std::string& name) const {
    auto it = byName_.find(name);
    if (it == byName_.end()) {
        return std::nullopt;
    }
    return it->second;
}

std::vector<PackageEntry> Repository::list() const {
    std::vector<PackageEntry> result;
    result.reserve(byName_.size());
    for (const auto& [name, entry] : byName_) {
        result.push_back(entry);
    }
    return result;
}

