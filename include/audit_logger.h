#pragma once

#include "models.h"

#include <string>
#include <mutex>

class AuditLogger {
public:
    explicit AuditLogger(const std::string& filePath);

    void log(const AuditEvent& event);

    std::string getFilePath() const;

private:
    std::string filePath_;
    mutable std::mutex mutex_;
};