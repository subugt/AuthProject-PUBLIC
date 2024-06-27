#ifndef ENCRYPTION_HPP
#define ENCRYPTION_HPP

#include <string>
#include <vector>

class Encryption {
public:
    static std::string decrypt(const std::vector<uint8_t>& data, const std::string& key);
    static std::vector<uint8_t> encrypt(const std::string& data, const std::string& key);
};

#endif // ENCRYPTION_HPP
