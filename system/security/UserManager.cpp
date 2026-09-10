// UserManager.cpp - in-memory user database, seeded with root (uid 0).
#include "UserManager.hpp"

#include <fstream>
#include <sstream>

namespace {
std::vector<std::string> splitFields(const std::string& line, char delim) {
    std::vector<std::string> fields;
    std::stringstream ss(line);
    std::string field;
    while (std::getline(ss, field, delim)) {
        fields.push_back(field);
    }
    return fields;
}
}  // namespace

UserManager::UserManager(ILogger& logger) : logger_(logger) {
    // Every real Unix-like system has uid 0; seed it so lookups never fail on root.
    addUser(UserRecord{"root", 0, 0, "/root", "/bin/sh"});
}

bool UserManager::addUser(const UserRecord& user) {
    if (byUid_.count(user.uid) != 0) {
        logger_.warn("UserManager", "addUser: uid already exists: " + std::to_string(user.uid));
        return false;
    }
    byUid_[user.uid] = user;
    logger_.info("UserManager", "added user " + user.username + " (uid " + std::to_string(user.uid) + ")");
    return true;
}

bool UserManager::removeUser(const std::string& username) {
    for (auto it = byUid_.begin(); it != byUid_.end(); ++it) {
        if (it->second.username == username) {
            byUid_.erase(it);
            logger_.info("UserManager", "removed user " + username);
            return true;
        }
    }
    return false;
}

std::optional<UserRecord> UserManager::findByName(const std::string& username) const {
    for (const auto& [uid, user] : byUid_) {
        if (user.username == username) {
            return user;
        }
    }
    return std::nullopt;
}

std::optional<UserRecord> UserManager::findByUid(unsigned int uid) const {
    auto it = byUid_.find(uid);
    if (it == byUid_.end()) {
        return std::nullopt;
    }
    return it->second;
}

std::vector<UserRecord> UserManager::listUsers() const {
    std::vector<UserRecord> users;
    users.reserve(byUid_.size());
    for (const auto& [uid, user] : byUid_) {
        users.push_back(user);
    }
    return users;
}

bool UserManager::loadFromFile(const std::string& path) {
    std::ifstream in(path);
    if (!in.is_open()) {
        logger_.warn("UserManager", "loadFromFile: cannot open " + path);
        return false;
    }

    std::string line;
    size_t loaded = 0;
    while (std::getline(in, line)) {
        if (line.empty() || line[0] == '#') {
            continue;
        }
        const std::vector<std::string> fields = splitFields(line, ':');
        if (fields.size() < 7) {
            continue;
        }
        UserRecord user;
        user.username = fields[0];
        user.uid = static_cast<unsigned int>(std::stoul(fields[2]));
        user.gid = static_cast<unsigned int>(std::stoul(fields[3]));
        user.home = fields[5];
        user.shell = fields[6];
        byUid_[user.uid] = user;
        ++loaded;
    }
    logger_.info("UserManager", "loaded " + std::to_string(loaded) + " users from " + path);
    return true;
}

bool UserManager::saveToFile(const std::string& path) const {
    std::ofstream out(path, std::ios::trunc);
    if (!out.is_open()) {
        logger_.warn("UserManager", "saveToFile: cannot open " + path);
        return false;
    }
    for (const auto& [uid, user] : byUid_) {
        out << user.username << ":x:" << user.uid << ":" << user.gid << "::"
            << user.home << ":" << user.shell << "\n";
    }
    return true;
}

