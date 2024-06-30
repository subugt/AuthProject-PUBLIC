#include <enet/enet.h>
#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <fstream>
#include <ctime>
#include "encryption.hpp"
#include "user_manager.hpp"
#include "ddos_protection.hpp"
#include <nlohmann/json.hpp>
#include <openssl/aes.h>
UserManager userManager;
DDoSProtection ddosProtection(100, 60, 300, 1024); // 100 requests 60 seconds window 300 seconds block 1024 byte max packet size

const std::string AesKey = "passwordforaes256!";
const std::string aesiv = "saymyname12c";


std::string encryptAES(const std::string& plainText) {
    AES_KEY aesKeyEncrypt;
    AES_set_encrypt_key(reinterpret_cast<const unsigned char*>(aesKey.c_str()), 256, &aesKeyEncrypt);

    std::string encryptedText;
    int padding = 0;
    int plainTextLength = plainText.length();

    if (plainTextLength % AES_BLOCK_SIZE != 0) {
        padding = AES_BLOCK_SIZE - (plainTextLength % AES_BLOCK_SIZE);
    }

    int encryptedLength = plainTextLength + padding;
    unsigned char iv[AES_BLOCK_SIZE];
    memset(iv, 0, AES_BLOCK_SIZE);

    AES_cbc_encrypt(reinterpret_cast<const unsigned char*>(plainText.c_str()), 
                    reinterpret_cast<unsigned char*>(encryptedText.data()), 
                    encryptedLength, &aesKeyEncrypt, iv, AES_ENCRYPT);

    return encryptedText;
}


std::string decryptAES(const std::string& encryptedText) {
    AES_KEY aesKeyDecrypt;
    AES_set_decrypt_key(reinterpret_cast<const unsigned char*>(aesKey.c_str()), 256, &aesKeyDecrypt);

    std::string decryptedText;
    unsigned char iv[AES_BLOCK_SIZE];
    memset(iv, 0, AES_BLOCK_SIZE);

    AES_cbc_encrypt(reinterpret_cast<const unsigned char*>(encryptedText.c_str()), 
                    reinterpret_cast<unsigned char*>(decryptedText.data()), 
                    encryptedText.length(), &aesKeyDecrypt, iv, AES_DECRYPT);

    return decryptedText;
}

std::string getCurrentTimeAsString() {
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    return std::ctime(&now_c);
}


void writeToBlockedRequestsLog(const std::string& ip, int packetSize, std::chrono::seconds blockTime, std::chrono::seconds attackTime) {
    std::ofstream logFile("blocked_requests.log", std::ios::app);
    if (logFile.is_open()) {
        logFile << "IP: " << ip << " - Packet Size: " << packetSize
                << " - Block Time: " << blockTime.count() << " seconds"
                << " - Attack Time: " << attackTime.count() << " seconds"
                << " - Time: " << getCurrentTimeAsString();
        logFile.close();
    } else {
        std::cerr << "Unable to open blocked requests log file for writing." << std::endl;
    }
}


void writeToEncryptedLog(const std::string& encryptedMessage) {
    std::ofstream logFile("server_log.dat", std::ios::app | std::ios::binary);
    if (logFile.is_open()) {
        logFile << encryptedMessage << std::endl;
        logFile.close();
    } else {
        std::cerr << "Unable to open encrypted log file for writing." << std::endl;
    }
}


void handleCommand(const std::string& command) {
    nlohmann::json json_cmd;
    try {
        json_cmd = nlohmann::json::parse(command);
    } catch (const nlohmann::json::parse_error& e) {
        std::cout << "Invalid JSON format" << std::endl;
        return;
    }

    std::string cmd_type = json_cmd.value("command", "");
    std::string token = json_cmd.value("token", "");

    const std::string valid_token = "maybe_next_update_will_able_to_generate_random_tokens_and_share_with_real_clients_i_will_inform_you_soon";

    if (token != valid_token) {
        std::cout << "Invalid token" << std::endl;
        return;
    }

    if (cmd_type == "maintenance") {
        bool status = json_cmd.value("status", false);
        userManager.setMaintenance(status);
        std::cout << "Maintenance mode " << (status ? "enabled" : "disabled") << std::endl;
    } else if (cmd_type == "block_requests") {
        bool status = json_cmd.value("status", false);
        userManager.blockAllRequests(status);
        std::cout << "All requests " << (status ? "blocked" : "unblocked") << std::endl;
    } else if (cmd_type == "add_user") {
        std::string username = json_cmd.value("username", "");
        int version = json_cmd.value("version", 1);
        auto expire_time = std::chrono::system_clock::now() + std::chrono::hours(json_cmd.value("expire_hours", 24 * 30));
        userManager.addUser(username, version, expire_time);
        std::cout << "Added user: " << username << std::endl;
    } else if (cmd_type == "remove_user") {
        std::string username = json_cmd.value("username", "");
        userManager.removeUser(username);
        std::cout << "Removed user: " << username << std::endl;
    } else if (cmd_type == "search_user") {
        std::string username = json_cmd.value("username", "");
        userManager.searchByUsername(username);
    } else if (cmd_type == "ban_user") {
        std::string username = json_cmd.value("username", "");
        userManager.banByUsername(username);
        std::cout << "Banned user: " << username << std::endl;
    } else if (cmd_type == "extend_version") {
        int new_version = json_cmd.value("new_version", 1);
        bool block_old_users = json_cmd.value("block_old_users", false);
        userManager.extendVersion(new_version, block_old_users);
        std::cout << "Extended version to: " << new_version << std::endl;
    } else if (cmd_type == "extend_user_expire") {
        std::string username = json_cmd.value("username", "");
        auto duration = std::chrono::hours(json_cmd.value("hours", 24 * 30));
        userManager.extendUserExpireTimeByUsername(username, duration);
        std::cout << "Extended expire time for user: " << username << std::endl;
    } else if (cmd_type == "extend_all_users_expire") {
        auto duration = std::chrono::hours(json_cmd.value("hours", 24 * 30));
        userManager.extendAllUsersExpireTime(duration);
        std::cout << "Extended expire time for all users" << std::endl;
    }
}

