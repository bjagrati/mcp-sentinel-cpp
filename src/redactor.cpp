#include "redactor.h"

std::unordered_map<std::string, std::string> Redactor::redact(
    const std::unordered_map<std::string, std::string>& arguments
) const {
    std::unordered_map<std::string, std::string> redacted = arguments;

    for (auto& pair : redacted) {
        if (sensitiveFields_.find(pair.first) != sensitiveFields_.end()) {
            pair.second = "***REDACTED***";
        }
    }

    return redacted;
}