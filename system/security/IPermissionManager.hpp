// IPermissionManager.hpp - access-control interface (depend on this, not PermissionManager).
#pragma once

#include <string>

enum class Permission { Read, Write, Execute, Admin };

class IPermissionManager {
public:
    virtual ~IPermissionManager() = default;

    // uid 0 (root) is always allowed, regardless of the rule table.
    virtual bool check(unsigned int uid, const std::string& resource, Permission perm) const = 0;

    virtual void grant(unsigned int uid, const std::string& resource, Permission perm) = 0;
    virtual void revoke(unsigned int uid, const std::string& resource, Permission perm) = 0;
};
