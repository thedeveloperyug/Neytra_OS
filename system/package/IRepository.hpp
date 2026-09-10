// IRepository.hpp - package index interface (depend on this, not Repository).
#pragma once

#include <optional>
#include <string>
#include <vector>

struct PackageEntry {
    std::string name;
    std::string version;
    std::string url;
};

class IRepository {
public:
    virtual ~IRepository() = default;

    // Parses a flat text index: name|version|url per line.
    virtual bool loadFromFile(const std::string& path) = 0;

    virtual std::optional<PackageEntry> find(const std::string& name) const = 0;
    virtual std::vector<PackageEntry> list() const = 0;
};
