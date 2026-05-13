#pragma once

#include "models.h"

#include <optional>
#include <string>

struct AgentPlan {
    std::string intent;
    ToolCallRequest toolCallRequest;
};

class AgentSimulator {
public:
    std::optional<AgentPlan> createPlan(
        const std::string& userId,
        const std::string& role,
        const std::string& prompt
    ) const;

private:
    std::optional<AgentPlan> parseRefundPrompt(
        const std::string& userId,
        const std::string& role,
        const std::string& prompt
    ) const;

    std::string extractCustomerId(const std::string& prompt) const;
    std::string extractAmount(const std::string& prompt) const;
    std::string extractSsn(const std::string& prompt) const;

    std::string toLower(std::string value) const;
};