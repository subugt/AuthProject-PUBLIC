#ifndef USER_MANAGER_HPP
#define USER_MANAGER_HPP

#include <string>
#include <unordered_map>
#include <chrono>
#include <nlohmann/json.hpp>

class UserManager {
public:
    struct UserInfo {
        std::string username;
        int version;
        std::chrono::system_clock::time_point expire_time;
        bool banned = false;

        NLOHMANN_DEFINE_TYPE_INTRUSIVE(UserInfo, username, version, expire_time, banned)
    };

    void addUser(const std::string& username, int version, std::chrono::system_clock::time_point expire_time);
    void removeUser(const std::string& username);
    UserInfo* getUser(const std::string& username);

    void extendVersion(int new_version, bool block_old_users);
    void searchByUsername(const std::string& username);
    void banByUsername(const std::string& username);
    void extendUserExpireTimeByUsername(const std::string& username, std::chrono::system_clock::duration duration);
    void extendAllUsersExpireTime(std::chrono::system_clock::duration duration);
    void setMaintenance(bool status);
    void blockAllRequests(bool status);

    bool isMaintenance() const;
    bool areRequestsBlocked() const;

    void loadUsers(const std::string& filename);
    void saveUsers(const std::string& filename) const;

private:
    std::unordered_map<std::string, UserInfo> users_;
    bool maintenance_ = false;
    bool block_requests_ = false;
};

#endif // USER_MANAGER_HPP
