#include "audit_logger.h"
#include <fstream>
#include <chrono>
#include <ctime>

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

    out << "{"
        << "\"timestamp\":\"" << std::ctime(&currentTime) << "\","
        << "\"userId\":\"" << event.userId << "\","
        << "\"role\":\"" << event.role << "\","
        << "\"toolName\":\"" << event.toolName << "\","
        << "\"decision\":\"" << event.decision << "\","
        << "\"reason\":\"" << event.reason << "\""
        << "}"
        << std::endl;
}