// UserManager.hpp - in-memory user database, seeded with root (uid 0).
#pragma once

#include "IUserManager.hpp"
#include "ILogger.hpp"

#include <map>

class UserManager : public IUserManager {
public:
    explicit UserManager(ILogger& logger);

    bool addUser(const UserRecord& user) override;
    bool removeUser(const std::string& username) override;

    std::optional<UserRecord> findByName(const std::string& username) const override;
    std::optional<UserRecord> findByUid(unsigned int uid) const override;
    std::vector<UserRecord> listUsers() const override;

    bool loadFromFile(const std::string& path) override;
    bool saveToFile(const std::string& path) const override;

private:
    ILogger& logger_;
    std::map<unsigned int, UserRecord> byUid_;
};