int main(int argc, char** argv) {
    if (enet_initialize() != 0) {
        std::cerr << "An error occurred while initializing ENet." << std::endl;
        return EXIT_FAILURE;
    }
    atexit(enet_deinitialize);

    ENetAddress address;
    ENetHost* server;
    enet_address_set_host(&address, "127.0.0.1");
    address.port = 1234;
    server = enet_host_create(&address, 32, 2, 0, 0);

    if (server == nullptr) {
        std::cerr << "Server creation failed!" << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << "Server started..." << std::endl;

    // Aes256 key
    std::string key = "coolpasswordforaes256keynicee!"; 

    // Adding some users
    auto now = std::chrono::system_clock::now();
    userManager.addUser("user1", 1, now + std::chrono::hours(24 * 30));  
    userManager.addUser("user2", 1, now + std::chrono::hours(24 * 60));  

    // Main server loop
    ENetEvent event;
    while (true) {
        while (enet_host_service(server, &event, 1000) > 0) {
            if (userManager.isMaintenance()) {
                std::cout << "Server is under maintenance, rejecting connections." << std::endl;
                enet_peer_reset(event.peer);
                continue;
            }

            enet_uint32 clientIP = event.peer->address.host;
            switch (event.type) {
                case ENET_EVENT_TYPE_CONNECT:
                    if (ddosProtection.isBlocked(clientIP) || userManager.areRequestsBlocked()) {
                        std::cout << "Connection from " << clientIP << " blocked." << std::endl;
 std::string logMessage = "IP blocked due to DDoS or invalid packet size: " + std::to_string(clientIP);
                        writeToEncryptedLog(encryptAES(logMessage));
                        enet_peer_reset(event.peer);
                    } else {
                        std::cout << "A new client connected from " << clientIP << std::endl;
                        ddosProtection.registerRequest(clientIP);
                    }
                    break;

                case ENET_EVENT_TYPE_RECEIVE:
                    if (ddosProtection.isBlocked(clientIP) || !ddosProtection.isPacketSizeValid(event.packet->dataLength)) {
                        std::cout << "Packet from " << clientIP << " blocked due to DDoS protection." << std::endl;
                        enet_packet_destroy(event.packet);
                    } else {
                        std::cout << "A packet received from "
                            << clientIP << " containing "
                            << event.packet->dataLength << " bytes." << std::endl;

                        std::vector<uint8_t> received_data(event.packet->data, event.packet->data + event.packet->dataLength);
                        std::string decrypted_message = Encryption::decrypt(received_data, key);

                        std::cout << "Decrypted message: " << decrypted_message << std::endl;

                        if (decrypted_message.rfind("command:", 0) == 0) {
                            std::string command = decrypted_message.substr(8);
                            handleCommand(command);
                        } else {
                            std::cout << "Invalid request format or unauthorized access, redirecting to Google." << std::endl;
                            const char* redirect_msg = "HTTP/1.1 302 Found\r\nLocation: https://www.google.com/\r\n\r\n";
                            ENetPacket* packet = enet_packet_create(redirect_msg, strlen(redirect_msg) + 1, ENET_PACKET_FLAG_RELIABLE);
                            enet_peer_send(event.peer, 0, packet);
                        }
                    }

                    enet_packet_destroy(event.packet);
                    break;

                case ENET_EVENT_TYPE_DISCONNECT:
                    std::cout << "Client " << clientIP << " disconnected." << std::endl;
                    break;

                default:
                    break;
            }
        }
    }

    enet_host_destroy(server);
    return EXIT_SUCCESS;
} // I WILL CONTINUE DONT WORRY 