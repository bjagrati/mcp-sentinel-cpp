#pragma once

#include "models.h"
#include <vector>

class PolicyEngine {
public:
    void addPolicy(const Policy& policy);
    PolicyDecision evaluate(const ToolCallRequest& request, const Tool& tool) const;

private:
    std::vector<Policy> policies_;

    long long getAmountFromRequest(const ToolCallRequest& request) const;
};