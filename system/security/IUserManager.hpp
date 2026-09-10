// IUserManager.hpp - user account interface (depend on this, not UserManager).
#pragma once

#include <optional>
#include <string>
#include <vector>

struct UserRecord {
    std::string username;
    unsigned int uid = 0;
    unsigned int gid = 0;
    std::string home;
    std::string shell;
};

class IUserManager {
public:
    virtual ~IUserManager() = default;

    virtual bool addUser(const UserRecord& user) = 0;
    virtual bool removeUser(const std::string& username) = 0;

    virtual std::optional<UserRecord> findByName(const std::string& username) const = 0;
    virtual std::optional<UserRecord> findByUid(unsigned int uid) const = 0;
    virtual std::vector<UserRecord> listUsers() const = 0;

    // Parses a passwd(5)-style file: name:x:uid:gid:gecos:home:shell
    virtual bool loadFromFile(const std::string& path) = 0;
    virtual bool saveToFile(const std::string& path) const = 0;
};
