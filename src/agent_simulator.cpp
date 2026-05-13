#include "agent_simulator.h"

#include <algorithm>
#include <cctype>
#include <regex>

std::optional<AgentPlan> AgentSimulator::createPlan(
    const std::string& userId,
    const std::string& role,
    const std::string& prompt
) const {
    std::string lowerPrompt = toLower(prompt);

    if (lowerPrompt.find("refund") != std::string::npos) {
        return parseRefundPrompt(userId, role, prompt);
    }

    return std::nullopt;
}

std::optional<AgentPlan> AgentSimulator::parseRefundPrompt(
    const std::string& userId,
    const std::string& role,
    const std::string& prompt
) const {
    std::string customerId = extractCustomerId(prompt);
    std::string amount = extractAmount(prompt);
    std::string ssn = extractSsn(prompt);

    if (customerId.empty() || amount.empty()) {
        return std::nullopt;
    }

    ToolCallRequest request;
    request.userId = userId;
    request.role = role;
    request.toolName = "refund_payment";
    request.arguments["customerId"] = customerId;
    request.arguments["amount"] = amount;
    request.arguments["reason"] = "Parsed from natural language refund prompt";

    if (!ssn.empty()) {
        request.arguments["ssn"] = ssn;
    }

    AgentPlan plan;
    plan.intent = "refund_customer";
    plan.toolCallRequest = request;

    return plan;
}

std::string AgentSimulator::extractCustomerId(const std::string& prompt) const {
    std::regex pattern(R"((cus_[A-Za-z0-9]+))");
    std::smatch match;

    if (std::regex_search(prompt, match, pattern)) {
        return match[1].str();
    }

    return "";
}

std::string AgentSimulator::extractAmount(const std::string& prompt) const {
    std::regex dollarPattern(R"(\$([0-9]+))");
    std::smatch match;

    if (std::regex_search(prompt, match, dollarPattern)) {
        return match[1].str();
    }

    std::regex amountPattern(R"(amount\s+([0-9]+))");

    if (std::regex_search(prompt, match, amountPattern)) {
        return match[1].str();
    }

    return "";
}

std::string AgentSimulator::extractSsn(const std::string& prompt) const {
    std::regex pattern(R"((\d{3}-\d{2}-\d{4}))");
    std::smatch match;

    if (std::regex_search(prompt, match, pattern)) {
        return match[1].str();
    }

    return "";
}

std::string AgentSimulator::toLower(std::string value) const {
    std::transform(
        value.begin(),
        value.end(),
        value.begin(),
        [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        }
    );

    return value;
}