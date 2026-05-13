#include "server.h"

#include <httplib.h>
#include <nlohmann/json.hpp>

#include <fstream>
#include <iostream>
#include <sstream>
#include <unordered_map>

using json = nlohmann::json;

namespace {

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

std::string statusToString(ToolCallStatus status) {
    switch (status) {
        case ToolCallStatus::PENDING:
            return "PENDING";
        case ToolCallStatus::APPROVED:
            return "APPROVED";
        case ToolCallStatus::REJECTED:
            return "REJECTED";
        case ToolCallStatus::EXECUTED:
            return "EXECUTED";
        case ToolCallStatus::DENIED:
            return "DENIED";
        default:
            return "UNKNOWN";
    }
}

RiskLevel parseRiskLevel(const std::string& value) {
    if (value == "LOW") {
        return RiskLevel::LOW;
    }

    if (value == "MEDIUM") {
        return RiskLevel::MEDIUM;
    }

    return RiskLevel::HIGH;
}

PolicyEffect parsePolicyEffect(const std::string& value) {
    if (value == "ALLOW") {
        return PolicyEffect::ALLOW;
    }

    if (value == "REQUIRES_APPROVAL") {
        return PolicyEffect::REQUIRES_APPROVAL;
    }

    return PolicyEffect::DENY;
}

std::unordered_map<std::string, std::string> parseArguments(const json& argsJson) {
    std::unordered_map<std::string, std::string> arguments;

    if (!argsJson.is_object()) {
        return arguments;
    }

    for (auto it = argsJson.begin(); it != argsJson.end(); ++it) {
        if (it.value().is_string()) {
            arguments[it.key()] = it.value().get<std::string>();
        } else if (it.value().is_number_integer()) {
            arguments[it.key()] = std::to_string(it.value().get<long long>());
        } else if (it.value().is_number_float()) {
            arguments[it.key()] = std::to_string(it.value().get<double>());
        } else if (it.value().is_boolean()) {
            arguments[it.key()] = it.value().get<bool>() ? "true" : "false";
        } else {
            arguments[it.key()] = it.value().dump();
        }
    }

    return arguments;
}

json mapToJson(const std::unordered_map<std::string, std::string>& values) {
    json result = json::object();

    for (const auto& pair : values) {
        result[pair.first] = pair.second;
    }

    return result;
}

void sendJson(httplib::Response& res, int status, const json& body) {
    res.status = status;
    res.set_content(body.dump(4), "application/json");
}

} // namespace

Server::Server(
    ToolRegistry& toolRegistry,
    PolicyEngine& policyEngine,
    Redactor& redactor,
    RateLimiter& rateLimiter,
    AuditLogger& auditLogger,
    ApprovalManager& approvalManager
)
    : toolRegistry_(toolRegistry),
      policyEngine_(policyEngine),
      redactor_(redactor),
      rateLimiter_(rateLimiter),
      auditLogger_(auditLogger),
      approvalManager_(approvalManager) {}

