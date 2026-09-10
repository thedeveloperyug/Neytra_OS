// Repository.hpp - in-memory package index loaded from a flat text file.
#pragma once

#include "IRepository.hpp"
#include "ILogger.hpp"

#include <map>

class Repository : public IRepository {
public:
    explicit Repository(ILogger& logger);

    bool loadFromFile(const std::string& path) override;
    std::optional<PackageEntry> find(const std::string& name) const override;
    std::vector<PackageEntry> list() const override;

private:
    ILogger& logger_;
    std::map<std::string, PackageEntry> byName_;
};
