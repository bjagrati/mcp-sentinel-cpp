#include "rate_limiter.h"

RateLimiter::RateLimiter(int maxRequests, int windowSeconds)
    : maxRequests_(maxRequests), windowSeconds_(windowSeconds) {}

bool RateLimiter::allow(const std::string& userId, const std::string& toolName) {
    std::lock_guard<std::mutex> lock(mutex_);

    const auto now = std::chrono::steady_clock::now();
    const auto window = std::chrono::seconds(windowSeconds_);
    const std::string key = makeKey(userId, toolName);

    auto& timestamps = requests_[key];

    std::vector<std::chrono::steady_clock::time_point> recentRequests;

    for (const auto& timestamp : timestamps) {
        if (now - timestamp <= window) {
            recentRequests.push_back(timestamp);
        }
    }

    timestamps = std::move(recentRequests);

    if (static_cast<int>(timestamps.size()) >= maxRequests_) {
        return false;
    }

    timestamps.push_back(now);
    return true;
}

std::string RateLimiter::makeKey(const std::string& userId, const std::string& toolName) const {
    return userId + ":" + toolName;
}