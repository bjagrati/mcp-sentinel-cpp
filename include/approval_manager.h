#pragma once

#include "models.h"
#include <unordered_map>
#include <string>
#include <optional>
#include <mutex>

class ApprovalManager {
public:
    std::string createApproval(const ToolCallRequest& request);

    bool approve(
        const std::string& approvalId,
        const std::string& approvedBy,
        const std::string& comment
    );

    bool reject(
        const std::string& approvalId,
        const std::string& rejectedBy,
        const std::string& comment
    );

    std::optional<Approval> getApproval(const std::string& approvalId) const;

private:
    mutable std::mutex mutex_;
    int nextApprovalNumber_ = 1;
    std::unordered_map<std::string, Approval> approvals_;

    std::string generateApprovalId();
};