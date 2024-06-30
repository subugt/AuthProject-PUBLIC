#ifndef DDOS_PROTECTION_HPP
#define DDOS_PROTECTION_HPP

#include <enet/enet.h>
#include <unordered_map>
#include <chrono>
#include <mutex>
#include <shared_mutex>

class DDoSProtection {
public:
    DDoSProtection(int requestLimit, int timeWindow, int blockDuration, size_t maxPacketSize);

    bool isBlocked(enet_uint32 ip);
    void registerRequest(enet_uint32 ip);
    bool isPacketSizeValid(size_t packetSize);

private:
    struct RequestData {
        int requestCount;
        std::chrono::time_point<std::chrono::steady_clock> firstRequestTime;
    };

    int requestLimit;
    int timeWindow;
    int blockDuration;
    size_t maxPacketSize;

    std::unordered_map<enet_uint32, RequestData> requestMap;
    std::unordered_map<enet_uint32, std::chrono::time_point<std::chrono::steady_clock>> blockedIPs;
    std::shared_mutex mutex;

    void blockIP(enet_uint32 ip);
    void cleanUp();
};

#endif // DDOS_PROTECTION_HPP