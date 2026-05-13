#pragma once

#include <string>
#include <unordered_map>
#include <unordered_set>

class Redactor {
public:
    std::unordered_map<std::string, std::string> redact(
        const std::unordered_map<std::string, std::string>& arguments
    ) const;

private:
    std::unordered_set<std::string> sensitiveFields_ = {
        "ssn",
        "password",
        "creditCardNumber",
        "accessToken",
        "secret"
    };
};