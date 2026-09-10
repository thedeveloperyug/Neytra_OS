// PermissionManager.hpp - simple per-uid/resource rule table.
#pragma once

#include "IPermissionManager.hpp"
#include "ILogger.hpp"

#include <map>
#include <set>
#include <utility>

class PermissionManager : public IPermissionManager {
public:
    explicit PermissionManager(ILogger& logger);

    bool check(unsigned int uid, const std::string& resource, Permission perm) const override;
    void grant(unsigned int uid, const std::string& resource, Permission perm) override;
    void revoke(unsigned int uid, const std::string& resource, Permission perm) override;

private:
    using Key = std::pair<unsigned int, std::string>;

    ILogger& logger_;
    std::map<Key, std::set<Permission>> rules_;
};
