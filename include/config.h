#pragma once

#include <string>

struct Config {
    int port;
    std::string auditLogPath;
};

class ConfigLoader {
public:
    static Config load();

private:
    static std::string getEnvOrDefault(
        const std::string& key,
        const std::string& defaultValue
    );

    static int getEnvIntOrDefault(
        const std::string& key,
        int defaultValue
    );
};