void Server::start(int port) {
    httplib::Server server;

    server.Get("/health", [](const httplib::Request&, httplib::Response& res) {
        sendJson(res, 200, {
            {"status", "ok"},
            {"service", "mcp-sentinel-cpp"}
        });
    });

    server.Post("/tools", [this](const httplib::Request& req, httplib::Response& res) {
        try {
            json body = json::parse(req.body);

            Tool tool;
            tool.name = body.value("name", "");
            tool.description = body.value("description", "");
            tool.riskLevel = parseRiskLevel(body.value("riskLevel", "HIGH"));
            tool.requiresApproval = body.value("requiresApproval", false);

            if (tool.name.empty()) {
                sendJson(res, 400, {
                    {"error", "tool name is required"}
                });
                return;
            }

            bool created = toolRegistry_.registerTool(tool);

            sendJson(res, 201, {
                {"created", created},
                {"toolName", tool.name}
            });
        } catch (const std::exception& ex) {
            sendJson(res, 400, {
                {"error", ex.what()}
            });
        }
    });

    server.Post("/policies", [this](const httplib::Request& req, httplib::Response& res) {
        try {
            json body = json::parse(req.body);

            Policy policy;
            policy.toolName = body.value("toolName", "");
            policy.role = body.value("role", "");
            policy.effect = parsePolicyEffect(body.value("effect", "DENY"));

            if (body.contains("maxAmount") && !body["maxAmount"].is_null()) {
                policy.maxAmount = body["maxAmount"].get<long long>();
            }

            if (policy.toolName.empty() || policy.role.empty()) {
                sendJson(res, 400, {
                    {"error", "toolName and role are required"}
                });
                return;
            }

            if (!toolRegistry_.exists(policy.toolName)) {
                sendJson(res, 404, {
                    {"error", "tool does not exist"}
                });
                return;
            }

            policyEngine_.addPolicy(policy);

            sendJson(res, 201, {
                {"created", true},
                {"toolName", policy.toolName},
                {"role", policy.role}
            });
        } catch (const std::exception& ex) {
            sendJson(res, 400, {
                {"error", ex.what()}
            });
        }
    });

    server.Post("/tool-calls/evaluate", [this](const httplib::Request& req, httplib::Response& res) {
        try {
            json body = json::parse(req.body);

            ToolCallRequest request;
            request.userId = body.value("userId", "");
            request.role = body.value("role", "");
            request.toolName = body.value("toolName", "");
            request.arguments = parseArguments(body.value("arguments", json::object()));

            if (request.userId.empty() || request.role.empty() || request.toolName.empty()) {
                sendJson(res, 400, {
                    {"error", "userId, role, and toolName are required"}
                });
                return;
            }

            auto tool = toolRegistry_.getTool(request.toolName);

            if (!tool.has_value()) {
                auditLogger_.log({
                    request.userId,
                    request.role,
                    request.toolName,
                    "DENY",
                    "Tool not found"
                });

                sendJson(res, 404, {
                    {"decision", "DENY"},
                    {"reason", "Tool not found"}
                });
                return;
            }

            bool allowedByRateLimit = rateLimiter_.allow(request.userId, request.toolName);

            if (!allowedByRateLimit) {
                auditLogger_.log({
                    request.userId,
                    request.role,
                    request.toolName,
                    "DENY",
                    "Rate limit exceeded"
                });

                sendJson(res, 429, {
                    {"decision", "DENY"},
                    {"reason", "Rate limit exceeded"}
                });
                return;
            }

            auto redactedArguments = redactor_.redact(request.arguments);

            PolicyDecision decision = policyEngine_.evaluate(request, tool.value());

            std::string approvalId;

            if (decision.effect == PolicyEffect::REQUIRES_APPROVAL) {
                approvalId = approvalManager_.createApproval(request);
            }

            auditLogger_.log({
                request.userId,
                request.role,
                request.toolName,
                policyEffectToString(decision.effect),
                decision.reason
            });

            json responseBody = {
                {"decision", policyEffectToString(decision.effect)},
                {"reason", decision.reason},
                {"redactedArguments", mapToJson(redactedArguments)}
            };

            if (!approvalId.empty()) {
                responseBody["approvalId"] = approvalId;
            }

            sendJson(res, 200, responseBody);
        } catch (const std::exception& ex) {
            sendJson(res, 400, {
                {"error", ex.what()}
            });
        }
    });

    server.Post(R"(/approvals/([^/]+)/approve)", [this](const httplib::Request& req, httplib::Response& res) {
        try {
            std::string approvalId = req.matches[1];

            json body = json::parse(req.body);

            std::string approvedBy = body.value("approvedBy", "");
            std::string comment = body.value("comment", "");

            if (approvedBy.empty()) {
                sendJson(res, 400, {
                    {"error", "approvedBy is required"}
                });
                return;
            }

            bool approved = approvalManager_.approve(approvalId, approvedBy, comment);

            if (!approved) {
                sendJson(res, 404, {
                    {"error", "approval not found or not pending"}
                });
                return;
            }

            sendJson(res, 200, {
                {"approvalId", approvalId},
                {"status", "APPROVED"}
            });
        } catch (const std::exception& ex) {
            sendJson(res, 400, {
                {"error", ex.what()}
            });
        }
    });

    server.Post(R"(/approvals/([^/]+)/reject)", [this](const httplib::Request& req, httplib::Response& res) {
        try {
            std::string approvalId = req.matches[1];

            json body = json::parse(req.body);

            std::string rejectedBy = body.value("rejectedBy", "");
            std::string comment = body.value("comment", "");

            if (rejectedBy.empty()) {
                sendJson(res, 400, {
                    {"error", "rejectedBy is required"}
                });
                return;
            }

            bool rejected = approvalManager_.reject(approvalId, rejectedBy, comment);

            if (!rejected) {
                sendJson(res, 404, {
                    {"error", "approval not found or not pending"}
                });
                return;
            }

            sendJson(res, 200, {
                {"approvalId", approvalId},
                {"status", "REJECTED"}
            });
        } catch (const std::exception& ex) {
            sendJson(res, 400, {
                {"error", ex.what()}
            });
        }
    });

    server.Get(R"(/approvals/([^/]+))", [this](const httplib::Request& req, httplib::Response& res) {
        std::string approvalId = req.matches[1];

        auto approval = approvalManager_.getApproval(approvalId);

        if (!approval.has_value()) {
            sendJson(res, 404, {
                {"error", "approval not found"}
            });
            return;
        }

        sendJson(res, 200, {
            {"approvalId", approval->approvalId},
            {"status", statusToString(approval->status)},
            {"reviewedBy", approval->reviewedBy},
            {"comment", approval->comment},
            {"toolName", approval->request.toolName},
            {"userId", approval->request.userId},
            {"role", approval->request.role}
        });
    });

        server.Get("/audit-logs", [this](const httplib::Request&, httplib::Response& res) {
        std::ifstream in(auditLogger_.getFilePath());

        if (!in.is_open()) {
            sendJson(res, 200, {
                {"logs", json::array()}
            });
            return;
        }

        json logs = json::array();
        std::string line;

        while (std::getline(in, line)) {
            if (!line.empty()) {
                try {
                    logs.push_back(json::parse(line));
                } catch (...) {
                    logs.push_back(line);
                }
            }
        }

        sendJson(res, 200, {
            {"logs", logs}
        });
    });

    std::cout << "MCP Sentinel HTTP server running on port " << port << std::endl;
    std::cout << "Health check: http://localhost:" << port << "/health" << std::endl;

    server.listen("0.0.0.0", port);
}