# MCP Sentinel C++

A high-performance C++ gateway for securing AI agent and MCP-style tool execution using policy evaluation, RBAC, rate limiting, redaction, approval workflows, and audit logging.

---

## What This Project Does

Modern AI agents can call external tools such as payment systems, databases, ticketing systems, deployment systems, and customer-management platforms.

That creates a serious backend/security problem:

> Should an AI agent be allowed to execute this tool call?

MCP Sentinel acts as a security control plane between an AI agent and enterprise tools.

Before a tool call is allowed, the gateway evaluates:

- Does the tool exist?
- Is this user role allowed to call it?
- Does the request exceed a policy limit?
- Is the tool high-risk?
- Should a human approve it first?
- Are sensitive fields being exposed?
- Has the user exceeded rate limits?
- Should this decision be logged for auditability?

---

## Why This Is an AI Infrastructure Project

This is not an ML model training project.

The goal is to demonstrate the backend infrastructure needed to safely deploy AI agents in real enterprise environments.

AI agents are becoming capable of calling tools and taking actions. For example:

```text
Refund this customer.
Create a support ticket.
Query customer data.
Trigger a deployment rollback.
Delete a user account.
Send an email.