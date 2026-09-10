// PermissionManager.cpp - simple per-uid/resource rule table.
#include "PermissionManager.hpp"

namespace {
const char* permissionName(Permission perm) {
    switch (perm) {
        case Permission::Read:    return "Read";
        case Permission::Write:   return "Write";
        case Permission::Execute: return "Execute";
        case Permission::Admin:   return "Admin";
    }
    return "?";
}
}  // namespace

PermissionManager::PermissionManager(ILogger& logger) : logger_(logger) {}

bool PermissionManager::check(unsigned int uid, const std::string& resource, Permission perm) const {
    if (uid == 0) {
        return true;  // root bypasses the rule table, like real Unix permission checks.
    }
    const auto it = rules_.find({uid, resource});
    const bool allowed = it != rules_.end() && it->second.count(perm) != 0;
    if (!allowed) {
        logger_.warn("PermissionManager", "denied uid " + std::to_string(uid) + " " +
                     permissionName(perm) + " on " + resource);
    }
    return allowed;
}

void PermissionManager::grant(unsigned int uid, const std::string& resource, Permission perm) {
    rules_[{uid, resource}].insert(perm);
    logger_.info("PermissionManager", "granted uid " + std::to_string(uid) + " " +
                 permissionName(perm) + " on " + resource);
}

void PermissionManager::revoke(unsigned int uid, const std::string& resource, Permission perm) {
    auto it = rules_.find({uid, resource});
    if (it != rules_.end()) {
        it->second.erase(perm);
    }
    logger_.info("PermissionManager", "revoked uid " + std::to_string(uid) + " " +
                 permissionName(perm) + " on " + resource);
}

