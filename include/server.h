#pragma once

#include "tool_registry.h"
#include "policy_engine.h"
#include "redactor.h"
#include "rate_limiter.h"
#include "audit_logger.h"
#include "approval_manager.h"

class Server {
public:
    Server(
        ToolRegistry& toolRegistry,
        PolicyEngine& policyEngine,
        Redactor& redactor,
        RateLimiter& rateLimiter,
        AuditLogger& auditLogger,
        ApprovalManager& approvalManager
    );

    void start(int port);

private:
    ToolRegistry& toolRegistry_;
    PolicyEngine& policyEngine_;
    Redactor& redactor_;
    RateLimiter& rateLimiter_;
    AuditLogger& auditLogger_;
    ApprovalManager& approvalManager_;
};