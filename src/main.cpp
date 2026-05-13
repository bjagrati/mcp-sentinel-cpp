#include "models.h"
#include "tool_registry.h"
#include "policy_engine.h"
#include "redactor.h"
#include "rate_limiter.h"
#include "audit_logger.h"
#include "approval_manager.h"

#include <iostream>

std::string policyEffectToString(PolicyEffect effect) {
    switch (effect) {
        case PolicyEffect::ALLOW:
            return "ALLOW";
        case PolicyEffect::DENY:
            return "DENY";
        case PolicyEffect::REQUIRES_APPROVAL:
            return "REQUIRES_APPROVAL";
        default:
            return "UNKNOWN";
    }
}

void printArguments(const std::unordered_map<std::string, std::string>& arguments) {
    for (const auto& pair : arguments) {
        std::cout << "  " << pair.first << ": " << pair.second << std::endl;
    }
}

int main() {
    ToolRegistry toolRegistry;
    PolicyEngine policyEngine;
    Redactor redactor;
    RateLimiter rateLimiter(3, 60);
    AuditLogger auditLogger("audit.log");
    ApprovalManager approvalManager;

    Tool refundTool;
    refundTool.name = "refund_payment";
    refundTool.description = "Refund a customer payment";
    refundTool.riskLevel = RiskLevel::HIGH;
    refundTool.requiresApproval = true;

    toolRegistry.registerTool(refundTool);

    Policy supportPolicy;
    supportPolicy.toolName = "refund_payment";
    supportPolicy.role = "support_agent";
    supportPolicy.effect = PolicyEffect::ALLOW;
    supportPolicy.maxAmount = 2500;

    policyEngine.addPolicy(supportPolicy);

    ToolCallRequest request;
    request.userId = "user_123";
    request.role = "support_agent";
    request.toolName = "refund_payment";
    request.arguments = {
        {"customerId", "cus_456"},
        {"amount", "5000"},
        {"reason", "Customer complaint"},
        {"ssn", "123-45-6789"}
    };

    std::cout << "Evaluating AI tool call..." << std::endl;
    std::cout << "Tool: " << request.toolName << std::endl;
    std::cout << "User: " << request.userId << std::endl;
    std::cout << "Role: " << request.role << std::endl;

    auto tool = toolRegistry.getTool(request.toolName);

    if (!tool.has_value()) {
        std::cout << "Decision: DENY" << std::endl;
        std::cout << "Reason: Tool not found" << std::endl;
        return 0;
    }

    bool rateLimitAllowed = rateLimiter.allow(request.userId, request.toolName);

    if (!rateLimitAllowed) {
        std::cout << "Decision: DENY" << std::endl;
        std::cout << "Reason: Rate limit exceeded" << std::endl;

        auditLogger.log({
            request.userId,
            request.role,
            request.toolName,
            "DENY",
            "Rate limit exceeded"
        });

        return 0;
    }

    auto redactedArguments = redactor.redact(request.arguments);

    PolicyDecision decision = policyEngine.evaluate(request, tool.value());

    std::string approvalId;

    if (decision.effect == PolicyEffect::REQUIRES_APPROVAL) {
        approvalId = approvalManager.createApproval(request);
    }

    auditLogger.log({
        request.userId,
        request.role,
        request.toolName,
        policyEffectToString(decision.effect),
        decision.reason
    });

    std::cout << std::endl;
    std::cout << "Decision: " << policyEffectToString(decision.effect) << std::endl;
    std::cout << "Reason: " << decision.reason << std::endl;

    if (!approvalId.empty()) {
        std::cout << "Approval ID: " << approvalId << std::endl;
    }

    std::cout << std::endl;
    std::cout << "Redacted Arguments:" << std::endl;
    printArguments(redactedArguments);

    if (!approvalId.empty()) {
        std::cout << std::endl;
        std::cout << "Simulating manager approval..." << std::endl;

        bool approved = approvalManager.approve(
            approvalId,
            "manager_123",
            "Approved after reviewing customer account"
        );

        if (approved) {
            std::cout << "Approval Status: APPROVED" << std::endl;
        } else {
            std::cout << "Approval Status: FAILED" << std::endl;
        }
    }

    std::cout << std::endl;
    std::cout << "Audit log written to audit.log" << std::endl;

    return 0;
}