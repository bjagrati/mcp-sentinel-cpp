#pragma once

#include <string>
#include <unordered_map>
#include <optional>

enum class RiskLevel {
    LOW,
    MEDIUM,
    HIGH
};

enum class PolicyEffect {
    ALLOW,
    DENY,
    REQUIRES_APPROVAL
};

enum class ToolCallStatus {
    PENDING,
    APPROVED,
    REJECTED,
    EXECUTED,
    DENIED
};

struct Tool {
    std::string name;
    std::string description;
    RiskLevel riskLevel;
    bool requiresApproval;
};

struct Policy {
    std::string toolName;
    std::string role;
    PolicyEffect effect;
    std::optional<long long> maxAmount;
};

struct ToolCallRequest {
    std::string userId;
    std::string role;
    std::string toolName;
    std::unordered_map<std::string, std::string> arguments;
};

struct PolicyDecision {
    PolicyEffect effect;
    std::string reason;
};

struct Approval {
    std::string approvalId;
    ToolCallRequest request;
    ToolCallStatus status;
    std::string reviewedBy;
    std::string comment;
};

struct AuditEvent {
    std::string userId;
    std::string role;
    std::string toolName;
    std::string decision;
    std::string reason;
};