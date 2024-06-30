#include "ddos_protection.hpp"
#include <iostream>

DDoSProtection::DDoSProtection(int requestLimit, int timeWindow, int blockDuration, size_t maxPacketSize)
    : requestLimit(requestLimit), timeWindow(timeWindow), blockDuration(blockDuration), maxPacketSize(maxPacketSize) {}

bool DDoSProtection::isBlocked(enet_uint32 ip) {
    std::shared_lock lock(mutex);
    cleanUp();

    auto it = blockedIPs.find(ip);
    if (it != blockedIPs.end()) {
        auto now = std::chrono::steady_clock::now();
        if (now < it->second) {
            return true;
        } else {
            blockedIPs.erase(it);
        }
    }
    return false;
}

void DDoSProtection::registerRequest(enet_uint32 ip) {
    std::unique_lock lock(mutex);
    cleanUp();

    auto now = std::chrono::steady_clock::now();
    auto& requestData = requestMap[ip];
    
    if (requestData.requestCount == 0 || now > requestData.firstRequestTime + std::chrono::seconds(timeWindow)) {
        requestData.requestCount = 1;
        requestData.firstRequestTime = now;
    } else {
        requestData.requestCount++;
        if (requestData.requestCount > requestLimit) {
            blockIP(ip);
        }
    }
}

bool DDoSProtection::isPacketSizeValid(size_t packetSize) {
    return packetSize <= maxPacketSize;
}

void DDoSProtection::blockIP(enet_uint32 ip) {
    blockedIPs[ip] = std::chrono::steady_clock::now() + std::chrono::seconds(blockDuration);
    requestMap.erase(ip);
}

void DDoSProtection::cleanUp() {
    auto now = std::chrono::steady_clock::now();
    for (auto it = requestMap.begin(); it != requestMap.end();) {
        if (now > it->second.firstRequestTime + std::chrono::seconds(timeWindow)) {
            it = requestMap.erase(it);
        } else {
            ++it;
        }
    }

    for (auto it = blockedIPs.begin(); it != blockedIPs.end();) {
        if (now > it->second) {
            it = blockedIPs.erase(it);
        } else {
            ++it;
        }
    }
}