# MCP Sentinel C++

MCP Sentinel C++ is a high-performance C++ control-plane gateway for securing AI agent and MCP-style tool execution.

Modern AI agents can call external tools such as payment systems, databases, ticketing systems, deployment systems, and customer-management platforms. In enterprise environments, unrestricted tool execution is dangerous.

This project explores how a backend gateway can evaluate every AI tool call before execution using policy rules, role-based access control, rate limits, sensitive-data redaction, approval workflows, and audit logging.

## Phase 1: Core Logic

This first version is intentionally in-memory and does not use HTTP, databases, Redis, or external services yet.

It includes:

- Tool registry
- Policy engine
- Role-based access control
- Amount-based policy checks
- Sensitive-data redaction
- In-memory rate limiter
- Human approval workflow
- Audit logging to file

## Example Scenario

A support agent asks an AI agent to call:

```text
refund_payment

## Run with Docker

Build and run:

```bash
docker compose up --build

## Phase 4: AI Agent Prompt Simulator

MCP Sentinel now includes an AI-agent simulation layer.

Instead of calling the gateway directly with structured JSON, a user can send a natural language prompt:

```text
Refund customer cus_456 $5000 because they complained. Their SSN is 123-45-6789.