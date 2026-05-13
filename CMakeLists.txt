cmake_minimum_required(VERSION 3.16)

project(mcp_sentinel_cpp VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

add_executable(mcp_sentinel
    src/main.cpp
    src/tool_registry.cpp
    src/policy_engine.cpp
    src/redactor.cpp
    src/rate_limiter.cpp
    src/audit_logger.cpp
    src/approval_manager.cpp
)