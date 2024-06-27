#include "user_manager.hpp"
#include <fstream>
#include <iomanip>

void UserManager::addUser(const std::string& username, int version, std::chrono::system_clock::time_point expire_time) {
    users_[username] = { username, version, expire_time };
}

void UserManager::removeUser(const std::string& username) {
    users_.erase(username);
}

UserManager::UserInfo* UserManager::getUser(const std::string& username) {
    auto it = users_.find(username);
    return it != users_.end() ? &it->second : nullptr;
}

void UserManager::extendVersion(int new_version, bool block_old_users) {
    for (auto& [username, user] : users_) {
        if (block_old_users && user.version < new_version) {
            user.banned = true;
        }
        else {
            user.version = new_version;
        }
    }
}

void UserManager::searchByUsername(const std::string& username) {
    auto user = getUser(username);
    if (user) {
        std::cout << "User: " << user->username << ", Version: " << user->version
            << ", Expire Time: " << std::chrono::system_clock::to_time_t(user->expire_time)
            << ", Banned: " << (user->banned ? "Yes" : "No") << std::endl;
    }
    else {
        std::cout << "User not found" << std::endl;
    }
}

void UserManager::banByUsername(const std::string& username) {
    auto user = getUser(username);
    if (user) {
        user->banned = true;
    }
}

void UserManager::extendUserExpireTimeByUsername(const std::string& username, std::chrono::system_clock::duration duration) {
    auto user = getUser(username);
    if (user) {
        user->expire_time += duration;
    }
}

void UserManager::extendAllUsersExpireTime(std::chrono::system_clock::duration duration) {
    for (auto& [username, user] : users_) {
        user.expire_time += duration;
    }
}

void UserManager::setMaintenance(bool status) {
    maintenance_ = status;
}

void UserManager::blockAllRequests(bool status) {
    block_requests_ = status;
}

bool UserManager::isMaintenance() const {
    return maintenance_;
}

bool UserManager::areRequestsBlocked() const {
    return block_requests_;
}

void UserManager::loadUsers(const std::string& filename) {
    std::ifstream file(filename);
    if (file.is_open()) {
        nlohmann::json j;
        file >> j;
        for (const auto& item : j.items()) {
            UserInfo user = item.value();
            users_[user.username] = user;
        }
    }
}

void UserManager::saveUsers(const std::string& filename) const {
    nlohmann::json j;
    for (const auto& [username, user] : users_) {
        j[username] = user;
    }

    std::ofstream file(filename);
    if (file.is_open()) {
        file << std::setw(4) << j << std::endl;
    }
}
