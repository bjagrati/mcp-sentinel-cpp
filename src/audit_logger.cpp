#include "audit_logger.h"

#include <fstream>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

AuditLogger::AuditLogger(const std::string& filePath)
    : filePath_(filePath) {}

void AuditLogger::log(const AuditEvent& event) {
    std::lock_guard<std::mutex> lock(mutex_);

    std::ofstream out(filePath_, std::ios::app);

    if (!out.is_open()) {
        return;
    }

    auto now = std::chrono::system_clock::now();
    std::time_t currentTime = std::chrono::system_clock::to_time_t(now);

    std::tm timeInfo{};

#if defined(_WIN32)
    gmtime_s(&timeInfo, &currentTime);
#else
    gmtime_r(&currentTime, &timeInfo);
#endif

    std::ostringstream timestamp;
    timestamp << std::put_time(&timeInfo, "%Y-%m-%dT%H:%M:%SZ");

    out << "{"
        << "\"timestamp\":\"" << timestamp.str() << "\","
        << "\"userId\":\"" << event.userId << "\","
        << "\"role\":\"" << event.role << "\","
        << "\"toolName\":\"" << event.toolName << "\","
        << "\"decision\":\"" << event.decision << "\","
        << "\"reason\":\"" << event.reason << "\""
        << "}"
        << std::endl;
}

std::string AuditLogger::getFilePath() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return filePath_;
}