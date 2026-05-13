#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <chrono>
#include <mutex>

class RateLimiter {
public:
    RateLimiter(int maxRequests, int windowSeconds);

    bool allow(const std::string& userId, const std::string& toolName);

private:
    int maxRequests_;
    int windowSeconds_;

    std::mutex mutex_;
    std::unordered_map<std::string, std::vector<std::chrono::steady_clock::time_point>> requests_;

    std::string makeKey(const std::string& userId, const std::string& toolName) const;
};