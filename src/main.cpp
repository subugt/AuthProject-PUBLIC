#include <enet/enet.h>
#include <iostream>
#include <vector>
#include <string>
#include "encryption.hpp"
#include "user_manager.hpp"
#include <nlohmann/json.hpp>

UserManager userManager;

void handleCommand(const std::string& command) {
    nlohmann::json json_cmd;
    try {
        json_cmd = nlohmann::json::parse(command);
    }
    catch (const nlohmann::json::parse_error& e) {
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
    }
    else if (cmd_type == "block_requests") {
        bool status = json_cmd.value("status", false);
        userManager.blockAllRequests(status);
        std::cout << "All requests " << (status ? "blocked" : "unblocked") << std::endl;
    }
    else if (cmd_type == "add_user") {
        std::string username = json_cmd.value("username", "");
        int version = json_cmd.value("version", 1);
        auto expire_time = std::chrono::system_clock::now() + std::chrono::hours(json_cmd.value("expire_hours", 24 * 30));
        userManager.addUser(username, version, expire_time);
        std::cout << "Added user: " << username << std::endl;
    }
    else if (cmd_type == "remove_user") {
        std::string username = json_cmd.value("username", "");
        userManager.removeUser(username);
        std::cout << "Removed user: " << username << std::endl;
    }
    else if (cmd_type == "search_user") {
        std::string username = json_cmd.value("username", "");
        userManager.searchByUsername(username);
    }
    else if (cmd_type == "ban_user") {
        std::string username = json_cmd.value("username", "");
        userManager.banByUsername(username);
        std::cout << "Banned user: " << username << std::endl;
    }
    else if (cmd_type == "extend_version") {
        int new_version = json_cmd.value("new_version", 1);
        bool block_old_users = json_cmd.value("block_old_users", false);
        userManager.extendVersion(new_version, block_old_users);
        std::cout << "Extended version to: " << new_version << std::endl;
    }
    else if (cmd_type == "extend_user_expire") {
        std::string username = json_cmd.value("username", "");
        auto duration = std::chrono::hours(json_cmd.value("hours", 24 * 30));
        userManager.extendUserExpireTimeByUsername(username, duration);
        std::cout << "Extended expire time for user: " << username << std::endl;
    }
    else if (cmd_type == "extend_all_users_expire") {
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

    // NEXT UPDATE WILL ABLE TO ADD USERS WHILE RUNTIME DONT WORRY
    auto now = std::chrono::system_clock::now();
    userManager.addUser("user1", 1, now + std::chrono::hours(24 * 30));  // 24 hour X days so that means 24.1 = 1 day 24.2 = 2 day
    userManager.addUser("user2", 1, now + std::chrono::hours(24 * 60));  

    //main server 
    ENetEvent event;
    while (true) {
        while (enet_host_service(server, &event, 1000) > 0) {
            if (userManager.isMaintenance()) {
                std::cout << "Server is under maintenance, rejecting connections." << std::endl;
                enet_peer_reset(event.peer);
                continue;
            }

            switch (event.type) {
            case ENET_EVENT_TYPE_CONNECT:
                if (userManager.areRequestsBlocked()) {
                    std::cout << "All requests are blocked, rejecting connection." << std::endl;
                    enet_peer_reset(event.peer);
                }
                else {
                    std::cout << "A new client connected from "
                        << event.peer->address.host << ":"
                        << event.peer->address.port << std::endl;
                    event.peer->data = (void*)"Client information";
                }
                break;

            case ENET_EVENT_TYPE_RECEIVE: {
                if (userManager.areRequestsBlocked()) {
                    std::cout << "All requests are blocked, discarding received packet." << std::endl;
                    enet_packet_destroy(event.packet);
                    break;
                }

                std::cout << "A packet received from "
                    << (char*)event.peer->data << " containing "
                    << event.packet->dataLength << " bytes." << std::endl;

                std::vector<uint8_t> received_data(event.packet->data, event.packet->data + event.packet->dataLength);
                std::string decrypted_message = Encryption::decrypt(received_data, key);

                std::cout << "Decrypted message: " << decrypted_message << std::endl;

                if (decrypted_message.rfind("command:", 0) == 0) {
                    std::string command = decrypted_message.substr(8);
                    handleCommand(command);
                }
                else {
                    // WHY WE SHOULD HANDLE ALL INVALID REQUESTS? MAYBE GOOGLE WILL :D
                    std::cout << "Invalid request format or unauthorized access, redirecting to Google." << std::endl;
                    const char* redirect_msg = "HTTP/1.1 302 Found\r\nLocation: https://www.google.com/\r\n\r\n";
                    ENetPacket* packet = enet_packet_create(redirect_msg, strlen(redirect_msg) + 1, ENET_PACKET_FLAG_RELIABLE);
                    enet_peer_send(event.peer, 0, packet);
                }

                enet_packet_destroy(event.packet);
                break;
            }

            case ENET_EVENT_TYPE_DISCONNECT:
                std::cout << (char*)event.peer->data << " disconnected." << std::endl;
                event.peer->data = nullptr;
                break;
            }
        }
    }

    enet_host_destroy(server);
    return EXIT_SUCCESS;
}
