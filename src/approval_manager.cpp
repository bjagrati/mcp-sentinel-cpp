#include "approval_manager.h"

std::string ApprovalManager::createApproval(const ToolCallRequest& request) {
    std::lock_guard<std::mutex> lock(mutex_);

    std::string approvalId = generateApprovalId();

    Approval approval;
    approval.approvalId = approvalId;
    approval.request = request;
    approval.status = ToolCallStatus::PENDING;

    approvals_[approvalId] = approval;

    return approvalId;
}

bool ApprovalManager::approve(
    const std::string& approvalId,
    const std::string& approvedBy,
    const std::string& comment
) {
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = approvals_.find(approvalId);

    if (it == approvals_.end()) {
        return false;
    }

    if (it->second.status != ToolCallStatus::PENDING) {
        return false;
    }

    it->second.status = ToolCallStatus::APPROVED;
    it->second.reviewedBy = approvedBy;
    it->second.comment = comment;

    return true;
}

bool ApprovalManager::reject(
    const std::string& approvalId,
    const std::string& rejectedBy,
    const std::string& comment
) {
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = approvals_.find(approvalId);

    if (it == approvals_.end()) {
        return false;
    }

    if (it->second.status != ToolCallStatus::PENDING) {
        return false;
    }

    it->second.status = ToolCallStatus::REJECTED;
    it->second.reviewedBy = rejectedBy;
    it->second.comment = comment;

    return true;
}

std::optional<Approval> ApprovalManager::getApproval(const std::string& approvalId) const {
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = approvals_.find(approvalId);

    if (it == approvals_.end()) {
        return std::nullopt;
    }

    return it->second;
}

std::string ApprovalManager::generateApprovalId() {
    return "approval_" + std::to_string(nextApprovalNumber_++);
}