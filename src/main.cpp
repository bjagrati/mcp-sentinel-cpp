#include "models.h"
#include "tool_registry.h"
#include "policy_engine.h"
#include "redactor.h"
#include "rate_limiter.h"
#include "audit_logger.h"
#include "approval_manager.h"
#include "server.h"

#include <iostream>

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

    std::cout << "Seeded default tool: refund_payment" << std::endl;
    std::cout << "Seeded default policy: support_agent can refund up to 2500" << std::endl;

    Server server(
        toolRegistry,
        policyEngine,
        redactor,
        rateLimiter,
        auditLogger,
        approvalManager
    );

    server.start(8080);

    return 0;
}