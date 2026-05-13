#include "config.h"

#include <cstdlib>
#include <string>

Config ConfigLoader::load() {
    Config config;

    config.port = getEnvIntOrDefault("MCP_SENTINEL_PORT", 8080);
    config.auditLogPath = getEnvOrDefault("MCP_SENTINEL_AUDIT_LOG_PATH", "audit.log");

    return config;
}

std::string ConfigLoader::getEnvOrDefault(
    const std::string& key,
    const std::string& defaultValue
) {
    const char* value = std::getenv(key.c_str());

    if (value == nullptr || std::string(value).empty()) {
        return defaultValue;
    }

    return std::string(value);
}

int ConfigLoader::getEnvIntOrDefault(
    const std::string& key,
    int defaultValue
) {
    const char* value = std::getenv(key.c_str());

    if (value == nullptr || std::string(value).empty()) {
        return defaultValue;
    }

    try {
        return std::stoi(value);
    } catch (...) {
        return defaultValue;
    }
}