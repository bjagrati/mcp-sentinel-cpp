#include "policy_engine.h"
#include <cstdlib>

void PolicyEngine::addPolicy(const Policy& policy) {
    policies_.push_back(policy);
}

PolicyDecision PolicyEngine::evaluate(const ToolCallRequest& request, const Tool& tool) const {
    for (const auto& policy : policies_) {
        if (policy.toolName != request.toolName || policy.role != request.role) {
            continue;
        }

        if (policy.effect == PolicyEffect::DENY) {
            return {
                PolicyEffect::DENY,
                "Policy explicitly denies this role from using the tool"
            };
        }

        if (policy.maxAmount.has_value()) {
            long long amount = getAmountFromRequest(request);

            if (amount > policy.maxAmount.value()) {
                return {
                    PolicyEffect::REQUIRES_APPROVAL,
                    "Amount exceeds role policy limit"
                };
            }
        }

        if (policy.effect == PolicyEffect::REQUIRES_APPROVAL) {
            return {
                PolicyEffect::REQUIRES_APPROVAL,
                "Policy requires human approval"
            };
        }

        if (policy.effect == PolicyEffect::ALLOW) {
            if (tool.requiresApproval || tool.riskLevel == RiskLevel::HIGH) {
                return {
                    PolicyEffect::REQUIRES_APPROVAL,
                    "High-risk tool requires approval"
                };
            }

            return {
                PolicyEffect::ALLOW,
                "Policy allows this tool call"
            };
        }
    }

    return {
        PolicyEffect::DENY,
        "No matching policy found"
    };
}

long long PolicyEngine::getAmountFromRequest(const ToolCallRequest& request) const {
    auto it = request.arguments.find("amount");

    if (it == request.arguments.end()) {
        return 0;
    }

    try {
        return std::stoll(it->second);
    } catch (...) {
        return 0;
    }
}